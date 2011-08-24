/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_QUAD_H
#define LOVE_GRAPHICS_OPENGL_QUAD_H

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
	class Quad : public Object
	{
	public:

		struct Viewport
		{
			float x, y, w, h;
		};

	private:

		vertex vertices[4];

		Viewport viewport;
		float sw, sh;

	public:

		/**
		* Creates a new Quad of size (w,h), using (x,y) as the top-left
		* anchor point in the source image. The size of the source image is
		* is specified by (sw,sh).
		* 
		* @param sw Width of the source image.
		* @param sh Height of the source image.
		**/
		Quad(const Viewport & v, float sw, float sh);

		virtual ~Quad();

		void refresh(const Viewport & v, float sw, float sh);

		void setViewport(const Viewport & v);
		Viewport getViewport() const;

		void flip(bool x, bool y);

		/**
		* Gets a pointer to the vertices.
		**/
		const vertex * getVertices() const;
	};
} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_QUAD_H
