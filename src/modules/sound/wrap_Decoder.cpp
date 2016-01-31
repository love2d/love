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

#include "wrap_Decoder.h"

namespace love
{
namespace sound
{

Decoder *luax_checkdecoder(lua_State *L, int idx)
{
	return luax_checktype<Decoder>(L, idx, SOUND_DECODER_ID);
}

int w_Decoder_getChannels(lua_State *L)
{
	Decoder *t = luax_checkdecoder(L, 1);
	lua_pushinteger(L, t->getChannels());
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

static const luaL_Reg w_Decoder_functions[] =
{
	{ "getChannels", w_Decoder_getChannels },
	{ "getBitDepth", w_Decoder_getBitDepth },
	{ "getSampleRate", w_Decoder_getSampleRate },
	{ "getDuration", w_Decoder_getDuration },
	{ 0, 0 }
};

extern "C" int luaopen_decoder(lua_State *L)
{
	return luax_register_type(L, SOUND_DECODER_ID, "Decoder", w_Decoder_functions, nullptr);
}

} // sound
} // love
