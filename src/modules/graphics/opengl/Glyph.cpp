/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#include "Glyph.h"

// STD
#include <cstring> // For memcpy

namespace love
{
namespace graphics
{
namespace opengl
{

	Glyph::Glyph(love::font::GlyphData * data)
		: data(data), width((float)data->getWidth()), height((float)data->getHeight()), texture(0)
	{
		data->retain();

		memset(vertices, 255, sizeof(vertex)*4);

		vertices[0].x = 0; vertices[0].y = 0;
		vertices[1].x = 0; vertices[1].y = height;
		vertices[2].x = width; vertices[2].y = height;
		vertices[3].x = width; vertices[3].y = 0;

		vertices[0].s = 0; vertices[0].t = 0;
		vertices[1].s = 0; vertices[1].t = 1;
		vertices[2].s = 1; vertices[2].t = 1;
		vertices[3].s = 1; vertices[3].t = 0;

	}

	Glyph::~Glyph()
	{
		if(data != 0)
			data->release();
		unload();
	}

	void Glyph::draw(float x, float y, float angle, float sx, float sy, float ox, float oy) const
	{
		static Matrix t;
		
		t.setTransformation(x, y, angle, sx, sy, ox, oy);

		if(texture != 0)
			glBindTexture(GL_TEXTURE_2D,texture);

		glPushMatrix();

		glMultMatrixf((const GLfloat*)t.getElements());
		glTranslatef(data->getBearingX(), -data->getBearingY(), 0.0f);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].x);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)&vertices[0].s);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glPopMatrix();

	}
	
	bool Glyph::load()
	{
		return loadVolatile();
	}

	void Glyph::unload()
	{
		unloadVolatile();
	}

	bool Glyph::loadVolatile()
	{
		GLint format = GL_RGBA;
		if (data->getFormat() == love::font::GlyphData::FORMAT_LUMINANCE_ALPHA) format = GL_LUMINANCE_ALPHA;
		
		glGenTextures(1,&texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 
			0, 
			GL_RGBA, 
			(GLsizei)width, 
			(GLsizei)height, 
			0, 
			format, 
			GL_UNSIGNED_BYTE, 
			data->getData());

		return true;
	}

	void Glyph::unloadVolatile()
	{
		// Delete the hardware texture.
		if(texture != 0)
		{
			glDeleteTextures(1, (GLuint*)&texture);
			texture = 0;
		}
	}
	
	float Glyph::getWidth() const
	{
		return width;
	}
	
	float Glyph::getHeight() const
	{
		return height;
	}

} // opengl
} // graphics
} // love
