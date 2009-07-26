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

#include "wrap_World.h"

namespace love
{
namespace physics
{
namespace box2d
{

	World * luax_checkworld(lua_State * L, int idx)
	{
		return luax_checktype<World>(L, idx, "World", LOVE_PHYSICS_WORLD_BITS);
	}

	int _wrap_World_update(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		float dt = (float)luaL_checknumber(L, 2);
		t->update(dt);
		return 0;
	}

	int _wrap_World_setCallback(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		lua_remove(L, 1);
		return t->setCallback(L);
	}

	int _wrap_World_getCallback(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		lua_remove(L, 1);
		return t->getCallback(L);
	}

	int _wrap_World_setGravity(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setGravity(arg1, arg2);
		return 0;
	}

	int _wrap_World_getGravity(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		lua_remove(L, 1);
		return t->getGravity(L);
	}

	int _wrap_World_setAllowSleep(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		bool b = luax_toboolean(L, 2);
		t->setAllowSleep(b);
		return 0;
	}

	int _wrap_World_isAllowSleep(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		luax_pushboolean(L, t->isAllowSleep());
		return 1;
	}

	int _wrap_World_getBodyCount(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		lua_pushinteger(L, t->getBodyCount());
		return 1;
	}

	int _wrap_World_getJointCount(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		lua_pushinteger(L, t->getJointCount());
		return 1;
	}
	
	int _wrap_World_setMeter(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		int arg1 = luaL_checkint(L, 2);
		t->setMeter(arg1);
		return 0;
		
	}
	int _wrap_World_getMeter(lua_State * L)
	{
		World * t = luax_checkworld(L, 1);
		lua_pushinteger(L, t->getMeter());
		return 1;
	}
	
	static const luaL_Reg wrap_World_functions[] = {
		{ "update", _wrap_World_update },
		{ "setCallback", _wrap_World_setCallback },
		{ "getCallback", _wrap_World_getCallback },
		{ "setGravity", _wrap_World_setGravity },
		{ "getGravity", _wrap_World_getGravity },
		{ "setAllowSleep", _wrap_World_setAllowSleep },
		{ "isAllowSleep", _wrap_World_isAllowSleep },
		{ "getBodyCount", _wrap_World_getBodyCount },
		{ "getJointCount", _wrap_World_getJointCount },
		{ "setMeter", _wrap_World_setMeter },
		{ "getMeter", _wrap_World_getMeter },
		{ 0, 0 }
	};

	int wrap_World_open(lua_State * L)
	{
		luax_register_type(L, "World", wrap_World_functions);
		return 0;
	}

} // box2d
} // physics
} // love
