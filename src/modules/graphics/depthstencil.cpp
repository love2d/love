/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "depthstencil.h"
#include "common/StringMap.h"

namespace love
{
namespace graphics
{

CompareMode getReversedCompareMode(CompareMode mode)
{
	switch (mode)
	{
	case COMPARE_LESS:
		return COMPARE_GREATER;
	case COMPARE_LEQUAL:
		return COMPARE_GEQUAL;
	case COMPARE_GEQUAL:
		return COMPARE_LEQUAL;
	case COMPARE_GREATER:
		return COMPARE_LESS;
	default:
		return mode;
	}
}

static StringMap<StencilAction, STENCIL_MAX_ENUM>::Entry stencilActionEntries[] =
{
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
