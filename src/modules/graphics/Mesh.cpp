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

static const char *getBuiltinAttribName(BuiltinVertexAttribute attribid)
{
	const char *name = "";
	getConstant(attribid, name);
	return name;
}

static_assert(offsetof(Vertex, x) == sizeof(float) * 0, "Incorrect position offset in Vertex struct");
static_assert(offsetof(Vertex, s) == sizeof(float) * 2, "Incorrect texture coordinate offset in Vertex struct");
static_assert(offsetof(Vertex, color.r) == sizeof(float) * 4, "Incorrect color offset in Vertex struct");

std::vector<Buffer::DataDeclaration> Mesh::getDefaultVertexFormat()
{
	// Corresponds to the love::Vertex struct.
	std::vector<Buffer::DataDeclaration> vertexformat = {
		{ getBuiltinAttribName(ATTRIB_POS),      DATAFORMAT_FLOAT_VEC2,  0 },
		{ getBuiltinAttribName(ATTRIB_TEXCOORD), DATAFORMAT_FLOAT_VEC2,  0 },
		{ getBuiltinAttribName(ATTRIB_COLOR),    DATAFORMAT_UNORM8_VEC4, 0 },
	};

	return vertexformat;
}

love::Type Mesh::type("Mesh", &Drawable::type);

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<Buffer::DataDeclaration> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, BufferUsage usage)
	: vertexBuffer(nullptr)
	, vertexCount(0)
	, vertexStride(0)
	, indexBuffer(nullptr)
	, useIndexBuffer(false)
	, indexCount(0)
	, indexDataType(INDEX_UINT16)
	, primitiveType(drawmode)
	, rangeStart(-1)
	, rangeCount(-1)
{
	Buffer::Settings settings(Buffer::TYPEFLAG_VERTEX, Buffer::MAP_EXPLICIT_RANGE_MODIFY | Buffer::MAP_READ, usage);
	vertexBuffer.set(gfx->newBuffer(settings, vertexformat, data, datasize, 0), Acquire::NORETAIN);

	vertexCount = vertexBuffer->getArrayLength();
	vertexStride = vertexBuffer->getArrayStride();
	vertexFormat = vertexBuffer->getDataMembers();

	setupAttachedAttributes();

	indexDataType = getIndexDataTypeFromMax(vertexCount);

	vertexScratchBuffer = new char[vertexStride];
}

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<Buffer::DataDeclaration> &vertexformat, int vertexcount, PrimitiveType drawmode, BufferUsage usage)
	: vertexBuffer(nullptr)
	, vertexCount((size_t) vertexcount)
	, vertexStride(0)
	, indexBuffer(nullptr)
	, useIndexBuffer(false)
	, indexCount(0)
	, indexDataType(getIndexDataTypeFromMax(vertexcount))
	, primitiveType(drawmode)
	, rangeStart(-1)
	, rangeCount(-1)
{
	if (vertexcount <= 0)
		throw love::Exception("Invalid number of vertices (%d).", vertexcount);

	Buffer::Settings settings(Buffer::TYPEFLAG_VERTEX, Buffer::MAP_EXPLICIT_RANGE_MODIFY | Buffer::MAP_READ, usage);
	vertexBuffer.set(gfx->newBuffer(settings, vertexformat, nullptr, 0, vertexcount), Acquire::NORETAIN);

	vertexStride = vertexBuffer->getArrayStride();
	vertexFormat = vertexBuffer->getDataMembers();

	setupAttachedAttributes();

	memset(vertexBuffer->map(), 0, vertexBuffer->getSize());
	vertexBuffer->setMappedRangeModified(0, vertexBuffer->getSize());
	vertexBuffer->unmap();

	vertexScratchBuffer = new char[vertexStride];
}

Mesh::~Mesh()
{
	delete vertexScratchBuffer;
}

void Mesh::setupAttachedAttributes()
{
	for (size_t i = 0; i < vertexFormat.size(); i++)
	{
		const std::string &name = vertexFormat[i].decl.name;

		if (attachedAttributes.find(name) != attachedAttributes.end())
			throw love::Exception("Duplicate vertex attribute name: %s", name.c_str());

		attachedAttributes[name] = {vertexBuffer, (int) i, STEP_PER_VERTEX, true};
	}
}

