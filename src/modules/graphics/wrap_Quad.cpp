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
#include "wrap_Quad.h"

namespace love
{
namespace graphics
{

Quad *luax_checkquad(lua_State *L, int idx)
{
	return luax_checktype<Quad>(L, idx, GRAPHICS_QUAD_ID);
}

int w_Quad_setViewport(lua_State *L)
{
	Quad *quad = luax_checkquad(L, 1);

	Quad::Viewport v;
	v.x = luaL_checknumber(L, 2);
	v.y = luaL_checknumber(L, 3);
	v.w = luaL_checknumber(L, 4);
	v.h = luaL_checknumber(L, 5);

	if (lua_isnoneornil(L, 6))
		quad->setViewport(v);
	else
	{
		double sw = luaL_checknumber(L, 6);
		double sh = luaL_checknumber(L, 7);
		quad->refresh(v, sw, sh);
	}

	return 0;
}

int w_Quad_getViewport(lua_State *L)
{
	Quad *quad = luax_checkquad(L, 1);
	Quad::Viewport v = quad->getViewport();
	lua_pushnumber(L, v.x);
	lua_pushnumber(L, v.y);
	lua_pushnumber(L, v.w);
	lua_pushnumber(L, v.h);
	return 4;
}

static const luaL_Reg w_Quad_functions[] =
{
	{ "setViewport", w_Quad_setViewport },
	{ "getViewport", w_Quad_getViewport },
	{ 0, 0 }
};

extern "C" int luaopen_quad(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_QUAD_ID, "Quad", w_Quad_functions, nullptr);
}

} // graphics
} // love
