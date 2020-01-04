/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

#include "renderstate.h"
#include "common/StringMap.h"

namespace love
{
namespace graphics
{

// These are all with premultiplied alpha. computeBlendState adjusts for
// alpha-multiply if needed.
static const BlendState states[BLEND_MAX_ENUM] =
{
	// BLEND_ALPHA
	{BLENDOP_ADD, BLENDOP_ADD, BLENDFACTOR_ONE, BLENDFACTOR_ONE, BLENDFACTOR_ONE_MINUS_SRC_ALPHA, BLENDFACTOR_ONE_MINUS_SRC_ALPHA},

	// BLEND_ADD
	{BLENDOP_ADD, BLENDOP_ADD, BLENDFACTOR_ONE, BLENDFACTOR_ZERO, BLENDFACTOR_ONE, BLENDFACTOR_ONE},

	// BLEND_SUBTRACT
	{BLENDOP_REVERSE_SUBTRACT, BLENDOP_REVERSE_SUBTRACT, BLENDFACTOR_ONE, BLENDFACTOR_ZERO, BLENDFACTOR_ONE, BLENDFACTOR_ONE},

	// BLEND_MULTIPLY
	{BLENDOP_ADD, BLENDOP_ADD, BLENDFACTOR_DST_COLOR, BLENDFACTOR_DST_COLOR, BLENDFACTOR_ZERO, BLENDFACTOR_ZERO},

	// BLEND_LIGHTEN
	{BLENDOP_MAX, BLENDOP_MAX, BLENDFACTOR_ZERO, BLENDFACTOR_ZERO, BLENDFACTOR_ONE, BLENDFACTOR_ONE},

	// BLEND_DARKEN
	{BLENDOP_MAX, BLENDOP_MAX, BLENDFACTOR_ONE, BLENDFACTOR_ONE, BLENDFACTOR_ONE, BLENDFACTOR_ONE},

	// BLEND_SCREEN
	{BLENDOP_ADD, BLENDOP_ADD, BLENDFACTOR_ONE, BLENDFACTOR_ONE, BLENDFACTOR_ONE_MINUS_SRC_COLOR, BLENDFACTOR_ONE_MINUS_SRC_COLOR},

	// BLEND_REPLACE
	{BLENDOP_ADD, BLENDOP_ADD, BLENDFACTOR_ONE, BLENDFACTOR_ONE, BLENDFACTOR_ZERO, BLENDFACTOR_ZERO},

	// BLEND_NONE
	{},

	// BLEND_CUSTOM - N/A
	{},
};

BlendState computeBlendState(BlendMode mode, BlendAlpha alphamode)
{
	BlendState s = states[mode];

	// We can only do alpha-multiplication when srcRGB would have been unmodified.
	if (s.srcFactorRGB == BLENDFACTOR_ONE && alphamode == BLENDALPHA_MULTIPLY && mode != BLEND_NONE)
		s.srcFactorRGB = BLENDFACTOR_SRC_ALPHA;

	return s;
}

BlendMode computeBlendMode(BlendState s, BlendAlpha &alphamode)
{
	if (!s.enable)
	{
		alphamode = BLENDALPHA_PREMULTIPLIED;
		return BLEND_NONE;
	}

	// Temporarily disable alpha multiplication when comparing to our list.
	bool alphamultiply = s.srcFactorRGB == BLENDFACTOR_SRC_ALPHA;
	if (alphamultiply)
		s.srcFactorRGB = BLENDFACTOR_ONE;

	for (int i = 0; i < (int) BLEND_MAX_ENUM; i++)
	{
		if (i != (int) BLEND_CUSTOM && states[i] == s)
		{
			alphamode = alphamultiply ? BLENDALPHA_MULTIPLY : BLENDALPHA_PREMULTIPLIED;
			return (BlendMode) i;
		}
	}

	alphamode = BLENDALPHA_PREMULTIPLIED;
	return BLEND_CUSTOM;
}

bool isAlphaMultiplyBlendSupported(BlendMode mode)
{
	switch (mode)
	{
	case BLEND_LIGHTEN:
	case BLEND_DARKEN:
	case BLEND_MULTIPLY:
		return false;
	default:
		return true;
	}
}

CompareMode getReversedCompareMode(CompareMode mode)
{
	switch (mode)
	{
		case COMPARE_LESS: return COMPARE_GREATER;
		case COMPARE_LEQUAL: return COMPARE_GEQUAL;
		case COMPARE_GEQUAL: return COMPARE_LEQUAL;
		case COMPARE_GREATER: return COMPARE_LESS;
		default: return mode;
	}
}

static StringMap<BlendMode, BLEND_MAX_ENUM>::Entry blendModeEntries[] =
{
	{ "alpha",    BLEND_ALPHA    },
	{ "add",      BLEND_ADD      },
	{ "subtract", BLEND_SUBTRACT },
	{ "multiply", BLEND_MULTIPLY },
	{ "lighten",  BLEND_LIGHTEN  },
	{ "darken",   BLEND_DARKEN   },
	{ "screen",   BLEND_SCREEN   },
	{ "replace",  BLEND_REPLACE  },
	{ "none",     BLEND_NONE     },
	{ "custom",   BLEND_CUSTOM   },
};

static StringMap<BlendMode, BLEND_MAX_ENUM> blendModes(blendModeEntries, sizeof(blendModeEntries));

static StringMap<BlendAlpha, BLENDALPHA_MAX_ENUM>::Entry blendAlphaEntries[] =
{
	{ "alphamultiply", BLENDALPHA_MULTIPLY      },
	{ "premultiplied", BLENDALPHA_PREMULTIPLIED },
};

static StringMap<BlendAlpha, BLENDALPHA_MAX_ENUM> blendAlphaModes(blendAlphaEntries, sizeof(blendAlphaEntries));

static StringMap<BlendFactor, BLENDFACTOR_MAX_ENUM>::Entry blendFactorEntries[] =
{
	{ "zero",              BLENDFACTOR_ZERO                 },
	{ "one",               BLENDFACTOR_ONE                  },
	{ "srccolor",          BLENDFACTOR_SRC_COLOR            },
	{ "oneminussrccolor",  BLENDFACTOR_ONE_MINUS_SRC_COLOR  },
	{ "srcalpha",          BLENDFACTOR_SRC_ALPHA            },
	{ "oneminussrcalpha",  BLENDFACTOR_ONE_MINUS_SRC_ALPHA  },
	{ "dstcolor",          BLENDFACTOR_DST_COLOR            },
	{ "oneminusdstcolor",  BLENDFACTOR_ONE_MINUS_DST_COLOR  },
	{ "dstalpha",          BLENDFACTOR_DST_ALPHA            },
	{ "oneminusdstalpha",  BLENDFACTOR_ONE_MINUS_DST_ALPHA  },
	{ "srcalphasaturated", BLENDFACTOR_SRC_ALPHA_SATURATED  },
};

static StringMap<BlendFactor, BLENDFACTOR_MAX_ENUM> blendFactors(blendFactorEntries, sizeof(blendFactorEntries));

static StringMap<BlendOperation, BLENDOP_MAX_ENUM>::Entry blendOperationEntries[] =
{
	{ "add",             BLENDOP_ADD              },
	{ "subtract",        BLENDOP_SUBTRACT         },
	{ "reversesubtract", BLENDOP_REVERSE_SUBTRACT },
	{ "min",             BLENDOP_MIN              },
	{ "max",             BLENDOP_MAX              },
};

static StringMap<BlendOperation, BLENDOP_MAX_ENUM> blendOperations(blendOperationEntries, sizeof(blendOperationEntries));

static StringMap<StencilAction, STENCIL_MAX_ENUM>::Entry stencilActionEntries[] =
{
	{ "keep",          STENCIL_KEEP           },
	{ "zero",          STENCIL_ZERO           },
	{ "replace",       STENCIL_REPLACE        },
	{ "increment",     STENCIL_INCREMENT      },
	{ "decrement",     STENCIL_DECREMENT      },
	{ "incrementwrap", STENCIL_INCREMENT_WRAP },
	{ "decrementwrap", STENCIL_DECREMENT_WRAP },
	{ "invert",        STENCIL_INVERT         },
};

static StringMap<StencilAction, STENCIL_MAX_ENUM> stencilActions(stencilActionEntries, sizeof(stencilActionEntries));

static StringMap<CompareMode, COMPARE_MAX_ENUM>::Entry compareModeEntries[] =
{
	{ "less",     COMPARE_LESS     },
	{ "lequal",   COMPARE_LEQUAL   },
	{ "equal",    COMPARE_EQUAL    },
	{ "gequal",   COMPARE_GEQUAL   },
	{ "greater",  COMPARE_GREATER  },
	{ "notequal", COMPARE_NOTEQUAL },
	{ "always",   COMPARE_ALWAYS   },
	{ "never",    COMPARE_NEVER    },
};

static StringMap<CompareMode, COMPARE_MAX_ENUM> compareModes(compareModeEntries, sizeof(compareModeEntries));

bool getConstant(const char *in, BlendMode &out)
{
	return blendModes.find(in, out);
}

bool getConstant(BlendMode in, const char *&out)
{
	return blendModes.find(in, out);
}

std::vector<std::string> getConstants(BlendMode)
{
	return blendModes.getNames();
}

bool getConstant(const char *in, BlendAlpha &out)
{
	return blendAlphaModes.find(in, out);
}

bool getConstant(BlendAlpha in, const char *&out)
{
	return blendAlphaModes.find(in, out);
}

std::vector<std::string> getConstants(BlendAlpha)
{
	return blendAlphaModes.getNames();
}

bool getConstant(const char *in, BlendFactor &out)
{
	return blendFactors.find(in, out);
}

bool getConstant(BlendFactor in, const char *&out)
{
	return blendFactors.find(in, out);
}

std::vector<std::string> getConstants(BlendFactor)
{
	return blendFactors.getNames();
}

bool getConstant(const char *in, BlendOperation &out)
{
	return blendOperations.find(in, out);
}

bool getConstant(BlendOperation in, const char *&out)
{
	return blendOperations.find(in, out);
}

std::vector<std::string> getConstants(BlendOperation)
{
	return blendOperations.getNames();
}

bool getConstant(const char *in, StencilAction &out)
{
	return stencilActions.find(in, out);
}

bool getConstant(StencilAction in, const char *&out)
{
	return stencilActions.find(in, out);
}

std::vector<std::string> getConstants(StencilAction)
{
	return stencilActions.getNames();
}

bool getConstant(const char *in, CompareMode &out)
{
	return compareModes.find(in, out);
}

bool getConstant(CompareMode in, const char *&out)
{
	return compareModes.find(in, out);
}

std::vector<std::string> getConstants(CompareMode)
{
	return compareModes.getNames();
}

} // graphics
} // love
