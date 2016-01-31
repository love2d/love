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
#include "common/config.h"
#include "wrap_Font.h"
#include "wrap_Text.h"

// C++
#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

Font *luax_checkfont(lua_State *L, int idx)
{
	return luax_checktype<Font>(L, idx, GRAPHICS_FONT_ID);
}

int w_Font_getHeight(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	lua_pushnumber(L, t->getHeight());
	return 1;
}

int w_Font_getWidth(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	const char *str = luaL_checkstring(L, 2);

	luax_catchexcept(L, [&](){ lua_pushinteger(L, t->getWidth(str)); });
	return 1;
}

int w_Font_getWrap(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);

	std::vector<Font::ColoredString> text;
	luax_checkcoloredstring(L, 2, text);

	float wrap = (float) luaL_checknumber(L, 3);
	int max_width = 0;
	std::vector<std::string> lines;
	std::vector<int> widths;

	luax_catchexcept(L, [&]() { t->getWrap(text, wrap, lines, &widths); });

	for (int width : widths)
		max_width = std::max(max_width, width);

	lua_pushinteger(L, max_width);
	lua_createtable(L, (int) lines.size(), 0);

	for (int i = 0; i < (int) lines.size(); i++)
	{
		lua_pushstring(L, lines[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 2;
}

int w_Font_setLineHeight(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	float h = (float)luaL_checknumber(L, 2);
	t->setLineHeight(h);
	return 0;
}

int w_Font_getLineHeight(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	lua_pushnumber(L, t->getLineHeight());
	return 1;
}

int w_Font_setFilter(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	Texture::Filter f = t->getFilter();

	const char *minstr = luaL_checkstring(L, 2);
	const char *magstr = luaL_optstring(L, 3, minstr);

	if (!Texture::getConstant(minstr, f.min))
		return luaL_error(L, "Invalid filter mode: %s", minstr);
	if (!Texture::getConstant(magstr, f.mag))
		return luaL_error(L, "Invalid filter mode: %s", magstr);

	f.anisotropy = (float) luaL_optnumber(L, 4, 1.0);

	luax_catchexcept(L, [&](){ t->setFilter(f); });
	return 0;
}

int w_Font_getFilter(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	const Texture::Filter f = t->getFilter();
	const char *minstr;
	const char *magstr;
	Texture::getConstant(f.min, minstr);
	Texture::getConstant(f.mag, magstr);
	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, f.anisotropy);
	return 3;
}

int w_Font_getAscent(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	lua_pushnumber(L, t->getAscent());
	return 1;
}

int w_Font_getDescent(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	lua_pushnumber(L, t->getDescent());
	return 1;
}

int w_Font_getBaseline(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	lua_pushnumber(L, t->getBaseline());
	return 1;
}

int w_Font_hasGlyphs(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	bool hasglyph = false;

	int count = std::max(lua_gettop(L) - 1, 1);

	luax_catchexcept(L, [&]() {
		 for (int i = 2; i < count + 2; i++)
		 {
			 if (lua_type(L, i) == LUA_TSTRING)
				 hasglyph = t->hasGlyphs(luax_checkstring(L, i));
			 else
				 hasglyph = t->hasGlyph((uint32) luaL_checknumber(L, i));

			 if (!hasglyph)
				 break;
		 }
	});

	luax_pushboolean(L, hasglyph);
	return 1;
}

int w_Font_setFallbacks(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	std::vector<Font *> fallbacks;

	for (int i = 2; i <= lua_gettop(L); i++)
		fallbacks.push_back(luax_checkfont(L, i));

	luax_catchexcept(L, [&](){ t->setFallbacks(fallbacks); });
	return 0;
}

static const luaL_Reg w_Font_functions[] =
{
	{ "getHeight", w_Font_getHeight },
	{ "getWidth", w_Font_getWidth },
	{ "getWrap", w_Font_getWrap },
	{ "setLineHeight", w_Font_setLineHeight },
	{ "getLineHeight", w_Font_getLineHeight },
	{ "setFilter", w_Font_setFilter },
	{ "getFilter", w_Font_getFilter },
	{ "getAscent", w_Font_getAscent },
	{ "getDescent", w_Font_getDescent },
	{ "getBaseline", w_Font_getBaseline },
	{ "hasGlyphs", w_Font_hasGlyphs },
	{ "setFallbacks", w_Font_setFallbacks },
	{ 0, 0 }
};

extern "C" int luaopen_font(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_FONT_ID, "Font", w_Font_functions, nullptr);
}

} // opengl
} // graphics
} // love
