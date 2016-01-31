/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "wrap_ParticleSystem.h"
#include "common/Vector.h"

#include "Image.h"
#include "Canvas.h"
#include "graphics/wrap_Texture.h"

// C
#include <cstring>

// C++
#include <typeinfo>

namespace love
{
namespace graphics
{
namespace opengl
{

ParticleSystem *luax_checkparticlesystem(lua_State *L, int idx)
{
	return luax_checktype<ParticleSystem>(L, idx, GRAPHICS_PARTICLE_SYSTEM_ID);
}

int w_ParticleSystem_clone(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);

	ParticleSystem *clone = nullptr;
	luax_catchexcept(L, [&](){ clone = t->clone(); });

	luax_pushtype(L, GRAPHICS_PARTICLE_SYSTEM_ID, clone);
	clone->release();
	return 1;
}

int w_ParticleSystem_setTexture(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	Texture *tex = luax_checktexture(L, 2);
	t->setTexture(tex);
	return 0;
}

int w_ParticleSystem_getTexture(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	Texture *tex = t->getTexture();

	// FIXME: big hack right here.
	if (typeid(*tex) == typeid(Image))
		luax_pushtype(L, GRAPHICS_IMAGE_ID, tex);
	else if (typeid(*tex) == typeid(Canvas))
		luax_pushtype(L, GRAPHICS_CANVAS_ID, tex);
	else
		return luaL_error(L, "Unable to determine texture type.");

	return 1;
}

int w_ParticleSystem_setBufferSize(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_Number arg1 = luaL_checknumber(L, 2);
	if (arg1 < 1.0 || arg1 > ParticleSystem::MAX_PARTICLES)
		return luaL_error(L, "Invalid buffer size");

	luax_catchexcept(L, [&](){ t->setBufferSize((uint32) arg1); });
	return 0;
}

int w_ParticleSystem_getBufferSize(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushinteger(L, t->getBufferSize());
	return 1;
}

int w_ParticleSystem_setInsertMode(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	ParticleSystem::InsertMode mode;
	const char *str = luaL_checkstring(L, 2);
	if (!ParticleSystem::getConstant(str, mode))
		return luaL_error(L, "Invalid insert mode: '%s'", str);
	t->setInsertMode(mode);
	return 0;
}

int w_ParticleSystem_getInsertMode(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	ParticleSystem::InsertMode mode;
	mode = t->getInsertMode();
	const char *str;
	if (!ParticleSystem::getConstant(mode, str))
		return luaL_error(L, "Unknown insert mode");
	lua_pushstring(L, str);
	return 1;
}

int w_ParticleSystem_setEmissionRate(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float) luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setEmissionRate(arg1); });
	return 0;
}

int w_ParticleSystem_getEmissionRate(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getEmissionRate());
	return 1;
}

int w_ParticleSystem_setEmitterLifetime(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setEmitterLifetime(arg1);
	return 0;
}

int w_ParticleSystem_getEmitterLifetime(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getEmitterLifetime());
	return 1;
}

int w_ParticleSystem_setParticleLifetime(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setParticleLifetime(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getParticleLifetime(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float min, max;
	t->getParticleLifetime(min, max);
	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	return 2;
}

int w_ParticleSystem_setPosition(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	t->setPosition(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getPosition(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	love::Vector pos = t->getPosition();
	lua_pushnumber(L, pos.getX());
	lua_pushnumber(L, pos.getY());
	return 2;
}

int w_ParticleSystem_moveTo(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	t->moveTo(arg1, arg2);
	return 0;
}

int w_ParticleSystem_setAreaSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);

	ParticleSystem::AreaSpreadDistribution distribution = ParticleSystem::DISTRIBUTION_NONE;
	float x = 0.f, y = 0.f;

	const char *str = lua_isnoneornil(L, 2) ? 0 : luaL_checkstring(L, 2);
	if (str && !ParticleSystem::getConstant(str, distribution))
		return luaL_error(L, "Invalid particle distribution: %s", str);

	if (distribution != ParticleSystem::DISTRIBUTION_NONE)
	{
		x = (float) luaL_checknumber(L, 3);
		y = (float) luaL_checknumber(L, 4);
		if (x < 0.0f || y < 0.0f)
			return luaL_error(L, "Invalid area spread parameters (must be >= 0)");
	}

	t->setAreaSpread(distribution, x, y);
	return 0;
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

int w_ParticleSystem_setDirection(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setDirection(arg1);
	return 0;
}

int w_ParticleSystem_getDirection(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getDirection());
	return 1;
}

int w_ParticleSystem_setSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setSpread(arg1);
	return 0;
}

int w_ParticleSystem_getSpread(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getSpread());
	return 1;
}

