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

#include "Font.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Font::Font(int size) 
		: size(size), lineHeight(1), mSpacing(1)
	{
		for(unsigned int i = 0; i < MAX_CHARS; i++)
		{
			widths[i] = 0;
			spacing[i] = 0;
		}
	}

	Font::~Font()
	{
	}

	float Font::getHeight() const
	{
		return (float)size;
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
