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

#include "wrap_ParticleSystem.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	ParticleSystem * luax_checkparticlesystem(lua_State * L, int idx)
	{
		return luax_checktype<ParticleSystem>(L, idx, "ParticleSystem", LOVE_GRAPHICS_PARTICLE_SYSTEM_BITS);
	}

	int _wrap_ParticleSystem_setSprite(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		Image * i = luax_checkimage(L, 2);
		t->setSprite(i);
		return 0;
	}

	int _wrap_ParticleSystem_setBufferSize(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		int arg1 = luaL_checkint(L, 2);
		t->setBufferSize((unsigned int)arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setEmissionRate(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		int arg1 = luaL_checkint(L, 2);
		t->setEmissionRate((unsigned int)arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setLifetime(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setLifetime(arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setParticleLife(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setParticleLife(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setPosition(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setPosition(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setDirection(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setDirection(arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setSpread(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setSpread(arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setRelativeDirection(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		bool arg1 = (bool)luax_toboolean(L, 2);
		t->setRelativeDirection(arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setSpeed(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		t->setSpeed(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setGravity(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		t->setGravity(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setRadialAcceleration(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		t->setRadialAcceleration(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setTangentialAcceleration(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		t->setTangentialAcceleration(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setSize(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		float arg3 = (float)luaL_optnumber(L, 3, 0);
		t->setSize(arg1, arg2, arg3);
		return 0;
	}

	int _wrap_ParticleSystem_setSizeVariation(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setSizeVariation(arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setRotation(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		t->setRotation(arg1, arg2);
		return 0;
	}

	int _wrap_ParticleSystem_setSpin(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_optnumber(L, 3, arg1);
		float arg3 = (float)luaL_optnumber(L, 3, 0);
		t->setSpin(arg1, arg2, arg3);
		return 0;
	}

	int _wrap_ParticleSystem_setSpinVariation(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setSpinVariation(arg1);
		return 0;
	}

	int _wrap_ParticleSystem_setColor(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		Color * start = luax_checkcolor(L, 2);
		Color * end = (lua_gettop(L) == 3) ? luax_checkcolor(L, 3) : start;
		t->setColor(start, end);
		return 0;
	}

	int _wrap_ParticleSystem_getX(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		lua_pushnumber(L, t->getX());
		return 1;
	}

	int _wrap_ParticleSystem_getY(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		lua_pushnumber(L, t->getY());
		return 1;
	}

	int _wrap_ParticleSystem_getDirection(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		lua_pushnumber(L, t->getDirection());
		return 1;
	}

	int _wrap_ParticleSystem_getSpread(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		lua_pushnumber(L, t->getSpread());
		return 1;
	}

	int _wrap_ParticleSystem_count(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		lua_pushnumber(L, t->count());
		return 1;
	}

	int _wrap_ParticleSystem_start(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		t->start();
		return 0;
	}

	int _wrap_ParticleSystem_stop(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		t->stop();
		return 0;
	}

	int _wrap_ParticleSystem_pause(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		t->pause();
		return 0;
	}

	int _wrap_ParticleSystem_reset(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		t->reset();
		return 0;
	}

	int _wrap_ParticleSystem_isActive(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		luax_pushboolean(L, t->isActive());
		return 1;
	}

	int _wrap_ParticleSystem_isEmpty(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		luax_pushboolean(L, t->isEmpty());
		return 1;
	}

	int _wrap_ParticleSystem_isFull(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		luax_pushboolean(L, t->isFull());
		return 1;
	}

	int _wrap_ParticleSystem_update(lua_State * L)
	{
		ParticleSystem * t = luax_checkparticlesystem(L, 1);
		float dt = (float)luaL_checknumber(L, 2);
		t->update(dt);
		return 0;
	}

	static const luaL_Reg wrap_ParticleSystem_functions[] = {
		{ "setSprite", _wrap_ParticleSystem_setSprite },
		{ "setBufferSize", _wrap_ParticleSystem_setBufferSize },
		{ "setEmissionRate", _wrap_ParticleSystem_setEmissionRate },
		{ "setLifeTime", _wrap_ParticleSystem_setLifetime },
		{ "setParticleLife", _wrap_ParticleSystem_setParticleLife },
		{ "setPosition", _wrap_ParticleSystem_setPosition },
		{ "setDirection", _wrap_ParticleSystem_setDirection },
		{ "setSpread", _wrap_ParticleSystem_setSpread },
		{ "setRelativeDirection", _wrap_ParticleSystem_setRelativeDirection },
		{ "setSpeed", _wrap_ParticleSystem_setSpeed },
		{ "setGravity", _wrap_ParticleSystem_setGravity },
		{ "setRadialAcceleration", _wrap_ParticleSystem_setRadialAcceleration },
		{ "setTangentialAcceleration", _wrap_ParticleSystem_setTangentialAcceleration },
		{ "setSize", _wrap_ParticleSystem_setSize },
		{ "setSizeVariation", _wrap_ParticleSystem_setSizeVariation },
		{ "setRotation", _wrap_ParticleSystem_setRotation },
		{ "setSpin", _wrap_ParticleSystem_setSpin },
		{ "setSpinVariation", _wrap_ParticleSystem_setSpinVariation },
		{ "setColor", _wrap_ParticleSystem_setColor },
		{ "getX", _wrap_ParticleSystem_getX },
		{ "getY", _wrap_ParticleSystem_getY },
		{ "getDirection", _wrap_ParticleSystem_getDirection },
		{ "getSpread", _wrap_ParticleSystem_getSpread },
		{ "count", _wrap_ParticleSystem_count },
		{ "start", _wrap_ParticleSystem_start },
		{ "stop", _wrap_ParticleSystem_stop },
		{ "pause", _wrap_ParticleSystem_pause },
		{ "reset", _wrap_ParticleSystem_reset },
		{ "isActive", _wrap_ParticleSystem_isActive },
		{ "isEmpty", _wrap_ParticleSystem_isEmpty },
		{ "isFull", _wrap_ParticleSystem_isFull },
		{ "update", _wrap_ParticleSystem_update },
		{ 0, 0 }
	};

	int wrap_ParticleSystem_open(lua_State * L)
	{
		luax_register_type(L, "ParticleSystem", wrap_ParticleSystem_functions);
		return 0;
	}

} // opengl
} // graphics
} // love
