/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

static const char *getBuiltinAttribName(VertexAttribID attribid)
{
	const char *name = "";
	vertex::getConstant(attribid, name);
	return name;
}

static_assert(offsetof(Vertex, x) == sizeof(float) * 0, "Incorrect position offset in Vertex struct");
static_assert(offsetof(Vertex, s) == sizeof(float) * 2, "Incorrect texture coordinate offset in Vertex struct");
static_assert(offsetof(Vertex, color.r) == sizeof(float) * 4, "Incorrect color offset in Vertex struct");

std::vector<Mesh::AttribFormat> Mesh::getDefaultVertexFormat()
{
	// Corresponds to the love::Vertex struct.
	std::vector<Mesh::AttribFormat> vertexformat = {
		{getBuiltinAttribName(ATTRIB_POS),      Mesh::DATA_FLOAT, 2},
		{getBuiltinAttribName(ATTRIB_TEXCOORD), Mesh::DATA_FLOAT, 2},
		{getBuiltinAttribName(ATTRIB_COLOR),    Mesh::DATA_BYTE,  4},
	};

	return vertexformat;
}

love::Type Mesh::type("Mesh", &Drawable::type);

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, DrawMode drawmode, vertex::Usage usage)
	: vertexFormat(vertexformat)
	, vbo(nullptr)
	, vertexCount(0)
	, vertexStride(0)
	, ibo(nullptr)
	, useIndexBuffer(false)
	, elementCount(0)
	, elementDataType(INDEX_UINT16)
	, drawMode(drawmode)
	, rangeStart(-1)
	, rangeCount(-1)
{
	setupAttachedAttributes();
	calculateAttributeSizes();

	vertexCount = datasize / vertexStride;
	elementDataType = vertex::getIndexDataTypeFromMax(vertexCount);

	if (vertexCount == 0)
		throw love::Exception("Data size is too small for specified vertex attribute formats.");

	vbo = gfx->newBuffer(datasize, data, BUFFER_VERTEX, usage, Buffer::MAP_EXPLICIT_RANGE_MODIFY | Buffer::MAP_READ);

	vertexScratchBuffer = new char[vertexStride];
}

Mesh::Mesh(graphics::Graphics *gfx, const std::vector<AttribFormat> &vertexformat, int vertexcount, DrawMode drawmode, vertex::Usage usage)
	: vertexFormat(vertexformat)
	, vbo(nullptr)
	, vertexCount((size_t) vertexcount)
	, vertexStride(0)
	, ibo(nullptr)
	, useIndexBuffer(false)
	, elementCount(0)
	, elementDataType(vertex::getIndexDataTypeFromMax(vertexcount))
	, drawMode(drawmode)
	, rangeStart(-1)
	, rangeCount(-1)
{
	if (vertexcount <= 0)
		throw love::Exception("Invalid number of vertices (%d).", vertexcount);

	setupAttachedAttributes();
	calculateAttributeSizes();

	size_t buffersize = vertexCount * vertexStride;

	vbo = gfx->newBuffer(buffersize, nullptr, BUFFER_VERTEX, usage, Buffer::MAP_EXPLICIT_RANGE_MODIFY | Buffer::MAP_READ);

	// Initialize the buffer's contents to 0.
	memset(vbo->map(), 0, buffersize);
	vbo->setMappedRangeModified(0, vbo->getSize());
	vbo->unmap();

	vertexScratchBuffer = new char[vertexStride];
}

Mesh::~Mesh()
{
	delete vbo;
	delete ibo;
	delete vertexScratchBuffer;

	for (const auto &attrib : attachedAttributes)
	{
		if (attrib.second.mesh != this)
			attrib.second.mesh->release();
	}
}

void Mesh::setupAttachedAttributes()
{
	for (size_t i = 0; i < vertexFormat.size(); i++)
	{
		const std::string &name = vertexFormat[i].name;

		if (attachedAttributes.find(name) != attachedAttributes.end())
			throw love::Exception("Duplicate vertex attribute name: %s", name.c_str());

		attachedAttributes[name] = {this, (int) i, STEP_PER_VERTEX, true};
	}
}

