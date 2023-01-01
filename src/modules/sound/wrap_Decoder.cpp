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

#include "wrap_Decoder.h"
#include "SoundData.h"
#include "Sound.h"

#define instance() (Module::getInstance<Sound>(Module::M_SOUND))

namespace love
{
namespace sound
{

Decoder *luax_checkdecoder(lua_State *L, int idx)
{
	return luax_checktype<Decoder>(L, idx);
}

int w_Decoder_clone(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	Decoder *c = nullptr;
	luax_catchexcept(L, [&]() { c = t->clone(); });
	luax_pushtype(L, c);
	c->release();
	return 1;
}

int w_Decoder_getChannelCount(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	lua_pushinteger(L, t->getChannelCount());
	return 1;
}

int w_Decoder_getBitDepth(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	lua_pushinteger(L, t->getBitDepth());
	return 1;
}

int w_Decoder_getSampleRate(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	lua_pushinteger(L, t->getSampleRate());
	return 1;
}

int w_Decoder_getDuration(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	lua_pushnumber(L, t->getDuration());
	return 1;
}

int w_Decoder_decode(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);

	int decoded = t->decode();
	if (decoded > 0)
	{
		luax_catchexcept(L, [&]() {
			SoundData *s = instance()->newSoundData(t->getBuffer(),
				decoded / (t->getBitDepth() / 8 * t->getChannelCount()),
				t->getSampleRate(), t->getBitDepth(), t->getChannelCount());

			luax_pushtype(L, s);
			s->release();
		});
	}
	else
		lua_pushnil(L);
	return 1;
}

int w_Decoder_seek(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	double offset = luaL_checknumber(L, 2);
	if (offset < 0)
		return luaL_argerror(L, 2, "can't seek to a negative position");
	else if (offset == 0)
		t->rewind();
	else
		t->seek(offset);
	return 0;
}

int w_Decoder_getChannels(lua_State *L)
{
	luax_markdeprecated(L, "Decoder:getChannels", API_METHOD, DEPRECATED_RENAMED, "Decoder:getChannelCount");
	return w_Decoder_getChannelCount(L);
}

static const luaL_Reg w_Decoder_functions[] =
{
	{ "clone", w_Decoder_clone },
	{ "getChannelCount", w_Decoder_getChannelCount },
	{ "getBitDepth", w_Decoder_getBitDepth },
	{ "getSampleRate", w_Decoder_getSampleRate },
	{ "getDuration", w_Decoder_getDuration },
	{ "decode", w_Decoder_decode },
	{ "seek", w_Decoder_seek },

	// Deprecated
	{ "getChannels", w_Decoder_getChannels },

	{ 0, 0 }
};

extern "C" int luaopen_decoder(lua_State *L)
{
	return luax_register_type(L, &Decoder::type, w_Decoder_functions, nullptr);
}

} // sound
} // love
