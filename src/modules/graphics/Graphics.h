/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "common/Module.h"
#include "common/StringMap.h"
#include "Color.h"

// C++
#include <string>

namespace love
{
namespace graphics
{

/**
 * Globally sets whether gamma correction is enabled. Ideally this should be set
 * prior to using any Graphics module function.
 **/
void setGammaCorrect(bool gammacorrect);

/**
 * Gets whether global gamma correction is enabled.
 **/
bool isGammaCorrect();

/**
 * Gamma-corrects a color (converts it from sRGB to linear RGB, if
 * gamma correction is enabled.)
 * The color's components are expected to be in the range of [0, 1].
 **/
void gammaCorrectColor(Colorf &c);

/**
 * Un-gamma-corrects a color (converts it from linear RGB to sRGB, if
 * gamma correction is enabled.)
 * The color's components are expected to be in the range of [0, 1].
 **/
void unGammaCorrectColor(Colorf &c);

class Graphics : public Module
{
public:

	enum DrawMode
	{
		DRAW_LINE,
		DRAW_FILL,
		DRAW_MAX_ENUM
	};

	enum ArcMode
	{
		ARC_OPEN,
		ARC_CLOSED,
		ARC_PIE,
		ARC_MAX_ENUM
	};

	enum BlendMode
	{
		BLEND_ALPHA,
		BLEND_ADD,
		BLEND_SUBTRACT,
		BLEND_MULTIPLY,
		BLEND_LIGHTEN,
		BLEND_DARKEN,
		BLEND_SCREEN,
		BLEND_REPLACE,
		BLEND_MAX_ENUM
	};

	enum BlendAlpha
	{
		BLENDALPHA_MULTIPLY,
		BLENDALPHA_PREMULTIPLIED,
		BLENDALPHA_MAX_ENUM
	};

	enum LineStyle
	{
		LINE_ROUGH,
		LINE_SMOOTH,
		LINE_MAX_ENUM
	};

	enum LineJoin
	{
		LINE_JOIN_NONE,
		LINE_JOIN_MITER,
		LINE_JOIN_BEVEL,
		LINE_JOIN_MAX_ENUM
	};

	enum StencilAction
	{
		STENCIL_REPLACE,
		STENCIL_INCREMENT,
		STENCIL_DECREMENT,
		STENCIL_INCREMENT_WRAP,
		STENCIL_DECREMENT_WRAP,
		STENCIL_INVERT,
		STENCIL_MAX_ENUM
	};

	enum CompareMode
	{
		COMPARE_LESS,
		COMPARE_LEQUAL,
		COMPARE_EQUAL,
		COMPARE_GEQUAL,
		COMPARE_GREATER,
		COMPARE_NOTEQUAL,
		COMPARE_ALWAYS,
		COMPARE_MAX_ENUM
	};

	enum Feature
	{
		FEATURE_MULTI_CANVAS_FORMATS,
		FEATURE_CLAMP_ZERO,
		FEATURE_LIGHTEN,
		FEATURE_MAX_ENUM
	};

	enum Renderer
	{
		RENDERER_OPENGL = 0,
		RENDERER_OPENGLES,
		RENDERER_MAX_ENUM
	};

	enum SystemLimit
	{
		LIMIT_POINT_SIZE,
		LIMIT_TEXTURE_SIZE,
		LIMIT_MULTI_CANVAS,
		LIMIT_CANVAS_MSAA,
		LIMIT_MAX_ENUM
	};

	enum StackType
	{
		STACK_ALL,
		STACK_TRANSFORM,
		STACK_MAX_ENUM
	};

	struct RendererInfo
	{
		std::string name;
		std::string version;
		std::string vendor;
		std::string device;
	};

	struct Stats
	{
		int drawCalls;
		int canvasSwitches;
		int shaderSwitches;
		int canvases;
		int images;
		int fonts;
		size_t textureMemory;
	};

	struct ColorMask
	{
		bool r, g, b, a;

		ColorMask()
			: r(true), g(true), b(true), a(true)
		{}

		ColorMask(bool _r, bool _g, bool _b, bool _a)
			: r(_r), g(_g), b(_b), a(_a)
		{}

		bool operator == (const ColorMask &m) const
		{
			return r == m.r && g == m.g && b == m.b && a == m.a;
		}

		bool operator != (const ColorMask &m) const
		{
			return !(operator == (m));
		}
	};

	struct ScissorRect
	{
		int x, y;
		int w, h;

		bool operator == (const ScissorRect &rhs) const
		{
			return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
		}
	};

