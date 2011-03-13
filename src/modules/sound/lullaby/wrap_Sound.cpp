/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "Sound.h"

namespace love
{
namespace sound
{
	extern Sound * instance;
namespace lullaby
{	
	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "newSoundData",  w_newSoundData },
		{ "newDecoder",  w_newDecoder },
		{ 0, 0 }
	};

	static const lua_CFunction types[] = {
		luaopen_sounddata,
		luaopen_decoder,
		0
	};

	int luaopen_love_sound(lua_State * L)
	{
		if(instance == 0)
		{
			try
			{
				instance = new Sound();
			}
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}
		else
			instance->retain();


		WrappedModule w;
		w.module = instance;
		w.name = "sound";
		w.flags = MODULE_SOUND_T;
		w.functions = functions;
		w.types = types;

		return luax_register_module(L, w);
	}

} // lullaby
} // sound
} // love
