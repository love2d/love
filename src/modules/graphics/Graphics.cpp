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

#include "Graphics.h"
#include "math/MathModule.h"

namespace love
{
namespace graphics
{

static bool gammaCorrect = false;

void setGammaCorrect(bool gammacorrect)
{
	gammaCorrect = gammacorrect;
}

bool isGammaCorrect()
{
	return gammaCorrect;
}

void gammaCorrectColor(Colorf &c)
{
	if (isGammaCorrect())
	{
		c.r = math::Math::instance.gammaToLinear(c.r);
		c.g = math::Math::instance.gammaToLinear(c.g);
		c.b = math::Math::instance.gammaToLinear(c.b);
	}
}

void unGammaCorrectColor(Colorf &c)
{
	if (isGammaCorrect())
	{
		c.r = math::Math::instance.linearToGamma(c.r);
		c.g = math::Math::instance.linearToGamma(c.g);
		c.b = math::Math::instance.linearToGamma(c.b);
	}
}

Graphics::~Graphics()
{
}

bool Graphics::getConstant(const char *in, DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(DrawMode in, const char *&out)
{
	return drawModes.find(in, out);
}

bool Graphics::getConstant(const char *in, ArcMode &out)
{
	return arcModes.find(in, out);
}

bool Graphics::getConstant(ArcMode in, const char *&out)
{
	return arcModes.find(in, out);
}

bool Graphics::getConstant(const char *in, BlendMode &out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(BlendMode in, const char *&out)
{
	return blendModes.find(in, out);
}

bool Graphics::getConstant(const char *in, BlendAlpha &out)
{
	return blendAlphaModes.find(in, out);
}

bool Graphics::getConstant(BlendAlpha in, const char *&out)
{
	return blendAlphaModes.find(in, out);
}

bool Graphics::getConstant(const char *in, LineStyle &out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(LineStyle in, const char *&out)
{
	return lineStyles.find(in, out);
}

bool Graphics::getConstant(const char *in, LineJoin &out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(LineJoin in, const char *&out)
{
	return lineJoins.find(in, out);
}

bool Graphics::getConstant(const char *in, StencilAction &out)
{
	return stencilActions.find(in, out);
}

bool Graphics::getConstant(StencilAction in, const char *&out)
{
	return stencilActions.find(in, out);
}

bool Graphics::getConstant(const char *in, CompareMode &out)
{
	return compareModes.find(in, out);
}

bool Graphics::getConstant(CompareMode in, const char *&out)
{
	return compareModes.find(in, out);
}

bool Graphics::getConstant(const char *in, Support &out)
{
	return support.find(in, out);
}

bool Graphics::getConstant(Support in, const char *&out)
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
	{ "line", DRAW_LINE },
	{ "fill", DRAW_FILL },
};

StringMap<Graphics::DrawMode, Graphics::DRAW_MAX_ENUM> Graphics::drawModes(Graphics::drawModeEntries, sizeof(Graphics::drawModeEntries));

StringMap<Graphics::ArcMode, Graphics::ARC_MAX_ENUM>::Entry Graphics::arcModeEntries[] =
{
	{ "open",   ARC_OPEN   },
	{ "closed", ARC_CLOSED },
	{ "pie",    ARC_PIE    },
};

StringMap<Graphics::ArcMode, Graphics::ARC_MAX_ENUM> Graphics::arcModes(Graphics::arcModeEntries, sizeof(Graphics::arcModeEntries));

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM>::Entry Graphics::blendModeEntries[] =
{
	{ "alpha",    BLEND_ALPHA    },
	{ "add",      BLEND_ADD      },
	{ "subtract", BLEND_SUBTRACT },
	{ "multiply", BLEND_MULTIPLY },
	{ "lighten",  BLEND_LIGHTEN  },
	{ "darken",   BLEND_DARKEN   },
	{ "screen",   BLEND_SCREEN   },
	{ "replace",  BLEND_REPLACE  },
};

StringMap<Graphics::BlendMode, Graphics::BLEND_MAX_ENUM> Graphics::blendModes(Graphics::blendModeEntries, sizeof(Graphics::blendModeEntries));

StringMap<Graphics::BlendAlpha, Graphics::BLENDALPHA_MAX_ENUM>::Entry Graphics::blendAlphaEntries[] =
{
	{ "alphamultiply", BLENDALPHA_MULTIPLY      },
	{ "premultiplied", BLENDALPHA_PREMULTIPLIED },
};

StringMap<Graphics::BlendAlpha, Graphics::BLENDALPHA_MAX_ENUM> Graphics::blendAlphaModes(Graphics::blendAlphaEntries, sizeof(Graphics::blendAlphaEntries));

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM>::Entry Graphics::lineStyleEntries[] =
{
	{ "smooth", LINE_SMOOTH },
	{ "rough",  LINE_ROUGH  }
};

StringMap<Graphics::LineStyle, Graphics::LINE_MAX_ENUM> Graphics::lineStyles(Graphics::lineStyleEntries, sizeof(Graphics::lineStyleEntries));

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM>::Entry Graphics::lineJoinEntries[] =
{
	{ "none",  LINE_JOIN_NONE  },
	{ "miter", LINE_JOIN_MITER },
	{ "bevel", LINE_JOIN_BEVEL }
};

StringMap<Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM> Graphics::lineJoins(Graphics::lineJoinEntries, sizeof(Graphics::lineJoinEntries));

StringMap<Graphics::StencilAction, Graphics::STENCIL_MAX_ENUM>::Entry Graphics::stencilActionEntries[] =
{
	{ "replace", STENCIL_REPLACE },
	{ "increment", STENCIL_INCREMENT },
	{ "decrement", STENCIL_DECREMENT },
	{ "incrementwrap", STENCIL_INCREMENT_WRAP },
	{ "decrementwrap", STENCIL_DECREMENT_WRAP },
	{ "invert", STENCIL_INVERT },
};

StringMap<Graphics::StencilAction, Graphics::STENCIL_MAX_ENUM> Graphics::stencilActions(Graphics::stencilActionEntries, sizeof(Graphics::stencilActionEntries));

StringMap<Graphics::CompareMode, Graphics::COMPARE_MAX_ENUM>::Entry Graphics::compareModeEntries[] =
{
	{ "less",     COMPARE_LESS     },
	{ "lequal",   COMPARE_LEQUAL   },
	{ "equal",    COMPARE_EQUAL    },
	{ "gequal",   COMPARE_GEQUAL   },
	{ "greater",  COMPARE_GREATER  },
	{ "notequal", COMPARE_NOTEQUAL },
	{ "always",   COMPARE_ALWAYS   },
};

StringMap<Graphics::CompareMode, Graphics::COMPARE_MAX_ENUM> Graphics::compareModes(Graphics::compareModeEntries, sizeof(Graphics::compareModeEntries));

StringMap<Graphics::Support, Graphics::SUPPORT_MAX_ENUM>::Entry Graphics::supportEntries[] =
{
	{ "multicanvasformats", SUPPORT_MULTI_CANVAS_FORMATS },
	{ "clampzero", SUPPORT_CLAMP_ZERO },
	{ "lighten", SUPPORT_LIGHTEN },
};

StringMap<Graphics::Support, Graphics::SUPPORT_MAX_ENUM> Graphics::support(Graphics::supportEntries, sizeof(Graphics::supportEntries));

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM>::Entry Graphics::systemLimitEntries[] =
{
	{ "pointsize",   LIMIT_POINT_SIZE   },
	{ "texturesize", LIMIT_TEXTURE_SIZE },
	{ "multicanvas", LIMIT_MULTI_CANVAS },
	{ "canvasmsaa",  LIMIT_CANVAS_MSAA  },
};

StringMap<Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM> Graphics::systemLimits(Graphics::systemLimitEntries, sizeof(Graphics::systemLimitEntries));

StringMap<Graphics::StackType, Graphics::STACK_MAX_ENUM>::Entry Graphics::stackTypeEntries[] =
{
	{ "all", STACK_ALL },
	{ "transform", STACK_TRANSFORM },
};

StringMap<Graphics::StackType, Graphics::STACK_MAX_ENUM> Graphics::stackTypes(Graphics::stackTypeEntries, sizeof(Graphics::stackTypeEntries));

StringMap<Graphics::StatType, Graphics::STAT_MAX_ENUM>::Entry Graphics::statTypeEntries[] =
{
	{ "drawcalls", STAT_DRAW_CALLS },
	{ "canvasswitches", STAT_CANVAS_SWITCHES },
	{ "canvases", STAT_CANVASES },
	{ "images", STAT_IMAGES },
	{ "fonts", STAT_FONTS },
	{ "texturememory", STAT_TEXTURE_MEMORY },
};

StringMap<Graphics::StatType, Graphics::STAT_MAX_ENUM> Graphics::statTypes(Graphics::statTypeEntries, sizeof(Graphics::statTypeEntries));

} // graphics
} // love
