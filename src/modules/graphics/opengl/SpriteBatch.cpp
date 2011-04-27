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

#include "SpriteBatch.h"

// STD
#include <iostream>

// LOVE
#include "Image.h"
#include "Quad.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	SpriteBatch::SpriteBatch(Image * image, int size, int usage)
		: image(image), size(size), next(0), usage(usage), lockp(0)
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

		loadVolatile();
	}

	SpriteBatch::~SpriteBatch()
	{
		image->release();

		if(vbo[0] != 0 && vbo[1] != 0)
			glDeleteBuffers(2, vbo);

		delete [] vertices;
		delete [] indices;
	}

	bool SpriteBatch::loadVolatile()
	{
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
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*size*6, indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return true;
	}

	void SpriteBatch::unloadVolatile()
	{
		vertex * v = (vertex *)lock();

		// Copy to system memory.
		memcpy(vertices, v, sizeof(vertex)*size*4);

		unlock();

		// Delete the buffers.
		if(vbo[0] != 0 && vbo[1] != 0)
			glDeleteBuffers(2, vbo);
	}

	void SpriteBatch::add(float x, float y, float a, float sx, float sy, float ox, float oy)
	{
		// Only do this if there's a free slot.
		if(next < size)
		{
			// Get a pointer to the correct insertion position.
			vertex * v = vertices + next*4;

			// Needed for texture coordinates.
			memcpy(v, image->getVertices(), sizeof(vertex)*4);

			// Transform.
			Matrix t;
			t.setTransformation(x, y, a, sx, sy, ox, oy);
			t.transform(v, v, 4);

			addv(v);

			// Increment counter.
			next++;
		}
	}

	void SpriteBatch::addq(Quad * quad, float x, float y, float a, float sx, float sy, float ox, float oy)
	{
		// Only do this if there's a free slot.
		if(next < size)
		{
			// Get a pointer to the correct insertion position.
			vertex * v = vertices + next*4;

			// Needed for colors.
			memcpy(v, quad->getVertices(), sizeof(vertex)*4);

			// Transform.
			Matrix t;
			t.setTransformation(x, y, a, sx, sy, ox, oy);
			t.transform(v, v, 4);

			addv(v);

			// Increment counter.
			next++;
		}
	}

	void SpriteBatch::clear()
	{
		// Reset the position of the next index.
		next = 0;
	}

	void * SpriteBatch::lock()
	{
		// If already locked, prevent from locking again.
		if(lockp != 0)
			return lockp;

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		lockp = (vertex *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		return lockp;
	}

	void SpriteBatch::unlock()
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
		lockp = 0;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void SpriteBatch::setImage(Image * newimage)
	{
		image->release();
		image = newimage;
		image->retain();
	}

	void SpriteBatch::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
	{
		static Matrix t;

		glPushMatrix();

		t.setTransformation(x, y, angle, sx, sy, ox, oy);
		glMultMatrixf((const GLfloat*)t.getElements());

		image->bind();

		// Enable vertex arrays.
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// Bind the VBO buffer.
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)(sizeof(unsigned char)*4));
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)(sizeof(unsigned char)*4+sizeof(float)*2));
		
		glDrawElements(GL_TRIANGLES, next*6, GL_UNSIGNED_SHORT, 0);

		// Disable vertex arrays.
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glPopMatrix();
	}

	void SpriteBatch::addv(const vertex * v)
	{
		if(lockp != 0)
		{
			// Copy into mapped memory if buffer is locked.
			memcpy(lockp + (next*4), v,  sizeof(vertex)*4);
		}
		else
		{
			// ... use glBufferSubData otherwise.
			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
			glBufferSubData(GL_ARRAY_BUFFER, (next*4)*sizeof(vertex), sizeof(vertex)*4, v);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

} // opengl
} // graphics
} // love
