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

// LOVE
#include "Mesh.h"
#include "common/Matrix.h"
#include "common/Exception.h"
#include "Shader.h"
#include "Graphics.h"

// C++
#include <algorithm>
#include <limits>

namespace love
{
namespace graphics
{

static_assert(offsetof(Vertex, x) == sizeof(float) * 0, "Incorrect position offset in Vertex struct");
static_assert(offsetof(Vertex, s) == sizeof(float) * 2, "Incorrect texture coordinate offset in Vertex struct");
static_assert(offsetof(Vertex, color.r) == sizeof(float) * 4, "Incorrect color offset in Vertex struct");

std::vector<Buffer::DataDeclaration> Mesh::getDefaultVertexFormat()
{
	return Buffer::getCommonFormatDeclaration(CommonFormat::XYf_STf_RGBAub);
}

love::Type Mesh::type("Mesh", &Drawable::type);

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<Buffer::DataDeclaration> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, BufferDataUsage usage)
	: primitiveType(drawmode)
{
	try
	{
		vertexData = new uint8[datasize];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory");
	}

	memcpy(vertexData, data, datasize);

	Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, usage);
	vertexBuffer.set(gfx->newBuffer(settings, vertexformat, vertexData, datasize, 0), Acquire::NORETAIN);

	vertexCount = vertexBuffer->getArrayLength();
	vertexStride = vertexBuffer->getArrayStride();
	vertexFormat = vertexBuffer->getDataMembers();

	setupAttachedAttributes();

	indexDataType = getIndexDataTypeFromMax(vertexCount);
}

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<Buffer::DataDeclaration> &vertexformat, int vertexcount, PrimitiveType drawmode, BufferDataUsage usage)
	: vertexCount((size_t) vertexcount)
	, indexDataType(getIndexDataTypeFromMax(vertexcount))
	, primitiveType(drawmode)
{
	if (vertexcount <= 0)
		throw love::Exception("Invalid number of vertices (%d).", vertexcount);

	Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, usage);
	vertexBuffer.set(gfx->newBuffer(settings, vertexformat, nullptr, 0, vertexcount), Acquire::NORETAIN);

	vertexStride = vertexBuffer->getArrayStride();
	vertexFormat = vertexBuffer->getDataMembers();

	setupAttachedAttributes();

	try
	{
		vertexData = new uint8[vertexBuffer->getSize()];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory");
	}

	memset(vertexData, 0, vertexBuffer->getSize());
	vertexBuffer->fill(0, vertexBuffer->getSize(), vertexData);
}

Mesh::Mesh(const std::vector<Mesh::BufferAttribute> &attributes, PrimitiveType drawmode)
	: primitiveType(drawmode)
{
	if (attributes.size() == 0)
		throw love::Exception("At least one buffer attribute must be specified in this constructor.");

	attachedAttributes = attributes;
	vertexCount = attachedAttributes.size() > 0 ? LOVE_UINT32_MAX : 0;

	for (int i = 0; i < (int) attachedAttributes.size(); i++)
	{
		auto &attrib = attachedAttributes[i];

		finalizeAttribute(attrib);

		int attributeIndex = getAttachedAttributeIndex(attrib.name);
		if (attributeIndex != i && attributeIndex != -1)
			throw love::Exception("Duplicate vertex attribute name: %s", attrib.name.c_str());

		vertexCount = std::min(vertexCount, attrib.buffer->getArrayLength());
	}

	indexDataType = getIndexDataTypeFromMax(vertexCount);
}

Mesh::~Mesh()
{
	delete vertexData;
	if (indexData != nullptr)
		free(indexData);
}

void Mesh::setupAttachedAttributes()
{
	for (size_t i = 0; i < vertexFormat.size(); i++)
	{
		const std::string &name = vertexFormat[i].decl.name;

		if (getAttachedAttributeIndex(name) != -1)
			throw love::Exception("Duplicate vertex attribute name: %s", name.c_str());

		BuiltinVertexAttribute builtinattrib;
		int builtinAttribIndex = -1;
		if (getConstant(name.c_str(), builtinattrib))
			builtinAttribIndex = (int)builtinattrib;

		attachedAttributes.push_back({name, vertexBuffer, nullptr, name, (int) i, 0, STEP_PER_VERTEX, builtinAttribIndex, true});
	}
}

int Mesh::getAttachedAttributeIndex(const std::string &name) const
{
	for (int i = 0; i < (int) attachedAttributes.size(); i++)
	{
		if (attachedAttributes[i].name == name)
			return i;
	}

	return -1;
}

