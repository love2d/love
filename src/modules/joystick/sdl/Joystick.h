/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#ifndef LOVE_JOYSTICK_SDL_JOYSTICK_H
#define LOVE_JOYSTICK_SDL_JOYSTICK_H

// LOVE
#include <joystick/Joystick.h>
#include <common/EnumMap.h>

// SDL
#include <SDL.h>

namespace love
{
namespace joystick
{
namespace sdl
{
	class Joystick : public love::joystick::Joystick
	{
	private:
		SDL_Joystick ** joysticks;
	public:
		Joystick();
		virtual ~Joystick();

		// Implements Module.
		const char * getName() const;

		bool checkIndex(int index);
		int getNumJoysticks();
		const char * getName(int index);
		bool open(int index);
		bool isOpen(int index);
		bool verifyJoystick(int index);
		int getNumAxes(int index);
		int getNumBalls(int index);
		int getNumButtons(int index);
		int getNumHats(int index);
		float clampval(float x);
		float getAxis(int index, int axis);
		int getAxes(lua_State * L);
		int getBall(lua_State * L);
		bool isDown(int index, int button);
		Hat getHat(int index, int hat);
		void close(int index);

	private:

		static EnumMap<Hat, Uint8, Joystick::HAT_MAX_ENUM>::Entry hatEntries[];
		static EnumMap<Hat, Uint8, Joystick::HAT_MAX_ENUM> hats;

	}; // Joystick

} // sdl
} // joystick
} // love

#endif // LOVE_JOYSTICK_SDL_JOYSTICK_H