void Mesh::calculateAttributeSizes()
{
	size_t stride = 0;

	for (const AttribFormat &format : vertexFormat)
	{
		// Hardware really doesn't like attributes that aren't 32 bit-aligned.
		if (format.type == DATA_BYTE && format.components != 4)
			throw love::Exception("byte vertex attributes must have 4 components.");

		if (format.components <= 0 || format.components > 4)
			throw love::Exception("Vertex attributes must have between 1 and 4 components.");

		// Total size in bytes of each attribute in a single vertex.
		attributeSizes.push_back(getAttribFormatSize(format));
		stride += attributeSizes.back();
	}

	vertexStride = stride;
}

size_t Mesh::getAttributeOffset(size_t attribindex) const
{
	size_t offset = 0;

	for (size_t i = 0; i < attribindex; i++)
		offset += attributeSizes[i];

	return offset;
}

void Mesh::setVertex(size_t vertindex, const void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	size_t offset = vertindex * vertexStride;
	size_t size = std::min(datasize, vertexStride);

	uint8 *bufferdata = (uint8 *) vbo->map();
	memcpy(bufferdata + offset, data, size);

	vbo->setMappedRangeModified(offset, size);
}

size_t Mesh::getVertex(size_t vertindex, void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	size_t offset = vertindex * vertexStride;
	size_t size = std::min(datasize, vertexStride);

	// We're relying on vbo->map() returning read/write data... ew.
	const uint8 *bufferdata = (const uint8 *) vbo->map();
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

	size_t offset = vertindex * vertexStride + getAttributeOffset(attribindex);
	size_t size = std::min(datasize, attributeSizes[attribindex]);

	uint8 *bufferdata = (uint8 *) vbo->map();
	memcpy(bufferdata + offset, data, size);

	vbo->setMappedRangeModified(offset, size);
}

