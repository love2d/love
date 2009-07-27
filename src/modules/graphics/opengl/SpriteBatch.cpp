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

#include "SpriteBatch.h"

// STD
#include <iostream>

// LOVE
#include "Image.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	SpriteBatch::SpriteBatch(Image * image, int size, int usage)
		: image(image), size(size), next(0), usage(usage)
	{
		image->retain();

		vertices = new vertex[size*4];
		indices = new GLushort[size*6];

		for(int i = 0; i<size; i++)
		{
			indices[i*6+0] = 0+(i*4);
			indices[i*6+1] = 1+(i*4);
			indices[i*6+2] = 2+(i*4);

			indices[i*6+3] = 0+(i*4);
			indices[i*6+4] = 2+(i*4);
			indices[i*6+5] = 3+(i*4);
		}

		// Find out which OpenGL VBO usage hint to use.
		gl_usage = GL_STREAM_DRAW;
		gl_usage = (usage == USAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : gl_usage;
		gl_usage = (usage == USAGE_STATIC) ? GL_STATIC_DRAW : gl_usage;
		gl_usage = (usage == USAGE_STREAM) ? GL_STREAM_DRAW : gl_usage;

		glGenBuffers(2, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*size*4, vertices, gl_usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*size*6, indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

	SpriteBatch::~SpriteBatch()
	{
		image->release();

		if(vbo[0] != 0 && vbo[1] != 0)
			glDeleteBuffers(2, vbo);

		delete [] vertices;
		delete [] indices;
	}

	void SpriteBatch::add(float x, float y, float a, float sx, float sy, float ox, float oy)
	{
		// Only do this if there's a free slot.
		if(next < size)
		{
			// Get a pointer to the correct insertion position.
			vertex * v = vertices + next*4;

			memcpy(v, image->getVertices(), sizeof(vertex)*4);

			// Transform.
			Matrix t;
			t.translate(x, y);
			t.scale(sx, sy);
			t.rotate(a);
			t.transform(v, v, 4);

			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
			glBufferSubData(GL_ARRAY_BUFFER, (next*4)*sizeof(vertex), sizeof(vertex)*4, v);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Increment counter.
			next++;			
		}
	}

	void SpriteBatch::clear()
	{
		// Reset the position of the next index.
		next = 0;
	}

	void SpriteBatch::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
	{
		glPushMatrix();
		glTranslatef(x, y, 0);

		image->bind();

		// Enable vertex arrays.
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		// Bind the VBO buffer.
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), (GLvoid*)0);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)(sizeof(unsigned char)*4));
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)(sizeof(unsigned char)*4+sizeof(float)*2));
		
		glDrawElements(GL_TRIANGLES, next*6, GL_UNSIGNED_SHORT, 0);

		// Disable vertex arrays.
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glPopMatrix();
	}

} // opengl
} // graphics
} // love
