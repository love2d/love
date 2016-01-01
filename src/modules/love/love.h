/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_LOVE_H
#define LOVE_LOVE_H

// LOVE
#include "common/config.h"

// Forward declare lua_State.
struct lua_State;

#ifdef __cplusplus
extern "C"
{
#endif

LOVE_EXPORT const char *love_version();
LOVE_EXPORT const char *love_codename();
LOVE_EXPORT int luaopen_love(lua_State *L);
LOVE_EXPORT int luaopen_love_nogame(lua_State *L);
LOVE_EXPORT int luaopen_love_boot(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // LOVE_LOVE_H
