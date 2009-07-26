/**
* Copyright (c) 2006-2009 LOVE Development Team
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

namespace love
{
namespace joystick
{
	class Joystick : public Module
	{
	public:

		enum JoystickAxis
		{
			JOYSTICK_AXIS_HORIZONTAL = 0,
			JOYSTICK_AXIS_VERITCAL = 1,
		};

		enum JoystickHat
		{
			JOYSTICK_HAT_CENTERED = 0,
			JOYSTICK_HAT_UP = 1,
			JOYSTICK_HAT_RIGHT = 2,
			JOYSTICK_HAT_DOWN = 4,
			JOYSTICK_HAT_LEFT = 8,
			JOYSTICK_HAT_RIGHTUP = (2|1),
			JOYSTICK_HAT_RIGHTDOWN = (2|4),
			JOYSTICK_HAT_LEFTUP = (8|1),
			JOYSTICK_HAT_LEFTDOWN = (8|4)
		};

		virtual ~Joystick(){};

	}; // Joystick

} // joystick
} // love

#endif // LOVE_JOYSTICK_JOYSTICK_H