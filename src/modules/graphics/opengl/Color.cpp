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

#include "Color.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Color::Color() 
		: red(255), green(255), blue(255), alpha(255)
	{
	}

	Color::Color(int r, int g, int b, int a) 
		: red(r), green(g), blue(b), alpha(a)
	{
	}

	Color::~Color()
	{
	}

	void Color::setRed(int red)
	{
		this->red = red;
	}

	void Color::setGreen(int green)
	{
		this->green = green;
	}

	void Color::setBlue(int blue)
	{
		this->blue = blue;
	}

	void Color::setAlpha(int alpha)
	{
		this->alpha = alpha;
	}

	int Color::getRed() const
	{
		return red;
	}

	int Color::getGreen() const
	{
		return green;
	}

	int Color::getBlue() const
	{
		return blue;
	}

	int Color::getAlpha() const
	{
		return alpha;
	}

} // opengl
} // graphics
} // love