int w_ParticleSystem_setSpeed(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setSpeed(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getSpeed(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float min, max;
	t->getSpeed(min, max);
	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	return 2;
}

int w_ParticleSystem_setLinearAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float xmin = (float) luaL_checknumber(L, 2);
	float ymin = (float) luaL_checknumber(L, 3);
	float xmax = (float) luaL_optnumber(L, 4, xmin);
	float ymax = (float) luaL_optnumber(L, 5, ymin);
	t->setLinearAcceleration(xmin, ymin, xmax, ymax);
	return 0;
}

int w_ParticleSystem_getLinearAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	love::Vector min, max;
	t->getLinearAcceleration(min, max);
	lua_pushnumber(L, min.x);
	lua_pushnumber(L, min.y);
	lua_pushnumber(L, max.x);
	lua_pushnumber(L, max.y);
	return 4;
}

int w_ParticleSystem_setRadialAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setRadialAcceleration(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getRadialAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float min, max;
	t->getRadialAcceleration(min, max);
	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	return 2;
}

int w_ParticleSystem_setTangentialAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setTangentialAcceleration(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getTangentialAcceleration(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float min, max;
	t->getTangentialAcceleration(min, max);
	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	return 2;
}

int w_ParticleSystem_setLinearDamping(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setLinearDamping(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getLinearDamping(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float min, max;
	t->getLinearDamping(min, max);
	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	return 2;
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
			sizes[i] = luax_checkfloat(L, (int) (1 + i + 1));

		t->setSizes(sizes);
	}
	return 0;
}

int w_ParticleSystem_getSizes(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	const std::vector<float> &sizes = t->getSizes();

	for (size_t i = 0; i < sizes.size(); i++)
		lua_pushnumber(L, sizes[i]);

	return (int) sizes.size();
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

int w_ParticleSystem_getSizeVariation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getSizeVariation());
	return 1;
}

int w_ParticleSystem_setRotation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setRotation(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getRotation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float min, max;
	t->getRotation(min, max);
	lua_pushnumber(L, min);
	lua_pushnumber(L, max);
	return 2;
}

int w_ParticleSystem_setSpin(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_optnumber(L, 3, arg1);
	t->setSpin(arg1, arg2);
	return 0;
}

int w_ParticleSystem_getSpin(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float start, end;
	t->getSpin(start, end);
	lua_pushnumber(L, start);
	lua_pushnumber(L, end);
	return 2;
}

int w_ParticleSystem_setSpinVariation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setSpinVariation(arg1);
	return 0;
}

int w_ParticleSystem_getSpinVariation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getSpinVariation());
	return 1;
}

int w_ParticleSystem_setOffset(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	t->setOffset(x, y);
	return 0;
}

int w_ParticleSystem_getOffset(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	love::Vector offset = t->getOffset();
	lua_pushnumber(L, offset.getX());
	lua_pushnumber(L, offset.getY());
	return 2;
}

