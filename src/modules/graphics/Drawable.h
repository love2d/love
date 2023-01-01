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

#ifndef LOVE_GRAPHICS_DRAWABLE_H
#define LOVE_GRAPHICS_DRAWABLE_H

// LOVE
#include "common/Object.h"
#include "common/Matrix.h"

namespace love
{
namespace graphics
{

class Graphics;

/**
 * A Drawable is anything that can be drawn on screen with a
 * position, scale and orientation.
 **/
class Drawable : public Object
{
public:

	static love::Type type;

	/**
	 * Destructor.
	 **/
	virtual ~Drawable() {}

	/**
	 * Draws the object with the specified transformation matrix.
	 **/
	virtual void draw(Graphics *gfx, const Matrix4 &m) = 0;
};

} // graphics
} // love

#endif // LOVE_GRAPHICS_DRAWABLE_H
