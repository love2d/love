/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include <limits>

#include "sound/SoundData.h"
#include "wrap_Source.h"

#include <cmath>
#include <iostream>

namespace love
{
namespace audio
{

Source *luax_checksource(lua_State *L, int idx)
{
	return luax_checktype<Source>(L, idx);
}

int w_Source_clone(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	Source *clone = nullptr;
	luax_catchexcept(L, [&](){ clone = t->clone(); });
	luax_pushtype(L, clone);
	clone->release();
	return 1;
}

int w_Source_play(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_pushboolean(L, t->play());
	return 1;
}

int w_Source_stop(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	t->stop();
	return 0;
}

int w_Source_pause(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	t->pause();
	return 0;
}

int w_Source_setPitch(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float p = (float)luaL_checknumber(L, 2);
	if (p != p)
		return luaL_error(L, "Pitch cannot be NaN.");
	if (p > std::numeric_limits<lua_Number>::max() ||
			p <= 0.0f)
		return luaL_error(L, "Pitch has to be non-zero, positive, finite number.");
	t->setPitch(p);
	return 0;
}

int w_Source_getPitch(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	lua_pushnumber(L, t->getPitch());
	return 1;
}

int w_Source_setVolume(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float p = (float)luaL_checknumber(L, 2);
	t->setVolume(p);
	return 0;
}

int w_Source_getVolume(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	lua_pushnumber(L, t->getVolume());
	return 1;
}

int w_Source_seek(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	double offset = luaL_checknumber(L, 2);
	if (offset < 0)
		return luaL_argerror(L, 2, "can't seek to a negative position");

	Source::Unit u = Source::UNIT_SECONDS;
	const char *unit = lua_isnoneornil(L, 3) ? 0 : lua_tostring(L, 3);
	if (unit && !t->getConstant(unit, u))
		return luax_enumerror(L, "time unit", Source::getConstants(u), unit);

	t->seek(offset, u);
	return 0;
}

int w_Source_tell(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	Source::Unit u = Source::UNIT_SECONDS;
	const char *unit = lua_isnoneornil(L, 2) ? 0 : lua_tostring(L, 2);
	if (unit && !t->getConstant(unit, u))
		return luax_enumerror(L, "time unit", Source::getConstants(u), unit);

	lua_pushnumber(L, t->tell(u));
	return 1;
}

int w_Source_getDuration(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	Source::Unit u = Source::UNIT_SECONDS;
	const char *unit = lua_isnoneornil(L, 2) ? 0 : lua_tostring(L, 2);
	if (unit && !t->getConstant(unit, u))
		return luax_enumerror(L, "time unit", Source::getConstants(u), unit);

	lua_pushnumber(L, t->getDuration(u));
	return 1;
}

int w_Source_setPosition(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float v[3];
	v[0] = (float)luaL_checknumber(L, 2);
	v[1] = (float)luaL_checknumber(L, 3);
	v[2] = (float)luaL_optnumber(L, 4, 0);
	luax_catchexcept(L, [&](){ t->setPosition(v); });
	return 0;
}

int w_Source_getPosition(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float v[3];
	luax_catchexcept(L, [&](){ t->getPosition(v); });
	lua_pushnumber(L, v[0]);
	lua_pushnumber(L, v[1]);
	lua_pushnumber(L, v[2]);
	return 3;
}

int w_Source_setVelocity(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float v[3];
	v[0] = (float)luaL_checknumber(L, 2);
	v[1] = (float)luaL_checknumber(L, 3);
	v[2] = (float)luaL_optnumber(L, 4, 0);
	luax_catchexcept(L, [&](){ t->setVelocity(v); });
	return 0;
}

int w_Source_getVelocity(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float v[3];
	luax_catchexcept(L, [&](){ t->getVelocity(v); });
	lua_pushnumber(L, v[0]);
	lua_pushnumber(L, v[1]);
	lua_pushnumber(L, v[2]);
	return 3;
}

int w_Source_setDirection(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float v[3];
	v[0] = (float)luaL_checknumber(L, 2);
	v[1] = (float)luaL_checknumber(L, 3);
	v[2] = (float)luaL_optnumber(L, 4, 0);
	luax_catchexcept(L, [&](){ t->setDirection(v); });
	return 0;
}

int w_Source_getDirection(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float v[3];
	luax_catchexcept(L, [&](){ t->getDirection(v); });
	lua_pushnumber(L, v[0]);
	lua_pushnumber(L, v[1]);
	lua_pushnumber(L, v[2]);
	return 3;
}

int w_Source_setCone(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float innerAngle = (float) luaL_checknumber(L, 2);
	float outerAngle = (float) luaL_checknumber(L, 3);
	float outerVolume = (float) luaL_optnumber(L, 4, 0.0);
	float outerHighGain = (float) luaL_optnumber(L, 5, 1.0);
	luax_catchexcept(L, [&](){ t->setCone(innerAngle, outerAngle, outerVolume, outerHighGain); });
	return 0;
}

int w_Source_getCone(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float innerAngle, outerAngle, outerVolume, outerHighGain;
	luax_catchexcept(L, [&](){ t->getCone(innerAngle, outerAngle, outerVolume, outerHighGain); });
	lua_pushnumber(L, innerAngle);
	lua_pushnumber(L, outerAngle);
	lua_pushnumber(L, outerVolume);
	lua_pushnumber(L, outerHighGain);
	return 4;
}

int w_Source_setRelative(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&](){ t->setRelative(luax_checkboolean(L, 2)); });
	return 0;
}