void Mesh::finalizeAttribute(BufferAttribute &attrib) const
{
	if ((attrib.buffer->getUsageFlags() & BUFFERUSAGEFLAG_VERTEX) == 0)
		throw love::Exception("Buffer must be created with vertex buffer support to be used as a Mesh vertex attribute.");

	if (attrib.startArrayIndex < 0 || attrib.startArrayIndex >= (int)attrib.buffer->getArrayLength())
		throw love::Exception("Invalid start array index %d.", attrib.startArrayIndex + 1);

	int indexInBuffer = attrib.buffer->getDataMemberIndex(attrib.nameInBuffer);
	if (indexInBuffer < 0)
		throw love::Exception("Buffer does not have a vertex attribute with name '%s'.", attrib.nameInBuffer.c_str());

	BuiltinVertexAttribute builtinattrib;
	if (getConstant(attrib.name.c_str(), builtinattrib))
		attrib.builtinAttributeIndex = (int)builtinattrib;
	else
		attrib.builtinAttributeIndex = -1;

	attrib.indexInBuffer = indexInBuffer;
}

void *Mesh::checkVertexDataOffset(size_t vertindex, size_t *byteoffset)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	if (vertexData == nullptr)
		throw love::Exception("Mesh must own its own vertex buffer.");

	size_t offset = vertindex * vertexStride;

	if (byteoffset != nullptr)
		*byteoffset = offset;
	return vertexData + offset;
}

size_t Mesh::getVertexCount() const
{
	return vertexCount;
}

size_t Mesh::getVertexStride() const
{
	return vertexStride;
}

Buffer *Mesh::getVertexBuffer() const
{
	return vertexBuffer;
}

const std::vector<Buffer::DataMember> &Mesh::getVertexFormat() const
{
	return vertexFormat;
}

void Mesh::setAttributeEnabled(const std::string &name, bool enable)
{
	int index = getAttachedAttributeIndex(name);
	if (index == -1)
		throw love::Exception("Mesh does not have an attached vertex attribute named '%s'", name.c_str());

	attachedAttributes[index].enabled = enable;
}

bool Mesh::isAttributeEnabled(const std::string &name) const
{
	int index = getAttachedAttributeIndex(name);
	if (index == -1)
		throw love::Exception("Mesh does not have an attached vertex attribute named '%s'", name.c_str());

	return attachedAttributes[index].enabled;
}

void Mesh::attachAttribute(const std::string &name, Buffer *buffer, Mesh *mesh, const std::string &attachname, int startindex, AttributeStep step)
{
	BufferAttribute oldattrib = {};
	BufferAttribute newattrib = {};

	int oldindex = getAttachedAttributeIndex(name);
	if (oldindex != -1)
		oldattrib = attachedAttributes[oldindex];
	else if (attachedAttributes.size() + 1 > VertexAttributes::MAX)
		throw love::Exception("A maximum of %d attributes can be attached at once.", VertexAttributes::MAX);

	newattrib.name = name;
	newattrib.buffer = buffer;
	newattrib.mesh = mesh;
	newattrib.enabled = oldattrib.buffer.get() ? oldattrib.enabled : true;
	newattrib.nameInBuffer = attachname;
	newattrib.indexInBuffer = -1;
	newattrib.startArrayIndex = startindex;
	newattrib.step = step;

	finalizeAttribute(newattrib);

	if (newattrib.indexInBuffer < 0)
		throw love::Exception("The specified vertex buffer does not have a vertex attribute named '%s'", attachname.c_str());

	if (oldindex != -1)
		attachedAttributes[oldindex] = newattrib;
	else
		attachedAttributes.push_back(newattrib);
}

bool Mesh::detachAttribute(const std::string &name)
{
	int index = getAttachedAttributeIndex(name);
	if (index == -1)
		return false;

	attachedAttributes.erase(attachedAttributes.begin() + index);

	if (vertexBuffer.get() && vertexBuffer->getDataMemberIndex(name) != -1)
		attachAttribute(name, vertexBuffer, nullptr, name);

	return true;
}

const std::vector<Mesh::BufferAttribute> &Mesh::getAttachedAttributes() const
{
	return attachedAttributes;
}

void *Mesh::getVertexData() const
{
	return vertexData;
}

