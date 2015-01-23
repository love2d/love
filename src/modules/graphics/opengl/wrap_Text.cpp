/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "wrap_Text.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Text *luax_checktext(lua_State *L, int idx)
{
	return luax_checktype<Text>(L, idx, "Text", GRAPHICS_TEXT_T);
}

int w_Text_set(lua_State *L)
{
	Text *t = luax_checktext(L, 1);

	if (lua_isnoneornil(L, 2))
	{
		// No argument: clear all current text.
		luax_catchexcept(L, [&](){ t->set(); });
	}
	else if (lua_isnoneornil(L, 3))
	{
		// Single argument: unformatted text.
		std::string newtext = luax_checkstring(L, 2);
		luax_catchexcept(L, [&](){ t->set(newtext); });
	}
	else
	{
		// Multiple arguments: formatted text.
		float wraplimit = (float) luaL_checknumber(L, 3);

		Font::AlignMode align;
		const char *alignstr = luaL_checkstring(L, 4);
		if (!Font::getConstant(alignstr, align))
			return luaL_error(L, "Invalid align mode: %s", alignstr);

		std::string newtext = luax_checkstring(L, 2);

		luax_catchexcept(L, [&](){ t->set(newtext, wraplimit, align); });
	}

	return 0;
}

int w_Text_setf(lua_State *L)
{
	Text *t = luax_checktext(L, 1);

	float wraplimit = (float) luaL_checknumber(L, 3);

	Font::AlignMode align;
	const char *alignstr = luaL_checkstring(L, 4);
	if (!Font::getConstant(alignstr, align))
		return luaL_error(L, "Invalid align mode: %s", alignstr);

	std::string newtext = luax_checkstring(L, 2);

	luax_catchexcept(L, [&](){ t->set(newtext, wraplimit, align); });

	return 0;
}

int w_Text_add(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	std::string text = luax_checkstring(L, 2);

	float x  = (float) luaL_optnumber(L, 3, 0.0);
	float y  = (float) luaL_optnumber(L, 4, 0.0);
	float a  = (float) luaL_optnumber(L, 5, 0.0);
	float sx = (float) luaL_optnumber(L, 6, 1.0);
	float sy = (float) luaL_optnumber(L, 7, sx);
	float ox = (float) luaL_optnumber(L, 8, 0.0);
	float oy = (float) luaL_optnumber(L, 9, 0.0);
	float kx = (float) luaL_optnumber(L, 10, 0.0);
	float ky = (float) luaL_optnumber(L, 11, 0.0);

	luax_catchexcept(L, [&](){ t->add(text, x, y, a, sx, sy, ox, oy, kx, ky); });
	return 0;
}

int w_Text_addf(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	std::string text = luax_checkstring(L, 2);
	float wrap = (float) luaL_checknumber(L, 3);

	Font::AlignMode align = Font::ALIGN_MAX_ENUM;
	const char *alignstr = luaL_checkstring(L, 4);

	if (!Font::getConstant(alignstr, align))
		return luaL_error(L, "Invalid align mode: %s", alignstr);

	float x  = (float) luaL_optnumber(L, 5, 0.0);
	float y  = (float) luaL_optnumber(L, 6, 0.0);
	float a  = (float) luaL_optnumber(L, 7, 0.0);
	float sx = (float) luaL_optnumber(L, 8, 1.0);
	float sy = (float) luaL_optnumber(L, 9, sx);
	float ox = (float) luaL_optnumber(L, 10, 0.0);
	float oy = (float) luaL_optnumber(L, 11, 0.0);
	float kx = (float) luaL_optnumber(L, 12, 0.0);
	float ky = (float) luaL_optnumber(L, 13, 0.0);

	luax_catchexcept(L, [&](){ t->addf(text, wrap, align, x, y, a, sx, sy, ox, oy, kx, ky); });
	return 0;
}

int w_Text_clear(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	luax_catchexcept(L, [&](){ t->clear(); });
	return 0;
}

int w_Text_getFont(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	Font *f = t->getFont();
	luax_pushtype(L, "Font", GRAPHICS_FONT_T, f);
	return 1;
}

int w_Text_getWidth(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	lua_pushnumber(L, t->getWidth());
	return 1;
}

int w_Text_getHeight(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	lua_pushnumber(L, t->getHeight());
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "set", w_Text_set },
	{ "setf", w_Text_setf },
	{ "add", w_Text_add },
	{ "addf", w_Text_addf },
	{ "clear", w_Text_clear },
	{ "getFont", w_Text_getFont },
	{ "getWidth", w_Text_getWidth },
	{ "getHeight", w_Text_getHeight },
	{ 0, 0 }
};

extern "C" int luaopen_text(lua_State *L)
{
	return luax_register_type(L, "Text", functions);
}

} // opengl
} // graphics
} // love