int w_Source_isRelative(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&](){ luax_pushboolean(L, t->isRelative()); });
	return 1;
}

int w_Source_setLooping(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&](){ t->setLooping(luax_checkboolean(L, 2)); });
	return 0;
}

int w_Source_isLooping(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_pushboolean(L, t->isLooping());
	return 1;
}

int w_Source_isPlaying(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_pushboolean(L, t->isPlaying());
	return 1;
}

int w_Source_setVolumeLimits(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float vmin = (float)luaL_checknumber(L, 2);
	float vmax = (float)luaL_checknumber(L, 3);
	if (vmin < .0f || vmin > 1.f || vmax < .0f || vmax > 1.f)
		return luaL_error(L, "Invalid volume limits: [%f:%f]. Must be in [0:1]", vmin, vmax);
	t->setMinVolume(vmin);
	t->setMaxVolume(vmax);
	return 0;
}

int w_Source_getVolumeLimits(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	lua_pushnumber(L, t->getMinVolume());
	lua_pushnumber(L, t->getMaxVolume());
	return 2;
}

int w_Source_setAttenuationDistances(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float dref = (float)luaL_checknumber(L, 2);
	float dmax = (float)luaL_checknumber(L, 3);
	if (dref < .0f || dmax < .0f)
		return luaL_error(L, "Invalid distances: %f, %f. Must be > 0", dref, dmax);
	luax_catchexcept(L, [&]() {
		t->setReferenceDistance(dref);
		t->setMaxDistance(dmax);
	});
	return 0;
}

int w_Source_getAttenuationDistances(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&]() {
		lua_pushnumber(L, t->getReferenceDistance());
		lua_pushnumber(L, t->getMaxDistance());
	});
	return 2;
}

int w_Source_setRolloff(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float rolloff = (float)luaL_checknumber(L, 2);
	if (rolloff < .0f)
		return luaL_error(L, "Invalid rolloff: %f. Must be > 0.", rolloff);
	luax_catchexcept(L, [&](){ t->setRolloffFactor(rolloff); });
	return 0;
}

int w_Source_getRolloff(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&](){ lua_pushnumber(L, t->getRolloffFactor()); });
	return 1;
}

int w_Source_setAirAbsorption(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float factor = (float)luaL_checknumber(L, 2);
	if (factor < 0.0f)
		return luaL_error(L, "Invalid air absorption factor: %f. Must be > 0.", factor);
	luax_catchexcept(L, [&](){ t->setAirAbsorptionFactor(factor); });
	return 0;
}

int w_Source_getAirAbsorption(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&](){ lua_pushnumber(L, t->getAirAbsorptionFactor()); });
	return 1;
}

int w_Source_getChannelCount(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	lua_pushinteger(L, t->getChannelCount());
	return 1;
}

