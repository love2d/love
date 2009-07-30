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

#ifndef LOVE_GRAPHICS_OPENGL_GLYPH_H
#define LOVE_GRAPHICS_OPENGL_GLYPH_H

// LOVE
#include <common/config.h>
#include <common/math.h>
#include <common/Matrix.h>
#include <font/GlyphData.h>
#include <graphics/Drawable.h>

// OpenGL
#include "GLee.h"
#include <SDL/SDL_opengl.h>

namespace love
{
namespace graphics
{
namespace opengl
{
	
	class Glyph : public Drawable
	{
	private:

		love::font::GlyphData * data;

		float width, height;

		GLuint texture;

		vertex vertices[4];

	public:


		Glyph(love::font::GlyphData * data);
		virtual ~Glyph();

		bool load();
		void unload();

		// Implements Volatile.
		bool loadVolatile();
		void unloadVolatile();		

		void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;

	}; // Glyph
	
} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GLYPH_H
