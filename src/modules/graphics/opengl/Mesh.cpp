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
#include "graphics/Graphics.h"

// C++
#include <algorithm>
#include <limits>

namespace love
{
namespace graphics
{
namespace opengl
{

static const char *getBuiltinAttribName(VertexAttribID attribid)
{
	const char *name = "";
	Shader::getConstant(attribid, name);
	return name;
}

static_assert(offsetof(Vertex, x) == sizeof(float) * 0, "Incorrect position offset in Vertex struct");
static_assert(offsetof(Vertex, s) == sizeof(float) * 2, "Incorrect texture coordinate offset in Vertex struct");
static_assert(offsetof(Vertex, color.r) == sizeof(float) * 4, "Incorrect color offset in Vertex struct");

static std::vector<Mesh::AttribFormat> getDefaultVertexFormat()
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

Mesh::Mesh(const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, DrawMode drawmode, vertex::Usage usage)
	: vertexFormat(vertexformat)
	, vbo(nullptr)
	, vertexCount(0)
	, vertexStride(0)
	, ibo(nullptr)
	, useIndexBuffer(false)
	, elementCount(0)
	, elementDataType(0)
	, drawMode(drawmode)
	, rangeStart(-1)
	, rangeCount(-1)
{
	setupAttachedAttributes();
	calculateAttributeSizes();

	vertexCount = datasize / vertexStride;
	elementDataType = getGLDataTypeFromMax(vertexCount);

	if (vertexCount == 0)
		throw love::Exception("Data size is too small for specified vertex attribute formats.");

	vbo = new GLBuffer(datasize, data, BUFFER_VERTEX, usage, GLBuffer::MAP_EXPLICIT_RANGE_MODIFY);

	vertexScratchBuffer = new char[vertexStride];
}

Mesh::Mesh(const std::vector<AttribFormat> &vertexformat, int vertexcount, DrawMode drawmode, vertex::Usage usage)
	: vertexFormat(vertexformat)
	, vbo(nullptr)
	, vertexCount((size_t) vertexcount)
	, vertexStride(0)
	, ibo(nullptr)
	, useIndexBuffer(false)
	, elementCount(0)
	, elementDataType(getGLDataTypeFromMax(vertexcount))
	, drawMode(drawmode)
	, rangeStart(-1)
	, rangeCount(-1)
{
	if (vertexcount <= 0)
		throw love::Exception("Invalid number of vertices (%d).", vertexcount);

	setupAttachedAttributes();
	calculateAttributeSizes();

	size_t buffersize = vertexCount * vertexStride;

	vbo = new GLBuffer(buffersize, nullptr, BUFFER_VERTEX, usage, GLBuffer::MAP_EXPLICIT_RANGE_MODIFY);

	// Initialize the buffer's contents to 0.
	memset(vbo->map(), 0, buffersize);
	vbo->setMappedRangeModified(0, vbo->getSize());
	vbo->unmap();

	vertexScratchBuffer = new char[vertexStride];
}

Mesh::Mesh(const std::vector<Vertex> &vertices, DrawMode drawmode, vertex::Usage usage)
	: Mesh(getDefaultVertexFormat(), &vertices[0], vertices.size() * sizeof(Vertex), drawmode, usage)
{
}

Mesh::Mesh(int vertexcount, DrawMode drawmode, vertex::Usage usage)
	: Mesh(getDefaultVertexFormat(), vertexcount, drawmode, usage)
{
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

		attachedAttributes[name] = {this, (int) i, true};
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

void Mesh::attachAttribute(const std::string &name, Mesh *mesh, const std::string &attachname)
{
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
static void copyToIndexBuffer(const std::vector<uint32> &indices, GLBuffer::Mapper &buffermap, size_t maxval)
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

	GLenum datatype = getGLDataTypeFromMax(maxval);

	// Calculate the size in bytes of the index buffer data.
	size_t size = map.size() * getGLDataTypeSize(datatype);

	if (ibo && size > ibo->getSize())
	{
		delete ibo;
		ibo = nullptr;
	}

	if (!ibo && size > 0)
		ibo = new GLBuffer(size, nullptr, BUFFER_INDEX, vbo->getUsage());

	useIndexBuffer = true;
	elementCount = map.size();

	if (!ibo || elementCount == 0)
		return;

	GLBuffer::Mapper ibomap(*ibo);

	// Fill the buffer with the index values from the vector.
	switch (datatype)
	{
	case GL_UNSIGNED_SHORT:
		copyToIndexBuffer<uint16>(map, ibomap, maxval);
		break;
	case GL_UNSIGNED_INT:
	default:
		copyToIndexBuffer<uint32>(map, ibomap, maxval);
		break;
	}

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
	case GL_UNSIGNED_SHORT:
		copyFromIndexBuffer<uint16>(buffer, elementCount, map);
		break;
	case GL_UNSIGNED_INT:
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

int Mesh::bindAttributeToShaderInput(int attributeindex, const std::string &inputname)
{
	const AttribFormat &format = vertexFormat[attributeindex];

	GLint attriblocation = -1;

	// If the attribute is one of the LOVE-defined ones, use the constant
	// attribute index for it, otherwise query the index from the shader.
	VertexAttribID builtinattrib;
	if (Shader::getConstant(inputname.c_str(), builtinattrib))
		attriblocation = (GLint) builtinattrib;
	else if (Shader::current)
		attriblocation = ((Shader *) Shader::current)->getAttribLocation(inputname);

	// The active shader might not use this vertex attribute name.
	if (attriblocation < 0)
		return attriblocation;

	// Needed for glVertexAttribPointer.
	vbo->bind();

	// Make sure the buffer isn't mapped (sends data to GPU if needed.)
	vbo->unmap();

	const void *gloffset = vbo->getPointer(getAttributeOffset(attributeindex));
	GLenum datatype = getGLDataType(format.type);
	GLboolean normalized = (datatype == GL_UNSIGNED_BYTE);

	glVertexAttribPointer(attriblocation, format.components, datatype, normalized, (GLsizei) vertexStride, gloffset);

	return attriblocation;
}

void Mesh::draw(Graphics *gfx, const Matrix4 &m)
{
	if (vertexCount <= 0)
		return;

	gfx->flushStreamDraws();

	OpenGL::TempDebugGroup debuggroup("Mesh draw");

	uint32 enabledattribs = 0;

	for (const auto &attrib : attachedAttributes)
	{
		if (!attrib.second.enabled)
			continue;

		Mesh *mesh = attrib.second.mesh;
		int location = mesh->bindAttributeToShaderInput(attrib.second.index, attrib.first);

		if (location >= 0)
			enabledattribs |= 1u << (uint32) location;
	}

	// Not supported on all platforms or GL versions, I believe.
	if (!(enabledattribs & ATTRIBFLAG_POS))
		throw love::Exception("Mesh must have an enabled VertexPosition attribute to be drawn.");

	gl.useVertexAttribArrays(enabledattribs);

	gl.bindTextureToUnit(texture, 0, false);

	Graphics::TempTransform transform(gfx, m);

	gl.prepareDraw();

	if (useIndexBuffer && ibo && elementCount > 0)
	{
		// Use the custom vertex map (index buffer) to draw the vertices.
		ibo->bind();

		// Make sure the index buffer isn't mapped (sends data to GPU if needed.)
		ibo->unmap();

		int start = std::min(std::max(0, rangeStart), (int) elementCount - 1);

		int count = (int) elementCount;
		if (rangeCount > 0)
			count = std::min(count, rangeCount);

		count = std::min(count, (int) elementCount - start);

		GLenum type = elementDataType;
		const void *indices = ibo->getPointer(start * getGLDataTypeSize(type));

		if (count > 0)
			gl.drawElements(getGLDrawMode(drawMode), count, type, indices);
	}
	else
	{
		int start = std::min(std::max(0, rangeStart), (int) vertexCount - 1);

		int count = (int) vertexCount;
		if (rangeCount > 0)
			count = std::min(count, rangeCount);

		count = std::min(count, (int) vertexCount - start);

		// Normal non-indexed drawing (no custom vertex map.)
		if (count > 0)
			gl.drawArrays(getGLDrawMode(drawMode), start, count);
	}
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

GLenum Mesh::getGLDrawMode(DrawMode mode)
{
	switch (mode)
	{
	case DRAWMODE_FAN:
		return GL_TRIANGLE_FAN;
	case DRAWMODE_STRIP:
		return GL_TRIANGLE_STRIP;
	case DRAWMODE_TRIANGLES:
	default:
		return GL_TRIANGLES;
	case DRAWMODE_POINTS:
		return GL_POINTS;
	}
}

GLenum Mesh::getGLDataType(DataType type)
{
	switch (type)
	{
	case DATA_BYTE:
		return GL_UNSIGNED_BYTE;
	case DATA_FLOAT:
		return GL_FLOAT;
	default:
		return 0;
	}
}

GLenum Mesh::getGLDataTypeFromMax(size_t maxvalue)
{
	if (maxvalue > LOVE_UINT16_MAX)
		return GL_UNSIGNED_INT;
	else
		return GL_UNSIGNED_SHORT;
}

size_t Mesh::getGLDataTypeSize(GLenum datatype)
{
	switch (datatype)
	{
	case GL_UNSIGNED_BYTE:
		return sizeof(uint8);
	case GL_UNSIGNED_SHORT:
		return sizeof(uint16);
	case GL_UNSIGNED_INT:
		return sizeof(uint32);
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

StringMap<Mesh::DrawMode, Mesh::DRAWMODE_MAX_ENUM>::Entry Mesh::drawModeEntries[] =
{
	{"fan", DRAWMODE_FAN},
	{"strip", DRAWMODE_STRIP},
	{"triangles", DRAWMODE_TRIANGLES},
	{"points", DRAWMODE_POINTS},
};

StringMap<Mesh::DrawMode, Mesh::DRAWMODE_MAX_ENUM> Mesh::drawModes(Mesh::drawModeEntries, sizeof(Mesh::drawModeEntries));

StringMap<Mesh::DataType, Mesh::DATA_MAX_ENUM>::Entry Mesh::dataTypeEntries[] =
{
	{"byte", DATA_BYTE},
	{"float", DATA_FLOAT},
};

StringMap<Mesh::DataType, Mesh::DATA_MAX_ENUM> Mesh::dataTypes(Mesh::dataTypeEntries, sizeof(Mesh::dataTypeEntries));

} // opengl
} // graphics
} // love
