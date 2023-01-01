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

// LOVE
#include "wrap_Audio.h"

#include "openal/Audio.h"
#include "null/Audio.h"

#include "common/runtime.h"

// C++
#include <iostream>
#include <cmath>

namespace love
{
namespace audio
{

#define instance() (Module::getInstance<Audio>(Module::M_AUDIO))

int w_getActiveSourceCount(lua_State *L)
{
	lua_pushinteger(L, instance()->getActiveSourceCount());
	return 1;
}

int w_newSource(lua_State *L)
{
	Source::Type stype = Source::TYPE_STREAM;

	if (!luax_istype(L, 1, love::sound::SoundData::type) && !luax_istype(L, 1, love::sound::Decoder::type))
	{
		const char *stypestr = luaL_checkstring(L, 2);
		if (stypestr && !Source::getConstant(stypestr, stype))
			return luax_enumerror(L, "source type", Source::getConstants(stype), stypestr);

		if (stype == Source::TYPE_QUEUE)
			return luaL_error(L, "Cannot create queueable sources using newSource. Use newQueueableSource instead.");
	}

	if (lua_isstring(L, 1) || luax_istype(L, 1, love::filesystem::File::type) || luax_istype(L, 1, love::filesystem::FileData::type))
		luax_convobj(L, 1, "sound", "newDecoder");

	if (stype == Source::TYPE_STATIC && luax_istype(L, 1, love::sound::Decoder::type))
		luax_convobj(L, 1, "sound", "newSoundData");

	Source *t = nullptr;

	luax_catchexcept(L, [&]() {
		if (luax_istype(L, 1, love::sound::SoundData::type))
			t = instance()->newSource(luax_totype<love::sound::SoundData>(L, 1));
		else if (luax_istype(L, 1, love::sound::Decoder::type))
			t = instance()->newSource(luax_totype<love::sound::Decoder>(L, 1));
	});

	if (t != nullptr)
	{
		luax_pushtype(L, t);
		t->release();
		return 1;
	}
	else
		return luax_typerror(L, 1, "Decoder or SoundData");
}

int w_newQueueableSource(lua_State *L)
{
	Source *t = nullptr;

	luax_catchexcept(L, [&]() {
		t = instance()->newSource((int)luaL_checkinteger(L, 1), (int)luaL_checkinteger(L, 2), (int)luaL_checkinteger(L, 3), (int)luaL_optinteger(L, 4, 0));
	});

	if (t != nullptr)
	{
		luax_pushtype(L, t);
		t->release();
		return 1;
	}
	else
		return 0; //all argument type errors are checked in above constructor
}

static std::vector<Source*> readSourceList(lua_State *L, int n)
{
	if (n < 0)
		n += lua_gettop(L) + 1;

	int items = (int) luax_objlen(L, n);
	std::vector<Source*> sources(items);

	for (int i = 0; i < items; i++)
	{
		lua_rawgeti(L, n, i+1);
		sources[i] = luax_checksource(L, -1);
		lua_pop(L, 1);
	}

	return sources;
}

static std::vector<Source*> readSourceVararg(lua_State *L, int i)
{
	const int top = lua_gettop(L);

	if (i < 0)
		i += top + 1;

	int items = top - i + 1;
	std::vector<Source*> sources(items);

	for (int pos = 0; i <= top; i++, pos++)
		sources[pos] = luax_checksource(L, i);

	return sources;
}

int w_play(lua_State *L)
{
	if (lua_istable(L, 1))
		luax_pushboolean(L, instance()->play(readSourceList(L, 1)));
	else if (lua_gettop(L) > 1)
		luax_pushboolean(L, instance()->play(readSourceVararg(L, 1)));
	else
	{
		Source *s = luax_checksource(L, 1);
		luax_pushboolean(L, instance()->play(s));
	}

	return 1;
}

int w_stop(lua_State *L)
{
	if (lua_isnone(L, 1))
		instance()->stop();
	else if (lua_istable(L, 1))
		instance()->stop(readSourceList(L, 1));
	else if (lua_gettop(L) > 1)
		instance()->stop(readSourceVararg(L, 1));
	else
	{
		Source *s = luax_checksource(L, 1);
		s->stop();
	}
	return 0;
}

int w_pause(lua_State *L)
{
	if (lua_isnone(L, 1))
	{
		auto sources = instance()->pause();

		lua_createtable(L, (int) sources.size(), 0);
		for (int i = 0; i < (int) sources.size(); i++)
		{
			luax_pushtype(L, sources[i]);
			lua_rawseti(L, -2, i+1);
		}
		return 1;
	}
	else if (lua_istable(L, 1))
		instance()->pause(readSourceList(L, 1));
	else if (lua_gettop(L) > 1)
		instance()->pause(readSourceVararg(L, 1));
	else
	{
		Source *s = luax_checksource(L, 1);
		s->pause();
	}

	return 0;
}

int w_setVolume(lua_State *L)
{
	float v = (float)luaL_checknumber(L, 1);
	instance()->setVolume(v);
	return 0;
}

int w_getVolume(lua_State *L)
{
	lua_pushnumber(L, instance()->getVolume());
	return 1;
}

int w_setPosition(lua_State *L)
{
	float v[3];
	v[0] = (float)luaL_checknumber(L, 1);
	v[1] = (float)luaL_checknumber(L, 2);
	v[2] = (float)luaL_optnumber(L, 3, 0);
	instance()->setPosition(v);
	return 0;
}

int w_getPosition(lua_State *L)
{
	float v[3];
	instance()->getPosition(v);
	lua_pushnumber(L, v[0]);
	lua_pushnumber(L, v[1]);
	lua_pushnumber(L, v[2]);
	return 3;
}

int w_setOrientation(lua_State *L)
{
	float v[6];
	v[0] = (float)luaL_checknumber(L, 1);
	v[1] = (float)luaL_checknumber(L, 2);
	v[2] = (float)luaL_checknumber(L, 3);
	v[3] = (float)luaL_checknumber(L, 4);
	v[4] = (float)luaL_checknumber(L, 5);
	v[5] = (float)luaL_checknumber(L, 6);
	instance()->setOrientation(v);
	return 0;
}

int w_getOrientation(lua_State *L)
{
	float v[6];
	instance()->getOrientation(v);
	lua_pushnumber(L, v[0]);
	lua_pushnumber(L, v[1]);
	lua_pushnumber(L, v[2]);
	lua_pushnumber(L, v[3]);
	lua_pushnumber(L, v[4]);
	lua_pushnumber(L, v[5]);
	return 6;
}

int w_setVelocity(lua_State *L)
{
	float v[3];
	v[0] = (float)luaL_checknumber(L, 1);
	v[1] = (float)luaL_checknumber(L, 2);
	v[2] = (float)luaL_optnumber(L, 3, 0);
	instance()->setVelocity(v);
	return 0;
}

int w_getVelocity(lua_State *L)
{
	float v[3];
	instance()->getVelocity(v);
	lua_pushnumber(L, v[0]);
	lua_pushnumber(L, v[1]);
	lua_pushnumber(L, v[2]);
	return 3;
}

int w_setDopplerScale(lua_State *L)
{
	instance()->setDopplerScale(luax_checkfloat(L, 1));
	return 0;
}

int w_getDopplerScale(lua_State *L)
{
	lua_pushnumber(L, instance()->getDopplerScale());
	return 1;
}
/*
int w_setMeter(lua_State *L)
{
	instance()->setMeter(luax_checkfloat(L, 1));
	return 0;
}

int w_getMeter(lua_State *L)
{
	lua_pushnumber(L, instance()->getMeter());
	return 1;
}
*/
int w_setDistanceModel(lua_State *L)
{
	const char *modelStr = luaL_checkstring(L, 1);
	Audio::DistanceModel distanceModel;
	if (!Audio::getConstant(modelStr, distanceModel))
		return luax_enumerror(L, "distance model", Audio::getConstants(distanceModel), modelStr);
	instance()->setDistanceModel(distanceModel);
	return 0;
}

int w_getDistanceModel(lua_State *L)
{
	Audio::DistanceModel distanceModel = instance()->getDistanceModel();
	const char *modelStr;
	if (!Audio::getConstant(distanceModel, modelStr))
		return 0;
	lua_pushstring(L, modelStr);
	return 1;
}

int w_getRecordingDevices(lua_State *L)
{
	const std::vector<RecordingDevice*> &devices = instance()->getRecordingDevices();

	lua_createtable(L, devices.size(), 0);

	for (unsigned int i = 0; i < devices.size(); i++)
	{
		luax_pushtype(L, devices[i]);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int w_setEffect(lua_State *L)
{
	const char *namestr = luaL_checkstring(L, 1);

	if (lua_isnoneornil(L, 2) || (lua_gettop(L) == 2 && lua_isboolean(L, 2) && !lua_toboolean(L, 2)))
	{
		lua_pushboolean(L, instance()->unsetEffect(namestr));
		return 1;
	}

	luaL_checktype(L, 2, LUA_TTABLE);

	const char *paramstr = nullptr;

	//find type (mandatory)
	Effect::getConstant(Effect::EFFECT_TYPE, paramstr, Effect::TYPE_BASIC);
	lua_pushstring(L, paramstr);
	lua_rawget(L, 2);
	if (lua_type(L, -1) == LUA_TNIL)
		return luaL_error(L, "Effect type not specificed.");

	Effect::Type type = Effect::TYPE_MAX_ENUM;
	const char *typestr = luaL_checkstring(L, -1);
	if (!Effect::getConstant(typestr, type))
		return luax_enumerror(L, "effect type", Effect::getConstants(type), typestr);

	lua_pop(L, 1);
	std::map<Effect::Parameter, float> params;
	params[Effect::EFFECT_TYPE] = static_cast<int>(type);

	// Iterate over the whole table, reading valid parameters and erroring on invalid ones
	lua_pushnil(L);
	while (lua_next(L, 2))
	{
		const char *keystr = luaL_checkstring(L, -2);
		Effect::Parameter param;

		if(Effect::getConstant(keystr, param, type) || Effect::getConstant(keystr, param, Effect::TYPE_BASIC))
		{
#define luax_effecterror(l,t) luaL_error(l,"Bad parameter type for %s %s: " t " expected, got %s", typestr, keystr, lua_typename(L, -1))
			switch(Effect::getParameterType(param))
			{
			case Effect::PARAM_FLOAT:
				if (!lua_isnumber(L, -1))
					return luax_effecterror(L, "number");
				params[param] = lua_tonumber(L, -1);
				break;
			case Effect::PARAM_BOOL:
				if (!lua_isboolean(L, -1))
					return luax_effecterror(L, "boolean");
				params[param] = lua_toboolean(L, -1) ? 1.0 : 0.0;
				break;
			case Effect::PARAM_WAVEFORM:
			{
				if (!lua_isstring(L, -1))
					return luax_effecterror(L, "string");
				paramstr = lua_tostring(L, -1);
				Effect::Waveform waveform;
				if (!Effect::getConstant(paramstr, waveform))
					return luax_enumerror(L, "waveform type", paramstr);
				params[param] = static_cast<int>(waveform);
				break;
			}
			/*
			case Effect::PARAM_DIRECTION:
			{
				if (!lua_isstring(L, -1))
					return luax_effecterror(L, "string");
				paramstr = lua_tostring(L, -1);
				Effect::Direction direction;
				if (!Effect::getConstant(paramstr, direction))
					return luaL_error(L, "Invalid direction type: %s", paramstr);
				params[param] = static_cast<int>(direction);
				break;
			}
			case Effect::PARAM_PHONEME:
			{
				if (!lua_isstring(L, -1))
					return luax_effecterror(L, "string");
				paramstr = lua_tostring(L, -1);
				Effect::Phoneme phoneme;
				if (!Effect::getConstant(basicstr, phoneme))
					return luaL_error(L, "Invalid phoneme type: %s", paramstr);
				params[param] = static_cast<int>(phoneme);
				break;
			}
			*/
			case Effect::PARAM_TYPE:
			case Effect::PARAM_MAX_ENUM:
				break;
			}
#undef luax_effecterror
		}
		else
			luaL_error(L, "Invalid '%s' Effect parameter: %s", typestr, keystr);

		//remove the value (-1) from stack, keep the key (-2) to feed into lua_next
		lua_pop(L, 1);
	}

	luax_catchexcept(L, [&]() { lua_pushboolean(L, instance()->setEffect(namestr, params)); });
	return 1;
}

int w_getEffect(lua_State *L)
{
	const char *namestr = luaL_checkstring(L, 1);

	std::map<Effect::Parameter, float> params;

	if (!instance()->getEffect(namestr, params))
		return 0;

	const char *keystr, *valstr;
	Effect::Type type = static_cast<Effect::Type>((int)params[Effect::EFFECT_TYPE]);

	if (lua_istable(L, 2))
		lua_pushvalue(L, 2);
	else
		lua_createtable(L, 0, params.size());

	for (auto p : params)
	{
		if (!Effect::getConstant(p.first, keystr, type))
			Effect::getConstant(p.first, keystr, Effect::TYPE_BASIC);

		lua_pushstring(L, keystr);
		switch (Effect::getParameterType(p.first))
		{
		case Effect::PARAM_FLOAT:
			lua_pushnumber(L, p.second);
			break;
		case Effect::PARAM_BOOL:
			lua_pushboolean(L, p.second > 0.5 ? true : false);
			break;
		case Effect::PARAM_WAVEFORM:
			Effect::getConstant(static_cast<Effect::Waveform>((int)p.second), valstr);
			lua_pushstring(L, valstr);
			break;
/*
		case Effect::PARAM_DIRECTION:
			Effect::getConstant(static_cast<Effect::Direction>((int)p.second), valstr);
			lua_pushstring(L, valstr);
			break;
		case Effect::PARAM_PHONEME:
			Effect::getConstant(static_cast<Effect::Phoneme>((int)p.second), valstr);
			lua_pushstring(L, valstr);
			break;
*/
		case Effect::PARAM_TYPE:
			Effect::getConstant(static_cast<Effect::Type>((int)p.second), valstr);
			lua_pushstring(L, valstr);
			break;
		case Effect::PARAM_MAX_ENUM:
			break;
		}
		lua_rawset(L, -3);
	}
	return 1;
}

int w_getActiveEffects(lua_State *L)
{
	std::vector<std::string> list;
	instance()->getActiveEffects(list);

	lua_createtable(L, 0, (int) list.size());
	for (int i = 0; i < (int) list.size(); i++)
	{
		lua_pushnumber(L, i + 1);
		lua_pushstring(L, list[i].c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

int w_getMaxSceneEffects(lua_State *L)
{
	lua_pushnumber(L, instance()->getMaxSceneEffects());
	return 1;
}

int w_getMaxSourceEffects(lua_State *L)
{
	lua_pushnumber(L, instance()->getMaxSourceEffects());
	return 1;
}

int w_isEffectsSupported(lua_State *L)
{
	lua_pushboolean(L, instance()->isEFXsupported());
	return 1;
}

int w_setMixWithSystem(lua_State *L)
{
	luax_pushboolean(L, Audio::setMixWithSystem(luax_checkboolean(L, 1)));
	return 1;
}

int w_getSourceCount(lua_State *L)
{
	luax_markdeprecated(L, "love.audio.getSourceCount", API_FUNCTION, DEPRECATED_RENAMED, "love.audio.getActiveSourceCount");
	return w_getActiveSourceCount(L);
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "getActiveSourceCount", w_getActiveSourceCount },
	{ "newSource", w_newSource },
	{ "newQueueableSource", w_newQueueableSource },
	{ "play", w_play },
	{ "stop", w_stop },
	{ "pause", w_pause },
	{ "setVolume", w_setVolume },
	{ "getVolume", w_getVolume },
	{ "setPosition", w_setPosition },
	{ "getPosition", w_getPosition },
	{ "setOrientation", w_setOrientation },
	{ "getOrientation", w_getOrientation },
	{ "setVelocity", w_setVelocity },
	{ "getVelocity", w_getVelocity },
	{ "setDopplerScale", w_setDopplerScale },
	{ "getDopplerScale", w_getDopplerScale },
	//{ "setMeter", w_setMeter },
	//{ "getMeter", w_setMeter },
	{ "setDistanceModel", w_setDistanceModel },
	{ "getDistanceModel", w_getDistanceModel },
	{ "getRecordingDevices", w_getRecordingDevices },
	{ "setEffect", w_setEffect },
	{ "getEffect", w_getEffect },
	{ "getActiveEffects", w_getActiveEffects },
	{ "getMaxSceneEffects", w_getMaxSceneEffects },
	{ "getMaxSourceEffects", w_getMaxSourceEffects },
	{ "isEffectsSupported", w_isEffectsSupported },
	{ "setMixWithSystem", w_setMixWithSystem },

	// Deprecated
	{ "getSourceCount", w_getSourceCount },

	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_source,
	luaopen_recordingdevice,
	0
};

extern "C" int luaopen_love_audio(lua_State *L)
{
	Audio *instance = instance();

	if (instance == nullptr)
	{
		// Try OpenAL first.
		try
		{
			instance = new love::audio::openal::Audio();
		}
		catch(love::Exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}
	else
		instance->retain();

	if (instance == nullptr)
	{
		// Fall back to nullaudio.
		try
		{
			instance = new love::audio::null::Audio();
		}
		catch(love::Exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	if (instance == nullptr)
		return luaL_error(L, "Could not open any audio module.");

	WrappedModule w;
	w.module = instance;
	w.name = "audio";
	w.type = &Module::type;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	return n;
}

} // audio
} // love
