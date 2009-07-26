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

#ifndef LOVE_GRAPHICS_GRAPHICS_H
#define LOVE_GRAPHICS_GRAPHICS_H

// LOVE
#include <common/Module.h>

namespace love
{
namespace graphics
{
	class Graphics : public Module
	{
	public:

		enum DrawMode
		{
			DRAW_LINE, 
			DRAW_FILL
		};

		enum AlignMode
		{
			ALIGN_LEFT, 
			ALIGN_CENTER, 
			ALIGN_RIGHT
		};

		enum BlendMode
		{
			BLEND_ALPHA, 
			BLEND_ADDITIVE
		};

		enum ColorMode
		{
			COLOR_MODULATE, 
			COLOR_REPLACE
		};

		enum LineStyle
		{
			LINE_ROUGH, 
			LINE_SMOOTH
		};

		enum PointStyle
		{
			POINT_ROUGH,
			POINT_SMOOTH
		};

		virtual ~Graphics(){};
		
	}; // Graphics

} // graphics
} // love

#endif // LOVE_GRAPHICS_GRAPHICS_H
