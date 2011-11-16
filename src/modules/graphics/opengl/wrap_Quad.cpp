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
#include "wrap_Quad.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Quad * luax_checkframe(lua_State * L, int idx)
	{
		return luax_checktype<Quad>(L, idx, "Quad", GRAPHICS_QUAD_T);
	}

	int w_Quad_flip(lua_State *L)
	{
		Quad * quad = luax_checktype<Quad>(L, 1, "Quad", GRAPHICS_QUAD_T);
		quad->flip(luax_toboolean(L, 2), luax_toboolean(L, 3));
		return 0;
	}

	int w_Quad_setViewport(lua_State * L)
	{
		Quad * quad = luax_checktype<Quad>(L, 1, "Quad", GRAPHICS_QUAD_T);
		Quad::Viewport v;
		v.x = (float) luaL_checknumber(L, 2);
		v.y = (float) luaL_checknumber(L, 3);
		v.w = (float) luaL_checknumber(L, 4);
		v.h = (float) luaL_checknumber(L, 5);
		quad->setViewport(v);
		return 0;
	}

	int w_Quad_getViewport(lua_State * L)
	{
		Quad * quad = luax_checktype<Quad>(L, 1, "Quad", GRAPHICS_QUAD_T);
		Quad::Viewport v = quad->getViewport();
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		lua_pushnumber(L, v.w);
		lua_pushnumber(L, v.h);
		return 4;
	}

	static const luaL_Reg w_Quad_functions[] = {
		{ "flip", w_Quad_flip },
		{ "setViewport", w_Quad_setViewport },
		{ "getViewport", w_Quad_getViewport },
		{ 0, 0 }
	};

	int luaopen_frame(lua_State * L)
	{
		return luax_register_type(L, "Quad", w_Quad_functions);
	}

} // opengl
} // graphics
} // love