	virtual ~Graphics();

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_GRAPHICS; }

	/**
	 * Sets the current graphics display viewport dimensions.
	 **/
	virtual void setViewportSize(int width, int height) = 0;

	/**
	 * Sets the current graphics display viewport and initializes the renderer.
	 * @param width The viewport width.
	 * @param height The viewport height.
	 **/
	virtual bool setMode(int width, int height) = 0;

	/**
	 * Un-sets the current graphics display mode (uninitializing objects if
	 * necessary.)
	 **/
	virtual void unSetMode() = 0;

	/**
	 * Sets whether the module is active (internal use only.)
	 **/
	virtual void setActive(bool active) = 0;

	/**
	 * Gets whether the module is active. Graphics module methods are only
	 * guaranteed to work when it is active. Calling them otherwise may cause
	 * the program to crash (or worse.)
	 * Normally the module will always be active as long as a window exists, it
	 * may be different on some platforms (especially mobile ones.)
	 **/
	virtual bool isActive() const = 0;

	static bool getConstant(const char *in, DrawMode &out);
	static bool getConstant(DrawMode in, const char *&out);

	static bool getConstant(const char *in, ArcMode &out);
	static bool getConstant(ArcMode in, const char *&out);

	static bool getConstant(const char *in, BlendMode &out);
	static bool getConstant(BlendMode in, const char *&out);

	static bool getConstant(const char *in, BlendAlpha &out);
	static bool getConstant(BlendAlpha in, const char *&out);

	static bool getConstant(const char *in, LineStyle &out);
	static bool getConstant(LineStyle in, const char *&out);

	static bool getConstant(const char *in, LineJoin &out);
	static bool getConstant(LineJoin in, const char *&out);

	static bool getConstant(const char *in, StencilAction &out);
	static bool getConstant(StencilAction in, const char *&out);

	static bool getConstant(const char *in, CompareMode &out);
	static bool getConstant(CompareMode in, const char *&out);

	static bool getConstant(const char *in, Feature &out);
	static bool getConstant(Feature in, const char *&out);

	static bool getConstant(const char *in, SystemLimit &out);
	static bool getConstant(SystemLimit in, const char *&out);

	static bool getConstant(const char *in, StackType &out);
	static bool getConstant(StackType in, const char *&out);

private:

	static StringMap<DrawMode, DRAW_MAX_ENUM>::Entry drawModeEntries[];
	static StringMap<DrawMode, DRAW_MAX_ENUM> drawModes;

	static StringMap<ArcMode, ARC_MAX_ENUM>::Entry arcModeEntries[];
	static StringMap<ArcMode, ARC_MAX_ENUM> arcModes;

	static StringMap<BlendMode, BLEND_MAX_ENUM>::Entry blendModeEntries[];
	static StringMap<BlendMode, BLEND_MAX_ENUM> blendModes;

	static StringMap<BlendAlpha, BLENDALPHA_MAX_ENUM>::Entry blendAlphaEntries[];
	static StringMap<BlendAlpha, BLENDALPHA_MAX_ENUM> blendAlphaModes;

	static StringMap<LineStyle, LINE_MAX_ENUM>::Entry lineStyleEntries[];
	static StringMap<LineStyle, LINE_MAX_ENUM> lineStyles;

	static StringMap<LineJoin, LINE_JOIN_MAX_ENUM>::Entry lineJoinEntries[];
	static StringMap<LineJoin, LINE_JOIN_MAX_ENUM> lineJoins;

	static StringMap<StencilAction, STENCIL_MAX_ENUM>::Entry stencilActionEntries[];
	static StringMap<StencilAction, STENCIL_MAX_ENUM> stencilActions;

	static StringMap<CompareMode, COMPARE_MAX_ENUM>::Entry compareModeEntries[];
	static StringMap<CompareMode, COMPARE_MAX_ENUM> compareModes;

	static StringMap<Feature, FEATURE_MAX_ENUM>::Entry featureEntries[];
	static StringMap<Feature, FEATURE_MAX_ENUM> features;

	static StringMap<SystemLimit, LIMIT_MAX_ENUM>::Entry systemLimitEntries[];
	static StringMap<SystemLimit, LIMIT_MAX_ENUM> systemLimits;

	static StringMap<StackType, STACK_MAX_ENUM>::Entry stackTypeEntries[];
	static StringMap<StackType, STACK_MAX_ENUM> stackTypes;

}; // Graphics

} // graphics
} // love

#endif // LOVE_GRAPHICS_GRAPHICS_H
