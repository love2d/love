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

#pragma once

#include <vector>
#include <string>

namespace love
{
namespace graphics
{

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
	COMPARE_NEVER,
	COMPARE_MAX_ENUM
};

/**
 * GPU APIs do the comparison in the opposite way of what makes sense for some
 * of love's APIs. For example in OpenGL if the compare function is GL_GREATER,
 * then the stencil test will pass if the reference value is greater than the
 * value in the stencil buffer. With our stencil API it's more intuitive to
 * assume that setStencilTest(COMPARE_GREATER, 4) will make it pass if the
 * stencil buffer has a value greater than 4.
 **/
CompareMode getReversedCompareMode(CompareMode mode);

bool getConstant(const char *in, StencilAction &out);
bool getConstant(StencilAction in, const char *&out);
std::vector<std::string> getConstants(StencilAction);

bool getConstant(const char *in, CompareMode &out);
bool getConstant(CompareMode in, const char *&out);
std::vector<std::string> getConstants(CompareMode);

} // graphics
} // love