int setFilterReadFilter(lua_State *L, int idx, std::map<Filter::Parameter, float> &params)
{
	if (lua_gettop(L) < idx || lua_isnoneornil(L, idx))
		return 0;

	luaL_checktype(L, idx, LUA_TTABLE);

	const char *paramstr = nullptr;

	Filter::getConstant(Filter::FILTER_TYPE, paramstr, Filter::TYPE_BASIC);
	lua_pushstring(L, paramstr);
	lua_rawget(L, idx);
	if (lua_type(L, -1) == LUA_TNIL)
		return luaL_error(L, "Filter type not specificed.");

	Filter::Type type = Filter::TYPE_MAX_ENUM;
	const char *typestr = luaL_checkstring(L, -1);
	if (!Filter::getConstant(typestr, type))
		return luax_enumerror(L, "filter type", Filter::getConstants(type), typestr);

	lua_pop(L, 1);
	params[Filter::FILTER_TYPE] = static_cast<int>(type);

	lua_pushnil(L);
	while (lua_next(L, idx))
	{
		const char *keystr = luaL_checkstring(L, -2);
		Filter::Parameter param;

		if(Filter::getConstant(keystr, param, type) || Filter::getConstant(keystr, param, Filter::TYPE_BASIC))
		{
#define luax_effecterror(l,t) luaL_error(l,"Bad parameter type for %s %s: " t " expected, got %s", typestr, keystr, lua_typename(L, -1))
			switch(Filter::getParameterType(param))
			{
			case Filter::PARAM_FLOAT:
				if (!lua_isnumber(L, -1))
					return luax_effecterror(L, "number");
				params[param] = lua_tonumber(L, -1);
				break;
			case Filter::PARAM_TYPE:
			case Filter::PARAM_MAX_ENUM:
				break;
			}
#undef luax_effecterror
		}
		else
			luaL_error(L, "Invalid '%s' Effect parameter: %s", typestr, keystr);

		//remove the value (-1) from stack, keep the key (-2) to feed into lua_next
		lua_pop(L, 1);
	}

	return 1;
}

void getFilterWriteFilter(lua_State *L, int idx, std::map<Filter::Parameter, float> &params)
{
	const char *keystr, *valstr;
	Filter::Type type = static_cast<Filter::Type>((int)params[Filter::FILTER_TYPE]);

	if (lua_istable(L, idx))
		lua_pushvalue(L, idx);
	else
		lua_createtable(L, 0, params.size());

	for (auto p : params)
	{
		if (!Filter::getConstant(p.first, keystr, type))
			Filter::getConstant(p.first, keystr, Filter::TYPE_BASIC);

		lua_pushstring(L, keystr);
		switch (Filter::getParameterType(p.first))
		{
		case Filter::PARAM_FLOAT:
			lua_pushnumber(L, p.second);
			break;
		case Filter::PARAM_TYPE:
			Filter::getConstant(static_cast<Filter::Type>((int)p.second), valstr);
			lua_pushstring(L, valstr);
			break;
		case Filter::PARAM_MAX_ENUM:
			break;
		}
		lua_rawset(L, -3);
	}
}

int w_Source_setFilter(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	std::map<Filter::Parameter, float> params;

	if (setFilterReadFilter(L, 2, params) == 1)
		luax_catchexcept(L, [&]() { lua_pushboolean(L, t->setFilter(params)); });
	else
		luax_catchexcept(L, [&]() { lua_pushboolean(L, t->setFilter()); });

	return 1;
}

int w_Source_getFilter(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	std::map<Filter::Parameter, float> params;

	if (!t->getFilter(params))
		return 0;

	getFilterWriteFilter(L, 2, params);
	return 1;
}

int w_Source_setEffect(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	const char *namestr = luaL_checkstring(L, 2);

	const bool isBool = lua_gettop(L) >= 3 && lua_isboolean(L, 3);

	// :setEffect(effect, false) = clear effect
	if (isBool && !lua_toboolean(L, 3))
	{
		luax_catchexcept(L, [&]() { lua_pushboolean(L, t->unsetEffect(namestr)); });
		return 1;
	}

	std::map<Filter::Parameter, float> params;

	// :setEffect(effect, [true]) = set effect without filter
	if (isBool || setFilterReadFilter(L, 3, params) == 0)
		luax_catchexcept(L, [&]() { lua_pushboolean(L, t->setEffect(namestr)); });
	else
		luax_catchexcept(L, [&]() { lua_pushboolean(L, t->setEffect(namestr, params)); });
	return 1;
}

int w_Source_getEffect(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	const char *namestr = luaL_checkstring(L, 2);

	std::map<Filter::Parameter, float> params;
	if (!t->getEffect(namestr, params))
	{
		luax_pushboolean(L, false);
		return 1;
	}

	luax_pushboolean(L, true);

	// No filter associated, return nil as second argument
	if (params.size() == 0)
		return 1;

	// Return filter settings as second argument
	getFilterWriteFilter(L, 3, params);
	return 2;
}