void Mesh::setVertex(size_t vertindex, const void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	size_t offset = vertindex * vertexStride;
	size_t size = std::min(datasize, vertexStride);

	uint8 *bufferdata = (uint8 *) vertexBuffer->map();
	memcpy(bufferdata + offset, data, size);

	vertexBuffer->setMappedRangeModified(offset, size);
}

size_t Mesh::getVertex(size_t vertindex, void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	size_t offset = vertindex * vertexStride;
	size_t size = std::min(datasize, vertexStride);

	// We're relying on map() returning read/write data... ew.
	const uint8 *bufferdata = (const uint8 *) vertexBuffer->map();
	memcpy(data, bufferdata + offset, size);

	return size;
}

void *Mesh::getVertexScratchBuffer()
{
	return vertexScratchBuffer;
}

void Mesh::setVertexAttribute(size_t vertindex, int attribindex, const void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	if (attribindex >= (int) vertexFormat.size())
		throw love::Exception("Invalid vertex attribute index: %d", attribindex + 1);

	const auto &member = vertexFormat[attribindex];

	size_t offset = vertindex * vertexStride + member.offset;
	size_t size = std::min(datasize, member.info.size);

	uint8 *bufferdata = (uint8 *) vertexBuffer->map();
	memcpy(bufferdata + offset, data, size);

	vertexBuffer->setMappedRangeModified(offset, size);
}

size_t Mesh::getVertexAttribute(size_t vertindex, int attribindex, void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	if (attribindex >= (int) vertexFormat.size())
		throw love::Exception("Invalid vertex attribute index: %d", attribindex + 1);

	const auto &member = vertexFormat[attribindex];

	size_t offset = vertindex * vertexStride + member.offset;
	size_t size = std::min(datasize, member.info.size);

	// We're relying on map() returning read/write data... ew.
	const uint8 *bufferdata = (const uint8 *) vertexBuffer->map();
	memcpy(data, bufferdata + offset, size);

	return size;
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

int Mesh::getAttributeIndex(const std::string &name) const
{
	for (int i = 0; i < (int) vertexFormat.size(); i++)
	{
		if (vertexFormat[i].decl.name == name)
			return i;
	}

	return -1;
}

void Mesh::setAttributeEnabled(const std::string &name, bool enable)
{
	auto it = attachedAttributes.find(name);

	if (it == attachedAttributes.end())
		throw love::Exception("Mesh does not have an attached vertex attribute named '%s'", name.c_str());

	it->second.enabled = enable;
}

bool Mesh::isAttributeEnabled(const std::string &name) const
{
	const auto it = attachedAttributes.find(name);

	if (it == attachedAttributes.end())
		throw love::Exception("Mesh does not have an attached vertex attribute named '%s'", name.c_str());

	return it->second.enabled;
}

void Mesh::attachAttribute(const std::string &name, Buffer *buffer, const std::string &attachname, AttributeStep step)
{
	if ((buffer->getTypeFlags() & Buffer::TYPEFLAG_VERTEX) == 0)
		throw love::Exception("GraphicsBuffer must be created with vertex buffer support to be used as a Mesh vertex attribute.");

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (step == STEP_PER_INSTANCE && !gfx->getCapabilities().features[Graphics::FEATURE_INSTANCING])
		throw love::Exception("Vertex attribute instancing is not supported on this system.");

	AttachedAttribute oldattrib = {};
	AttachedAttribute newattrib = {};

	auto it = attachedAttributes.find(name);
	if (it != attachedAttributes.end())
		oldattrib = it->second;
	else if (attachedAttributes.size() + 1 > VertexAttributes::MAX)
		throw love::Exception("A maximum of %d attributes can be attached at once.", VertexAttributes::MAX);

	newattrib.buffer = buffer;
	newattrib.enabled = oldattrib.buffer.get() ? oldattrib.enabled : true;
	newattrib.index = buffer->getDataMemberIndex(attachname);
	newattrib.step = step;

	if (newattrib.index < 0)
		throw love::Exception("The specified vertex buffer does not have a vertex attribute named '%s'", attachname.c_str());

	attachedAttributes[name] = newattrib;
}

bool Mesh::detachAttribute(const std::string &name)
{
	auto it = attachedAttributes.find(name);

	if (it != attachedAttributes.end())
	{
		attachedAttributes.erase(it);

		if (getAttributeIndex(name) != -1)
			attachAttribute(name, vertexBuffer, name);

		return true;
	}

	return false;
}

void *Mesh::mapVertexData()
{
	return vertexBuffer->map();
}

void Mesh::unmapVertexData(size_t modifiedoffset, size_t modifiedsize)
{
	vertexBuffer->setMappedRangeModified(modifiedoffset, modifiedsize);
	vertexBuffer->unmap();
}

void Mesh::flush()
{
	vertexBuffer->unmap();

	if (indexBuffer != nullptr)
		indexBuffer->unmap();
}

/**
 * Copies index data from a vector to a mapped index buffer.
 **/
template <typename T>
static void copyToIndexBuffer(const std::vector<uint32> &indices, Buffer::Mapper &buffermap, size_t maxval)
{
	T *elems = (T *) buffermap.data;

	for (size_t i = 0; i < indices.size(); i++)
	{
		if (indices[i] >= maxval)
			throw love::Exception("Invalid vertex map value: %d", indices[i] + 1);

		elems[i] = (T) indices[i];
	}
}

void Mesh::setVertexMap(const std::vector<uint32> &map)
{
	size_t maxval = getVertexCount();

	IndexDataType datatype = getIndexDataTypeFromMax(maxval);
	DataFormat dataformat = getIndexDataFormat(datatype);

	// Calculate the size in bytes of the index buffer data.
	size_t size = map.size() * getIndexDataSize(datatype);

	if (indexBuffer.get() == nullptr || size > indexBuffer->getSize() || indexBuffer->getDataMember(0).decl.format != dataformat)
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		Buffer::Settings settings(Buffer::TYPEFLAG_INDEX, Buffer::MAP_READ, vertexBuffer->getUsage());
		indexBuffer.set(gfx->newBuffer(settings, dataformat, nullptr, size, 0), Acquire::NORETAIN);
	}

	useIndexBuffer = true;
	indexCount = map.size();

	if (!indexBuffer || indexCount == 0)
		return;

	Buffer::Mapper ibomap(*indexBuffer);

	// Fill the buffer with the index values from the vector.
	switch (datatype)
	{
	case INDEX_UINT16:
		copyToIndexBuffer<uint16>(map, ibomap, maxval);
		break;
	case INDEX_UINT32:
	default:
		copyToIndexBuffer<uint32>(map, ibomap, maxval);
		break;
	}

	indexDataType = datatype;
}

