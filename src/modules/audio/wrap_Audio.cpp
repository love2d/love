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

namespace love
{
namespace audio
{
	static Audio * instance = 0;

	int w_getNumSources(lua_State * L)
	{
		lua_pushinteger(L, instance->getNumSources());
		return 1;
	}

	int w_newSound(lua_State * L)
	{
		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_convobj(L, 1, "filesystem", "newFile");

		// Convert to SoundData, if necessary.
		if(luax_istype(L, 1, FILESYSTEM_FILE_T))
			luax_convobj(L, 1, "sound", "newSoundData");

		love::sound::SoundData * data = luax_checktype<love::sound::SoundData>(L, 1, "SoundData", SOUND_SOUND_DATA_T);
		Sound * t = instance->newSound(data);
		luax_newtype(L, "Sound", AUDIO_SOUND_T, (void*)t);
		return 1;
	}


	int w_newMusic(lua_State * L)
	{
		// Convert to Decoder, if necessary.
		if(!luax_istype(L, 1, SOUND_DECODER_T))
			luax_convobj(L, 1, "sound", "newDecoder");

		love::sound::Decoder * decoder = luax_checktype<love::sound::Decoder>(L, 1, "Decoder", SOUND_DECODER_T);
		Music * t = instance->newMusic(decoder);
		luax_newtype(L, "Music", AUDIO_MUSIC_T, (void*)t);
		return 1;
	}

	int w_newSource(lua_State * L)
	{
		Audible * a = luax_checktype<Audible>(L, 1, "Audible", AUDIO_AUDIBLE_T);
		Source * t = instance->newSource(a);
		luax_newtype(L, "Source", AUDIO_SOURCE_T, (void*)t);
		return 1;
	}

	int w_play(lua_State * L)
	{
		int argn = lua_gettop(L);

		if(luax_istype(L, 1, AUDIO_SOUND_T))
		{
			Sound * s = luax_checksound(L, 1);
			instance->play(s);
			return 0;
		} 
		else if(luax_istype(L, 1, AUDIO_MUSIC_T))
		{
			Music * m = luax_checkmusic(L, 1);
			instance->play(m);
			return 0;
		}
		else if(luax_istype(L, 1, AUDIO_SOURCE_T))
		{
			Source * s = luax_checksource(L, 1);
			instance->play(s);
			return 0;
		}

		return luaL_error(L, "No matching overload");
	}

	int w_stop(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->stop(c);
		return 0;
	}

	int w_pause(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->pause(c);
		return 0;
	}

	int w_resume(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->resume(c);
		return 0;
	}

	int w_rewind(lua_State * L)
	{
		Source * c = luax_checksource(L, 1);
		instance->rewind(c);
		return 0;
	}

	int w_setVolume(lua_State * L)
	{
		float v = (float)luaL_checknumber(L, 1);
		instance->setVolume(v);
		return 0;
	}

	int w_getVolume(lua_State * L)
	{
		lua_pushnumber(L, instance->getVolume());
		return 1;
	}

	int w_setPosition(lua_State * L)
	{
		float v[3];
		v[0] = (float)luaL_checknumber(L, 1);
		v[1] = (float)luaL_checknumber(L, 2);
		v[2] = (float)luaL_checknumber(L, 3);
		instance->setPosition(v);
		return 0;
	}

	int w_getPosition(lua_State * L)
	{
		float v[3];
		instance->getPosition(v);
		lua_pushnumber(L, v[0]);
		lua_pushnumber(L, v[1]);
		lua_pushnumber(L, v[2]);
		return 3;
	}

	int w_setOrientation(lua_State * L)
	{
		float v[6];
		v[0] = (float)luaL_checknumber(L, 1);
		v[1] = (float)luaL_checknumber(L, 2);
		v[2] = (float)luaL_checknumber(L, 3);
		v[3] = (float)luaL_checknumber(L, 4);
		v[4] = (float)luaL_checknumber(L, 5);
		v[5] = (float)luaL_checknumber(L, 6);
		instance->setOrientation(v);
		return 0;
	}

	int w_getOrientation(lua_State * L)
	{
		float v[6];
		instance->getOrientation(v);
		lua_pushnumber(L, v[0]);
		lua_pushnumber(L, v[1]);
		lua_pushnumber(L, v[2]);
		lua_pushnumber(L, v[3]);
		lua_pushnumber(L, v[4]);
		lua_pushnumber(L, v[5]);
		return 6;
	}

	int w_setVelocity(lua_State * L)
	{
		float v[3];
		v[0] = (float)luaL_checknumber(L, 1);
		v[1] = (float)luaL_checknumber(L, 2);
		v[2] = (float)luaL_checknumber(L, 3);
		instance->setVelocity(v);
		return 0;
	}

	int w_getVelocity(lua_State * L)
	{
		float v[3];
		instance->getVelocity(v);
		lua_pushnumber(L, v[0]);
		lua_pushnumber(L, v[1]);
		lua_pushnumber(L, v[2]);
		return 3;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "getNumSources", w_getNumSources },
		{ "newSound", w_newSound },
		{ "newMusic", w_newMusic },
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
		{ 0, 0 }
	};

	static const lua_CFunction types[] = {
		luaopen_source,
		luaopen_music,
		luaopen_sound,
		0
	};	

	int luaopen_love_audio(lua_State * L)
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

		WrappedModule w;
		w.module = instance;
		w.name = "audio";
		w.flags = MODULE_T;
		w.functions = functions;
		w.types = types;

		return luax_register_module(L, w);
	}

} // audio
} // love