int w_ParticleSystem_setColors(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);

	if (lua_istable(L, 2)) // setColors({r,g,b,a}, {r,g,b,a}, ...)
	{
		int nColors = (int) lua_gettop(L) - 1;

		if (nColors > 8)
			return luaL_error(L, "At most eight (8) colors may be used.");

		std::vector<Colorf> colors(nColors);

		for (int i = 0; i < nColors; i++)
		{
			luaL_checktype(L, i + 2, LUA_TTABLE);

			if (luax_objlen(L, i + 2) < 3)
				return luaL_argerror(L, i + 2, "expected 4 color components");

			for (int j = 0; j < 4; j++)
				// push args[i+2][j+1] onto the stack
				lua_rawgeti(L, i + 2, j + 1);

			colors[i].r = (float) luaL_checknumber(L, -4);
			colors[i].g = (float) luaL_checknumber(L, -3);
			colors[i].b = (float) luaL_checknumber(L, -2);
			colors[i].a = (float) luaL_optnumber(L, -1, 255);

			// pop the color components from the stack
			lua_pop(L, 4);
		}

		t->setColor(colors);
	}
	else // setColors(r,g,b,a, r,g,b,a, ...)
	{
		int cargs = lua_gettop(L) - 1;
		int nColors = (cargs + 3) / 4; // nColors = ceil(color_args / 4)

		if (cargs != 3 && (cargs % 4 != 0 || cargs == 0))
			return luaL_error(L, "Expected red, green, blue, and alpha. Only got %d of 4 components.", cargs % 4);

		if (nColors > 8)
			return luaL_error(L, "At most eight (8) colors may be used.");

		std::vector<Colorf> colors(nColors);

		for (int i = 0; i < nColors; ++i)
		{
			colors[i].r = (float) luaL_checknumber(L, 1 + i*4 + 1);
			colors[i].g = (float) luaL_checknumber(L, 1 + i*4 + 2);
			colors[i].b = (float) luaL_checknumber(L, 1 + i*4 + 3);
			colors[i].a = (float) luaL_checknumber(L, 1 + i*4 + 4);
		}

		t->setColor(colors);
	}

	return 0;
}

int w_ParticleSystem_getColors(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);

	const std::vector<Colorf> &colors =t->getColor();

	for (size_t i = 0; i < colors.size(); i++)
	{
		lua_createtable(L, 4, 0);

		lua_pushnumber(L, colors[i].r);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, colors[i].g);
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, colors[i].b);
		lua_rawseti(L, -2, 3);
		lua_pushnumber(L, colors[i].a);
		lua_rawseti(L, -2, 4);
	}

	return (int) colors.size();
}

int w_ParticleSystem_setQuads(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	std::vector<Quad *> quads;

	if (lua_istable(L, 2))
	{
		for (int i = 1; i <= (int) luax_objlen(L, 2); i++)
		{
			lua_rawgeti(L, 2, i);

			Quad *q = luax_checktype<Quad>(L, -1, GRAPHICS_QUAD_ID);
			quads.push_back(q);

			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 2; i <= lua_gettop(L); i++)
		{
			Quad *q = luax_checktype<Quad>(L, i, GRAPHICS_QUAD_ID);
			quads.push_back(q);
		}
	}

	t->setQuads(quads);
	return 0;
}

int w_ParticleSystem_getQuads(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	const std::vector<Quad *> quads = t->getQuads();

	lua_createtable(L, (int) quads.size(), 0);

	for (int i = 0; i < (int) quads.size(); i++)
	{
		luax_pushtype(L, GRAPHICS_QUAD_ID, quads[i]);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int w_ParticleSystem_setRelativeRotation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	t->setRelativeRotation(luax_toboolean(L, 2));
	return 0;
}

int w_ParticleSystem_hasRelativeRotation(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->hasRelativeRotation());
	return 1;
}

int w_ParticleSystem_getCount(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	lua_pushnumber(L, t->getCount());
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

int w_ParticleSystem_emit(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	int num = (int) luaL_checknumber(L, 2);
	t->emit(num);
	return 0;
}

int w_ParticleSystem_isActive(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->isActive());
	return 1;
}

int w_ParticleSystem_isPaused(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->isPaused());
	return 1;
}

int w_ParticleSystem_isStopped(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	luax_pushboolean(L, t->isStopped());
	return 1;
}

int w_ParticleSystem_update(lua_State *L)
{
	ParticleSystem *t = luax_checkparticlesystem(L, 1);
	float dt = (float)luaL_checknumber(L, 2);
	t->update(dt);
	return 0;
}

