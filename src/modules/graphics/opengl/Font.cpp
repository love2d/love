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

#include "Font.h"
#include <font/GlyphData.h>

#include <common/math.h>
#include <math.h>

namespace love
{
namespace graphics
{
namespace opengl
{

	Font::Font(love::font::FontData * data, const Image::Filter& filter)
	: height(data->getHeight()), lineHeight(1), mSpacing(1)
	{
		glyphs = new Glyph*[MAX_CHARS];
		type = FONT_UNKNOWN;
		love::font::GlyphData * gd;

		for(unsigned int i = 0; i < MAX_CHARS; i++)
		{
			gd = data->getGlyphData(i);
			glyphs[i] = new Glyph(gd, filter);
			glyphs[i]->load();
			widths[i] = gd->getWidth();
			spacing[i] = gd->getAdvance();
			bearingX[i] = gd->getBearingX();
			bearingY[i] = gd->getBearingY();
			if (type == FONT_UNKNOWN) type = (gd->getFormat() == love::font::GlyphData::FORMAT_LUMINANCE_ALPHA ? FONT_TRUETYPE : FONT_IMAGE);
		}
	}

	Font::~Font()
	{
		for(unsigned int i = 0; i < MAX_CHARS; i++)
		{
			glyphs[i]->release();
		}
		delete[] glyphs;
	}

	float Font::getHeight() const
	{
		return static_cast<float>(height);
	}

	void Font::print(std::string text, float x, float y, float angle, float sx, float sy) const
	{
		float dx = 0.0f; // spacing counter for newline handling
		glPushMatrix();

		glTranslatef(ceil(x), ceil(y), 0.0f);
		glRotatef(LOVE_TODEG(angle), 0, 0, 1.0f);
		glScalef(sx, sy, 1.0f);
		for (unsigned int i = 0; i < text.size(); i++) {
			unsigned char g = (unsigned char)text[i];
			if (g == '\n') { // wrap newline, but do not print it
				glTranslatef(-dx, floor(getHeight() * getLineHeight() + 0.5f), 0);
				dx = 0.0f;
				continue;
			}
			if (!glyphs[g]) g = 32; // space
			glPushMatrix();
			// 1.25 is magic line height for true type fonts
			if (type == FONT_TRUETYPE) glTranslatef(0, floor(getHeight() / 1.25f + 0.5f), 0);
			glyphs[g]->draw(0, 0, 0, 1, 1, 0, 0);
			glPopMatrix();
			glTranslatef(static_cast<GLfloat>(spacing[g]), 0, 0);
			dx += spacing[g];
		}
		glPopMatrix();
	}

	void Font::print(char character, float x, float y) const
	{
		if (!glyphs[(int)character]) character = ' ';
		glPushMatrix();
		glTranslatef(x, floor(y+getHeight() + 0.5f), 0.0f);
		glCallList(list+character);
		glPopMatrix();
	}

	int Font::getWidth(const std::string & line) const
	{
		if(line.size() == 0) return 0;
		int temp = 0;

		for(unsigned int i = 0; i < line.size(); i++)
		{
			temp += static_cast<int>((spacing[(int)line[i]] * mSpacing));
		}

		return temp;
	}

	int Font::getWidth(const char * line) const
	{
		return this->getWidth(std::string(line));
	}

	int Font::getWidth(const char character) const
	{
		return spacing[(int)character];
	}

	int Font::getWrap(const std::string & line, float wrap, int * lines) const
	{
		if(line.size() == 0) return 0;
		int maxw = 0;
		int linen = 1;
		int temp = 0;
		std::string text;

		for(unsigned int i = 0; i < line.size(); i++)
		{
			if(temp > wrap && text.find(" ") != std::string::npos)
			{
				unsigned int space = text.find_last_of(' ');
				std::string tmp = text.substr(0, space);
				int w = getWidth(tmp);
				if(w > maxw) maxw = w;
				text = text.substr(space+1);
				temp = getWidth(text);
				linen++;
			}
			temp += static_cast<int>((spacing[(int)line[i]] * mSpacing));
			text += line[i];
		}

		if(temp > maxw) maxw = temp;
		if(lines) *lines = linen;

		return maxw;
	}

	int Font::getWrap(const char * line, float wrap, int * lines) const
	{
		return getWrap(std::string(line), wrap, lines);
	}

	void Font::setLineHeight(float height)
	{
		this->lineHeight = height;
	}

	float Font::getLineHeight() const
	{
		return lineHeight;
	}

	void Font::setSpacing(float amount)
	{
		mSpacing = amount;
	}

	float Font::getSpacing() const
	{
		return mSpacing;
	}

	bool Font::loadVolatile()
	{
		// reload all glyphs
		for(unsigned int i = 0; i < MAX_CHARS; i++)
		{
			glyphs[i]->load();
			glNewList(list + i, GL_COMPILE);
			glyphs[i]->draw(0, 0, 0, 1, 1, 0, 0);
			glEndList();
		}
		return true;
	}

	void Font::unloadVolatile()
	{
		// delete the glyphs
		glDeleteLists(list, MAX_CHARS);
	}

} // opengl
} // graphics
} // love
