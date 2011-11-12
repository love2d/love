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

#ifndef LOVE_GRAPHICS_QUAD_H
#define LOVE_GRAPHICS_QUAD_H

// LOVE
#include <common/Object.h>
#include <common/math.h>

namespace love
{
namespace graphics
{
	class Quad : public Object
	{
	public:

		struct Viewport
		{
			float x, y, w, h;
		};

	public:
		Quad();

		virtual ~Quad();

		virtual void refresh(const Viewport & v, float sw, float sh) = 0;

		virtual void setViewport(const Viewport & v) = 0;
		virtual Viewport getViewport() const = 0;

		virtual void flip(bool x, bool y) = 0;

		/**
		 * Mirror texture coordinates around 0.5
		 */
		virtual void mirror(bool x, bool y) = 0;

		/**
		* Gets a pointer to the vertices.
		**/
		virtual const vertex * getVertices() const = 0;
	};
} // graphics
} // love

#endif // LOVE_GRAPHICS_QUAD_H