void Mesh::setVertexMap(IndexDataType datatype, const void *data, size_t datasize)
{
	DataFormat dataformat = getIndexDataFormat(datatype);

	if (indexBuffer.get() == nullptr || datasize > indexBuffer->getSize() || indexBuffer->getDataMember(0).decl.format != dataformat)
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		Buffer::Settings settings(Buffer::TYPEFLAG_INDEX, Buffer::MAP_READ, vertexBuffer->getUsage());
		indexBuffer.set(gfx->newBuffer(settings, dataformat, nullptr, datasize, 0), Acquire::NORETAIN);
	}

	indexCount = datasize / getIndexDataSize(datatype);

	if (!indexBuffer || indexCount == 0)
		return;

	Buffer::Mapper ibomap(*indexBuffer);
	memcpy(ibomap.data, data, datasize);

	useIndexBuffer = true;
	indexDataType = datatype;
}

void Mesh::setVertexMap()
{
	useIndexBuffer = false;
}

/**
 * Copies index data from a mapped buffer to a vector.
 **/
template <typename T>
static void copyFromIndexBuffer(void *buffer, size_t count, std::vector<uint32> &indices)
{
	T *elems = (T *) buffer;
	for (size_t i = 0; i < count; i++)
		indices.push_back((uint32) elems[i]);
}

bool Mesh::getVertexMap(std::vector<uint32> &map) const
{
	if (!useIndexBuffer)
		return false;

	map.clear();
	map.reserve(indexCount);

	if (!indexBuffer || indexCount == 0)
		return true;

	// We unmap the buffer in Mesh::draw, Mesh::setVertexMap, and Mesh::flush.
	void *buffer = indexBuffer->map();

	// Fill the vector from the buffer.
	switch (indexDataType)
	{
	case INDEX_UINT16:
		copyFromIndexBuffer<uint16>(buffer, indexCount, map);
		break;
	case INDEX_UINT32:
	default:
		copyFromIndexBuffer<uint32>(buffer, indexCount, map);
		break;
	}

	return true;
}

size_t Mesh::getVertexMapCount() const
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

	rangeStart = start;
	rangeCount = count;
}

