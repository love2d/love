/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_MESH_H
#define LOVE_GRAPHICS_OPENGL_MESH_H

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "common/math.h"
#include "common/StringMap.h"
#include "graphics/Drawable.h"
#include "graphics/Texture.h"
#include "GLBuffer.h"

// C++
#include <vector>
#include <unordered_map>

namespace love
{
namespace graphics
{
namespace opengl
{

/**
 * Holds and draws arbitrary vertex geometry.
 * Each vertex in the Mesh has a collection of vertex attributes specified on
 * creation.
 **/
class Mesh : public Drawable
{
public:

	// The expected usage pattern of the Mesh's vertex data.
	enum Usage
	{
		USAGE_STREAM,
		USAGE_DYNAMIC,
		USAGE_STATIC,
		USAGE_MAX_ENUM
	};

	// How the Mesh's vertices are used when drawing.
	// http://escience.anu.edu.au/lecture/cg/surfaceModeling/image/surfaceModeling015.png
	enum DrawMode
	{
		DRAWMODE_FAN,
		DRAWMODE_STRIP,
		DRAWMODE_TRIANGLES,
		DRAWMODE_POINTS,
		DRAWMODE_MAX_ENUM
	};

	// The type of data a vertex attribute can store.
	enum DataType
	{
		DATA_BYTE,
		DATA_FLOAT,
		DATA_MAX_ENUM
	};

	struct AttribFormat
	{
		std::string name;
		DataType type;
		int components; // max 4
	};

	Mesh(const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, DrawMode drawmode, Usage usage);
	Mesh(const std::vector<AttribFormat> &vertexformat, int vertexcount, DrawMode drawmode, Usage usage);

	Mesh(const std::vector<Vertex> &vertices, DrawMode drawmode, Usage usage);
	Mesh(int vertexcount, DrawMode drawmode, Usage usage);

	virtual ~Mesh();

	/**
	 * Sets the values of all attributes at a specific vertex index in the Mesh.
	 * The size of the data must be less than or equal to the total size of all
	 * vertex attributes.
	 **/
	void setVertex(size_t vertindex, const void *data, size_t datasize);
	size_t getVertex(size_t vertindex, void *data, size_t datasize);
	void *getVertexScratchBuffer();

	/**
	 * Sets the values for a single attribute at a specific vertex index in the
	 * Mesh. The size of the data must be less than or equal to the size of the
	 * attribute.
	 **/
	void setVertexAttribute(size_t vertindex, int attribindex, const void *data, size_t datasize);
	size_t getVertexAttribute(size_t vertindex, int attribindex, void *data, size_t datasize);

	/**
	 * Gets the total number of vertices that can be used when drawing the Mesh.
	 **/
	size_t getVertexCount() const;

	/**
	 * Gets the size in bytes of the start of one vertex to the start of the
	 * next, in the buffer.
	 **/
	size_t getVertexStride() const;

	/**
	 * Gets the format of each vertex attribute stored in the Mesh.
	 **/
	const std::vector<AttribFormat> &getVertexFormat() const;
	DataType getAttributeInfo(int attribindex, int &components) const;
	int getAttributeIndex(const std::string &name) const;

	/**
	 * Sets whether a specific vertex attribute is used when drawing the Mesh.
	 **/
	void setAttributeEnabled(const std::string &name, bool enable);
	bool isAttributeEnabled(const std::string &name) const;

	/**
	 * Attaches a vertex attribute from another Mesh to this one. The attribute
	 * will be used when drawing this Mesh.
	 **/
	void attachAttribute(const std::string &name, Mesh *mesh);

	void *mapVertexData();
	void unmapVertexData(size_t modifiedoffset = 0, size_t modifiedsize = -1);

	/**
	 * Flushes all modified data to the GPU.
	 **/
	void flush();

	/**
	 * Sets the vertex map to use when drawing the Mesh. The vertex map
	 * determines the order in which vertices are used by the draw mode.
	 * A 0-element vector is equivalent to the default vertex map:
	 * {0, 1, 2, 3, 4, ...}
	 **/
	void setVertexMap(const std::vector<uint32> &map);
	void setVertexMap();

	/**
	 * Fills the uint32 vector passed into the method with the previously set
	 * vertex map (index buffer) values.
	 **/
	bool getVertexMap(std::vector<uint32> &map) const;

	/**
	 * Gets the total number of elements in the vertex map array.
	 **/
	size_t getVertexMapCount() const;

	/**
	 * Sets the texture used when drawing the Mesh.
	 **/
	void setTexture(Texture *texture);

	/**
	 * Disables any texture from being used when drawing the Mesh.
	 **/
	void setTexture();

	/**
	 * Gets the texture used when drawing the Mesh. May return null if no
	 * texture is set.
	 **/
	Texture *getTexture() const;

	/**
	 * Sets the draw mode used when drawing the Mesh.
	 **/
	void setDrawMode(DrawMode mode);
	DrawMode getDrawMode() const;

	void setDrawRange(int min, int max);
	void setDrawRange();
	void getDrawRange(int &min, int &max) const;

	int bindAttributeToShaderInput(int attributeindex, const std::string &inputname);

	// Implements Drawable.
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) override;

	static GLenum getGLBufferUsage(Usage usage);

	static bool getConstant(const char *in, Usage &out);
	static bool getConstant(Usage in, const char *&out);

	static bool getConstant(const char *in, DrawMode &out);
	static bool getConstant(DrawMode in, const char *&out);

	static bool getConstant(const char *in, DataType &out);
	static bool getConstant(DataType in, const char *&out);

private:

	struct AttachedAttribute
	{
		Mesh *mesh;
		int index;
		bool enabled;
	};

	void setupAttachedAttributes();
	void calculateAttributeSizes();
	size_t getAttributeOffset(size_t attribindex) const;

	static size_t getAttribFormatSize(const AttribFormat &format);

	static GLenum getGLDrawMode(DrawMode mode);
	static GLenum getGLDataType(DataType type);
	static GLenum getGLDataTypeFromMax(size_t maxvalue);
	static size_t getGLDataTypeSize(GLenum datatype);

	std::vector<AttribFormat> vertexFormat;
	std::vector<size_t> attributeSizes;

	std::unordered_map<std::string, AttachedAttribute> attachedAttributes;

	// Vertex buffer, for the vertex data.
	GLBuffer *vbo;
	size_t vertexCount;
	size_t vertexStride;

	// Block of memory whose size is at least as large as a single vertex. Helps
	// avoid memory allocations when using Mesh::setVertex etc.
	char *vertexScratchBuffer;

	// Element (vertex index) buffer, for the vertex map.
	GLBuffer *ibo;
	bool useIndexBuffer;
	size_t elementCount;
	GLenum elementDataType;

	DrawMode drawMode;

	int rangeMin;
	int rangeMax;

	StrongRef<Texture> texture;

	static StringMap<Usage, USAGE_MAX_ENUM>::Entry usageEntries[];
	static StringMap<Usage, USAGE_MAX_ENUM> usages;

	static StringMap<DrawMode, DRAWMODE_MAX_ENUM>::Entry drawModeEntries[];
	static StringMap<DrawMode, DRAWMODE_MAX_ENUM> drawModes;

	static StringMap<DataType, DATA_MAX_ENUM>::Entry dataTypeEntries[];
	static StringMap<DataType, DATA_MAX_ENUM> dataTypes;

}; // Mesh

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_MESH_H