size_t Mesh::getVertexAttribute(size_t vertindex, int attribindex, void *data, size_t datasize)
{
	if (vertindex >= vertexCount)
		throw love::Exception("Invalid vertex index: %ld", vertindex + 1);

	if (attribindex >= (int) vertexFormat.size())
		throw love::Exception("Invalid vertex attribute index: %d", attribindex + 1);

	size_t offset = vertindex * vertexStride + getAttributeOffset(attribindex);
	size_t size = std::min(datasize, attributeSizes[attribindex]);

	// We're relying on vbo->map() returning read/write data... ew.
	const uint8 *bufferdata = (const uint8 *) vbo->map();
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

const std::vector<Mesh::AttribFormat> &Mesh::getVertexFormat() const
{
	return vertexFormat;
}

Mesh::DataType Mesh::getAttributeInfo(int attribindex, int &components) const
{
	if (attribindex < 0 || attribindex >= (int) vertexFormat.size())
		throw love::Exception("Invalid vertex attribute index: %d", attribindex + 1);

	DataType type = vertexFormat[attribindex].type;
	components = vertexFormat[attribindex].components;

	return type;
}

int Mesh::getAttributeIndex(const std::string &name) const
{
	for (int i = 0; i < (int) vertexFormat.size(); i++)
	{
		if (vertexFormat[i].name == name)
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

void Mesh::attachAttribute(const std::string &name, Mesh *mesh, const std::string &attachname, AttributeStep step)
{
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (step == STEP_PER_INSTANCE && !gfx->getCapabilities().features[Graphics::FEATURE_INSTANCING])
		throw love::Exception("Vertex attribute instancing is not supported on this system.");

	if (mesh != this)
	{
		for (const auto &it : mesh->attachedAttributes)
		{
			// If the supplied Mesh has attached attributes of its own, then we
			// prevent it from being attached to avoid reference cycles.
			if (it.second.mesh != mesh)
				throw love::Exception("Cannot attach a Mesh which has attached Meshes of its own.");
		}
	}

	AttachedAttribute oldattrib = {};
	AttachedAttribute newattrib = {};

	auto it = attachedAttributes.find(name);
	if (it != attachedAttributes.end())
		oldattrib = it->second;

	newattrib.mesh = mesh;
	newattrib.enabled = oldattrib.mesh ? oldattrib.enabled : true;
	newattrib.index = mesh->getAttributeIndex(attachname);
	newattrib.step = step;

	if (newattrib.index < 0)
		throw love::Exception("The specified mesh does not have a vertex attribute named '%s'", attachname.c_str());

	if (newattrib.mesh != this)
		newattrib.mesh->retain();

	attachedAttributes[name] = newattrib;

	if (oldattrib.mesh && oldattrib.mesh != this)
		oldattrib.mesh->release();
}

bool Mesh::detachAttribute(const std::string &name)
{
	auto it = attachedAttributes.find(name);

	if (it != attachedAttributes.end() && it->second.mesh != this)
	{
		it->second.mesh->release();
		attachedAttributes.erase(it);

		if (getAttributeIndex(name) != -1)
			attachAttribute(name, this, name);

		return true;
	}

	return false;
}

void *Mesh::mapVertexData()
{
	return vbo->map();
}

void Mesh::unmapVertexData(size_t modifiedoffset, size_t modifiedsize)
{
	vbo->setMappedRangeModified(modifiedoffset, modifiedsize);
	vbo->unmap();
}

void Mesh::flush()
{
	vbo->unmap();

	if (ibo != nullptr)
		ibo->unmap();
}

/**
 * Copies index data from a vector to a mapped index buffer.
 **/
template <typename T>
static void copyToIndexBuffer(const std::vector<uint32> &indices, Buffer::Mapper &buffermap, size_t maxval)
{
	T *elems = (T *) buffermap.get();

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

	IndexDataType datatype = vertex::getIndexDataTypeFromMax(maxval);

	// Calculate the size in bytes of the index buffer data.
	size_t size = map.size() * vertex::getIndexDataSize(datatype);

	if (ibo && size > ibo->getSize())
	{
		delete ibo;
		ibo = nullptr;
	}

	if (!ibo && size > 0)
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		ibo = gfx->newBuffer(size, nullptr, BUFFER_INDEX, vbo->getUsage(), Buffer::MAP_READ);
	}

	useIndexBuffer = true;
	elementCount = map.size();

	if (!ibo || elementCount == 0)
		return;

	Buffer::Mapper ibomap(*ibo);

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

	elementDataType = datatype;
}

void Mesh::setVertexMap(IndexDataType datatype, const void *data, size_t datasize)
{
	if (ibo && datasize > ibo->getSize())
	{
		delete ibo;
		ibo = nullptr;
	}

	if (!ibo && datasize > 0)
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		ibo = gfx->newBuffer(datasize, nullptr, BUFFER_INDEX, vbo->getUsage(), Buffer::MAP_READ);
	}

	elementCount = datasize / vertex::getIndexDataSize(datatype);

	if (!ibo || elementCount == 0)
		return;

	Buffer::Mapper ibomap(*ibo);
	memcpy(ibomap.get(), data, datasize);

	useIndexBuffer = true;
	elementDataType = datatype;
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
	map.reserve(elementCount);

	if (!ibo || elementCount == 0)
		return true;

	// We unmap the buffer in Mesh::draw, Mesh::setVertexMap, and Mesh::flush.
	void *buffer = ibo->map();

	// Fill the vector from the buffer.
	switch (elementDataType)
	{
	case INDEX_UINT16:
		copyFromIndexBuffer<uint16>(buffer, elementCount, map);
		break;
	case INDEX_UINT32:
	default:
		copyFromIndexBuffer<uint32>(buffer, elementCount, map);
		break;
	}

	return true;
}

size_t Mesh::getVertexMapCount() const
{
	return elementCount;
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

void Mesh::setDrawMode(DrawMode mode)
{
	drawMode = mode;
}

Mesh::DrawMode Mesh::getDrawMode() const
{
	return drawMode;
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

	uint32 enabledattribs = 0;
	uint32 instancedattribs = 0;

	for (const auto &attrib : attachedAttributes)
	{
		if (!attrib.second.enabled)
			continue;

		love::graphics::Mesh *mesh = attrib.second.mesh;
		int location = mesh->bindAttributeToShaderInput(attrib.second.index, attrib.first);

		if (location >= 0)
		{
			uint32 bit = 1u << (uint32) location;

			enabledattribs |= bit;

			if (attrib.second.step == STEP_PER_INSTANCE)
				instancedattribs |= bit;
		}
	}

	// Not supported on all platforms or GL versions, I believe.
	if (!(enabledattribs & ATTRIBFLAG_POS))
		throw love::Exception("Mesh must have an enabled VertexPosition attribute to be drawn.");

	bool useindexbuffer = useIndexBuffer && ibo != nullptr && elementCount > 0;

	int start = 0;
	int count = 0;

	if (useindexbuffer)
	{
		// Make sure the index buffer isn't mapped (sends data to GPU if needed.)
		ibo->unmap();

		start = std::min(std::max(0, rangeStart), (int) elementCount - 1);

		count = (int) elementCount;
		if (rangeCount > 0)
			count = std::min(count, rangeCount);

		count = std::min(count, (int) elementCount - start);
	}
	else
	{
		start = std::min(std::max(0, rangeStart), (int) vertexCount - 1);

		count = (int) vertexCount;
		if (rangeCount > 0)
			count = std::min(count, rangeCount);

		count = std::min(count, (int) vertexCount - start);
	}

	Graphics::TempTransform transform(gfx, m);

	if (count > 0)
		drawInternal(start, count, instancecount, useindexbuffer, enabledattribs, instancedattribs);
}

size_t Mesh::getAttribFormatSize(const AttribFormat &format)
{
	switch (format.type)
	{
	case DATA_BYTE:
		return format.components * sizeof(uint8);
	case DATA_FLOAT:
		return format.components * sizeof(float);
	default:
		return 0;
	}
}

bool Mesh::getConstant(const char *in, Mesh::DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Mesh::getConstant(Mesh::DrawMode in, const char *&out)
{
	return drawModes.find(in, out);
}

bool Mesh::getConstant(const char *in, DataType &out)
{
	return dataTypes.find(in, out);
}

bool Mesh::getConstant(DataType in, const char *&out)
{
	return dataTypes.find(in, out);
}

bool Mesh::getConstant(const char *in, AttributeStep &out)
{
	return attributeSteps.find(in, out);
}

bool Mesh::getConstant(AttributeStep in, const char *&out)
{
	return attributeSteps.find(in, out);
}

StringMap<Mesh::DrawMode, Mesh::DRAWMODE_MAX_ENUM>::Entry Mesh::drawModeEntries[] =
{
	{ "fan",       DRAWMODE_FAN       },
	{ "strip",     DRAWMODE_STRIP     },
	{ "triangles", DRAWMODE_TRIANGLES },
	{ "points",    DRAWMODE_POINTS    },
};

StringMap<Mesh::DrawMode, Mesh::DRAWMODE_MAX_ENUM> Mesh::drawModes(Mesh::drawModeEntries, sizeof(Mesh::drawModeEntries));

StringMap<Mesh::DataType, Mesh::DATA_MAX_ENUM>::Entry Mesh::dataTypeEntries[] =
{
	{ "byte", DATA_BYTE   },
	{ "float", DATA_FLOAT },
};

StringMap<Mesh::DataType, Mesh::DATA_MAX_ENUM> Mesh::dataTypes(Mesh::dataTypeEntries, sizeof(Mesh::dataTypeEntries));

StringMap<Mesh::AttributeStep, Mesh::STEP_MAX_ENUM>::Entry Mesh::attributeStepEntries[] =
{
	{ "pervertex",   STEP_PER_VERTEX   },
	{ "perinstance", STEP_PER_INSTANCE },
};

StringMap<Mesh::AttributeStep, Mesh::STEP_MAX_ENUM> Mesh::attributeSteps(Mesh::attributeStepEntries, sizeof(Mesh::attributeStepEntries));

} // graphics
} // love
