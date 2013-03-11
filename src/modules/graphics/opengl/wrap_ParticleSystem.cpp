/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "wrap_Quad.h"

#include "common/Vector.h"

#include <cstring>

namespace love
{
namespace graphics
{
namespace opengl
{

ParticleSystem *luax_checkparticlesystem(lua_State *L, int idx)
{
	return luax_checktype<ParticleSystem>(L, idx, "ParticleSystem", GRAPHICS_PARTICLE_SYSTEM_T);
}

int w_ParticleSystem_setSprite(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	Image *i = luax_checkimage(L, 2);
	t->setSprite(i);
	return 0;
}

int w_ParticleSystem_setBufferSize(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	int arg1 = luaL_checkint(L, 2);
	t->setBufferSize((unsigned int)arg1);
	return 0;
}

int w_ParticleSystem_setEmissionRate(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	int arg1 = luaL_checkint(L, 2);
	t->setEmissionRate((unsigned int)arg1);
	return 0;
}

int w_ParticleSystem_setLifetime(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setLifetime(arg1);
	return 0;
}

int w_ParticleSystem_setParticleLife(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setParticleLife(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setPosition(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	t->setPosition(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setAreaSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);

	ParticleSystem::AreaSpreadDistribution distribution;
	const char *str = luaL_checkstring(L, 2);
	if (!ParticleSystem::getConstant(str, distribution))
		return luaL_error(L, "Invalid distribution: '%s'", str);

	float x = (float)luaL_checknumber(L, 3);
	float y = (float)luaL_checknumber(L, 4);
	if (x < 0.0f || y < 0.0f)
		return luaL_error(L, "Invalid area spread parameters (must be >= 0)");

	t->setAreaSpread(distribution, x, y);
	return 0;
}

int w_ParticleSystem_setDirection(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setDirection(arg1);
	return 0;
}

int w_ParticleSystem_setSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setSpread(arg1);
	return 0;
}

int w_ParticleSystem_setRelativeDirection(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	bool arg1 = (bool)luax_toboolean(L, 2);
	t->setRelativeDirection(arg1);
	return 0;
}

int w_ParticleSystem_setSpeed(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setSpeed(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setGravity(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setGravity(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setRadialAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setRadialAcceleration(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setTangentialAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setTangentialAcceleration(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setSizes(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	size_t nSizes = lua_gettop(L) - 1;

	if (nSizes > 8)
		return luaL_error(L, "At most eight (8) sizes may be used.");

	if (nSizes <= 1)
	{
		float size = luax_checkfloat(L, 2);
		t->setSize(size);
	}
	else
	{
		std::vector<float> sizes(nSizes);
		for (size_t i = 0; i < nSizes; ++i)
			sizes[i] = luax_checkfloat(L, 1 + i + 1);

		t->setSize(sizes);
	}
	return 0;
}

int w_ParticleSystem_setSizeVariation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	if (arg1 < 0.0f || arg1 > 1.0f)
		return luaL_error(L, "Size variation has to be between 0 and 1, inclusive.");

	t->setSizeVariation(arg1);
	return 0;
}

int w_ParticleSystem_setRotation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setRotation(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setSpin(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	float arg3 = (float)luaL_optnumber(L, 4, 0);
	t->setSpin(arg1, arg2, arg3);
	return 0;
}

int w_ParticleSystem_setSpinVariation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setSpinVariation(arg1);
	return 0;
}

int w_ParticleSystem_setColors(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	int cargs = lua_gettop(L) - 1;
	size_t nColors = (cargs + 3) / 4; // nColors = ceil(color_args / 4)
	if (cargs % 4 != 0 || cargs == 0)
		return luaL_error(L, "Expected red, green, blue, and alpha. Only got %d of 4 components.", cargs % 4);

	if (nColors > 8)
		return luaL_error(L, "At most eight (8) colors may be used.");

	if (nColors == 1)
	{
		int r = luaL_checkint(L, 2);
		int g = luaL_checkint(L, 3);
		int b = luaL_checkint(L, 4);
		int a = luaL_checkint(L, 5);
		t->setColor(Color(r,g,b,a));
	}
	else
	{
		std::vector<Color> colors(nColors);
		for (size_t i = 0; i < nColors; ++i)
		{
			int r = luaL_checkint(L, 1 + i*4 + 1);
			int g = luaL_checkint(L, 1 + i*4 + 2);
			int b = luaL_checkint(L, 1 + i*4 + 3);
			int a = luaL_checkint(L, 1 + i*4 + 4);
			colors[i] = Color(r,g,b,a);
		}
		t->setColor(colors);
	}

	return 0;
}

int w_ParticleSystem_setQuads(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);

	int nQuads = lua_gettop(L) - 1;
	if (lua_istable(L, 2))
		nQuads = lua_objlen(L, 2);

	// Remove all quads if no argument is given.
	if (nQuads == 0)
	{
		t->setQuads();
		return 0;
	}

	std::vector<Quad *> quads(nQuads);

	if (lua_istable(L, 2))
	{
		for (int i = 0; i < nQuads; i++)
		{
			lua_pushnumber(L, i + 1); // array index
			lua_gettable(L, 2);
			quads[i] = luax_checkquad(L, -1);
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < nQuads; i++)
			quads[i] = luax_checkquad(L, i + 2);
	}

	t->setQuads(quads);

	return 0;
}

int w_ParticleSystem_setOffset(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	t->setOffset(x, y);
	return 0;
}

int w_ParticleSystem_getX(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getX());
	return 1;
}

int w_ParticleSystem_getY(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getY());
	return 1;
}

int w_ParticleSystem_getPosition(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	const love::Vector &p = t->getPosition();
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	return 2;
}

int w_ParticleSystem_getAreaSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	ParticleSystem::AreaSpreadDistribution distribution = t-> getAreaSpreadDistribution();
	const char *str;
	ParticleSystem::getConstant(distribution, str);
	const love::Vector &p = t->getAreaSpreadParameters();

	lua_pushstring(L, str);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);

	return 3;
}

int w_ParticleSystem_getDirection(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getDirection());
	return 1;
}

int w_ParticleSystem_getSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getSpread());
	return 1;
}

int w_ParticleSystem_getOffsetX(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getOffsetX());
	return 1;
}

