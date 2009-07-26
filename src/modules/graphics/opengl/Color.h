/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_COLOR_H
#define LOVE_GRAPHICS_OPENGL_COLOR_H

// LOVE
#include <common/Object.h>

namespace love
{
namespace graphics
{
namespace opengl
{

	/**
	* @author Michael Enger
	**/
	class Color : public Object
	{
	protected:

		// Color components. (0-255)
		int red, green, blue, alpha;
		
	public:

		Color();

		/**
		 * Creates a new color with the specified component values.
		 * Values must be unsigned bytes. (0-255).
		 **/
		Color(int r, int g, int b, int a);

		virtual ~Color();

		/**
		 * Sets the amount of red in the color.
		 **/
		void setRed(int red);

		/**
		 * Sets the amount of green in the color.
		 **/
		void setGreen(int green);

		/**
		 * Sets the amount of blue in the color.
		 **/
		void setBlue(int blue);

		/**
		 * Sets the amount of alpha.
		 **/
		void setAlpha(int alpha);

		/**
		 * Returns the amount of red in the color.
		 **/
		int getRed() const;

		/**
		 * Returns the amount of green in the color.
		 **/
		int getGreen() const;

		/**
		 * Returns the amount of blue in the color.
		 **/
		int getBlue() const;

		/**
		 * Returns the amount of alpha.
		 **/
		int getAlpha() const;

	}; // Color

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_COLOR_H