void Mesh::setVertexDataModified(size_t offset, size_t size)
{
	if (vertexData != nullptr)
		modifiedVertexData.encapsulate(offset, size);
}

void Mesh::flush()
{
	if (vertexBuffer.get() && vertexData != nullptr && modifiedVertexData.isValid())
	{
		if (vertexBuffer->getDataUsage() == BUFFERDATAUSAGE_STREAM)
		{
			vertexBuffer->fill(0, vertexBuffer->getSize(), vertexData);
		}
		else
		{
			size_t offset = modifiedVertexData.getOffset();
			size_t size = modifiedVertexData.getSize();
			vertexBuffer->fill(offset, size, vertexData + offset);
		}

		modifiedVertexData.invalidate();
	}

	if (indexDataModified && indexData != nullptr && indexBuffer != nullptr)
	{
		indexBuffer->fill(0, indexBuffer->getSize(), indexData);
		indexDataModified = false;
	}
}

/**
 * Copies index data from a vector to a mapped index buffer.
 **/
template <typename T>
static void copyToIndexBuffer(const std::vector<uint32> &indices, void *data, size_t maxval)
{
	T *elems = (T *) data;

	for (size_t i = 0; i < indices.size(); i++)
	{
		if (indices[i] >= maxval)
			throw love::Exception("Invalid vertex map value: %d", indices[i] + 1);

		elems[i] = (T) indices[i];
	}
}

void Mesh::setVertexMap(const std::vector<uint32> &map)
{
	if (map.empty())
		throw love::Exception("Vertex map array must not be empty.");

	size_t maxval = getVertexCount();

	IndexDataType datatype = getIndexDataTypeFromMax(maxval);
	DataFormat dataformat = getIndexDataFormat(datatype);

	// Calculate the size in bytes of the index buffer data.
	size_t size = map.size() * getIndexDataSize(datatype);

	bool recreate = indexData == nullptr || indexBuffer.get() == nullptr
		|| size > indexBuffer->getSize() || indexBuffer->getDataMember(0).decl.format != dataformat;

	if (recreate)
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		auto usage = vertexBuffer.get() ? vertexBuffer->getDataUsage() : BUFFERDATAUSAGE_DYNAMIC;
		Buffer::Settings settings(BUFFERUSAGEFLAG_INDEX, usage);
		auto buffer = StrongRef<Buffer>(gfx->newBuffer(settings, dataformat, nullptr, size, 0), Acquire::NORETAIN);

		auto data = (uint8 *) realloc(indexData, size);
		if (data == nullptr)
			throw love::Exception("Out of memory.");

		indexData = data;
		indexBuffer = buffer;
	}

	indexCount = map.size();
	useIndexBuffer = true;
	indexDataType = datatype;

	if (indexCount == 0)
		return;

	// Fill the buffer with the index values from the vector.
	switch (datatype)
	{
	case INDEX_UINT16:
		copyToIndexBuffer<uint16>(map, indexData, maxval);
		break;
	case INDEX_UINT32:
	default:
		copyToIndexBuffer<uint32>(map, indexData, maxval);
		break;
	}

	indexDataModified = true;
}

void Mesh::setVertexMap(IndexDataType datatype, const void *data, size_t datasize)
{
	DataFormat dataformat = getIndexDataFormat(datatype);

	bool recreate = indexData == nullptr || indexBuffer.get() == nullptr
		|| datasize > indexBuffer->getSize() || indexBuffer->getDataMember(0).decl.format != dataformat;

	if (recreate)
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		auto usage = vertexBuffer.get() ? vertexBuffer->getDataUsage() : BUFFERDATAUSAGE_DYNAMIC;
		Buffer::Settings settings(BUFFERUSAGEFLAG_INDEX, usage);
		auto buffer = StrongRef<Buffer>(gfx->newBuffer(settings, dataformat, nullptr, datasize, 0), Acquire::NORETAIN);

		auto data = (uint8 *) realloc(indexData, datasize);
		if (data == nullptr)
			throw love::Exception("Out of memory.");

		indexData = data;
		indexBuffer = buffer;
	}

	indexCount = datasize / getIndexDataSize(datatype);
	useIndexBuffer = true;
	indexDataType = datatype;

	if (indexCount == 0)
		return;

	memcpy(indexData, data, datasize);
	indexDataModified = true;
}

void Mesh::setVertexMap()
{
	useIndexBuffer = false;
}

/**
 * Copies index data from a mapped buffer to a vector.
 **/
