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

#include "wrap_RecordingDevice.h"
#include "wrap_Audio.h"

#include "sound/SoundData.h"
namespace love
{
namespace audio
{

RecordingDevice *luax_checkrecordingdevice(lua_State *L, int idx)
{
	return luax_checktype<RecordingDevice>(L, idx);
}

int w_RecordingDevice_start(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);

	int samples = d->getMaxSamples();
	int samplerate = d->getSampleRate();
	int bitdepth = d->getBitDepth();
	int channels = d->getChannelCount();

	if (lua_gettop(L) > 1)
	{
		samples = (int) luaL_checkinteger(L, 2);
		samplerate = (int) luaL_optinteger(L, 3, RecordingDevice::DEFAULT_SAMPLE_RATE);
		bitdepth = (int) luaL_optinteger(L, 4, RecordingDevice::DEFAULT_BIT_DEPTH);
		channels = (int) (int) luaL_optinteger(L, 5, RecordingDevice::DEFAULT_CHANNELS);
	}

	bool success = false;
	luax_catchexcept(L, [&]() { success = d->start(samples, samplerate, bitdepth, channels); });

	luax_pushboolean(L, success);
	return 1;
}

int w_RecordingDevice_stop(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	love::sound::SoundData *s = nullptr;

	luax_catchexcept(L, [&](){ s = d->getData(); });

	d->stop();

	if (s != nullptr)
	{
		luax_pushtype(L, s);
		s->release();
	}
	else
		lua_pushnil(L);

	return 1;
}

int w_RecordingDevice_getData(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	love::sound::SoundData *s = nullptr;

	luax_catchexcept(L, [&](){ s = d->getData(); });

	if (s != nullptr)
	{
		luax_pushtype(L, s);
		s->release();
	}
	else
		lua_pushnil(L);

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

int w_RecordingDevice_getChannelCount(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushnumber(L, d->getChannelCount());
	return 1;
}

int w_RecordingDevice_getName(lua_State *L)
{
	RecordingDevice *d = luax_checkrecordingdevice(L, 1);
	lua_pushstring(L, d->getName());
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
	{ "start", w_RecordingDevice_start },
	{ "stop", w_RecordingDevice_stop },
	{ "getData", w_RecordingDevice_getData },
	{ "getSampleCount", w_RecordingDevice_getSampleCount },
	{ "getSampleRate", w_RecordingDevice_getSampleRate },
	{ "getBitDepth", w_RecordingDevice_getBitDepth },
	{ "getChannelCount", w_RecordingDevice_getChannelCount },
	{ "getName", w_RecordingDevice_getName },
	{ "isRecording", w_RecordingDevice_isRecording },
	{ 0, 0 }
};

extern "C" int luaopen_recordingdevice(lua_State *L)
{
	int ret = luax_register_type(L, &RecordingDevice::type, w_RecordingDevice_functions, nullptr);
	return ret;
}

} //audio
} //love
