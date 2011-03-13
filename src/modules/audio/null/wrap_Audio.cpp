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

// LOVE
#include "wrap_Audio.h"

#include "Audio.h"

#include <scripts/audio.lua.h>

#include <common/runtime.h>

namespace love
{
namespace audio
{
	extern Audio * instance;
namespace null
{
	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "getNumSources", w_getNumSources },
		{ "newSource1", w_newSource1 },
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
		/*{ "record", w_record },
		{ "getRecordedData", w_getRecordedData },
		{ "stopRecording", w_stopRecording },*/
		{ 0, 0 }
	};

	static const lua_CFunction types[] = {
		luaopen_source,
		0
	};

	int luaopen_love_audio_null(lua_State * L)
	{
		if(instance == 0)
		{
			try
			{
				instance = new Audio();
			}
			catch(love::Exception & e)
			{
				return luaL_error(L, "Could not open any audio module.");
			}
		}
		else
			instance->retain();

		WrappedModule w;
		w.module = instance;
		w.name = "audio";
		w.flags = MODULE_T;
		w.functions = functions;
		w.types = types;

		luax_register_module(L, w);

		if (luaL_loadbuffer(L, (const char *)audio_lua, sizeof(audio_lua), "audio.lua") == 0)
			lua_call(L, 0, 0);

		return 0;
	}
	
} // null
} // audio
} // love