template <typename T>
static void copyFromIndexBuffer(const void *buffer, size_t count, std::vector<uint32> &indices)
{
	const T *elems = (const T *) buffer;
	for (size_t i = 0; i < count; i++)
		indices.push_back((uint32) elems[i]);
}

bool Mesh::getVertexMap(std::vector<uint32> &map) const
{
	if (!useIndexBuffer)
		return false;

	map.clear();

	if (indexData == nullptr || indexCount == 0)
		return true;

	map.reserve(indexCount);

	// Fill the vector from the buffer.
	switch (indexDataType)
	{
	case INDEX_UINT16:
		copyFromIndexBuffer<uint16>(indexData, indexCount, map);
		break;
	case INDEX_UINT32:
	default:
		copyFromIndexBuffer<uint32>(indexData, indexCount, map);
		break;
	}

	return true;
}

void Mesh::setIndexBuffer(Buffer *buffer)
{
	// Buffer constructor does the rest of the validation for index buffers
	// (data member formats, etc.)
	if (buffer != nullptr && (buffer->getUsageFlags() & BUFFERUSAGEFLAG_INDEX) == 0)
		throw love::Exception("setIndexBuffer requires a Buffer created as an index buffer.");

	indexBuffer.set(buffer);
	useIndexBuffer = buffer != nullptr;
	indexCount = buffer != nullptr ? buffer->getArrayLength() : 0;

	if (buffer != nullptr)
		indexDataType = getIndexDataType(buffer->getDataMember(0).decl.format);

	if (indexData != nullptr)
	{
		free(indexData);
		indexData = nullptr;
	}
}

Buffer *Mesh::getIndexBuffer() const
{
	return indexBuffer;
}

size_t Mesh::getIndexCount() const
{
	return indexCount;
}

void Mesh::setTexture(Texture *tex)
{
	texture.set(tex);
}

void Mesh::setTexture()
{
	texture.set(nullptr);
}

Texture *Mesh::getTexture() const
{
	return texture.get();
}

void Mesh::setDrawMode(PrimitiveType mode)
{
	primitiveType = mode;
}

PrimitiveType Mesh::getDrawMode() const
{
	return primitiveType;
}

void Mesh::setDrawRange(int start, int count)
{
	if (start < 0 || count <= 0)
		throw love::Exception("Invalid draw range.");

	drawRange = Range(start, count);
}

void Mesh::setDrawRange()
{
	drawRange.invalidate();
}

bool Mesh::getDrawRange(int &start, int &count) const
{
	if (!drawRange.isValid())
		return false;

	start = (int) drawRange.getOffset();
	count = (int) drawRange.getSize();
	return true;
}

void Mesh::draw(Graphics *gfx, const love::Matrix4 &m)
{
	drawInternal(gfx, m, 1, nullptr, 0);
}

void Mesh::drawInstanced(Graphics *gfx, const Matrix4 &m, int instancecount)
{
	drawInternal(gfx, m, instancecount, nullptr, 0);
}

void Mesh::drawIndirect(Graphics *gfx, const Matrix4 &m, Buffer *indirectargs, int argsindex)
{
	drawInternal(gfx, m, 0, indirectargs, argsindex);
}

