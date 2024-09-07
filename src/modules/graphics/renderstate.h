/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#pragma once

#include "common/int.h"
#include "common/math.h"
#include "common/StringMap.h"
#include "vertex.h"

#include <vector>
#include <string>

namespace love
{
namespace graphics
{

class Shader;

enum BlendMode // High level wrappers. Order is important (see renderstate.cpp)
{
	BLEND_ALPHA,
	BLEND_ADD,
	BLEND_SUBTRACT,
	BLEND_MULTIPLY,
	BLEND_LIGHTEN,
	BLEND_DARKEN,
	BLEND_SCREEN,
	BLEND_REPLACE,
	BLEND_NONE,
	BLEND_CUSTOM,
	BLEND_MAX_ENUM
};

enum BlendAlpha // High level wrappers
{
	BLENDALPHA_MULTIPLY,
	BLENDALPHA_PREMULTIPLIED,
	BLENDALPHA_MAX_ENUM
};

enum BlendFactor
{
	BLENDFACTOR_ZERO,
	BLENDFACTOR_ONE,
	BLENDFACTOR_SRC_COLOR,
	BLENDFACTOR_ONE_MINUS_SRC_COLOR,
	BLENDFACTOR_SRC_ALPHA,
	BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
	BLENDFACTOR_DST_COLOR,
	BLENDFACTOR_ONE_MINUS_DST_COLOR,
	BLENDFACTOR_DST_ALPHA,
	BLENDFACTOR_ONE_MINUS_DST_ALPHA,
	BLENDFACTOR_SRC_ALPHA_SATURATED,
	BLENDFACTOR_MAX_ENUM
};

enum BlendOperation
{
	BLENDOP_ADD,
	BLENDOP_SUBTRACT,
	BLENDOP_REVERSE_SUBTRACT,
	BLENDOP_MIN,
	BLENDOP_MAX,
	BLENDOP_MAX_ENUM
};

enum StencilMode // High level wrappers.
{
	STENCIL_MODE_OFF,
	STENCIL_MODE_DRAW,
	STENCIL_MODE_TEST,
	STENCIL_MODE_CUSTOM,
	STENCIL_MODE_MAX_ENUM
};

enum StencilAction
{
	STENCIL_KEEP,
	STENCIL_ZERO,
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
	COMPARE_NEVER,
	COMPARE_MAX_ENUM
};

struct BlendState
{
	BlendOperation operationRGB = BLENDOP_ADD;
	BlendOperation operationA = BLENDOP_ADD;
	BlendFactor srcFactorRGB = BLENDFACTOR_ONE;
	BlendFactor srcFactorA = BLENDFACTOR_ONE;
	BlendFactor dstFactorRGB = BLENDFACTOR_ZERO;
	BlendFactor dstFactorA = BLENDFACTOR_ZERO;
	bool enable = false;

	BlendState() {}

	BlendState(BlendOperation opRGB, BlendOperation opA, BlendFactor srcRGB, BlendFactor srcA, BlendFactor dstRGB, BlendFactor dstA)
		: operationRGB(opRGB)
		, operationA(opA)
		, srcFactorRGB(srcRGB)
		, srcFactorA(srcA)
		, dstFactorRGB(dstRGB)
		, dstFactorA(dstA)
		, enable(true)
	{}

	bool operator == (const BlendState &b) const
	{
		return enable == b.enable
			&& operationRGB == b.operationRGB && operationA == b.operationA
			&& srcFactorRGB == b.srcFactorRGB && srcFactorA == b.srcFactorA
			&& dstFactorRGB == b.dstFactorRGB && dstFactorA == b.dstFactorA;
	}

	uint32 toKey() const
	{
		return (dstFactorA << 0) | (dstFactorRGB << 4) | (srcFactorA << 8) | (srcFactorRGB << 12)
			| (operationA << 16) | (operationRGB << 20) | ((enable ? 1 : 0) << 24);
	}

	static BlendState fromKey(uint32 key)
	{
		BlendState b;
		b.enable = (bool)((key >> 24) & 1);
		b.operationRGB = (BlendOperation)((key >> 20) & 0x7);
		b.operationA = (BlendOperation)((key >> 16) & 0x7);
		b.srcFactorRGB = (BlendFactor)((key >> 12) & 0xF);
		b.srcFactorA = (BlendFactor)((key >> 8) & 0xF);
		b.dstFactorRGB = (BlendFactor)((key >> 4) & 0xF);
		b.dstFactorA = (BlendFactor)((key >> 0) & 0xF);
		return b;
	}
};

struct DepthState
{
	CompareMode compare = COMPARE_ALWAYS;
	bool write = false;

	bool operator == (const DepthState &d) const
	{
		return compare == d.compare && write == d.write;
	}
};

struct StencilState
{
	CompareMode compare = COMPARE_ALWAYS;
	StencilAction action = STENCIL_KEEP;
	int value = 0;
	uint32 readMask = 0xFFFFFFFF;
	uint32 writeMask = 0xFFFFFFFF;

	bool operator == (const StencilState &s) const
	{
		return compare == s.compare && action == s.action && value == s.value
			&& readMask == s.readMask && writeMask == s.writeMask;
	}
};

struct ColorChannelMask
{
	bool r = true;
	bool g = true;
	bool b = true;
	bool a = true;

	bool operator == (ColorChannelMask c) const
	{
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}

	bool operator != (ColorChannelMask c) const
	{
		return !(operator==(c));
	}
};

struct ScissorState
{
	Rect rect = {0, 0, 0, 0};
	bool enable = false;
};

BlendState computeBlendState(BlendMode mode, BlendAlpha alphamode);
BlendMode computeBlendMode(BlendState s, BlendAlpha &alphamode);
bool isAlphaMultiplyBlendSupported(BlendMode mode);

StencilState computeStencilState(StencilMode mode, int value);
StencilMode computeStencilMode(const StencilState &s);

/**
 * GPU APIs do the comparison in the opposite way of what makes sense for some
 * of love's APIs. For example in OpenGL if the compare function is GL_GREATER,
 * then the stencil test will pass if the reference value is greater than the
 * value in the stencil buffer. With our stencil API it's more intuitive to
 * assume that setStencilTest(COMPARE_GREATER, 4) will make it pass if the
 * stencil buffer has a value greater than 4.
 **/
CompareMode getReversedCompareMode(CompareMode mode);

STRINGMAP_DECLARE(BlendMode);
STRINGMAP_DECLARE(BlendAlpha);
STRINGMAP_DECLARE(BlendFactor);
STRINGMAP_DECLARE(BlendOperation);
STRINGMAP_DECLARE(StencilMode);
STRINGMAP_DECLARE(StencilAction);
STRINGMAP_DECLARE(CompareMode);

} // graphics
} // love
