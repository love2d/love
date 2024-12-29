/**
 * Copyright (c) 2006-2024 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Buffer.h"
#include "Graphics.h"
#include "common/memory.h"

namespace love
{
namespace graphics
{

love::Type Buffer::type("GraphicsBuffer", &Object::type);

int Buffer::bufferCount = 0;
int64 Buffer::totalGraphicsMemory = 0;

Buffer::Buffer(Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &bufferformat, size_t size, size_t arraylength)
	: arrayLength(0)
	, arrayStride(0)
	, size(size)
	, usageFlags(settings.usageFlags)
	, dataUsage(settings.dataUsage)
	, debugName(settings.debugName)
	, mapped(false)
	, mappedType(MAP_WRITE_INVALIDATE)
	, immutable(false)
{
	if (size == 0 && arraylength == 0)
		throw love::Exception("Size or array length must be specified.");

	if (bufferformat.size() == 0)
		throw love::Exception("Data format must contain values.");

	const auto &caps = gfx->getCapabilities();

	bool indexbuffer = usageFlags & BUFFERUSAGEFLAG_INDEX;
	bool vertexbuffer = usageFlags & BUFFERUSAGEFLAG_VERTEX;
	bool texelbuffer = usageFlags & BUFFERUSAGEFLAG_TEXEL;
	bool storagebuffer = usageFlags & BUFFERUSAGEFLAG_SHADER_STORAGE;
	bool indirectbuffer = usageFlags & BUFFERUSAGEFLAG_INDIRECT_ARGUMENTS;

	if (texelbuffer && !caps.features[Graphics::FEATURE_TEXEL_BUFFER])
		throw love::Exception("Texel buffers are not supported on this system.");

	if (storagebuffer && !caps.features[Graphics::FEATURE_GLSL4])
		throw love::Exception("Shader Storage buffers are not supported on this system (GLSL 4 support is necessary.)");

	if (storagebuffer && dataUsage == BUFFERDATAUSAGE_STREAM)
		throw love::Exception("Buffers created with 'stream' data usage cannot be used as a shader storage buffer.");

	if (indirectbuffer && !caps.features[Graphics::FEATURE_INDIRECT_DRAW])
		throw love::Exception("Indirect argument buffers are not supported on this system.");

	if (dataUsage == BUFFERDATAUSAGE_READBACK && (indexbuffer || vertexbuffer || texelbuffer || storagebuffer || indirectbuffer))
		throw love::Exception("Buffers created with 'readback' data usage cannot be index, vertex, texel, shaderstorage, or indirectarguments buffer types.");

	size_t offset = 0;
	size_t stride = 0;
	size_t structurealignment = 1;

	for (const DataDeclaration &decl : bufferformat)
	{
		DataMember member(decl);

		DataFormat format = member.decl.format;
		const DataFormatInfo &info = member.info;

		if (indexbuffer)
		{
			if (format != DATAFORMAT_UINT16 && format != DATAFORMAT_UINT32)
				throw love::Exception("Index buffers only support uint16 and uint32 data types.");

			if (bufferformat.size() > 1)
				throw love::Exception("Index buffers only support a single value per element.");

			if (decl.arrayLength > 0)
				throw love::Exception("Arrays are not supported in index buffers.");
		}

		if (vertexbuffer)
		{
			if (decl.arrayLength > 0)
				throw love::Exception("Arrays are not supported in vertex buffers.");

			if (info.isMatrix)
				throw love::Exception("Matrix types are not supported in vertex buffers.");

			if (info.baseType == DATA_BASETYPE_BOOL)
				throw love::Exception("Bool types are not supported in vertex buffers.");

			if (decl.bindingLocation < 0 || decl.bindingLocation >= (int) VertexAttributes::MAX)
			{
				if (decl.bindingLocation == -1 && !decl.name.empty())
					legacyVertexBindings = true;
				else
					throw love::Exception("Vertex buffer attributes must have a valid binding location value within [0, %d).", VertexAttributes::MAX);
			}
		}

		if (texelbuffer)
		{
			if (format != bufferformat[0].format)
				throw love::Exception("All values in a texel buffer must have the same format.");

			if (decl.arrayLength > 0)
				throw love::Exception("Arrays are not supported in texel buffers.");

			if (info.isMatrix)
				throw love::Exception("Matrix types are not supported in texel buffers.");

			if (info.baseType == DATA_BASETYPE_BOOL)
				throw love::Exception("Bool types are not supported in texel buffers.");

			if (info.components == 3)
				throw love::Exception("3-component formats are not supported in texel buffers.");

			if (info.baseType == DATA_BASETYPE_SNORM)
				throw love::Exception("Signed normalized formats are not supported in texel buffers.");
		}

		size_t memberoffset = offset;
		size_t membersize = member.info.size;

		// Storage buffers are always treated as being an array of a structure.
		// The structure's contents are the buffer format declaration.
		if (storagebuffer)
		{
			// TODO: We can support these.
			if (decl.arrayLength > 0)
				throw love::Exception("Arrays are not currently supported in shader storage buffers.");

			if (info.baseType == DATA_BASETYPE_BOOL)
				throw love::Exception("Bool types are not supported in shader storage buffers.");

			if (info.baseType == DATA_BASETYPE_UNORM || info.baseType == DATA_BASETYPE_SNORM)
				throw love::Exception("Normalized formats are not supported in shader storage buffers.");

			size_t alignment = 1;

			// GLSL's std430 packing rules. We also assume all matrices are
			// column-major.
			// https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf

			// "If the member is a column-major matrix with C columns and R rows,
			// the matrix is stored identically to an array of C column vectors
			// with R components each".
			// "If the member is a three-component vector with components
			// consuming N basic machine units, the base alignment is 4N."
			int c = info.isMatrix ? info.matrixRows : info.components;
			alignment = c == 3 ? 4 * info.componentSize : c * info.componentSize;

			// std430 will effectively turn a floatmat3x3 into a floatmat4x3
			// because of its vec3 padding rules. For now we'd rather not
			// support those formats at all, because it's not easy for users to
			// deal with.
			if (alignment != c * info.componentSize && (decl.arrayLength > 0 || info.isMatrix))
			{
				const char *fstr = "unknown";
				getConstant(decl.format, fstr);
				throw love::Exception("Data format %s%s is not currently supported in shader storage buffers.", fstr, decl.arrayLength > 0 ? " array" : "");
			}

			// "If the member is a structure, the base alignment of the structure
			// is N, where N is the largest base alignment value of any of its
			// members"
			structurealignment = std::max(structurealignment, alignment);

			memberoffset = alignUp(memberoffset, alignment);

			if (memberoffset != offset && (indexbuffer || vertexbuffer || texelbuffer))
				throw love::Exception("Cannot create Buffer:\nInternal alignment of member '%s' is preventing Buffer from being created as both a shader storage buffer and other buffer types\nMember byte offset needed for shader storage buffer: %d\nMember byte offset needed for other buffer types: %d",
					member.decl.name.c_str(), memberoffset, offset);
		}

		if (indirectbuffer)
		{
			if (info.isMatrix || info.components != 1
				|| (info.baseType != DATA_BASETYPE_UINT && info.baseType != DATA_BASETYPE_INT))
			{
				throw love::Exception("Indirect argument buffers must use single-component int or uint types.");
			}

			if (bufferformat.size() > 5)
				throw love::Exception("Indirect argument buffers only support up to 5 values per array element.");
		}

		member.offset = memberoffset;
		member.size = membersize;

		offset = member.offset + member.size;

		dataMembers.push_back(member);
	}

	stride = alignUp(offset, structurealignment);

	if (storagebuffer && (indexbuffer || vertexbuffer || texelbuffer))
	{
		if (stride != offset)
			throw love::Exception("Cannot create Buffer:\nBuffer used as a shader storage buffer would have a different number of bytes per array element (%d) than when used as other buffer types (%d)",
				stride, offset);
	}

	if (storagebuffer && stride > SHADER_STORAGE_BUFFER_MAX_STRIDE)
		throw love::Exception("Shader storage buffers cannot have more than %d bytes within each array element.", SHADER_STORAGE_BUFFER_MAX_STRIDE);

	if (size != 0)
	{
		size_t remainder = size % stride;
		if (remainder > 0)
			size += stride - remainder;
		arraylength = size / stride;
	}
	else
	{
		size = arraylength * stride;
	}

	this->arrayStride = stride;
	this->arrayLength = arraylength;
	this->size = size;

	if (texelbuffer && arraylength * dataMembers.size() > caps.limits[Graphics::LIMIT_TEXEL_BUFFER_SIZE])
		throw love::Exception("Cannot create texel buffer: total number of values in the buffer (%d * %d) is too large for this system (maximum %d).",
			(int) dataMembers.size(), (int) arraylength, caps.limits[Graphics::LIMIT_TEXEL_BUFFER_SIZE]);

	++bufferCount;
	totalGraphicsMemory += size;
}

Buffer::~Buffer()
{
	totalGraphicsMemory -= size;
	--bufferCount;
}

int Buffer::getDataMemberIndex(const std::string &name) const
{
	for (size_t i = 0; i < dataMembers.size(); i++)
	{
		if (dataMembers[i].decl.name == name)
			return (int) i;
	}

	return -1;
}

int Buffer::getDataMemberIndex(int bindingLocation) const
{
	for (size_t i = 0; i < dataMembers.size(); i++)
	{
		if (dataMembers[i].decl.bindingLocation == bindingLocation)
			return (int)i;
	}

	return -1;
}

void Buffer::clear(size_t offset, size_t size)
{
	if (isImmutable())
		throw love::Exception("Cannot clear an immutable Buffer.");
	else if (isMapped())
		throw love::Exception("Cannot clear a mapped Buffer.");
	else if (offset + size > getSize())
		throw love::Exception("The given offset and size parameters to clear() are not within the Buffer's size.");
	else if (offset % 4 != 0 || size % 4 != 0)
		throw love::Exception("clear() must be used with offset and size parameters that are multiples of 4 bytes.");

	clearInternal(offset, size);
}

std::vector<Buffer::DataDeclaration> Buffer::getCommonFormatDeclaration(CommonFormat format)
{
	switch (format)
	{
	case CommonFormat::NONE:
		return {};
	case CommonFormat::XYf:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_POS }
		};
	case CommonFormat::XYZf:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC3, 0, ATTRIB_POS }
		};
	case CommonFormat::RGBAub:
		return {
			{ getConstant(ATTRIB_COLOR), DATAFORMAT_UNORM8_VEC4, 0, ATTRIB_COLOR }
		};
	case CommonFormat::STf_RGBAub:
		return {
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_TEXCOORD },
			{ getConstant(ATTRIB_COLOR), DATAFORMAT_UNORM8_VEC4, 0, ATTRIB_COLOR },
		};
	case CommonFormat::STPf_RGBAub:
		return {
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC3, 0, ATTRIB_TEXCOORD },
			{ getConstant(ATTRIB_COLOR), DATAFORMAT_UNORM8_VEC4, 0, ATTRIB_COLOR },
		};
	case CommonFormat::XYf_STf:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_POS },
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_TEXCOORD },
		};
	case CommonFormat::XYf_STPf:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_POS },
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC3, 0, ATTRIB_TEXCOORD },
		};
	case CommonFormat::XYf_STf_RGBAub:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_POS },
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_TEXCOORD },
			{ getConstant(ATTRIB_COLOR), DATAFORMAT_UNORM8_VEC4, 0, ATTRIB_COLOR },
		};
	case CommonFormat::XYf_STus_RGBAub:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_POS },
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_UNORM16_VEC2, 0, ATTRIB_TEXCOORD },
			{ getConstant(ATTRIB_COLOR), DATAFORMAT_UNORM8_VEC4, 0, ATTRIB_COLOR },
		};
	case CommonFormat::XYf_STPf_RGBAub:
		return {
			{ getConstant(ATTRIB_POS), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_POS },
			{ getConstant(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC2, 0, ATTRIB_TEXCOORD },
			{ getConstant(ATTRIB_COLOR), DATAFORMAT_UNORM8_VEC4, 0, ATTRIB_COLOR },
		};
	case CommonFormat::COUNT:
		return {};
	}

	return {};
}

} // graphics
} // love
