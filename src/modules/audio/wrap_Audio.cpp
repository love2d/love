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

int w_getSourceCount(lua_State *L)
{
	lua_pushinteger(L, instance()->getSourceCount());
	return 1;
}

int w_newSource(lua_State *L)
{
	Source::Type stype = Source::TYPE_STREAM;

	if (!luax_istype(L, 1, love::sound::SoundData::type) && !luax_istype(L, 1, love::sound::Decoder::type))
	{
		const char *stypestr = luaL_checkstring(L, 2);
		if (stypestr && !Source::getConstant(stypestr, stype))
			return luaL_error(L, "Invalid source type: %s", stypestr);
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
		t = instance()->newSource((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3));
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

	int items = (int) lua_objlen(L, n);
	std::vector<Source*> sources(items);

	for (int i = 0; i < items; i++)
	{
		lua_rawgeti(L, n, i+1);
		sources[i] = luax_checksource(L, -1);
		lua_pop(L, 1);
	}

	return sources;
}

int w_play(lua_State *L)
{
	if (lua_istable(L, 1))
	{
		luax_pushboolean(L, instance()->play(readSourceList(L, 1)));
		return 1;
	}

	Source *s = luax_checksource(L, 1);
	luax_pushboolean(L, instance()->play(s));
	return 1;
}

int w_stop(lua_State *L)
{
	if (lua_isnone(L, 1))
		instance()->stop();
	else if (lua_istable(L, 1))
		instance()->stop(readSourceList(L, 1));
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
		return luaL_error(L, "Invalid distance model: %s", modelStr);
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

int setSceneEffectHandleParam(lua_State *L, int pos, Effect::ParameterType type, std::vector<float> &params)
{
	if (lua_isnoneornil(L, pos))
		params.push_back(nanf(""));
	else
	{
		const char *typestr;
		switch (type)
		{
		case Effect::PAR_FLOAT:
			params.push_back(luaL_checknumber(L, pos)); 
			break;
		case Effect::PAR_BOOL:
			if (!lua_isboolean(L, pos))
				return luax_typerror(L, pos, "boolean");
			params.push_back(lua_toboolean(L, pos) ? 1.0 : 0.0); 
			break;
		case Effect::PAR_WAVEFORM:
		{
			typestr = luaL_checkstring(L, pos);
			Effect::Waveform waveform;
			if (!Effect::getConstant(typestr, waveform))
				return luaL_error(L, "Invalid waveform type: %s", typestr);
			params.push_back(static_cast<int>(waveform));
			break;
		}
		case Effect::PAR_DIRECTION:
		{
			typestr = luaL_checkstring(L, pos);
			Effect::Direction direction;
			if (!Effect::getConstant(typestr, direction))
				return luaL_error(L, "Invalid direction type: %s", typestr);
			params.push_back(static_cast<int>(direction));
			break;
		}
		case Effect::PAR_PHONEME:
		{
			typestr = luaL_checkstring(L, pos);
			Effect::Phoneme phoneme;
			if (!Effect::getConstant(typestr, phoneme))
				return luaL_error(L, "Invalid phoneme type: %s", typestr);
			params.push_back(static_cast<int>(phoneme));
			break;
		}
		case Effect::PAR_MAX_ENUM:
			break;
		}
	}
	return 0;
}

int w_setSceneEffect(lua_State *L)
{
	int slot = luaL_checknumber(L, 1) - 1;

	const char *typestr;
	Effect::Type type;
	std::vector<float> params;

	if (lua_gettop(L) == 1)
	{
		lua_pushboolean(L, instance()->setSceneEffect(slot));
		return 1;
	}
	else if (lua_gettop(L) > 2)
	{
		typestr = luaL_checkstring(L, 2);
		if (!Effect::getConstant(typestr, type))
			return luaL_error(L, "Invalid effect type: %s", typestr);

		params.push_back(luaL_checknumber(L, 3));
		const std::vector<Effect::ParameterType> &paramtypes = Effect::getParameterTypes(type);
		for (unsigned int i = 0; i < paramtypes.size(); i++)
			setSceneEffectHandleParam(L, i + 4, paramtypes[i], params);
	}
	else if (lua_istable(L, 2))
	{
		if (lua_objlen(L, 2) == 0) //empty table also clears effect
		{
			lua_pushboolean(L, instance()->setSceneEffect(slot));
			return 1;
		}
		lua_rawgeti(L, 2, 1);
		typestr = luaL_checkstring(L, -1);
		if (!Effect::getConstant(typestr, type))
			return luaL_error(L, "Invalid effect type: %s", typestr);
		lua_pop(L, 1);

		const std::vector<Effect::ParameterType> &paramtypes = Effect::getParameterTypes(type);
		lua_rawgeti(L, 2, 2);
		params.push_back(luaL_checknumber(L, -1));
		lua_pop(L, 1);
		for (unsigned int i = 0; i < paramtypes.size(); i++)
		{
			lua_rawgeti(L, 2, i + 3);
			setSceneEffectHandleParam(L, -1, paramtypes[i], params);
			lua_pop(L, 1);
		}
	}
	else
		return luax_typerror(L, 2, "effect description");

	luax_catchexcept(L, [&]() { lua_pushboolean(L, instance()->setSceneEffect(slot, type, params)); });
	return 1;
}

int w_getSceneEffect(lua_State *L)
{
	int slot = luaL_checknumber(L, 1) - 1;

	Effect::Type type;
	std::vector<float> params;

	if (!instance()->getSceneEffect(slot, type, params))
		return 0;

	const char *str = nullptr;
	Effect::getConstant(type, str);
	lua_pushstring(L, str);
	lua_pushnumber(L, params[0]);

	const std::vector<Effect::ParameterType> &paramtypes = Effect::getParameterTypes(type);

	for (unsigned int i = 1; i < params.size(); i++)
	{
		if (isnanf(params[i]))
			lua_pushnil(L);
		else
			switch(paramtypes[i-1])
			{
			case Effect::PAR_FLOAT:
				lua_pushnumber(L, params[i]); 
				break;
			case Effect::PAR_BOOL:
				lua_pushboolean(L, params[i] < 0.5 ? false : true); 
				break;
			case Effect::PAR_WAVEFORM:
				Effect::getConstant(static_cast<Effect::Waveform>((int)params[i]), str);
				lua_pushstring(L, str);
				break;
			case Effect::PAR_DIRECTION:
				Effect::getConstant(static_cast<Effect::Direction>((int)params[i]), str);
				lua_pushstring(L, str);
				break;
			case Effect::PAR_PHONEME:
				Effect::getConstant(static_cast<Effect::Phoneme>((int)params[i]), str);
				lua_pushstring(L, str);
				break;
			case Effect::PAR_MAX_ENUM:
				break;
			}
	}
	return params.size() + 1;
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

int w_isSceneEffectsSupported(lua_State *L)
{
	lua_pushboolean(L, instance()->isEFXsupported());
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "getSourceCount", w_getSourceCount },
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
	{ "setSceneEffect", w_setSceneEffect },
	{ "getSceneEffect", w_getSceneEffect },
	{ "getMaxSceneEffects", w_getMaxSceneEffects },
	{ "getMaxSourceEffects", w_getMaxSourceEffects },
	{ "isSceneEffectsSupported", w_isSceneEffectsSupported },
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

#ifdef LOVE_ENABLE_AUDIO_OPENAL
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
#endif

#ifdef LOVE_ENABLE_AUDIO_NULL
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
#endif

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
