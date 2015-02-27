/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "Graphics.h"

namespace love
{
namespace graphics
{

Graphics::~Graphics()
{
}

bool Graphics::getConstant(const char *in, DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(DrawMode in, const char  *&out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(const char *in, BlendMode &out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(BlendMode in, const char  *&out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(const char *in, LineStyle &out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(LineStyle in, const char  *&out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(const char *in, LineJoin &out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(LineJoin in, const char  *&out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(const char *in, Support &out)
{
	return support.find(in, out);
}

bool Graphics::getConstant(Support in, const char  *&out)
{
	return support.find(in, out);
}

bool Graphics::getConstant(const char *in, SystemLimit &out)
{
	return systemLimits.find(in, out);
}

bool Graphics::getConstant(SystemLimit in, const char *&out)
{
	return systemLimits.find(in, out);
}

bool Graphics::getConstant(const char *in, StackType &out)
{
	return stackTypes.find(in, out);
}

bool Graphics::getConstant(StackType in, const char *&out)
{
	return stackTypes.find(in, out);
}

bool Graphics::getConstant(const char *in, StatType &out)
{
	return statTypes.find(in, out);
}

bool Graphics::getConstant(StatType in, const char *&out)
{
	return statTypes.find(in, out);
}

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM>::Entry Graphics::drawModeEntries[] =
{
	{ "line", Graphics::DRAW_LINE },
	{ "fill", Graphics::DRAW_FILL },
};

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM> Graphics::drawModes(Graphics::drawModeEntries, sizeof(Graphics::drawModeEntries));

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM>::Entry Graphics::blendModeEntries[] =
{
	{ "alpha", Graphics::BLEND_ALPHA },
	{ "add", Graphics::BLEND_ADD },
	{ "subtract", Graphics::BLEND_SUBTRACT },
	{ "multiply", Graphics::BLEND_MULTIPLY },
	{ "premultiplied", Graphics::BLEND_PREMULTIPLIED },
	{ "screen", Graphics::BLEND_SCREEN },
	{ "replace", Graphics::BLEND_REPLACE },
};

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM> Graphics::blendModes(Graphics::blendModeEntries, sizeof(Graphics::blendModeEntries));

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM>::Entry Graphics::lineStyleEntries[] =
{
	{ "smooth", Graphics::LINE_SMOOTH },
	{ "rough", Graphics::LINE_ROUGH }
};

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM> Graphics::lineStyles(Graphics::lineStyleEntries, sizeof(Graphics::lineStyleEntries));

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM>::Entry Graphics::lineJoinEntries[] =
{
	{ "none",  Graphics::LINE_JOIN_NONE  },
	{ "miter", Graphics::LINE_JOIN_MITER },
	{ "bevel", Graphics::LINE_JOIN_BEVEL }
};

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM> Graphics::lineJoins(Graphics::lineJoinEntries, sizeof(Graphics::lineJoinEntries));

StringMap<Graphics::Support, Graphics::SUPPORT_MAX_ENUM>::Entry Graphics::supportEntries[] =
{
	{ "multicanvas", Graphics::SUPPORT_MULTI_CANVAS },
	{ "srgb", Graphics::SUPPORT_SRGB },
};

StringMap<Graphics::Support, Graphics::SUPPORT_MAX_ENUM> Graphics::support(Graphics::supportEntries, sizeof(Graphics::supportEntries));

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM>::Entry Graphics::systemLimitEntries[] =
{
	{"pointsize", Graphics::LIMIT_POINT_SIZE},
	{"texturesize", Graphics::LIMIT_TEXTURE_SIZE},
	{"multicanvas", Graphics::LIMIT_MULTI_CANVAS},
	{"canvasmsaa", Graphics::LIMIT_CANVAS_MSAA},
};

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM> Graphics::systemLimits(Graphics::systemLimitEntries, sizeof(Graphics::systemLimitEntries));

StringMap<Graphics::StackType, Graphics::STACK_MAX_ENUM>::Entry Graphics::stackTypeEntries[] =
{
	{"all", Graphics::STACK_ALL},
	{"transform", Graphics::STACK_TRANSFORM},
};

StringMap<Graphics::StackType, Graphics::STACK_MAX_ENUM> Graphics::stackTypes(Graphics::stackTypeEntries, sizeof(Graphics::stackTypeEntries));

StringMap<Graphics::StatType, Graphics::STAT_MAX_ENUM>::Entry Graphics::statTypeEntries[] =
{
	{"drawcalls", Graphics::STAT_DRAW_CALLS},
	{"canvasswitches", Graphics::STAT_CANVAS_SWITCHES},
	{"canvases", Graphics::STAT_CANVASES},
	{"images", Graphics::STAT_IMAGES},
	{"fonts", Graphics::STAT_FONTS},
	{"texturememory", Graphics::STAT_TEXTURE_MEMORY},
};

StringMap<Graphics::StatType, Graphics::STAT_MAX_ENUM> Graphics::statTypes(Graphics::statTypeEntries, sizeof(Graphics::statTypeEntries));

} // graphics
} // love
