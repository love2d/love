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

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"

#include <stdlib.h>
#include <string.h>

// Lua
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* 
* MSVC did not require that every funtion be
* cast to (void*), but gcc did. Ah well, this
* is shorter than {"funcname", funcname} anyway.
*/
#define FVOID(f) { #f, (void*)f }

const tcc_function tcc_api[] =
{

	// cstring
	FVOID(memcpy),
	FVOID(memmove),
	FVOID(strcpy),
	FVOID(strncpy),
	FVOID(strcat),
	FVOID(strncat),
	FVOID(memcmp),
	FVOID(strcmp),
	FVOID(strcoll),
	FVOID(strncmp),
	FVOID(strxfrm),
	FVOID(memchr),
	FVOID(strchr),
	FVOID(strcspn),
	FVOID(strpbrk),
	FVOID(strrchr),
	FVOID(strspn),
	FVOID(strstr),
	FVOID(strtok),
	FVOID(memset),
	FVOID(strerror),
	FVOID(strlen),

	// Lua (some of it)
	FVOID(lua_isnumber),
	FVOID(lua_isstring),
	FVOID(lua_iscfunction),
	FVOID(lua_isuserdata),
	FVOID(lua_type),
	FVOID(lua_typename),

	FVOID(lua_equal),
	FVOID(lua_rawequal),
	FVOID(lua_lessthan),

	FVOID(lua_tonumber),
	FVOID(lua_tointeger),
	FVOID(lua_toboolean),
	FVOID(lua_tolstring),
	FVOID(lua_objlen),
	FVOID(lua_tocfunction),
	FVOID(lua_touserdata),
	FVOID(lua_tothread),
	FVOID(lua_topointer),

	FVOID(lua_pushnil),
	FVOID(lua_pushnumber),
	FVOID(lua_pushinteger),
	FVOID(lua_pushlstring),
	FVOID(lua_pushstring),
	FVOID(lua_pushvfstring),

	FVOID(lua_pushfstring),
	FVOID(lua_pushcclosure),
	FVOID(lua_pushboolean),
	FVOID(lua_pushlightuserdata),
	FVOID(lua_pushthread),

	FVOID(luaL_register),
	FVOID(luaL_error),

	// End
	{ 0, 0 }
};

#ifdef __cplusplus
}
#endif