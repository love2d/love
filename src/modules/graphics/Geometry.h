/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_GEOMETRY_H
#define LOVE_GRAPHICS_GEOMETRY_H

// LOVE
#include "common/Object.h"
#include "common/math.h"

// std
#include <vector>

namespace love
{
namespace graphics
{

class Geometry : public Object
{
public:
	/**
	 * Creates a new geometry object from a std::vector<vertex>.
	 * @param v
	 **/
	Geometry(const std::vector<vertex> &polygon);

	/**
	 * Creates a new geometry from (texture) quad information.
	 * @param x Top left position in the image.
	 * @param y Top left position in the image.
	 * @param w Width of the quad.
	 * @param h Height of the quad.
	 * @param sw The reference width, the width of the Image.
	 * @param sh The reference height, the height of the Image.
	 */
	Geometry(float x, float y, float w, float h, float sw, float sh);

	Geometry(const Geometry &other);
	Geometry &operator=(const Geometry &other);
	virtual ~Geometry();

	/**
	 * Returns number of vertices in the *polygon* defining the geometry.
	 **/
	size_t getNumVertices() const
	{
		return polygon.size();
	}

	const vertex &getVertex(size_t i) const;
	void setVertex(size_t i, const vertex &v);

	void flip(bool x, bool y);

	/**
	 * Returns a pointer to the vertex array.
	 **/
	const vertex *getVertexArray() const
	{
		return vertexArray;
	}

	/**
	 * Returns the size of the vertex array.
	 **/
	size_t getVertexArraySize() const
	{
		return vertexCount;
	}

private:
	void triangulate();

	std::vector<vertex> polygon;

	vertex *vertexArray;
	size_t vertexCount;

	float x_min;
	float x_max;

	float y_min;
	float y_max;
};

} // graphics
} // love

#endif // LOVE_GRAPHICS_GEOMETRY_H