void Mesh::setDrawRange()
{
	rangeStart = rangeCount = -1;
}

bool Mesh::getDrawRange(int &start, int &count) const
{
	if (rangeStart < 0 || rangeCount <= 0)
		return false;

	start = rangeStart;
	count = rangeCount;
	return true;
}

void Mesh::draw(Graphics *gfx, const love::Matrix4 &m)
{
	drawInstanced(gfx, m, 1);
}

void Mesh::drawInstanced(Graphics *gfx, const Matrix4 &m, int instancecount)
{
	if (vertexCount <= 0 || instancecount <= 0)
		return;

	if (instancecount > 1 && !gfx->getCapabilities().features[Graphics::FEATURE_INSTANCING])
		throw love::Exception("Instancing is not supported on this system.");

	gfx->flushStreamDraws();

	if (Shader::isDefaultActive())
		Shader::attachDefault(Shader::STANDARD_DEFAULT);

	if (Shader::current && texture.get())
		Shader::current->checkMainTexture(texture);

	VertexAttributes attributes;
	BufferBindings buffers;

	int activebuffers = 0;

	for (const auto &attrib : attachedAttributes)
	{
		if (!attrib.second.enabled)
			continue;

		Buffer *buffer = attrib.second.buffer.get();
		int attributeindex = -1;

		// If the attribute is one of the LOVE-defined ones, use the constant
		// attribute index for it, otherwise query the index from the shader.
		BuiltinVertexAttribute builtinattrib;
		if (getConstant(attrib.first.c_str(), builtinattrib))
			attributeindex = (int) builtinattrib;
		else if (Shader::current)
			attributeindex = Shader::current->getVertexAttributeIndex(attrib.first);

		if (attributeindex >= 0)
		{
			// Make sure the buffer isn't mapped (sends data to GPU if needed.)
			buffer->unmap();

			const auto &member = buffer->getDataMember(attrib.second.index);

			uint16 offset = (uint16) buffer->getMemberOffset(attrib.second.index);
			uint16 stride = (uint16) buffer->getArrayStride();

			attributes.set(attributeindex, member.decl.format, offset, activebuffers);
			attributes.setBufferLayout(activebuffers, stride, attrib.second.step);

			// TODO: Ideally we want to reuse buffers with the same stride+step.
			buffers.set(activebuffers, buffer, 0);
			activebuffers++;
		}
	}

	// Not supported on all platforms or GL versions, I believe.
	if (!attributes.isEnabled(ATTRIB_POS))
		throw love::Exception("Mesh must have an enabled VertexPosition attribute to be drawn.");

	Graphics::TempTransform transform(gfx, m);

	if (useIndexBuffer && indexBuffer != nullptr && indexCount > 0)
	{
		// Make sure the index buffer isn't mapped (sends data to GPU if needed.)
		indexBuffer->unmap();

		Graphics::DrawIndexedCommand cmd(&attributes, &buffers, indexBuffer);

		cmd.primitiveType = primitiveType;
		cmd.indexType = indexDataType;
		cmd.instanceCount = instancecount;
		cmd.texture = texture;
		cmd.cullMode = gfx->getMeshCullMode();

		int start = std::min(std::max(0, rangeStart), (int) indexCount - 1);
		cmd.indexBufferOffset = start * getIndexDataSize(indexDataType);

		cmd.indexCount = (int) indexCount;
		if (rangeCount > 0)
			cmd.indexCount = std::min(cmd.indexCount, rangeCount);

		cmd.indexCount = std::min(cmd.indexCount, (int) indexCount - start);

		if (cmd.indexCount > 0)
			gfx->draw(cmd);
	}
	else if (vertexCount > 0)
	{
		Graphics::DrawCommand cmd(&attributes, &buffers);

		cmd.primitiveType = primitiveType;
		cmd.vertexStart = std::min(std::max(0, rangeStart), (int) vertexCount - 1);

		cmd.vertexCount = (int) vertexCount;
		if (rangeCount > 0)
			cmd.vertexCount = std::min(cmd.vertexCount, rangeCount);

		cmd.vertexCount = std::min(cmd.vertexCount, (int) vertexCount - cmd.vertexStart);
		cmd.instanceCount = instancecount;
		cmd.texture = texture;
		cmd.cullMode = gfx->getMeshCullMode();

		if (cmd.vertexCount > 0)
			gfx->draw(cmd);
	}
}

} // graphics
} // love
