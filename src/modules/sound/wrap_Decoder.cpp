/**
* Copyright (c) 2006-2009 LOVE Development Team
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
	Decoder * luax_checkdecoder(lua_State * L, int idx)
	{
		return luax_checktype<Decoder>(L, idx, "Decoder", LOVE_SOUND_DECODER_BITS);
	}

	int _wrap_Decoder_getChannels(lua_State * L)
	{
		Decoder * t = luax_checkdecoder(L, 1);
		lua_pushinteger(L, t->getChannels());
		return 1;
	}

	int _wrap_Decoder_getBits(lua_State * L)
	{
		Decoder * t = luax_checkdecoder(L, 1);
		lua_pushinteger(L, t->getBits());
		return 1;
	}

	int _wrap_Decoder_getSampleRate(lua_State * L)
	{
		Decoder * t = luax_checkdecoder(L, 1);
		lua_pushinteger(L, t->getSampleRate());
		return 1;
	}

	static const luaL_Reg wrap_Decoder_functions[] = {
		{ "getChannels", _wrap_Decoder_getChannels },
		{ "getBits", _wrap_Decoder_getBits },
		{ "getSampleRate", _wrap_Decoder_getSampleRate },
		{ 0, 0 }
	};
	
	int wrap_Decoder_open(lua_State * L)
	{
		luax_register_type(L, "Decoder", wrap_Decoder_functions);
		return 0;
	}

} // sound
} // love
