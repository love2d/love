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

#ifndef LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H
#define LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H

// LOVE
#include <common/constants.h>
#include <common/math.h>
#include <common/Object.h>
#include <common/Vector.h>
#include <common/Matrix.h>
#include <graphics/Drawable.h>

// Module.
#include "Image.h"

// OpenGL
#include "GLee.h"
#include <SDL/SDL_opengl.h>

namespace love
{
namespace graphics
{
namespace opengl
{
	class VertexBuffer : public Drawable
	{
		friend class SpriteBatch;
	private:

		vertex * vertices;

		// Max number of vertices in the buffer.
		int size;

		// The next free element.
		int next;

		// The texture (optional).
		Image * image;

		// Contains the vbo_buffer.
		GLuint vbo_buf;

		// The uage hint for the vertex buffer.
		int usage;
		int gl_usage;

		// The type of primitives we're drawing.
		int type;
		int gl_type;

	private:

		bool useVBO() const;
		void update(int pos, int size);

	public:

		VertexBuffer(Image * image, int size, int type, int usage);
		virtual ~VertexBuffer();

		void setType(int type);
		int getType() const;

		void add(float x, float y, float s, float t, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
		void add(float x, float y, float s, float t);
		void clear();

		// Implements Drawable.
		void draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const;

	}; // VertexBuffer

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H
