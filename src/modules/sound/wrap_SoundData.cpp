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

#include "wrap_SoundData.h"

#include "common/wrap_Data.h"

// Shove the wrap_SoundData.lua code directly into a raw string literal.
static const char sounddata_lua[] =
#include "wrap_SoundData.lua"
;

namespace love
{
namespace sound
{

/**
 * NOTE: Additional wrapper code is in wrap_SoundData.lua. Be sure to keep it
 * in sync with any changes made to this file!
 **/

SoundData *luax_checksounddata(lua_State *L, int idx)
{
	return luax_checktype<SoundData>(L, idx, SOUND_SOUND_DATA_ID);
}

int w_SoundData_getChannels(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushinteger(L, t->getChannels());
	return 1;
}

int w_SoundData_getBitDepth(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushinteger(L, t->getBitDepth());
	return 1;
}

int w_SoundData_getSampleRate(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushinteger(L, t->getSampleRate());
	return 1;
}

int w_SoundData_getSampleCount(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushinteger(L, t->getSampleCount());
	return 1;
}

int w_SoundData_getDuration(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushnumber(L, t->getDuration());
	return 1;
}

int w_SoundData_setSample(lua_State *L)
{
	SoundData *sd = luax_checksounddata(L, 1);
	int i = (int) luaL_checkinteger(L, 2);
	float sample = (float) luaL_checknumber(L, 3);

	luax_catchexcept(L, [&](){ sd->setSample(i, sample); });
	return 0;
}

int w_SoundData_getSample(lua_State *L)
{
	SoundData *sd = luax_checksounddata(L, 1);
	int i = (int) luaL_checkinteger(L, 2);

	luax_catchexcept(L, [&](){ lua_pushnumber(L, sd->getSample(i)); });
	return 1;
}

static const luaL_Reg w_SoundData_functions[] =
{
	{ "getChannels", w_SoundData_getChannels },
	{ "getBitDepth", w_SoundData_getBitDepth },
	{ "getSampleRate", w_SoundData_getSampleRate },
	{ "getSampleCount", w_SoundData_getSampleCount },
	{ "getDuration", w_SoundData_getDuration },
	{ "setSample", w_SoundData_setSample },
	{ "getSample", w_SoundData_getSample },
	{ 0, 0 }
};

extern "C" int luaopen_sounddata(lua_State *L)
{
	int ret = luax_register_type(L, SOUND_SOUND_DATA_ID, "SoundData", w_Data_functions, w_SoundData_functions, nullptr);

	luax_gettypemetatable(L, SOUND_SOUND_DATA_ID);

	// Load and execute SoundData.lua, sending the metatable as an argument.
	if (lua_istable(L, -1))
	{
		luaL_loadbuffer(L, sounddata_lua, sizeof(sounddata_lua), "SoundData.lua");
		lua_pushvalue(L, -2);
		lua_call(L, 1, 0);
	}

	// Pop the metatable.
	lua_pop(L, 1);

	return ret;
}

} // sound
} // love
