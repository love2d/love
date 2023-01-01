/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#pragma once

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "common/math.h"
#include "common/StringMap.h"
#include "Drawable.h"
#include "Texture.h"
#include "vertex.h"
#include "Buffer.h"

// C++
#include <vector>
#include <unordered_map>

namespace love
{
namespace graphics
{

class Graphics;

/**
 * Holds and draws arbitrary vertex geometry.
 * Each vertex in the Mesh has a collection of vertex attributes specified on
 * creation.
 **/
class Mesh : public Drawable
{
public:

	static love::Type type;

	struct AttribFormat
	{
		std::string name;
		vertex::DataType type;
		int components; // max 4
	};

	Mesh(Graphics *gfx, const std::vector<AttribFormat> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, vertex::Usage usage);
	Mesh(Graphics *gfx, const std::vector<AttribFormat> &vertexformat, int vertexcount, PrimitiveType drawmode, vertex::Usage usage);

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
	vertex::DataType getAttributeInfo(int attribindex, int &components) const;
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
	void attachAttribute(const std::string &name, Mesh *mesh, const std::string &attachname, AttributeStep step = STEP_PER_VERTEX);
	bool detachAttribute(const std::string &name);

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
	void setVertexMap(IndexDataType datatype, const void *data, size_t datasize);
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
	void setDrawMode(PrimitiveType mode);
	PrimitiveType getDrawMode() const;

	void setDrawRange(int start, int count);
	void setDrawRange();
	bool getDrawRange(int &start, int &count) const;

	// Implements Drawable.
	void draw(Graphics *gfx, const Matrix4 &m) override;

	void drawInstanced(Graphics *gfx, const Matrix4 &m, int instancecount);

	static std::vector<AttribFormat> getDefaultVertexFormat();

private:

	friend class SpriteBatch;

	struct AttachedAttribute
	{
		Mesh *mesh;
		int index;
		AttributeStep step;
		bool enabled;
	};

	void setupAttachedAttributes();
	void calculateAttributeSizes();
	size_t getAttributeOffset(size_t attribindex) const;

	std::vector<AttribFormat> vertexFormat;
	std::vector<size_t> attributeSizes;

	std::unordered_map<std::string, AttachedAttribute> attachedAttributes;

	// Vertex buffer, for the vertex data.
	Buffer *vertexBuffer;
	size_t vertexCount;
	size_t vertexStride;

	// Block of memory whose size is at least as large as a single vertex. Helps
	// avoid memory allocations when using Mesh::setVertex etc.
	char *vertexScratchBuffer;

	// Index buffer, for the vertex map.
	Buffer *indexBuffer;
	bool useIndexBuffer;
	size_t indexCount;
	IndexDataType indexDataType;

	PrimitiveType primitiveType;

	int rangeStart;
	int rangeCount;

	StrongRef<Texture> texture;

}; // Mesh

} // graphics
} // love
