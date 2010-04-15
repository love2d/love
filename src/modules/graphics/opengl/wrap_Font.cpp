/**
* Copyright (c) 2006-2010 LOVE Development Team
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
#include "wrap_Font.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Font * luax_checkfont(lua_State * L, int idx)
	{
		return luax_checktype<Font>(L, idx, "Font", GRAPHICS_FONT_T);
	}

	int w_Font_getHeight(lua_State * L)
	{
		Font * t = luax_checkfont(L, 1);
		lua_pushnumber(L, t->getHeight());
		return 1;
	}

	int w_Font_getWidth(lua_State * L)
	{
		Font * t = luax_checkfont(L, 1);
		const char * str = luaL_checkstring(L, 2);
		lua_pushinteger(L, t->getWidth(str));
		return 1;
	}

	int w_Font_setLineHeight(lua_State * L)
	{
		Font * t = luax_checkfont(L, 1);
		float h = (float)luaL_checknumber(L, 2);
		t->setLineHeight(h);
		return 0;
	}

	int w_Font_getLineHeight(lua_State * L)
	{
		Font * t = luax_checkfont(L, 1);
		lua_pushnumber(L, t->getLineHeight());
		return 1;
	}

	static const luaL_Reg functions[] = {
		{ "getHeight", w_Font_getHeight },
		{ "getWidth", w_Font_getWidth },
		{ "setLineHeight", w_Font_setLineHeight },
		{ "getLineHeight", w_Font_getLineHeight },
		{ 0, 0 }
	};

	int luaopen_font(lua_State * L)
	{
		return luax_register_type(L, "Font", functions);
	}

} // opengl
} // graphics
} // love
