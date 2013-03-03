/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#ifndef LOVE_MOUSE_WRAP_MOUSE_H
#define LOVE_MOUSE_WRAP_MOUSE_H

// LOVE
#include "Mouse.h"

namespace love
{
namespace mouse
{

int w_getX(lua_State *L);
int w_getY(lua_State *L);
int w_getPosition(lua_State *L);
int w_setX(lua_State *L);
int w_setY(lua_State *L);
int w_setPosition(lua_State *L);
int w_isDown(lua_State *L);
int w_setVisible(lua_State *L);
int w_isVisible(lua_State *L);
int w_setGrap(lua_State *L);
int w_isGrabbed(lua_State *L);
extern "C" LOVE_EXPORT int luaopen_love_mouse(lua_State *L);

} // mouse
} // love

#endif // LOVE_MOUSE_WRAP_MOUSE_H

