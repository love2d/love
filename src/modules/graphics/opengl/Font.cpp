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

#include "Font.h"

#include <common/math.h>
#include <math.h>

namespace love
{
namespace graphics
{
namespace opengl
{
	
	Font::Font(love::font::FontData * data)
		: height(data->getHeight()), lineHeight(1.25)
	{
		glyphs = new Glyph*[MAX_CHARS];
		for(unsigned int i = 0; i < MAX_CHARS; i++)
		{
			glyphs[i] = new Glyph(data->getGlyphData(i));
			glyphs[i]->load();
			widths[i] = data->getGlyphData(i)->getWidth();
			spacing[i] = data->getGlyphData(i)->getAdvance();
			bearingX[i] = data->getGlyphData(i)->getBearingX();
			bearingY[i] = data->getGlyphData(i)->getBearingY();
		}
	}

	Font::~Font()
	{
		delete[] glyphs;
	}

	float Font::getHeight() const
	{
		return height;
	}
	
	void Font::print(std::string text, float x, float y) const
	{
		print(text, x, y, 0.0f, 1.0f, 1.0f);
	}
	
	void Font::print(std::string text, float x, float y, float angle, float sx, float sy) const
	{
		glPushMatrix();
		
		glTranslatef(ceil(x), ceil(y+getHeight()), 0.0f);
		glRotatef(LOVE_TODEG(angle), 0, 0, 1.0f);
		glScalef(sx, sy, 1.0f);
		int s = 0;
		for (unsigned int i = 0; i < text.size(); i++) {
			int g = (int)text[i];
			glyphs[g]->draw(bearingX[g] + s, -bearingY[g], 0, 1, 1, 0, 0);
			s += spacing[g];
		}
		glPopMatrix();
	}
	
	void Font::print(char character, float x, float y) const
	{
		glyphs[character]->draw(x, y+getHeight(), 0, 1, 1, 0, 0);
	}
	
	float Font::getWidth(const std::string & line) const
	{
		if(line.size() == 0) return 0;
		float temp = 0;

		for(unsigned int i = 0; i < line.size() - 1; i++)
		{
			temp += widths[(int)line[i]] + (spacing[(int)line[i]] * mSpacing);
		}
		temp += widths[(int)line[line.size() - 1]]; // the last character's spacing isn't counted

		return temp;
	}

	float Font::getWidth(const char * line) const
	{
		return this->getWidth(std::string(line));
	}
	
	float Font::getWidth(const char character) const
	{
		return (float)widths[(int)character];
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

} // opengl
} // graphics
} // love
