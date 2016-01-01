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
	if (lua_isstring(L, 1) || luax_istype(L, 1, FILESYSTEM_FILE_ID) || luax_istype(L, 1, FILESYSTEM_FILE_DATA_ID))
		luax_convobj(L, 1, "sound", "newDecoder");

	Source::Type stype = Source::TYPE_STREAM;

	const char *stypestr = lua_isnoneornil(L, 2) ? 0 : lua_tostring(L, 2);
	if (stypestr && !Source::getConstant(stypestr, stype))
		return luaL_error(L, "Invalid source type: %s", stypestr);

	if (stype == Source::TYPE_STATIC && luax_istype(L, 1, SOUND_DECODER_ID))
		luax_convobj(L, 1, "sound", "newSoundData");

	Source *t = 0;

	luax_catchexcept(L, [&]() {
		if (luax_istype(L, 1, SOUND_SOUND_DATA_ID))
			t = instance()->newSource(luax_totype<love::sound::SoundData>(L, 1, SOUND_SOUND_DATA_ID));
		else if (luax_istype(L, 1, SOUND_DECODER_ID))
			t = instance()->newSource(luax_totype<love::sound::Decoder>(L, 1, SOUND_DECODER_ID));
	});

	if (t)
	{
		luax_pushtype(L, AUDIO_SOURCE_ID, t);
		t->release();
		return 1;
	}
	else
		return luax_typerror(L, 1, "Decoder or SoundData");
}

int w_play(lua_State *L)
{
	Source *s = luax_checksource(L, 1);
	luax_pushboolean(L, instance()->play(s));
	return 1;
}

int w_stop(lua_State *L)
{
	if (lua_gettop(L) == 0)
	{
		instance()->stop();
	}
	else
	{
		Source *s = luax_checksource(L, 1);
		s->stop();
	}
	return 0;
}

int w_pause(lua_State *L)
{
	if (lua_gettop(L) == 0)
	{
		instance()->pause();
	}
	else
	{
		Source *s = luax_checksource(L, 1);
		s->pause();
	}

	return 0;
}

int w_resume(lua_State *L)
{
	if (lua_gettop(L) == 0)
	{
		instance()->resume();
	}
	else
	{
		Source *s = luax_checksource(L, 1);
		s->resume();
	}
	return 0;
}

int w_rewind(lua_State *L)
{
	if (lua_gettop(L) == 0)
	{
		instance()->rewind();
	}
	else
	{
		Source *s = luax_checksource(L, 1);
		s->rewind();
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

int w_record(lua_State *)
{
	instance()->record();
	return 0;
}

int w_getRecordedData(lua_State *L)
{
	love::sound::SoundData *sd = instance()->getRecordedData();
	if (!sd)
		lua_pushnil(L);
	else
	{
		luax_pushtype(L, SOUND_SOUND_DATA_ID, sd);
		sd->release();
	}
	return 1;
}

int w_stopRecording(lua_State *L)
{
	if (luax_optboolean(L, 1, true))
	{
		love::sound::SoundData *sd = instance()->stopRecording(true);
		if (!sd)
			lua_pushnil(L);
		else
		{
			luax_pushtype(L, SOUND_SOUND_DATA_ID, sd);
			sd->release();
		}
		return 1;
	}
	instance()->stopRecording(false);
	return 0;
}

int w_canRecord(lua_State *L)
{
	luax_pushboolean(L, instance()->canRecord());
	return 1;
}

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

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "getSourceCount", w_getSourceCount },
	{ "newSource", w_newSource },
	{ "play", w_play },
	{ "stop", w_stop },
	{ "pause", w_pause },
	{ "resume", w_resume },
	{ "rewind", w_rewind },
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
	/*{ "record", w_record },
	{ "getRecordedData", w_getRecordedData },
	{ "stopRecording", w_stopRecording },*/
	{ "setDistanceModel", w_setDistanceModel },
	{ "getDistanceModel", w_getDistanceModel },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_source,
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
	w.type = MODULE_ID;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	return n;
}

} // audio
} // love
