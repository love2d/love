/**
* Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_JOYSTICK_JOYSTICK_H
#define LOVE_JOYSTICK_JOYSTICK_H

// LOVE
#include <common/Module.h>
#include <common/StringMap.h>

namespace love
{
namespace joystick
{
	class Joystick : public Module
	{
	public:

		enum Hat
		{
			HAT_INVALID,
			HAT_CENTERED,
			HAT_UP,
			HAT_RIGHT,
			HAT_DOWN,
			HAT_LEFT,
			HAT_RIGHTUP,
			HAT_RIGHTDOWN,
			HAT_LEFTUP,
			HAT_LEFTDOWN,
			HAT_MAX_ENUM = 16
		};

		virtual ~Joystick();

		static bool getConstant(const char * in, Hat & out);
		static bool getConstant(Hat in, const char *& out);

	private:

		static StringMap<Hat, HAT_MAX_ENUM>::Entry hatEntries[];
		static StringMap<Hat, HAT_MAX_ENUM> hats;

	}; // Joystick

} // joystick
} // love

#endif // LOVE_JOYSTICK_JOYSTICK_H