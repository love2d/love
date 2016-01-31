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

#ifndef LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H
#define LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H

// C
#include <cstring>

// C++
#include <unordered_map>

// LOVE
#include "common/math.h"
#include "common/Matrix.h"
#include "graphics/Drawable.h"
#include "graphics/Volatile.h"
#include "graphics/Color.h"
#include "graphics/Quad.h"
#include "GLBuffer.h"
#include "Mesh.h"

namespace love
{
namespace graphics
{

// Forward declarations.
class Texture;

namespace opengl
{

class SpriteBatch : public Drawable
{
public:

	SpriteBatch(Texture *texture, int size, Mesh::Usage usage);
	virtual ~SpriteBatch();

	int add(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index = -1);
	int addq(Quad *quad, float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index = -1);
	void clear();

	void flush();

	void setTexture(Texture *newtexture);
	Texture *getTexture() const;

	/**
	 * Set the current color for this SpriteBatch. The sprites added
	 * after this call will use this color. Note that global color
	 * will not longer apply to the SpriteBatch if this is used.
	 *
	 * @param color The color to use for the following sprites.
	 */
	void setColor(const Color &color);

	/**
	 * Disable per-sprite colors for this SpriteBatch. The next call to
	 * draw will use the global color for all sprites.
	 */
	void setColor();

	/**
	 * Get the current color for this SpriteBatch. Returns NULL if no color is
	 * set.
	 **/
	const Color *getColor() const;

	/**
	 * Get the number of sprites currently in this SpriteBatch.
	 **/
	int getCount() const;

	/**
	 * Sets the total number of sprites this SpriteBatch can hold.
	 * Leaves existing sprite data intact when possible.
	 **/
	void setBufferSize(int newsize);

	/**
	 * Get the total number of sprites this SpriteBatch can hold.
	 **/
	int getBufferSize() const;

	/**
	 * Attaches a specific vertex attribute from a Mesh to this SpriteBatch.
	 * The vertex attribute will be used when drawing the SpriteBatch.
	 **/
	void attachAttribute(const std::string &name, Mesh *mesh);

	// Implements Drawable.
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

private:

	struct AttachedAttribute
	{
		StrongRef<Mesh> mesh;
		int index;
	};

	void addv(const Vertex *v, const Matrix3 &m, int index);

	/**
	 * Set the color for vertices.
	 *
	 * @param v The vertices to set the color for. Must be an array of
	 *          of size 4.
	 * @param color The color to assign to each vertex.
	 */
	void setColorv(Vertex *v, const Color &color);

	StrongRef<Texture> texture;

	// Max number of sprites in the batch.
	int size;

	// The next free element.
	int next;

	// Current color. This color, if present, will be applied to the next
	// added sprite.
	Color *color;

	GLBuffer *array_buf;
	QuadIndices quad_indices;

	std::unordered_map<std::string, AttachedAttribute> attached_attributes;

}; // SpriteBatch

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H
