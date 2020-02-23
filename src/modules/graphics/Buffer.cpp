/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

namespace love
{
namespace graphics
{

love::Type Buffer::type("GraphicsBuffer", &Object::type);

Buffer::Buffer(Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &bufferformat, size_t size, size_t arraylength)
	: arrayLength(0)
	, arrayStride(0)
	, size(size)
	, typeFlags(settings.typeFlags)
	, usage(settings.usage)
	, mapFlags(settings.mapFlags)
	, mapped(false)
{
	if (size == 0 && arraylength == 0)
		throw love::Exception("Size or array length must be specified.");

	if (bufferformat.size() == 0)
		throw love::Exception("Data format must contain values.");

	bool supportsGLSL3 = gfx->getCapabilities().features[Graphics::FEATURE_GLSL3];

	bool indexbuffer = settings.typeFlags & TYPEFLAG_INDEX;
	bool vertexbuffer = settings.typeFlags & TYPEFLAG_VERTEX;

	size_t offset = 0;
	size_t stride = 0;

	for (const auto &decl : bufferformat)
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
		}

		if (vertexbuffer)
		{
			if (decl.arraySize > 0)
				throw love::Exception("Arrays are not supported in vertex buffers.");

			if (info.isMatrix)
				throw love::Exception("Matrix types are not supported in vertex buffers.");

			if (info.baseType == DATA_BASETYPE_BOOL)
				throw love::Exception("Bool types are not supported in vertex buffers.");

			if ((info.baseType == DATA_BASETYPE_INT || info.baseType == DATA_BASETYPE_UINT) && !supportsGLSL3)
				throw love::Exception("Integer vertex attribute data types require GLSL 3 support.");
		}

		// TODO: alignment
		member.offset = offset;
		member.size = member.info.size;

		offset += member.size;

		dataMembers.push_back(member);
	}

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
}

Buffer::~Buffer()
{
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

std::vector<Buffer::DataDeclaration> Buffer::getCommonFormatDeclaration(CommonFormat format)
{
	switch (format)
	{
	case CommonFormat::NONE:
		return {};
	case CommonFormat::XYf:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC2}
		};
	case CommonFormat::XYZf:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC3}
		};
	case CommonFormat::RGBAub:
		return {
			{"VertexColor", DATAFORMAT_UNORM8_VEC4}
		};
	case CommonFormat::STf_RGBAub:
		return {
			{"VertexTexCoord", DATAFORMAT_FLOAT_VEC2},
			{"VertexColor", DATAFORMAT_UNORM8_VEC4},
		};
	case CommonFormat::STPf_RGBAub:
		return {
			{"VertexTexCoord", DATAFORMAT_FLOAT_VEC3},
			{"VertexColor", DATAFORMAT_UNORM8_VEC4},
		};
	case CommonFormat::XYf_STf:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC2},
			{"VertexTexCoord", DATAFORMAT_FLOAT_VEC2},
		};
	case CommonFormat::XYf_STPf:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC2},
			{"VertexTexCoord", DATAFORMAT_FLOAT_VEC3},
		};
	case CommonFormat::XYf_STf_RGBAub:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC2},
			{"VertexTexCoord", DATAFORMAT_FLOAT_VEC2},
			{"VertexColor", DATAFORMAT_UNORM8_VEC4},
		};
	case CommonFormat::XYf_STus_RGBAub:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC2},
			{"VertexTexCoord", DATAFORMAT_UNORM16_VEC2},
			{"VertexColor", DATAFORMAT_UNORM8_VEC4},
		};
	case CommonFormat::XYf_STPf_RGBAub:
		return {
			{"VertexPosition", DATAFORMAT_FLOAT_VEC2},
			{"VertexTexCoord", DATAFORMAT_FLOAT_VEC2},
			{"VertexColor", DATAFORMAT_UNORM8_VEC4},
		};
	}

	return {};
}

} // graphics
} // love
