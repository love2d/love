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

#ifndef LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H
#define LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H

// C
#include <cstring>

// LOVE
#include "common/math.h"
#include "common/Object.h"
#include "common/Matrix.h"
#include "common/StringMap.h"
#include "graphics/Drawable.h"
#include "graphics/Volatile.h"
#include "graphics/Color.h"
#include "graphics/Quad.h"

namespace love
{
namespace graphics
{
namespace opengl
{

// Forward declarations.
class Texture;
class VertexBuffer;
class VertexIndex;

class SpriteBatch : public Drawable
{
public:

	enum UsageHint
	{
		USAGE_DYNAMIC = 1,
		USAGE_STATIC,
		USAGE_STREAM,
		USAGE_MAX_ENUM
	};

	SpriteBatch(Texture *texture, int size, int usage);
	virtual ~SpriteBatch();

	int add(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index = -1);
	int addq(Quad *quad, float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index = -1);
	void clear();

	void *lock();
	void unlock();

	void setTexture(Texture *newtexture);
	Texture *getTexture();

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

	// Implements Drawable.
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	static bool getConstant(const char *in, UsageHint &out);
	static bool getConstant(UsageHint in, const char *&out);

private:

	void addv(const Vertex *v, int index);

	/**
	 * Set the color for vertices.
	 *
	 * @param v The vertices to set the color for. Must be an array of
	 *          of size 4.
	 * @param color The color to assign to each vertex.
	 */
	void setColorv(Vertex *v, const Color &color);

	Texture *texture;

	// Max number of sprites in the batch.
	int size;

	// The next free element.
	int next;

	// Current color. This color, if present, will be applied to the next
	// added sprite.
	Color *color;

	VertexBuffer *array_buf;
	VertexIndex *element_buf;

	static StringMap<UsageHint, USAGE_MAX_ENUM>::Entry usageHintEntries[];
	static StringMap<UsageHint, USAGE_MAX_ENUM> usageHints;

}; // SpriteBatch

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H
