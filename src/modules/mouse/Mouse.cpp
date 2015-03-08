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

#include "Mouse.h"

namespace love
{
namespace mouse
{

bool Mouse::getConstant(const char *in, Button &out)
{
	return buttons.find(in, out);
}

bool Mouse::getConstant(Button in, const char  *&out)
{
	return buttons.find(in, out);
}

StringMap<Mouse::Button, Mouse::BUTTON_MAX_ENUM>::Entry Mouse::buttonEntries[] =
{
	{"l", BUTTON_LEFT},
	{"m", BUTTON_MIDDLE},
	{"r", BUTTON_RIGHT},
	{"x1", BUTTON_X1},
	{"x2", BUTTON_X2},
	{"x3", BUTTON_X3},
	{"x4", BUTTON_X4},
	{"x5", BUTTON_X5},
};

StringMap<Mouse::Button, Mouse::BUTTON_MAX_ENUM> Mouse::buttons(Mouse::buttonEntries, sizeof(Mouse::buttonEntries));

} // mouse
} // love