static const luaL_Reg w_ParticleSystem_functions[] =
{
	{ "clone", w_ParticleSystem_clone },
	{ "setTexture", w_ParticleSystem_setTexture },
	{ "getTexture", w_ParticleSystem_getTexture },
	{ "setBufferSize", w_ParticleSystem_setBufferSize },
	{ "getBufferSize", w_ParticleSystem_getBufferSize },
	{ "setInsertMode", w_ParticleSystem_setInsertMode },
	{ "getInsertMode", w_ParticleSystem_getInsertMode },
	{ "setEmissionRate", w_ParticleSystem_setEmissionRate },
	{ "getEmissionRate", w_ParticleSystem_getEmissionRate },
	{ "setEmitterLifetime", w_ParticleSystem_setEmitterLifetime },
	{ "getEmitterLifetime", w_ParticleSystem_getEmitterLifetime },
	{ "setParticleLifetime", w_ParticleSystem_setParticleLifetime },
	{ "getParticleLifetime", w_ParticleSystem_getParticleLifetime },
	{ "setPosition", w_ParticleSystem_setPosition },
	{ "getPosition", w_ParticleSystem_getPosition },
	{ "moveTo", w_ParticleSystem_moveTo },
	{ "setAreaSpread", w_ParticleSystem_setAreaSpread },
	{ "getAreaSpread", w_ParticleSystem_getAreaSpread },
	{ "setDirection", w_ParticleSystem_setDirection },
	{ "getDirection", w_ParticleSystem_getDirection },
	{ "setSpread", w_ParticleSystem_setSpread },
	{ "getSpread", w_ParticleSystem_getSpread },
	{ "setSpeed", w_ParticleSystem_setSpeed },
	{ "getSpeed", w_ParticleSystem_getSpeed },
	{ "setLinearAcceleration", w_ParticleSystem_setLinearAcceleration },
	{ "getLinearAcceleration", w_ParticleSystem_getLinearAcceleration },
	{ "setRadialAcceleration", w_ParticleSystem_setRadialAcceleration },
	{ "getRadialAcceleration", w_ParticleSystem_getRadialAcceleration },
	{ "setTangentialAcceleration", w_ParticleSystem_setTangentialAcceleration },
	{ "getTangentialAcceleration", w_ParticleSystem_getTangentialAcceleration },
	{ "setLinearDamping", w_ParticleSystem_setLinearDamping },
	{ "getLinearDamping", w_ParticleSystem_getLinearDamping },
	{ "setSizes", w_ParticleSystem_setSizes },
	{ "getSizes", w_ParticleSystem_getSizes },
	{ "setSizeVariation", w_ParticleSystem_setSizeVariation },
	{ "getSizeVariation", w_ParticleSystem_getSizeVariation },
	{ "setRotation", w_ParticleSystem_setRotation },
	{ "getRotation", w_ParticleSystem_getRotation },
	{ "setSpin", w_ParticleSystem_setSpin },
	{ "getSpin", w_ParticleSystem_getSpin },
	{ "setSpinVariation", w_ParticleSystem_setSpinVariation },
	{ "getSpinVariation", w_ParticleSystem_getSpinVariation },
	{ "setColors", w_ParticleSystem_setColors },
	{ "getColors", w_ParticleSystem_getColors },
	{ "setQuads", w_ParticleSystem_setQuads },
	{ "getQuads", w_ParticleSystem_getQuads },
	{ "setOffset", w_ParticleSystem_setOffset },
	{ "getOffset", w_ParticleSystem_getOffset },
	{ "setRelativeRotation", w_ParticleSystem_setRelativeRotation },
	{ "hasRelativeRotation", w_ParticleSystem_hasRelativeRotation },
	{ "getCount", w_ParticleSystem_getCount },
	{ "start", w_ParticleSystem_start },
	{ "stop", w_ParticleSystem_stop },
	{ "pause", w_ParticleSystem_pause },
	{ "reset", w_ParticleSystem_reset },
	{ "emit", w_ParticleSystem_emit },
	{ "isActive", w_ParticleSystem_isActive },
	{ "isPaused", w_ParticleSystem_isPaused },
	{ "isStopped", w_ParticleSystem_isStopped },
	{ "update", w_ParticleSystem_update },
	{ 0, 0 }
};

extern "C" int luaopen_particlesystem(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_PARTICLE_SYSTEM_ID, "ParticleSystem", w_ParticleSystem_functions, nullptr);
}

} // opengl
} // graphics
} // love