int w_ParticleSystem_getOffsetY(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getOffsetY());
	return 1;
}

int w_ParticleSystem_count(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->count());
	return 1;
}

int w_ParticleSystem_start(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	t->start();
	return 0;
}

int w_ParticleSystem_stop(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	t->stop();
	return 0;
}

int w_ParticleSystem_pause(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	t->pause();
	return 0;
}

int w_ParticleSystem_reset(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	t->reset();
	return 0;
}

int w_ParticleSystem_isActive(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->isActive());
	return 1;
}

int w_ParticleSystem_isEmpty(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->isEmpty());
	return 1;
}

int w_ParticleSystem_isFull(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->isFull());
	return 1;
}

int w_ParticleSystem_update(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float dt = (float)luaL_checknumber(L, 2);
	t->update(dt);
	return 0;
}

static const luaL_Reg functions[] =
{
	{ "setSprite", w_ParticleSystem_setSprite },
	{ "setBufferSize", w_ParticleSystem_setBufferSize },
	{ "setEmissionRate", w_ParticleSystem_setEmissionRate },
	{ "setLifetime", w_ParticleSystem_setLifetime },
	{ "setParticleLife", w_ParticleSystem_setParticleLife },
	{ "setPosition", w_ParticleSystem_setPosition },
	{ "setAreaSpread", w_ParticleSystem_setAreaSpread },
	{ "setDirection", w_ParticleSystem_setDirection },
	{ "setSpread", w_ParticleSystem_setSpread },
	{ "setRelativeDirection", w_ParticleSystem_setRelativeDirection },
	{ "setSpeed", w_ParticleSystem_setSpeed },
	{ "setGravity", w_ParticleSystem_setGravity },
	{ "setRadialAcceleration", w_ParticleSystem_setRadialAcceleration },
	{ "setTangentialAcceleration", w_ParticleSystem_setTangentialAcceleration },
	{ "setSizes", w_ParticleSystem_setSizes },
	{ "setSizeVariation", w_ParticleSystem_setSizeVariation },
	{ "setRotation", w_ParticleSystem_setRotation },
	{ "setSpin", w_ParticleSystem_setSpin },
	{ "setSpinVariation", w_ParticleSystem_setSpinVariation },
	{ "setColors", w_ParticleSystem_setColors },
	{ "setQuads", w_ParticleSystem_setQuads },
	{ "setOffset", w_ParticleSystem_setOffset },
	{ "getX", w_ParticleSystem_getX },
	{ "getY", w_ParticleSystem_getY },
	{ "getPosition", w_ParticleSystem_getPosition },
	{ "getAreaSpread", w_ParticleSystem_getAreaSpread },
	{ "getDirection", w_ParticleSystem_getDirection },
	{ "getSpread", w_ParticleSystem_getSpread },
	{ "getOffsetX", w_ParticleSystem_getOffsetX },
	{ "getOffsetY", w_ParticleSystem_getOffsetY },
	{ "count", w_ParticleSystem_count },
	{ "start", w_ParticleSystem_start },
	{ "stop", w_ParticleSystem_stop },
	{ "pause", w_ParticleSystem_pause },
	{ "reset", w_ParticleSystem_reset },
	{ "isActive", w_ParticleSystem_isActive },
	{ "isEmpty", w_ParticleSystem_isEmpty },
	{ "isFull", w_ParticleSystem_isFull },
	{ "update", w_ParticleSystem_update },
	{ 0, 0 }
};

extern "C" int luaopen_particlesystem(lua_State *L)
{
	return luax_register_type(L, "ParticleSystem", functions);
}

} // opengl
} // graphics
} // love
