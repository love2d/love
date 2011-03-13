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

#include "null/Audio.h"

#include <scripts/audio.lua.h>

#include <common/runtime.h>

namespace love
{
namespace audio
{
	Audio * instance = 0;

	int w_getNumSources(lua_State * L)
	{
		lua_pushinteger(L, instance->getNumSources());
		return 1;
	}

	int w_newSource1(lua_State * L)
	{
		Source * t = 0;

		if(luax_istype(L, 1, SOUND_SOUND_DATA_T))
			t = instance->newSource(luax_totype<love::sound::SoundData>(L, 1, "SoundData", SOUND_SOUND_DATA_T));
		else if(luax_istype(L, 1, SOUND_DECODER_T))
			t = instance->newSource(luax_totype<love::sound::Decoder>(L, 1, "Decoder", SOUND_DECODER_T));

		if(t)
		{
			luax_newtype(L, "Source", AUDIO_SOURCE_T, (void*)t);
			return 1;
		}
		else
			return luaL_error(L, "No matching overload");

		return 0;
	}

	int w_play(lua_State * L)
	{
		Source * s = luax_checksource(L, 1);
		instance->play(s);
		return 0;
	}

	int w_stop(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->stop();
		}
		else
		{
			Source * s = luax_checksource(L, 1);
			s->stop();
		}
		return 0;
	}

	int w_pause(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->pause();
		}
		else
		{
			Source * s = luax_checksource(L, 1);
			s->pause();
		}

		return 0;
	}

	int w_resume(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->resume();
		}
		else
		{
			Source * s = luax_checksource(L, 1);
			s->resume();
		}
		return 0;
	}

	int w_rewind(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->rewind();
		}
		else
		{
			Source * s = luax_checksource(L, 1);
			s->rewind();
		}
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
	
	int w_record(lua_State *)
	{
		instance->record();
		return 0;
	}
	
	int w_getRecordedData(lua_State * L)
	{
		love::sound::SoundData * sd = instance->getRecordedData();
		if (!sd) lua_pushnil(L);
		else luax_newtype(L, "SoundData", SOUND_SOUND_DATA_T, (void*)sd);
		return 1;
	}
	
	int w_stopRecording(lua_State * L)
	{
		if (luax_optboolean(L, 1, true)) {
			love::sound::SoundData * sd = instance->stopRecording(true);
			if (!sd) lua_pushnil(L);
			else luax_newtype(L, "SoundData", SOUND_SOUND_DATA_T, (void*)sd);
			return 1;
		}
		instance->stopRecording(false);
		return 0;
	}
	
	int w_canRecord(lua_State * L) {
		luax_pushboolean(L, instance->canRecord());
		return 1;
	}
	

} // audio
} // love
