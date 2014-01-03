/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
#include "common/int.h"
#include "common/math.h"
#include "common/StringMap.h"
#include "graphics/Drawable.h"
#include "Texture.h"
#include "VertexBuffer.h"

// C++
#include <vector>

namespace love
{
namespace graphics
{
namespace opengl
{

/**
 * Holds and draws arbitrary vertex geometry.
 * Each vertex in the Mesh has a position, texture coordinate, and color.
 **/
class Mesh : public Drawable
{
public:

	// How the Mesh's vertices are used when drawing.
	// http://escience.anu.edu.au/lecture/cg/surfaceModeling/image/surfaceModeling015.png
	enum DrawMode
	{
		DRAW_MODE_FAN,
		DRAW_MODE_STRIP,
		DRAW_MODE_TRIANGLES,
		DRAW_MODE_POINTS,
		DRAW_MODE_MAX_ENUM
	};

	/**
	 * Constructor.
	 * @param verts The vertices to use in the Mesh.
	 * @param mode The draw mode to use when drawing the Mesh.
	 **/
	Mesh(const std::vector<Vertex> &verts, DrawMode mode = DRAW_MODE_FAN);
	virtual ~Mesh();

	/**
	 * Replaces all the vertices in the Mesh with a new set of vertices.
	 **/
	void setVertices(const std::vector<Vertex> &verts);

	/**
	 * Gets all of the vertices in the Mesh as an array.
	 **/
	const Vertex *getVertices() const;

	/**
	 * Sets an individual vertex in the Mesh.
	 * @param index The index into the list of vertices to use.
	 * @param v The new vertex.
	 **/
	void setVertex(size_t index, const Vertex &v);
	Vertex getVertex(size_t index) const;

	/**
	 * Gets the total number of vertices in the Mesh.
	 **/
	size_t getVertexCount() const;

	/**
	 * Sets the vertex map to use when drawing the Mesh. The vertex map
	 * determines the order in which vertices are used by the draw mode.
	 * A 0-element vector is equivalent to the default vertex map:
	 * {0, 1, 2, 3, 4, ...}
	 **/
	void setVertexMap(const std::vector<uint32> &map);

	/**
	 * Gets a pointer to the vertex map array. The pointer is only valid until
	 * the next function call in the graphics module.
	 * May return null if the vertex map is empty.
	 **/
	const uint32 *getVertexMap() const;

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

	/**
	 * Sets whether per-vertex colors are enabled. If this is disabled, the
	 * global color (love.graphics.setColor) will be used for the entire Mesh.
	 **/
	void setVertexColors(bool enable);
	bool hasVertexColors() const;

	/**
	 * Sets whether the Mesh will be drawn as wireframe lines instead of filled
	 * triangles (has no effect for DRAW_MODE_POINTS.)
	 * This should only be used as a debugging tool. The wireframe lines do not
	 * behave the same as regular love.graphics lines.
	 **/
	void setWireframe(bool enable);
	bool isWireframe() const;

	// Implements Drawable.
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	static bool getConstant(const char *in, DrawMode &out);
	static bool getConstant(DrawMode in, const char *&out);

private:

	GLenum getGLDrawMode(DrawMode mode) const;

	// Vertex buffer.
	VertexBuffer *vbo;
	size_t vertex_count;

	// Element (vertex index) buffer, for the vertex map.
	VertexBuffer *ibo;
	size_t element_count;

	DrawMode draw_mode;

	Texture *texture;

	// Whether the per-vertex colors are used when drawing.
	bool colors_enabled;

	bool wireframe;

	static StringMap<DrawMode, DRAW_MODE_MAX_ENUM>::Entry drawModeEntries[];
	static StringMap<DrawMode, DRAW_MODE_MAX_ENUM> drawModes;

}; // Mesh

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_MESH_H
