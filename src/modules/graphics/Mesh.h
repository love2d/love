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

#pragma once

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "common/math.h"
#include "common/StringMap.h"
#include "common/Range.h"
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

	struct BufferAttribute
	{
		std::string name;
		StrongRef<Buffer> buffer;
		StrongRef<Mesh> mesh;
		std::string nameInBuffer;
		int bindingLocationInBuffer = -1;
		int indexInBuffer = 0;
		int startArrayIndex = 0;
		AttributeStep step = STEP_PER_VERTEX;
		int bindingLocation = -1;
		bool enabled = false;
	};

	static love::Type type;

	Mesh(Graphics *gfx, const std::vector<Buffer::DataDeclaration> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, BufferDataUsage usage);
	Mesh(Graphics *gfx, const std::vector<Buffer::DataDeclaration> &vertexformat, int vertexcount, PrimitiveType drawmode, BufferDataUsage usage);
	Mesh(const std::vector<BufferAttribute> &attributes, PrimitiveType drawmode);

	virtual ~Mesh();

	/**
	 * Validates a vertex index and whether the Mesh has its own vertex buffer,
	 * and returns a pointer to the vertex data at the given vertex index.
	 **/
	void *checkVertexDataOffset(size_t vertindex, size_t *byteoffset);

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
	 * Gets the Buffer that holds the Mesh's vertices.
	 **/
	Buffer *getVertexBuffer() const;

	/**
	 * Gets the format of each vertex attribute stored in the Mesh.
	 **/
	const std::vector<Buffer::DataMember> &getVertexFormat() const;

	/**
	 * Sets whether a specific vertex attribute is used when drawing the Mesh.
	 **/
	void setAttributeEnabled(const std::string &name, bool enable);
	bool isAttributeEnabled(const std::string &name) const;
	void setAttributeEnabled(int bindingLocation, bool enable);
	bool isAttributeEnabled(int bindingLocation) const;

	/**
	 * Attaches a vertex attribute from another vertex buffer to this Mesh. The
	 * attribute will be used when drawing this Mesh.
	 * Attributes from other Meshes should also pass in the Mesh as an argument,
	 * to make sure this Mesh knows to flush the passed in Mesh's data to its
	 * buffer when drawing.
	 **/
	void attachAttribute(const std::string &name, Buffer *buffer, Mesh *mesh, const std::string &attachname, int startindex = 0, AttributeStep step = STEP_PER_VERTEX);
	bool detachAttribute(const std::string &name);
	void attachAttribute(int bindingLocation, Buffer *buffer, Mesh *mesh, int attachBindingLocation, int startindex = 0, AttributeStep step = STEP_PER_VERTEX);
	bool detachAttribute(int bindingLocation);
	const std::vector<BufferAttribute> &getAttachedAttributes() const;

	void *getVertexData() const;
	void setVertexDataModified(size_t offset, size_t size);

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

	void setIndexBuffer(Buffer *buffer);
	Buffer *getIndexBuffer() const;

	/**
	 * Gets the total number of elements in the vertex map array.
	 **/
	size_t getIndexCount() const;

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
	void drawIndirect(Graphics *gfx, const Matrix4 &m, Buffer *indirectargs, int argsindex);

	static std::vector<Buffer::DataDeclaration> getDefaultVertexFormat();

private:

	friend class SpriteBatch;

	void setupAttachedAttributes();
	int getAttachedAttributeIndex(const std::string &name) const;
	int getAttachedAttributeIndex(int bindingLocation) const;
	void finalizeAttribute(BufferAttribute &attrib) const;

	void updateVertexAttributes(Graphics *gfx);

	void drawInternal(Graphics *gfx, const Matrix4 &m, int instancecount, Buffer *indirectargs, int argsindex);

	std::vector<Buffer::DataMember> vertexFormat;

	std::vector<BufferAttribute> attachedAttributes;

	VertexAttributesID attributesID;

	// Vertex buffer, for the vertex data.
	StrongRef<Buffer> vertexBuffer;
	uint8 *vertexData = nullptr;
	Range modifiedVertexData = Range();

	size_t vertexCount = 0;
	size_t vertexStride = 0;

	// Index buffer, for the vertex map.
	StrongRef<Buffer> indexBuffer;
	uint8 *indexData = nullptr;
	bool indexDataModified = false;
	bool useIndexBuffer = false;
	size_t indexCount = 0;
	IndexDataType indexDataType = INDEX_UINT16;

	PrimitiveType primitiveType = PRIMITIVE_TRIANGLES;

	Range drawRange = Range();

	StrongRef<Texture> texture;

	BufferBindings bufferBindings;

}; // Mesh

} // graphics
} // love
