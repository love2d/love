/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "Joystick.h"

namespace love
{
namespace joystick
{

	Joystick::~Joystick()
	{
	}

	bool Joystick::getConstant(const char * in, Joystick::Hat & out)
	{
		return hats.find(in, out);
	}

	bool Joystick::getConstant(Joystick::Hat in, const char *& out)
	{
		return hats.find(in, out);
	}

	StringMap<Joystick::Hat, Joystick::HAT_MAX_ENUM>::Entry Joystick::hatEntries[] =
	{
		{"c", Joystick::HAT_CENTERED},
		{"u", Joystick::HAT_UP},
		{"r", Joystick::HAT_RIGHT},
		{"d", Joystick::HAT_DOWN},
		{"l", Joystick::HAT_LEFT},
		{"ru", Joystick::HAT_RIGHTUP},
		{"rd", Joystick::HAT_RIGHTDOWN},
		{"lu", Joystick::HAT_LEFTUP},
		{"ld", Joystick::HAT_LEFTDOWN},
	};

	StringMap<Joystick::Hat, Joystick::HAT_MAX_ENUM> Joystick::hats(Joystick::hatEntries, sizeof(Joystick::hatEntries));

} // joystick
} // love
