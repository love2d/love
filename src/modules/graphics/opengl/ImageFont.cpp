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

#include "ImageFont.h"

#include <SDL_opengl.h>

using std::string;

namespace love
{
namespace graphics
{
namespace opengl
{
	ImageFont::ImageFont(Image * image, std::string glyphs)
		: Font(0), glyphs(glyphs), image(image)

	{
		image->retain();
	}

	ImageFont::~ImageFont()
	{
		unload();
		image->release();
	}

	void ImageFont::print(string text, float x, float y) const
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
		glRotatef(angle * 57.29578f, 0, 0, 1.0f);
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

		// Reading texture data begins
		size = (int)image->getHeight();

		for(unsigned int i = 0; i < MAX_CHARS; i++) positions[i] = -1;

		love::image::pixel spacer = pixels[0];
		unsigned int current = 0;
		int width = 0;
		int space = 0;

		// Finds out where the first character starts
		int firstchar = 0;
		for(int i = 0; i != (int)image->getWidth(); i++)
		{
			if(spacer.r == pixels[i].r && spacer.g == pixels[i].g && spacer.b == pixels[i].b && spacer.a == pixels[i].a)
				continue;
			else
			{
				firstchar = i;
				break;
			}
		}

		for(int i = firstchar; i != (int)image->getWidth(); i++)
		{
			if(spacer.r == pixels[i].r && spacer.g == pixels[i].g && spacer.b == pixels[i].b && spacer.a == pixels[i].a)
			{
				if(width != 0) // this means we have found the end of our current character
				{
					if((unsigned int)glyphs[current] > MAX_CHARS)
						printf("Error reading texture font: Character '%c' is out of range.", glyphs[current]);
					else
					{
						widths[(int)glyphs[current]] = width - 1;
						positions[(int)glyphs[current]] = i - width;
					}

					width = 0;
					//space++; // start counting the spacing
				}
				space++;
			}
			else
			{
				if(space != 0) // this means we have found the end of our spacing
				{
					if((unsigned int)spacing[current] > MAX_CHARS)
						printf("Error reading image font: Character '%c' is out of range.", glyphs[current]);
					else
						spacing[(int)glyphs[current]] = space;

					current++;
					if(current == glyphs.size())
						i = (int)image->getWidth() - 1; // just to end it when the last character is found

					space = 0;
					//width++; // start counting the width
				}
				width++;
			}
		}
		// Reading image data ends

		// Replace spacer color with an empty pixel
		for(int i = 0; i < (int)(image->getWidth() * image->getHeight()); i++)
		{
			if(spacer.r == pixels[i].r && spacer.g == pixels[i].g && spacer.b == pixels[i].b && spacer.a == pixels[i].a)
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

				float x = (float)positions[i] + 1;
				float y = 1.0;
				float w = (float)widths[i];
				float h = (float)size+1;

				image->bind();

				float xTex = x/(float)image->getWidth();
				float yTex = y/(float)image->getHeight();

				float wTex = w/(float)image->getWidth();
				float hTex = h/(float)image->getHeight();

				glBegin(GL_QUADS);
					glTexCoord2f(xTex,yTex);				glVertex2f(0,0);
					glTexCoord2f(xTex,yTex+hTex);			glVertex2f(0,h);
					glTexCoord2f(xTex+wTex,yTex+hTex);		glVertex2f(w,h);
					glTexCoord2f(xTex+wTex,yTex);			glVertex2f(w,0);
				glEnd();

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

	inline int ImageFont::next_p2(int num)
	{
		int powered = 2;
		while(powered < num) powered <<= 1;
		return powered;
	}

} // opengl
} // graphics
} // love
