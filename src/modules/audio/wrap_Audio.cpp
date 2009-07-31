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

// LOVE
#include "wrap_Audio.h"

#include "openal/Audio.h"
#include "null/Audio.h"

#include <sound/wrap_Decoder.h>

namespace love
{
namespace audio
{
	static Audio * instance = 0;

	int _wrap_getNumSources(lua_State * L)
	{
		lua_pushinteger(L, instance->getNumSources());
		return 1;
	}

	int _wrap_newSound(lua_State * L)
	{
		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_strtofile(L, 1);

		// Convert to SoundData, if necessary.
		if(luax_istype(L, 1, LOVE_FILESYSTEM_FILE_BITS))
			luax_convobj(L, 1, "sound", "newSoundData");

		love::sound::SoundData * data = luax_checktype<love::sound::SoundData>(L, 1, "SoundData", LOVE_SOUND_SOUND_DATA_BITS);
		Sound * t = instance->newSound(data);
		luax_newtype(L, "Sound", LOVE_AUDIO_SOUND_BITS, (void*)t);
		return 1;
	}


	int _wrap_newMusic(lua_State * L)
	{
		// Convert to Decoder, if necessary.
		if(!luax_istype(L, 1, LOVE_SOUND_DECODER_BITS))
			luax_convobj(L, 1, "sound", "newDecoder");

		love::sound::Decoder * decoder = love::sound::luax_checkdecoder(L, 1);
		Music * t = instance->newMusic(decoder);
		luax_newtype(L, "Music", LOVE_AUDIO_MUSIC_BITS, (void*)t);
		return 1;
	}

	int _wrap_newSource(lua_State * L)
	{
		Audible * a = luax_checktype<Audible>(L, 1, "Audible", LOVE_AUDIO_AUDIBLE_BITS);
		Source * t = instance->newSource(a);
		luax_newtype(L, "Source", LOVE_AUDIO_SOURCE_BITS, (void*)t);
		return 1;
	}

	int _wrap_play(lua_State * L)
	{
		int argn = lua_gettop(L);

		if(luax_istype(L, 1, LOVE_AUDIO_SOUND_BITS))
		{
			Sound * s = luax_checksound(L, 1);
			instance->play(s);
			return 0;
		} 
		else if(luax_istype(L, 1, LOVE_AUDIO_MUSIC_BITS))
		{
			Music * m = luax_checkmusic(L, 1);
			instance->play(m);
			return 0;
		}
		else if(luax_istype(L, 1, LOVE_AUDIO_SOURCE_BITS))
		{
			Source * s = luax_checksource(L, 1);
			instance->play(s);
			return 0;
		}

		return luaL_error(L, "No matching overload");
	}

	int _wrap_stop(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->stop(c);
		return 0;
	}

	int _wrap_pause(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->pause(c);
		return 0;
	}

	int _wrap_rewind(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->rewind(c);
		return 0;
	}

	int _wrap_setVolume(lua_State * L)
	{
		float v = (float)luaL_checknumber(L, 1);
		instance->setVolume(v);
		return 0;
	}

	int _wrap_getVolume(lua_State * L)
	{
		lua_pushnumber(L, instance->getVolume());
		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg wrap_Audio_functions[] = {
		{ "getNumSources", _wrap_getNumSources },
		{ "newSound",  _wrap_newSound },
		{ "newMusic",  _wrap_newMusic },
		{ "newSource",  _wrap_newSource },
		{ "play",  _wrap_play },
		{ "stop",  _wrap_stop },
		{ "pause",  _wrap_pause },
		{ "rewind",  _wrap_rewind },
		{ "setVolume",  _wrap_setVolume },
		{ "getVolume",  _wrap_getVolume },
		{ 0, 0 }
	};

	static const lua_CFunction wrap_Audio_types[] = {
		wrap_Source_open,
		wrap_Music_open,
		wrap_Sound_open,
		0
	};

	int wrap_Audio_open(lua_State * L)
	{
		if(instance == 0)
		{
			// Try OpenAL first.
			try 
			{
				instance = new love::audio::openal::Audio();
			}
			catch(love::Exception & e)
			{
				std::cout << e.what() << std::endl;
			}
		}

		if(instance == 0)
		{
			// Fall back to nullaudio.
			try
			{
				instance = new love::audio::null::Audio();
			}
			catch(love::Exception & e)
			{
				std::cout << e.what() << std::endl;
			}
		}

		if(instance == 0)
			return luaL_error(L, "Could not open any audio module.");

		luax_register_gc(L, "love.audio", instance);

		return luax_register_module(L, wrap_Audio_functions, wrap_Audio_types, "audio");
	}

} // audio
} // love
