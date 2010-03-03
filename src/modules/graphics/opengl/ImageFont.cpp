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

#include "ImageFont.h"

#include <common/math.h>
#include <SDL_opengl.h>

namespace love
{
namespace graphics
{
namespace opengl
{
	ImageFont::ImageFont(Image * image, const std::string& glyphs)
		: Font(0), image(image), glyphs(glyphs)

	{
		image->retain();
	}

	ImageFont::~ImageFont()
	{
		unload();
		image->release();
	}

	void ImageFont::print(std::string text, float x, float y) const
	{
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		GLuint OpenGLFont = list;
		glListBase(OpenGLFont);
		glCallLists((int)text.length(), GL_UNSIGNED_BYTE, text.c_str());
		glPopMatrix();
	}

	void ImageFont::print(std::string text, float x, float y, float angle, float sx, float sy) const
	{
		glPushMatrix();

		glTranslatef(x, y, 0.0f);
		glRotatef(LOVE_TORAD(angle), 0, 0, 1.0f);
		glScalef(sx, sy, 1.0f);

		GLuint OpenGLFont = list;
		glListBase(OpenGLFont);
		glCallLists((int)text.length(), GL_UNSIGNED_BYTE, text.c_str());

		glPopMatrix();
	}

	void ImageFont::print(char character, float x, float y) const
	{
		glPushMatrix();
		glTranslatef(x, y, 0.0f);
		GLuint OpenGLFont = list;
		glListBase(OpenGLFont);
		glCallList(list + (int)character);
		glPopMatrix();
	}

	bool ImageFont::load()
	{
		return loadVolatile();
	}

	void ImageFont::unload()
	{
		unloadVolatile();
	}

	bool ImageFont::loadVolatile()
	{
		love::image::pixel * pixels = (love::image::pixel *)(image->getData()->getData());

		unsigned imgw = (unsigned)image->getWidth();
		unsigned imgh = (unsigned)image->getHeight();
		unsigned imgs = imgw*imgh;

		// Reading texture data begins
		size = imgh;

		for(unsigned int i = 0; i < MAX_CHARS; i++)
			positions[i] = -1;

		love::image::pixel spacer = pixels[0];
		
		unsigned num = glyphs.size();
		
		unsigned start = 0;
		unsigned end = 0;
		unsigned space = 0;

		for(unsigned i = 0; i < num; ++i)
		{
			if(i >= MAX_CHARS)
				break;

			start = end;

			// Finds out where the first character starts
			while(start < imgw && equal(pixels[start], spacer))
				++start;

			if(i > 0)
				spacing[glyphs[i - 1]] = (start > end) ? (start - end) : 0;

			end = start;

			// Find where glyph ends.
			while(end < imgw && !equal(pixels[end], spacer))
				++end;

			if(start >= end)
				break;

			unsigned c = glyphs[i];

			positions[c] = start;
			widths[c] = (end - start);
		}

		// Replace spacer color with an empty pixel
		for(unsigned int i = 0; i < imgs; ++i)
		{
			if(equal(pixels[i], spacer))
			{
				pixels[i].r = 0;
				pixels[i].g = 0;
				pixels[i].b = 0;
				pixels[i].a = 0;
			}
		}

		// Create display lists
		list = glGenLists(MAX_CHARS);

		for(unsigned int i = 0; i < MAX_CHARS; i++)
		{
			glNewList(list + i, GL_COMPILE);

			if(positions[i] != -1)
			{
				Quad::Viewport v;
				v.x = positions[i];
				v.y = 0;
				v.w = widths[i];
				v.h = imgh;
				Quad q(v, imgw, imgh);

				image->drawq(&q, 0, 0, 0, 1, 1, 0, 0);

				glTranslatef((float)widths[i] + ((float)spacing[i] * mSpacing), 0, 0);
			}
			else
				glTranslatef((float)widths[(int)' '], 0, 0); // empty character are replaced with a whitespace

			glEndList();
		}

		return true;
	}

	void ImageFont::unloadVolatile()
	{
		glDeleteLists(list, MAX_CHARS);
	}

	bool ImageFont::equal(const love::image::pixel& a, const love::image::pixel& b)
	{
		return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
	}

	int ImageFont::next_p2(int num)
	{
		int powered = 2;
		while(powered < num) powered <<= 1;
		return powered;
	}

} // opengl
} // graphics
} // love