void Mesh::drawInternal(Graphics *gfx, const Matrix4 &m, int instancecount, Buffer *indirectargs, int argsindex)
{
	if (vertexCount <= 0 || (instancecount <= 0 && indirectargs == nullptr))
		return;

	if (indirectargs != nullptr)
	{
		if (primitiveType == PRIMITIVE_TRIANGLE_FAN)
			throw love::Exception("The fan draw mode is not supported in indirect draws.");

		if (useIndexBuffer && indexBuffer != nullptr)
			gfx->validateIndirectArgsBuffer(Graphics::INDIRECT_ARGS_DRAW_INDICES, indirectargs, argsindex);
		else
			gfx->validateIndirectArgsBuffer(Graphics::INDIRECT_ARGS_DRAW_VERTICES, indirectargs, argsindex);
	}

	// Some graphics backends don't natively support triangle fans. So we'd
	// have to emulate them with triangles plus an index buffer... which doesn't
	// work so well when there's already a custom index buffer.
	if (primitiveType == PRIMITIVE_TRIANGLE_FAN && useIndexBuffer && indexBuffer != nullptr)
		throw love::Exception("The 'fan' Mesh draw mode cannot be used with an index buffer / vertex map.");

	gfx->flushBatchedDraws();

	flush();

	if (Shader::isDefaultActive())
		Shader::attachDefault(primitiveType == PRIMITIVE_POINTS ? Shader::STANDARD_POINTS : Shader::STANDARD_DEFAULT);

	if (Shader::current)
		Shader::current->validateDrawState(primitiveType, texture);

	VertexAttributes attributes;
	BufferBindings buffers;

	int activebuffers = 0;

	for (const auto &attrib : attachedAttributes)
	{
		if (!attrib.enabled)
			continue;

		Buffer *buffer = attrib.buffer.get();
		int attributeindex = attrib.builtinAttributeIndex;

		// If the attribute is one of the LOVE-defined ones, use the constant
		// attribute index for it, otherwise query the index from the shader.
		if (attributeindex < 0 && Shader::current)
			attributeindex = Shader::current->getVertexAttributeIndex(attrib.name);

		if (attributeindex >= 0)
		{
			if (attrib.mesh.get())
				attrib.mesh->flush();

			const auto &member = buffer->getDataMember(attrib.indexInBuffer);

			uint16 offset = (uint16) member.offset;
			uint16 stride = (uint16) buffer->getArrayStride();
			size_t bufferoffset = (size_t) stride * attrib.startArrayIndex;

			attributes.set(attributeindex, member.decl.format, offset, activebuffers);
			attributes.setBufferLayout(activebuffers, stride, attrib.step);

			// TODO: Ideally we want to reuse buffers with the same stride+step.
			buffers.set(activebuffers, buffer, bufferoffset);
			activebuffers++;
		}
	}

	// Not supported on all platforms or GL versions, I believe.
	if ((attributes.enableBits & ~(ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR)) == 0)
		throw love::Exception("Mesh must have an enabled VertexPosition or custom attribute to be drawn.");

	Graphics::TempTransform transform(gfx, m);

	Buffer *indexbuffer = useIndexBuffer ? indexBuffer : nullptr;
	int indexcount = (int) indexCount;
	Range range = drawRange;

	// Emulated triangle fan via an index buffer.
	if (primitiveType == PRIMITIVE_TRIANGLE_FAN && indexbuffer == nullptr && gfx->getFanIndexBuffer())
	{
		indexbuffer = gfx->getFanIndexBuffer();
		indexcount = graphics::getIndexCount(TRIANGLEINDEX_FAN, vertexCount);
		if (range.isValid())
		{
			int start = graphics::getIndexCount(TRIANGLEINDEX_FAN, (int) range.getOffset());
			int count = graphics::getIndexCount(TRIANGLEINDEX_FAN, (int) range.getSize());
			range = Range(start, count);
		}
	}

	if (indexbuffer != nullptr && (indexcount > 0 || indirectargs != nullptr))
	{
		Range r(0, indexcount);
		if (range.isValid())
			r.intersect(range);

		Graphics::DrawIndexedCommand cmd(&attributes, &buffers, indexbuffer);

		cmd.primitiveType = primitiveType;
		cmd.indexType = indexDataType;
		cmd.instanceCount = instancecount;
		cmd.texture = gfx->getTextureOrDefaultForActiveShader(texture);
		cmd.cullMode = gfx->getMeshCullMode();

		cmd.indexBufferOffset = r.getOffset() * indexbuffer->getArrayStride();
		cmd.indexCount = (int) r.getSize();

		cmd.indirectBuffer = indirectargs;
		cmd.indirectBufferOffset = argsindex * (indirectargs != nullptr ? indirectargs->getArrayStride() : 0);

		if (cmd.indexCount > 0)
			gfx->draw(cmd);
	}
	else if (vertexCount > 0 || indirectargs != nullptr)
	{
		Range r(0, vertexCount);
		if (range.isValid())
			r.intersect(range);

		Graphics::DrawCommand cmd(&attributes, &buffers);

		cmd.primitiveType = primitiveType;
		cmd.vertexStart = (int) r.getOffset();
		cmd.vertexCount = (int) r.getSize();
		cmd.instanceCount = instancecount;
		cmd.texture = gfx->getTextureOrDefaultForActiveShader(texture);
		cmd.cullMode = gfx->getMeshCullMode();

		cmd.indirectBuffer = indirectargs;
		cmd.indirectBufferOffset = argsindex * (indirectargs != nullptr ? indirectargs->getArrayStride() : 0);

		if (cmd.vertexCount > 0)
			gfx->draw(cmd);
	}
}

} // graphics
} // love
