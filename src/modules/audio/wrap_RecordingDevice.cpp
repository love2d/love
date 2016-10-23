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

#include "wrap_RecordingDevice.h"
#include "wrap_Audio.h"

#include "sound/SoundData.h"
#include "sound/Sound.h"

#define soundInstance() (Module::getInstance<love::sound::Sound>(Module::M_SOUND))

namespace love
{
namespace audio
{

RecordingDevice *luax_checkrecordingdevice(lua_State *L, int idx)
{
	return luax_checktype<RecordingDevice>(L, idx, AUDIO_RECORDING_DEVICE_ID);
}

int w_RecordingDevice_startRecording(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	if (lua_gettop(L) > 1)
	{
		int samples = luaL_checkinteger(L, 2);
		int sampleRate = luaL_checkinteger(L, 3);
		int bitDepth = luaL_checkinteger(L, 4);
		int channels = luaL_checkinteger(L, 5);
		luax_catchexcept(L, [&](){ 
			lua_pushboolean(L, d->startRecording(samples, sampleRate, bitDepth, channels)); 
		});
	}
	else
		luax_catchexcept(L, [&](){ 
			lua_pushboolean(L, d->startRecording()); 
		});
	return 1;
}

int w_RecordingDevice_stopRecording(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	int samples = d->getSampleCount();
	if (samples == 0)
	{
		lua_pushnil(L);
		return 1;
	}

	love::sound::SoundData *s = nullptr;
	if (lua_gettop(L) > 1)
	{
		if (luax_istype(L, 2, SOUND_SOUND_DATA_ID))
			s = luax_totype<love::sound::SoundData>(L, 2, SOUND_SOUND_DATA_ID);
		else
			return luaL_typerror(L, 2, "SoundData");

		s->retain();
	}
	else
	{
		luax_catchexcept(L, [&](){ 
			s = soundInstance()->newSoundData(samples, d->getSampleRate(), d->getBitDepth(), d->getChannels()); 
		});
	}

	luax_catchexcept(L, [&](){ 
		d->getData(s);
		d->stopRecording();
	});

	luax_pushtype(L, SOUND_SOUND_DATA_ID, s);
	s->release();
	return 1;
}

int w_RecordingDevice_getData(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	int samples = d->getSampleCount();
	if (samples == 0)
	{
		lua_pushnil(L);
		return 1;
	}

	love::sound::SoundData *s = nullptr;
	if (lua_gettop(L) > 1)
	{
		if (luax_istype(L, 2, SOUND_SOUND_DATA_ID))
			s = luax_totype<love::sound::SoundData>(L, 2, SOUND_SOUND_DATA_ID);
		else
			return luaL_typerror(L, 2, "SoundData");

		s->retain();
	}
	else
	{
		luax_catchexcept(L, [&](){ 
			s = soundInstance()->newSoundData(samples, d->getSampleRate(), d->getBitDepth(), d->getChannels()); 
		});
	}

	luax_catchexcept(L, [&](){ d->getData(s); });

	luax_pushtype(L, SOUND_SOUND_DATA_ID, s);
	s->release();
	return 1;
}

int w_RecordingDevice_getSampleCount(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushnumber(L, d->getSampleCount());
	return 1;
}

int w_RecordingDevice_getSampleRate(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushnumber(L, d->getSampleRate());
	return 1;
}

int w_RecordingDevice_getBitDepth(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushnumber(L, d->getBitDepth());
	return 1;
}

int w_RecordingDevice_getChannels(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushnumber(L, d->getChannels());
	return 1;
}

int w_RecordingDevice_getName(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushstring(L, d->getName());
	return 1;
}

int w_RecordingDevice_getID(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushnumber(L, d->getID());
	return 1;
}

int w_RecordingDevice_isRecording(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushboolean(L, d->isRecording());
	return 1;
}

static const luaL_Reg w_RecordingDevice_functions[] =
{
	{ "startRecording", w_RecordingDevice_startRecording },
	{ "stopRecording", w_RecordingDevice_stopRecording },
	{ "getData", w_RecordingDevice_getData },
	{ "getSampleCount", w_RecordingDevice_getSampleCount },
	{ "getSampleRate", w_RecordingDevice_getSampleRate },
	{ "getBitDepth", w_RecordingDevice_getBitDepth },
	{ "getChannels", w_RecordingDevice_getChannels },
	{ "getName", w_RecordingDevice_getName },
	{ "getID", w_RecordingDevice_getID },
	{ "isRecording", w_RecordingDevice_isRecording },
	{ 0, 0 }
};

extern "C" int luaopen_recordingdevice(lua_State *L)
{
	int ret = luax_register_type(L, AUDIO_RECORDING_DEVICE_ID, "RecordingDevice", w_RecordingDevice_functions, nullptr);
	return ret;
}

} //audio
} //love
