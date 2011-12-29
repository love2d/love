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

#include "wrap_Source.h"

namespace love
{
namespace audio
{
	Source * luax_checksource(lua_State * L, int idx)
	{
		return luax_checktype<Source>(L, idx, "Source", AUDIO_SOURCE_T);
	}

	int w_Source_play(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->play();
		return 0;
	}

	int w_Source_stop(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->stop();
		return 0;
	}

	int w_Source_pause(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->pause();
		return 0;
	}

	int w_Source_resume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->resume();
		return 0;
	}

	int w_Source_rewind(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->rewind();
		return 0;
	}

	int w_Source_setPitch(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float p = (float)luaL_checknumber(L, 2);
		t->setPitch(p);
		return 0;
	}

	int w_Source_getPitch(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		lua_pushnumber(L, t->getPitch());
		return 1;
	}

	int w_Source_setVolume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float p = (float)luaL_checknumber(L, 2);
		t->setVolume(p);
		return 0;
	}

	int w_Source_getVolume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		lua_pushnumber(L, t->getVolume());
		return 1;
	}

	int w_Source_seek(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float offset = (float)luaL_checknumber(L, 2);
		const char * unit = luaL_optstring(L, 3, "seconds");
		Source::Unit u;
		t->getConstant(unit, u);
		t->seek(offset, u);
		return 0;
	}

	int w_Source_tell(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		const char * unit = luaL_optstring(L, 2, "seconds");
		Source::Unit u;
		t->getConstant(unit, u);
		lua_pushnumber(L, t->tell(u));
		return 1;
	}

	int w_Source_setPosition(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float v[3];
		v[0] = (float)luaL_checknumber(L, 2);
		v[1] = (float)luaL_checknumber(L, 3);
		v[2] = (float)luaL_checknumber(L, 4);
		t->setPosition(v);
		return 0;
	}

	int w_Source_getPosition(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float v[3];
		t->getPosition(v);
		lua_pushnumber(L, v[0]);
		lua_pushnumber(L, v[1]);
		lua_pushnumber(L, v[2]);
		return 3;
	}

	int w_Source_setVelocity(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float v[3];
		v[0] = (float)luaL_checknumber(L, 2);
		v[1] = (float)luaL_checknumber(L, 3);
		v[2] = (float)luaL_checknumber(L, 4);
		t->setVelocity(v);
		return 0;
	}

	int w_Source_getVelocity(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float v[3];
		t->getVelocity(v);
		lua_pushnumber(L, v[0]);
		lua_pushnumber(L, v[1]);
		lua_pushnumber(L, v[2]);
		return 3;
	}

	int w_Source_setDirection(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float v[3];
		v[0] = (float)luaL_checknumber(L, 2);
		v[1] = (float)luaL_checknumber(L, 3);
		v[2] = (float)luaL_checknumber(L, 4);
		t->setDirection(v);
		return 0;
	}

	int w_Source_getDirection(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		float v[3];
		t->getDirection(v);
		lua_pushnumber(L, v[0]);
		lua_pushnumber(L, v[1]);
		lua_pushnumber(L, v[2]);
		return 3;
	}

	int w_Source_setLooping(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->setLooping(luax_toboolean(L, 2));
		return 0;
	}

	int w_Source_isLooping(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		luax_pushboolean(L, t->isLooping());
		return 1;
	}

	int w_Source_isStopped(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		luax_pushboolean(L, t->isStopped());
		return 1;
	}

	int w_Source_isPaused(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		luax_pushboolean(L, t->isPaused());
		return 1;
	}

	int w_Source_isStatic(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		luax_pushboolean(L, t->isStatic());
		return 1;
	}

	int w_Source_setMinVolume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->setMinVolume((float)luaL_checknumber(L, 2));
		return 0;
	}

	int w_Source_getMinVolume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		lua_pushnumber(L, t->getMinVolume());
		return 1;
	}

	int w_Source_setMaxVolume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->setMaxVolume((float)luaL_checknumber(L, 2));
		return 0;
	}

	int w_Source_getMaxVolume(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		lua_pushnumber(L, t->getMaxVolume());
		return 1;
	}

	int w_Source_setDistance(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->setReferenceDistance((float)luaL_checknumber(L, 2));
		if (!lua_isnoneornil(L, 3)) // lua_isnoneornil instead of lua_isnumber to throw an error
			t->setMaxDistance((float)luaL_checknumber(L, 3)); // in case of invalid argument type
		return 0;
	}

	int w_Source_getDistance(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		lua_pushnumber(L, t->getReferenceDistance());
		lua_pushnumber(L, t->getMaxDistance());
		return 2;
	}

	int w_Source_setRolloffFactor(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		t->setRolloffFactor((float)luaL_checknumber(L, 2));
		return 0;
	}

	int w_Source_getRolloffFactor(lua_State * L)
	{
		Source * t = luax_checksource(L, 1);
		lua_pushnumber(L, t->getRolloffFactor());
		return 1;
	}

	static const luaL_Reg functions[] = {
		{ "play", w_Source_play },
		{ "stop", w_Source_stop },
		{ "pause", w_Source_pause },
		{ "resume", w_Source_resume },
		{ "rewind", w_Source_rewind },

		{ "setPitch", w_Source_setPitch },
		{ "getPitch", w_Source_getPitch },
		{ "setVolume", w_Source_setVolume },
		{ "getVolume", w_Source_getVolume },
		{ "seek", w_Source_seek },
		{ "tell", w_Source_tell },
		{ "setPosition", w_Source_setPosition },
		{ "getPosition", w_Source_getPosition },
		{ "setVelocity", w_Source_setVelocity },
		{ "getVelocity", w_Source_getVelocity },
		{ "setDirection", w_Source_setDirection },
		{ "getDirection", w_Source_getDirection },

		{ "setLooping", w_Source_setLooping },
		{ "isLooping", w_Source_isLooping },
		{ "isStopped", w_Source_isStopped },
		{ "isPaused", w_Source_isPaused },
		{ "isStatic", w_Source_isStatic },

		{ "setMinVolume", w_Source_setMinVolume },
		{ "getMinVolume", w_Source_getMinVolume },
		{ "setMaxVolume", w_Source_setMaxVolume },
		{ "getMaxVolume", w_Source_getMaxVolume },
		{ "setDistance", w_Source_setDistance },
		{ "getDistance", w_Source_setDistance },
		{ "setRolloffFactor", w_Source_setRolloffFactor },
		{ "getRolloffFactor", w_Source_getRolloffFactor },

		{ 0, 0 }
	};

	extern "C" int luaopen_source(lua_State * L)
	{
		return luax_register_type(L, "Source", functions);
	}

} // audio
} // love
