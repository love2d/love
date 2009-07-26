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

#include "VertexBuffer.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	VertexBuffer::VertexBuffer(Image * image, int size, int type, int usage)
		: size(size), next(0), image(image), vbo_buf(0), type(type), gl_type(0), usage(usage), gl_usage(0)
	{
		if(image != 0)
			image->retain();

		vertices = new vertex[size];

		// If VBOs aren't supported, then we must use vertex arrays.
		if(!GLEE_ARB_vertex_buffer_object)
			usage = USAGE_ARRAY;

		// Find out which OpenGL VBO usage hint to use.
		gl_usage = (usage == USAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : gl_usage;
		gl_usage = (usage == USAGE_STATIC) ? GL_STATIC_DRAW : gl_usage;
		gl_usage = (usage == USAGE_STREAM) ? GL_STREAM_DRAW : gl_usage;

		setType(type);

		if(useVBO())
		{
			glGenBuffers(1, &vbo_buf);
			if(vbo_buf != 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_buf);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*size, vertices, gl_usage);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			else // FAIL. Use vertex arrays instead.
				usage = USAGE_ARRAY;
		}
	}

	VertexBuffer::~VertexBuffer()
	{
		if(image != 0)
			image->release();

		delete [] vertices;

		if(useVBO() && vbo_buf != 0)
			glDeleteBuffers(1, &vbo_buf);
	}

	bool VertexBuffer::useVBO() const
	{
		return usage >= USAGE_DYNAMIC;
	}

	void VertexBuffer::update(int pos, int size)
	{
		// Update VBO.
		if(useVBO())
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_buf);
			glBufferSubData(GL_ARRAY_BUFFER, pos*sizeof(vertex), sizeof(vertex)*size, &vertices[pos]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		// ... no need to update vertex arrays.

		next += size;
	}

	void VertexBuffer::setType(int type)
	{
		this->type = type;
		// Find out which OpenGL primitive type to use.
		gl_type = (type == TYPE_POINTS) ? GL_POINTS : gl_type;
		gl_type = (type == TYPE_LINES) ? GL_LINES : gl_type;
		gl_type = (type == TYPE_LINE_STRIP) ? GL_LINE_STRIP : gl_type;
		gl_type = (type == TYPE_TRIANGLES) ? GL_TRIANGLES : gl_type;
		gl_type = (type == TYPE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : gl_type;
		gl_type = (type == TYPE_TRIANGLE_FAN) ? GL_TRIANGLE_FAN : gl_type;
	}

	int VertexBuffer::getType() const
	{
		return type;
	}

	void VertexBuffer::add(float x, float y, float s, float t, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		// Only do this if there's a free slot.
		if(next < size)
		{
			vertex & e = vertices[next];

			e.x = x;
			e.y = y;
			e.s = s;
			e.t = t;
			e.r = r;
			e.g = g;
			e.b = b;
			e.a = a;

			update(next, 1);		
		}
	}

	void VertexBuffer::add(float x, float y, float s, float t)
	{
		add(x, y, s, y, 255, 255, 255, 255);
	}

	void VertexBuffer::clear()
	{
		// Reset the position of the next index.
		next = 0;
	}

	void VertexBuffer::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
	{
		glPushMatrix();
		glTranslatef(x, y, 0);

		if(image == 0)
			glDisable(GL_TEXTURE_2D);
		else
			image->bind();

		// Enable vertex arrays.
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		if(useVBO())
		{
			// Bind the VBO buffer.
			glBindBuffer(GL_ARRAY_BUFFER, vbo_buf);
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), (GLvoid*)0);
			glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)(sizeof(unsigned char)*4));
			glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)(sizeof(unsigned char)*4+sizeof(float)*2));
			glDrawArrays(gl_type, 0, next);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else
		{
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), (GLvoid*)&vertices[0].r);
			glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].x);
			glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].s);
			glDrawArrays(gl_type, 0, next);
		}

		// Disable vertex arrays.
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		
		// Enable textures again.
		if(image == 0)
			glEnable(GL_TEXTURE_2D);

		glPopMatrix();
	}

} // opengl
} // graphics
} // love
