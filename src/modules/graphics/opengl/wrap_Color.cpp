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

// LOVE
#include "wrap_Color.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	// This macro makes checking for the correct type slightly more compact.
	Color * luax_checkcolor(lua_State * L, int idx)
	{
		return luax_checktype<Color>(L, idx, "Color", LOVE_GRAPHICS_COLOR_BITS);
	}

	int _wrap_Color_setRed(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		int arg = luaL_checkinteger(L, 2);
		t->setRed(arg);
		return 0;
	}

	int _wrap_Color_setGreen(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		int arg = luaL_checkinteger(L, 2);
		t->setGreen(arg);
		return 0;
	}

	int _wrap_Color_setBlue(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		int arg = luaL_checkinteger(L, 2);
		t->setBlue(arg);
		return 0;
	}

	int _wrap_Color_setAlpha(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		int arg = luaL_checkinteger(L, 2);
		t->setAlpha(arg);
		return 0;
	}

	int _wrap_Color_getRed(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		lua_pushinteger(L, t->getRed());		
		return 1;
	}

	int _wrap_Color_getGreen(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		lua_pushinteger(L, t->getGreen());		
		return 1;
	}

	int _wrap_Color_getBlue(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		lua_pushinteger(L, t->getBlue());		
		return 1;
	}

	int _wrap_Color_getAlpha(lua_State * L)
	{
		Color * t = luax_checkcolor(L, 1);
		lua_pushinteger(L, t->getAlpha());		
		return 1;
	}

	static const luaL_Reg wrap_Color_functions[] = {
		{ "setRed", _wrap_Color_setRed },
		{ "setGreen", _wrap_Color_setGreen },
		{ "setBlue", _wrap_Color_setBlue },
		{ "setAlpha", _wrap_Color_setAlpha },
		{ "getRed", _wrap_Color_getRed },
		{ "getGreen", _wrap_Color_getGreen },
		{ "getBlue", _wrap_Color_getBlue },
		{ "getAlpha", _wrap_Color_getAlpha },
		{ 0, 0 }
	};

	int wrap_Color_open(lua_State * L)
	{
		luax_register_type(L, "Color", wrap_Color_functions);
		return 0;
	}

} // opengl
} // graphics
} // love

