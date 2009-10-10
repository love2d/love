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
#include <common/StringMap.h>

namespace love
{
namespace graphics
{
	class Graphics : public Module
	{
	public:

		enum DrawMode
		{
			DRAW_LINE = 1, 
			DRAW_FILL,
			DRAW_MAX_ENUM
		};

		enum AlignMode
		{
			ALIGN_LEFT = 1, 
			ALIGN_CENTER, 
			ALIGN_RIGHT,
			ALIGN_MAX_ENUM
		};

		enum BlendMode
		{
			BLEND_ALPHA = 1, 
			BLEND_ADDITIVE,
			BLEND_MAX_ENUM
		};

		enum ColorMode
		{
			COLOR_MODULATE = 1, 
			COLOR_REPLACE,
			COLOR_MAX_ENUM
		};

		enum LineStyle
		{
			LINE_ROUGH = 1, 
			LINE_SMOOTH,
			LINE_MAX_ENUM
		};

		enum PointStyle
		{
			POINT_ROUGH = 1,
			POINT_SMOOTH,
			POINT_MAX_ENUM
		};

		virtual ~Graphics();

		static bool getConstant(const char * in, DrawMode & out);
		static bool getConstant(DrawMode in, const char *& out);

		static bool getConstant(const char * in, AlignMode & out);
		static bool getConstant(AlignMode in, const char *& out);

		static bool getConstant(const char * in, BlendMode & out);
		static bool getConstant(BlendMode in, const char *& out);

		static bool getConstant(const char * in, ColorMode & out);
		static bool getConstant(ColorMode in, const char *& out);

		static bool getConstant(const char * in, LineStyle & out);
		static bool getConstant(LineStyle in, const char *& out);

		static bool getConstant(const char * in, PointStyle & out);
		static bool getConstant(PointStyle in, const char *& out);

	private:

		static StringMap<DrawMode, DRAW_MAX_ENUM>::Entry drawModeEntries[];
		static StringMap<DrawMode, DRAW_MAX_ENUM> drawModes;

		static StringMap<AlignMode, ALIGN_MAX_ENUM>::Entry alignModeEntries[];
		static StringMap<AlignMode, ALIGN_MAX_ENUM> alignModes;

		static StringMap<BlendMode, BLEND_MAX_ENUM>::Entry blendModeEntries[];
		static StringMap<BlendMode, BLEND_MAX_ENUM> blendModes;

		static StringMap<ColorMode, COLOR_MAX_ENUM>::Entry colorModeEntries[];
		static StringMap<ColorMode, COLOR_MAX_ENUM> colorModes;

		static StringMap<LineStyle, LINE_MAX_ENUM>::Entry lineStyleEntries[];
		static StringMap<LineStyle, LINE_MAX_ENUM> lineStyles;

		static StringMap<PointStyle, POINT_MAX_ENUM>::Entry pointStyleEntries[];
		static StringMap<PointStyle, POINT_MAX_ENUM> pointStyles;

	}; // Graphics

} // graphics
} // love

#endif // LOVE_GRAPHICS_GRAPHICS_H
