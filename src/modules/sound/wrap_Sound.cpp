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

#include "wrap_Sound.h"

#include "filesystem/wrap_Filesystem.h"

// Implementations.
#include "lullaby/Sound.h"

namespace love
{
namespace sound
{

#define instance() (Module::getInstance<Sound>(Module::M_SOUND))

int w_newDecoder(lua_State *L)
{
	love::filesystem::FileData *data = love::filesystem::luax_getfiledata(L, 1);
	int bufferSize = (int) luaL_optnumber(L, 2, Decoder::DEFAULT_BUFFER_SIZE);

	Decoder *t = nullptr;
	luax_catchexcept(L,
		[&]() { t = instance()->newDecoder(data, bufferSize); },
		[&](bool) { data->release(); }
	);

	if (t == nullptr)
		return luaL_error(L, "Extension \"%s\" not supported.", data->getExtension().c_str());

	luax_pushtype(L, SOUND_DECODER_ID, t);
	t->release();
	return 1;
}

int w_newSoundData(lua_State *L)
{
	SoundData *t = 0;

	if (lua_isnumber(L, 1))
	{
		int samples = (int) luaL_checknumber(L, 1);
		int sampleRate = (int) luaL_optnumber(L, 2, Decoder::DEFAULT_SAMPLE_RATE);
		int bitDepth = (int) luaL_optnumber(L, 3, Decoder::DEFAULT_BIT_DEPTH);
		int channels = (int) luaL_optnumber(L, 4, Decoder::DEFAULT_CHANNELS);

		luax_catchexcept(L, [&](){ t = instance()->newSoundData(samples, sampleRate, bitDepth, channels); });
	}
	// Must be string or decoder.
	else
	{
		// Convert to Decoder, if necessary.
		if (!luax_istype(L, 1, SOUND_DECODER_ID))
		{
			w_newDecoder(L);
			lua_replace(L, 1);
		}

		luax_catchexcept(L, [&](){ t = instance()->newSoundData(luax_checkdecoder(L, 1)); });
	}

	luax_pushtype(L, SOUND_SOUND_DATA_ID, t);
	t->release();
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "newDecoder",  w_newDecoder },
	{ "newSoundData",  w_newSoundData },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_sounddata,
	luaopen_decoder,
	0
};

extern "C" int luaopen_love_sound(lua_State *L)
{
	Sound *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new lullaby::Sound(); });
	}
	else
		instance->retain();


	WrappedModule w;
	w.module = instance;
	w.name = "sound";
	w.type = MODULE_SOUND_ID;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // sound
} // love
