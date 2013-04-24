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

// LOVE
#include "wrap_Geometry.h"
#include "common/Exception.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Geometry *luax_checkgeometry(lua_State *L, int idx)
{
	return luax_checktype<Geometry>(L, idx, "Geometry", GRAPHICS_GEOMETRY_T);
}

// different name than in Geometry.cpp to make the triangulation transparent
int w_Geometry_getVertexCount(lua_State *L)
{
	Geometry *geom = luax_checktype<Geometry>(L, 1, "Geometry", GRAPHICS_GEOMETRY_T);
	lua_pushinteger(L, geom->getNumVertices());
	return 1;
}

int w_Geometry_getVertex(lua_State *L)
{
	Geometry *geom = luax_checktype<Geometry>(L, 1, "Geometry", GRAPHICS_GEOMETRY_T);
	size_t i = size_t(luaL_checkint(L, 2));
	try
	{
		const vertex &v = geom->getVertex(i-1);
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		lua_pushnumber(L, v.s);
		lua_pushnumber(L, v.t);
		lua_pushnumber(L, v.r);
		lua_pushnumber(L, v.g);
		lua_pushnumber(L, v.b);
		lua_pushnumber(L, v.a);
	}
	catch (Exception &e)
	{
		return luaL_error(L, e.what());
	}

	return 8;
}

int w_Geometry_setVertex(lua_State *L)
{
	Geometry *geom = luax_checktype<Geometry>(L, 1, "Geometry", GRAPHICS_GEOMETRY_T);
	size_t i = size_t(luaL_checkint(L, 2));

	vertex v;
	v.x = luaL_checknumber(L, 3);
	v.y = luaL_checknumber(L, 4);
	v.s = luaL_checknumber(L, 5);
	v.t = luaL_checknumber(L, 6);
	v.r = luaL_optint(L,  7, 255);
	v.g = luaL_optint(L,  8, 255);
	v.b = luaL_optint(L,  9, 255);
	v.a = luaL_optint(L, 10, 255);

	try
	{
		geom->setVertex(i-1, v);
	}
	catch (Exception &e)
	{
		return luaL_error(L, e.what());
	}

	if (lua_gettop(L) > 6)
		geom->setVertexColors(true);

	return 0;
}

int w_Geometry_flip(lua_State *L)
{
	Geometry *geom = luax_checktype<Geometry>(L, 1, "Geometry", GRAPHICS_GEOMETRY_T);
	geom->flip(luax_toboolean(L, 2), luax_toboolean(L, 3));
	return 0;
}

static const luaL_Reg w_Geometry_functions[] =
{
	{ "getVertexCount", w_Geometry_getVertexCount },
	{ "getVertex", w_Geometry_getVertex },
	{ "setVertex", w_Geometry_setVertex },
	{ "flip", w_Geometry_flip },
	{ 0, 0 }
};

extern "C" int luaopen_geometry(lua_State *L)
{
	return luax_register_type(L, "Geometry", w_Geometry_functions);
}

} // opengl
} // graphics
} // love
