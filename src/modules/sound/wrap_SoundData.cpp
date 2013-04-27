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

#include "wrap_SoundData.h"

#include "common/wrap_Data.h"

namespace love
{
namespace sound
{

SoundData *luax_checksounddata(lua_State *L, int idx)
{
	return luax_checktype<SoundData>(L, idx, "SoundData", SOUND_SOUND_DATA_T);
}

int w_SoundData_getChannels(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushinteger(L, t->getChannels());
	return 1;
}

int w_SoundData_getBits(lua_State *L)
{
	SoundData *t = luax_checksounddata(L, 1);
	lua_pushinteger(L, t->getBits());
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
	int i = (int)lua_tointeger(L, 2);
	float sample = (float)lua_tonumber(L, 3);
	sd->setSample(i, sample);
	return 0;
}

int w_SoundData_getSample(lua_State *L)
{
	SoundData *sd = luax_checksounddata(L, 1);
	int i = (int)lua_tointeger(L, 2);
	lua_pushnumber(L, sd->getSample(i));
	return 1;
}

static const luaL_Reg functions[] =
{
	// Data
	{ "getString", w_Data_getString },
	{ "getSize", w_Data_getSize },

	{ "getChannels", w_SoundData_getChannels },
	{ "getBits", w_SoundData_getBits },
	{ "getSampleRate", w_SoundData_getSampleRate },
	{ "getSampleCount", w_SoundData_getSampleCount },
	{ "getDuration", w_SoundData_getDuration },
	{ "setSample", w_SoundData_setSample },
	{ "getSample", w_SoundData_getSample },
	{ 0, 0 }
};

extern "C" int luaopen_sounddata(lua_State *L)
{
	return luax_register_type(L, "SoundData", functions);
}

} // sound
} // love
