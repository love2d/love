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

#include "wrap_Sound.h"

// Implementations.
#include "lullaby/Sound.h"

namespace love
{
namespace sound
{
	static Sound * instance = 0;

	int w_newSoundData(lua_State * L)
	{
		SoundData * t = 0;

		if(lua_isnumber(L, 1))
		{
			int samples = luaL_checkint(L, 1);
			int sampleRate = luaL_optint(L, 2, Decoder::DEFAULT_SAMPLE_RATE);
			int bits = luaL_optint(L, 3, Decoder::DEFAULT_T);
			int channels = luaL_optint(L, 4, Decoder::DEFAULT_CHANNELS);

			try
			{
				t = instance->newSoundData(samples, sampleRate, bits, channels);
			}
			catch(love::Exception & e)
			{
				return luaL_error(L, e.what());
			}

		}
		// Must be string or decoder.
		else
		{

			// Convert to Decoder, if necessary.
			if(!luax_istype(L, 1, SOUND_DECODER_T))
			{
				w_newDecoder(L);
				lua_replace(L, 1);
			}

			try
			{
				t = instance->newSoundData(luax_checkdecoder(L, 1));
			}
			catch(love::Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_newtype(L, "SoundData", SOUND_SOUND_DATA_T, (void*)t);

		return 1;
	}

	int w_newDecoder(lua_State * L)
	{
		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_convobj(L, 1, "filesystem", "newFile");

		love::filesystem::File * file = luax_checktype<love::filesystem::File>(L, 1, "File", FILESYSTEM_FILE_T);
		int bufferSize = luaL_optint(L, 2, Decoder::DEFAULT_BUFFER_SIZE);
		int sampleRate = luaL_optint(L, 3, Decoder::DEFAULT_SAMPLE_RATE);

		try
		{
			Decoder * t = instance->newDecoder(file, bufferSize, sampleRate);
			if(t == 0)
				return luaL_error(L, "Extension \"%s\" not supported.", file->getExtension().c_str());
			luax_newtype(L, "Decoder", SOUND_DECODER_T, (void*)t);
		}
		catch(love::Exception & e)
		{
			return luaL_error(L, e.what());
		}

		return 1;
	}

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
				instance = new lullaby::Sound();
			}
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}


		WrappedModule w;
		w.module = instance;
		w.name = "sound";
		w.flags = MODULE_SOUND_T;
		w.functions = functions;
		w.types = types;
		w.constants = 0;

		return luax_register_module(L, w);
	}

} // sound
} // love
