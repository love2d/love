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

#ifndef LOVE_JOYSTICK_SDL_WRAP_JOYSTICK_H
#define LOVE_JOYSTICK_SDL_WRAP_JOYSTICK_H

// LOVE
#include <common/config.h>
#include "Joystick.h"

namespace love
{
namespace joystick
{
namespace sdl
{
	int _wrap_getNumJoysticks(lua_State * L);
	int _wrap_getName(lua_State * L);
	int _wrap_open(lua_State * L);
	int _wrap_isOpen(lua_State * L);
	int _wrap_getNumAxes(lua_State * L);
	int _wrap_getNumBalls(lua_State * L);
	int _wrap_getNumButtons(lua_State * L);
	int _wrap_getNumHats(lua_State * L);
	int _wrap_getAxis(lua_State * L);
	int _wrap_getAxes(lua_State * L);
	int _wrap_getBall(lua_State * L);
	int _wrap_isDown(lua_State * L);
	int _wrap_getHat(lua_State * L);
	int _wrap_close(lua_State * L);
	int wrap_Joystick_open(lua_State * L);

} // sdl
} // joystick
} // love

extern "C" LOVE_EXPORT int luaopen_love_joystick(lua_State * L);

#endif // LOVE_JOYSTICK_SDL_WRAP_JOYSTICK_H
