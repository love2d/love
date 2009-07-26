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

#ifndef LOVE_GRAPHICS_OPENGL_FRAME_H
#define LOVE_GRAPHICS_OPENGL_FRAME_H

// LOVE
#include <common/Object.h>
#include <common/math.h>
#include <graphics/Drawable.h>

namespace love
{
namespace graphics
{
namespace opengl
{
	class Frame : public Object
	{
		private:
			vertex vertices[4];
		public:

			/**
			* Creates a new Frame of size (w,h), using (x,y) as the top-left
			* anchor point in the source image. The size of the source image is
			* is specified by (sw,sh).
			* 
			* @param x Frame source position along the x-axis.
			* @param y Frame source position along the y-axis.
			* @param w Frame width.
			* @param h Frame width.
			* @param sw Width of the source image.
			* @param sh Height of the source image.
			**/
			Frame(int x, int y, int w, int h, int sw, int sh);

			virtual ~Frame();

			void flip(bool x, bool y);

			/**
			* Gets a pointer to the vertices.
			**/
			const vertex * getVertices() const;
	};
} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_FRAME_H
