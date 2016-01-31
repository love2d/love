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

#include "Reference.h"

namespace love
{

const char REFERENCE_TABLE_NAME[] = "love-references";

Reference::Reference()
	: pinnedL(nullptr)
	, idx(LUA_REFNIL)
{
}

Reference::Reference(lua_State *L)
	: pinnedL(nullptr)
	, idx(LUA_REFNIL)
{
	ref(L);
}

Reference::~Reference()
{
	unref();
}

void Reference::ref(lua_State *L)
{
	unref(); // Just to be safe.
	pinnedL = luax_getpinnedthread(L);
	luax_insist(L, LUA_REGISTRYINDEX, REFERENCE_TABLE_NAME);
	lua_insert(L, -2); // Move reference table behind value.
	idx = luaL_ref(L, -2);
	lua_pop(L, 1);
}

void Reference::unref()
{
	if (idx != LUA_REFNIL)
	{
		// We use a pinned thread/coroutine for the Lua state because we know it
		// hasn't been garbage collected and is valid, as long as the whole lua
		// state is still open.
		luax_insist(pinnedL, LUA_REGISTRYINDEX, REFERENCE_TABLE_NAME);
		luaL_unref(pinnedL, -1, idx);
		lua_pop(pinnedL, 1);
		idx = LUA_REFNIL;
	}
}

void Reference::push(lua_State *L)
{
	if (idx != LUA_REFNIL)
	{
		luax_insist(L, LUA_REGISTRYINDEX, REFERENCE_TABLE_NAME);
		lua_rawgeti(L, -1, idx);
		lua_remove(L, -2);
	}
	else
		lua_pushnil(L);
}

} // love
