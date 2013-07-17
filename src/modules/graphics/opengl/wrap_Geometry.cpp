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

int w_Geometry_getVertexCount(lua_State *L)
{
	Geometry *geom = luax_checkgeometry(L, 1);
	lua_pushinteger(L, geom->getVertexCount());
	return 1;
}

int w_Geometry_getVertex(lua_State *L)
{
	Geometry *geom = luax_checkgeometry(L, 1);
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
	Geometry *geom = luax_checkgeometry(L, 1);
	size_t i = size_t(luaL_checkinteger(L, 2));

	vertex v;

	if (lua_istable(L, 3))
	{
		for (int i = 1; i <= 8; i++)
			lua_rawgeti(L, 3, i);

		v.x = luaL_checknumber(L, -8);
		v.y = luaL_checknumber(L, -7);
		v.s = luaL_checknumber(L, -6);
		v.t = luaL_checknumber(L, -5);
		v.r = luaL_optinteger(L, -4, 255);
		v.g = luaL_optinteger(L, -3, 255);
		v.b = luaL_optinteger(L, -2, 255);
		v.a = luaL_optinteger(L, -1, 255);

		lua_pop(L, 8);
	}
	else
	{
		v.x = luaL_checknumber(L, 3);
		v.y = luaL_checknumber(L, 4);
		v.s = luaL_checknumber(L, 5);
		v.t = luaL_checknumber(L, 6);
		v.r = luaL_optinteger(L,  7, 255);
		v.g = luaL_optinteger(L,  8, 255);
		v.b = luaL_optinteger(L,  9, 255);
		v.a = luaL_optinteger(L, 10, 255);
	}

	try
	{
		geom->setVertex(i-1, v);
	}
	catch (Exception &e)
	{
		return luaL_error(L, e.what());
	}

	if (v.r != 255 || v.g != 255 || v.b != 255 || v.a != 255)
		geom->setVertexColors(true);

	return 0;
}

int w_Geometry_flip(lua_State *L)
{
	Geometry *geom = luax_checkgeometry(L, 1);
	geom->flip(luax_toboolean(L, 2), luax_toboolean(L, 3));
	return 0;
}

int w_Geometry_setVertexColors(lua_State *L)
{
	Geometry *geom = luax_checkgeometry(L, 1);
	geom->setVertexColors(luax_toboolean(L, 2));
	return 0;
}

int w_Geometry_hasVertexColors(lua_State *L)
{
	Geometry *geom = luax_checkgeometry(L, 1);
	luax_pushboolean(L, geom->hasVertexColors());
	return 1;
}

int w_Geometry_getDrawMode(lua_State *L)
{
	Geometry *geom = luax_checkgeometry(L, 1);

	Geometry::DrawMode mode = geom->getDrawMode();
	const char *str;

	if (!Geometry::getConstant(mode, str))
		return luaL_error(L, "Unknown Geometry draw mode");

	lua_pushstring(L, str);

	return 1;
}

int w_Geometry_getVertexMap(lua_State *L)
{
	Geometry *g = luax_checkgeometry(L, 1);

	size_t elemcount = g->getElementCount();
	const uint16 *elements = g->getElementArray();

	if (elemcount == 0 || elements == 0)
		return 0;

	lua_createtable(L, elemcount, 0);
	for (size_t i = 0; i < elemcount; i++)
	{
		lua_pushinteger(L, elements[i]);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int w_Geometry_setVertexMap(lua_State *L)
{
	Geometry *g = luax_checkgeometry(L, 1);

	for (int i = lua_gettop(L); i >= 2; i--)
	{
		if (lua_isnil(L, i))
			lua_pop(L, 1);
		else
			break;
	}

	bool is_table = lua_istable(L, 2);
	int nargs = is_table ? lua_objlen(L, 2) : lua_gettop(L) - 1;

	std::vector<uint16> vertexmap;
	vertexmap.reserve(nargs);

	for (int i = 0; i < nargs; i++)
	{
		if (is_table)
		{
			lua_rawgeti(L, 2, i + 1);
			vertexmap.push_back(luaL_checkinteger(L, -1) - 1);
			lua_pop(L, 1);
		}
		else
			vertexmap.push_back(luaL_checkinteger(L, i + 2) - 1);
	}

	try
	{
		g->setElementArray(&vertexmap[0], vertexmap.size());
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	return 0;
}

static const luaL_Reg w_Geometry_functions[] =
{
	{ "getVertexCount", w_Geometry_getVertexCount },
	{ "getVertex", w_Geometry_getVertex },
	{ "setVertex", w_Geometry_setVertex },
	{ "flip", w_Geometry_flip },
	{ "setVertexColors", w_Geometry_setVertexColors },
	{ "hasVertexColors", w_Geometry_hasVertexColors },
	{ "getDrawMode", w_Geometry_getDrawMode },
	{ "getVertexMap", w_Geometry_getVertexMap },
	{ "setVertexMap", w_Geometry_setVertexMap },
	{ 0, 0 }
};

extern "C" int luaopen_geometry(lua_State *L)
{
	return luax_register_type(L, "Geometry", w_Geometry_functions);
}

} // opengl
} // graphics
} // love