int w_Source_getActiveEffects(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	std::vector<std::string> list;
	t->getActiveEffects(list);

	lua_createtable(L, 0, (int) list.size());
	for (int i = 0; i < (int) list.size(); i++)
	{
		lua_pushnumber(L, i + 1);
		lua_pushstring(L, list[i].c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

int w_Source_getFreeBufferCount(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	lua_pushinteger(L, t->getFreeBufferCount());
	return 1;
}

int w_Source_queue(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	bool success;

	if (luax_istype(L, 2, love::sound::SoundData::type))
	{
		auto s = luax_totype<love::sound::SoundData>(L, 2);

		int offset = 0;
		size_t length = s->getSize();

		if (lua_gettop(L) == 4)
		{
			offset = luaL_checknumber(L, 3);
			length = luaL_checknumber(L, 4);
		}
		else if (lua_gettop(L) == 3)
			length = luaL_checknumber(L, 3);

		if (offset < 0 || length > s->getSize() - offset)
			return luaL_error(L, "Data region out of bounds.");

		luax_catchexcept(L, [&]() {
			success = t->queue((unsigned char *)s->getData() + offset, length,
			            s->getSampleRate(), s->getBitDepth(), s->getChannelCount());
		});
	}
	else if (lua_islightuserdata(L, 2))
	{
		int offset = luaL_checknumber(L, 3);
		int length = luaL_checknumber(L, 4);
		int sampleRate = luaL_checknumber(L, 5);
		int bitDepth = luaL_checknumber(L, 6);
		int channels = luaL_checknumber(L, 7);

		if (length < 0 || offset < 0)
			return luaL_error(L, "Data region out of bounds.");

		luax_catchexcept(L, [&]() {
			success = t->queue((void*)((uintptr_t)lua_touserdata(L, 2) + (uintptr_t)offset), length, sampleRate, bitDepth, channels);
		});
	}
	else
		return luax_typerror(L, 2, "SoundData or lightuserdata");

	luax_pushboolean(L, success);
	return 1;
}

int w_Source_getType(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	Source::Type type = t->getType();
	const char *str = nullptr;

	if (!Source::getConstant(type, str))
		return luaL_error(L, "Unknown Source type.");

	lua_pushstring(L, str);
	return 1;
}

// Deprecated

int w_Source_getChannels(lua_State *L)
{
	luax_markdeprecated(L, "Source:getChannels", API_METHOD, DEPRECATED_RENAMED, "Source:getChannelCount");
	return w_Source_getChannelCount(L);
}

static const luaL_Reg w_Source_functions[] =
{
	{ "clone", w_Source_clone },

	{ "play", w_Source_play },
	{ "stop", w_Source_stop },
	{ "pause", w_Source_pause },

	{ "setPitch", w_Source_setPitch },
	{ "getPitch", w_Source_getPitch },
	{ "setVolume", w_Source_setVolume },
	{ "getVolume", w_Source_getVolume },
	{ "seek", w_Source_seek },
	{ "tell", w_Source_tell },
	{ "getDuration", w_Source_getDuration },
	{ "setPosition", w_Source_setPosition },
	{ "getPosition", w_Source_getPosition },
	{ "setVelocity", w_Source_setVelocity },
	{ "getVelocity", w_Source_getVelocity },
	{ "setDirection", w_Source_setDirection },
	{ "getDirection", w_Source_getDirection },
	{ "setCone", w_Source_setCone },
	{ "getCone", w_Source_getCone },

	{ "setRelative", w_Source_setRelative },
	{ "isRelative", w_Source_isRelative },

	{ "setLooping", w_Source_setLooping },
	{ "isLooping", w_Source_isLooping },
	{ "isPlaying", w_Source_isPlaying },

	{ "setVolumeLimits", w_Source_setVolumeLimits },
	{ "getVolumeLimits", w_Source_getVolumeLimits },
	{ "setAttenuationDistances", w_Source_setAttenuationDistances },
	{ "getAttenuationDistances", w_Source_getAttenuationDistances },
	{ "setRolloff", w_Source_setRolloff },
	{ "getRolloff", w_Source_getRolloff },
	{ "setAirAbsorption", w_Source_setAirAbsorption },
	{ "getAirAbsorption", w_Source_getAirAbsorption },

	{ "getChannelCount", w_Source_getChannelCount },

	{ "setFilter", w_Source_setFilter },
	{ "getFilter", w_Source_getFilter },
	{ "setEffect", w_Source_setEffect },
	{ "getEffect", w_Source_getEffect },
	{ "getActiveEffects", w_Source_getActiveEffects },

	{ "getFreeBufferCount", w_Source_getFreeBufferCount },
	{ "queue", w_Source_queue },

	{ "getType", w_Source_getType },

	// Deprecated
	{ "getChannels", w_Source_getChannels },

	{ 0, 0 }
};

extern "C" int luaopen_source(lua_State *L)
{
	return luax_register_type(L, &love::audio::Source::type, w_Source_functions, nullptr);
}

} // audio
} // love
