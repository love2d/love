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

#ifndef LOVE_GRAPHICS_DRAWABLE_H
#define LOVE_GRAPHICS_DRAWABLE_H

// LOVE
#include "common/Object.h"

namespace love
{
namespace graphics
{

/**
 * A Drawable is anything that can be drawn on screen with a
 * position, scale and orientation.
 **/
class Drawable : public Object
{
public:

	/**
	 * Destructor.
	 **/
	virtual ~Drawable() {}

	/**
	 * Draws the object with the specified transformation.
	 *
	 * @param x The position of the object along the x-axis.
	 * @param y The position of the object along the y-axis.
	 * @param angle The angle of the object (in radians).
	 * @param sx The scale factor along the x-axis.
	 * @param sy The scale factor along the y-axis.
	 * @param ox The origin offset along the x-axis.
	 * @param oy The origin offset along the y-axis.
	 * @param kx Shear along the x-axis.
	 * @param ky Shear along the y-axis.
	 **/
	virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) = 0;
};

} // graphics
} // love

#endif // LOVE_GRAPHICS_DRAWABLE_H
