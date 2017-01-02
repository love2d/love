/**
 * Copyright (c) 2006-2017 LOVE Development Team
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
	if (p > std::numeric_limits<lua_Number>::max() ||
			p < std::numeric_limits<lua_Number>::min() ||
			p != p)
		return luaL_error(L, "Pitch has to be finite and not NaN.");
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
	float offset = (float)luaL_checknumber(L, 2);
	if (offset < 0)
		return luaL_argerror(L, 2, "can't seek to a negative position");

	Source::Unit u = Source::UNIT_SECONDS;
	const char *unit = lua_isnoneornil(L, 3) ? 0 : lua_tostring(L, 3);
	if (unit && !t->getConstant(unit, u))
		return luaL_error(L, "Invalid Source time unit: %s", unit);

	t->seek(offset, u);
	return 0;
}

int w_Source_tell(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	Source::Unit u = Source::UNIT_SECONDS;
	const char *unit = lua_isnoneornil(L, 2) ? 0 : lua_tostring(L, 2);
	if (unit && !t->getConstant(unit, u))
		return luaL_error(L, "Invalid Source time unit: %s", unit);

	lua_pushnumber(L, t->tell(u));
	return 1;
}

int w_Source_getDuration(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	Source::Unit u = Source::UNIT_SECONDS;
	const char *unit = lua_isnoneornil(L, 2) ? 0 : lua_tostring(L, 2);
	if (unit && !t->getConstant(unit, u))
		return luaL_error(L, "Invalid Source time unit: %s", unit);

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
	luax_catchexcept(L, [&](){ t->setCone(innerAngle, outerAngle, outerVolume); });
	return 0;
}

int w_Source_getCone(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	float innerAngle, outerAngle, outerVolume;
	luax_catchexcept(L, [&](){ t->getCone(innerAngle, outerAngle, outerVolume); });
	lua_pushnumber(L, innerAngle);
	lua_pushnumber(L, outerAngle);
	lua_pushnumber(L, outerVolume);
	return 3;
}

int w_Source_setRelative(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	luax_catchexcept(L, [&](){ t->setRelative(luax_toboolean(L, 2)); });
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
	t->setLooping(luax_toboolean(L, 2));
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

int w_Source_getChannels(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	lua_pushinteger(L, t->getChannels());
	return 1;
}

int w_Source_setFilter(lua_State *L)
{
	Source *t = luax_checksource(L, 1);

	Filter::Type type;
	std::vector<float> params;

	params.reserve(Filter::getParameterCount());

	if (lua_gettop(L) == 1)
	{
		lua_pushboolean(L, t->setFilter());
		return 1;
	}
	else if (lua_gettop(L) > 2)
	{
		const char *ftypestr = luaL_checkstring(L, 2);
		if (!Filter::getConstant(ftypestr, type))
			return luaL_error(L, "Invalid filter type: %s", ftypestr);

		unsigned int count = Filter::getParameterCount(type);
		for (unsigned int i = 0; i < count; i++)
			params.push_back(luaL_checknumber(L, i + 3));
	}
	else if (lua_istable(L, 2))
	{
		if (lua_objlen(L, 2) == 0) //empty table also clears filter
		{
			lua_pushboolean(L, t->setFilter());
			return 1;
		}
		lua_rawgeti(L, 2, 1);
		const char *ftypestr = luaL_checkstring(L, -1);
		if (!Filter::getConstant(ftypestr, type))
			return luaL_error(L, "Invalid filter type: %s", ftypestr);
		lua_pop(L, 1);

		unsigned int count = Filter::getParameterCount(type);
		for (unsigned int i = 0; i < count; i++)
		{
			lua_rawgeti(L, 2, i + 2);
			params.push_back(luaL_checknumber(L, -1));
			lua_pop(L, 1);
		}
	}
	else
		return luax_typerror(L, 2, "filter description");

	luax_catchexcept(L, [&]() { lua_pushboolean(L, t->setFilter(type, params)); });
	return 1;
}

int w_Source_getFilter(lua_State *L)
{
	Source *t = luax_checksource(L, 1);
	Filter::Type type;
	std::vector<float> params;
	if (!t->getFilter(type, params))
		return 0;

	const char *str = nullptr;
	Filter::getConstant(type, str);
	lua_pushstring(L, str);

	for (unsigned int i = 0; i < params.size(); i++)
		lua_pushnumber(L, params[i]);

	return params.size() + 1;
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
			            s->getSampleRate(), s->getBitDepth(), s->getChannels());
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
		return luax_typerror(L, 1, "Sound Data or lightuserdata");

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
	{ "setRolloff", w_Source_setRolloff},
	{ "getRolloff", w_Source_getRolloff},

	{ "getChannels", w_Source_getChannels },

	{ "setFilter", w_Source_setFilter },
	{ "getFilter", w_Source_getFilter },

	{ "getFreeBufferCount", w_Source_getFreeBufferCount },
	{ "queue", w_Source_queue },

	{ "getType", w_Source_getType },

	{ 0, 0 }
};

extern "C" int luaopen_source(lua_State *L)
{
	return luax_register_type(L, &love::audio::Source::type, w_Source_functions, nullptr);
}

} // audio
} // love
