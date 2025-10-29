/**
 * Copyright (c) 2006-2025 LOVE Development Team
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

// C++
#include <algorithm>

namespace love
{
namespace graphics
{

void luax_checkcoloredstring(lua_State *L, int idx, std::vector<love::font::ColoredString> &strings)
{
	love::font::ColoredString coloredstr;
	coloredstr.color = Colorf(1.0f, 1.0f, 1.0f, 1.0f);

	if (lua_istable(L, idx))
	{
		int len = (int) luax_objlen(L, idx);

		for (int i = 1; i <= len; i++)
		{
			lua_rawgeti(L, idx, i);

			if (lua_istable(L, -1))
			{
				for (int j = 1; j <= 4; j++)
					lua_rawgeti(L, -j, j);

				coloredstr.color.r = (float) luaL_checknumber(L, -4);
				coloredstr.color.g = (float) luaL_checknumber(L, -3);
				coloredstr.color.b = (float) luaL_checknumber(L, -2);
				coloredstr.color.a = (float) luaL_optnumber(L, -1, 1.0);

				lua_pop(L, 4);
			}
			else
			{
				size_t strl = 0;
				const char *str = luaL_checklstring(L, -1, &strl);
				coloredstr.str.assign(str, strl);
				strings.push_back(coloredstr);
			}

			lua_pop(L, 1);
		}
	}
	else
	{
		size_t strl = 0;
		const char *str = luaL_checklstring(L, idx, &strl);
		coloredstr.str.assign(str, strl);
		strings.push_back(coloredstr);
	}
}

Font *luax_checkfont(lua_State *L, int idx)
{
	return luax_checktype<Font>(L, idx);
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
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		const char *str = luaL_checkstring(L, 2);
		luax_catchexcept(L, [&](){ lua_pushinteger(L, t->getWidth(str)); });
	}
	else
	{
		uint32 glyph = (uint32) luaL_checknumber(L, 2);
		luax_catchexcept(L, [&](){ lua_pushinteger(L, t->getWidth(glyph)); });
	}
	return 1;
}

int w_Font_getGlyphPosition(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	int index = luaL_checkinteger(L, 2);
	const char *str = luaL_checkstring(L, 3);
	float wraplimit = (float) luaL_checknumber(L, 4);

	Font::AlignMode align = Font::ALIGN_LEFT;
	const char *astr = lua_isnoneornil(L, 5) ? nullptr : luaL_checkstring(L, 5);
	if (astr != nullptr && !Font::getConstant(astr, align))
		return luax_enumerror(L, "alignment", Font::getConstants(align), astr);

	Vector2 pos;
	float width;
	float height = t->getHeight();
	luax_catchexcept(L, [&]() { pos = t->getGlyphPosition(index - 1, str, wraplimit, align, &width); });

	lua_pushnumber(L, pos.x);
	lua_pushnumber(L, pos.y);
	lua_pushnumber(L, width);
	lua_pushnumber(L, height);
	return 4;
}

int w_Font_getWrap(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);

	std::vector<love::font::ColoredString> text;
	luax_checkcoloredstring(L, 2, text);

	float wrap = (float) luaL_checknumber(L, 3);
	float max_width = 0;
	std::vector<std::string> lines;
	std::vector<float> widths;

	luax_catchexcept(L, [&]() { t->getWrap(text, wrap, lines, &widths); });

	for (float width : widths)
		max_width = std::max(max_width, width);

	lua_pushnumber(L, max_width);
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
	SamplerState s = t->getSamplerState();

	const char *minstr = luaL_checkstring(L, 2);
	const char *magstr = luaL_optstring(L, 3, minstr);

	if (!SamplerState::getConstant(minstr, s.minFilter))
		return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.minFilter), minstr);
	if (!SamplerState::getConstant(magstr, s.magFilter))
		return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.magFilter), magstr);

	s.maxAnisotropy = std::min(std::max(1, (int) luaL_optnumber(L, 4, 1.0)), LOVE_UINT8_MAX);

	luax_catchexcept(L, [&](){ t->setSamplerState(s); });
	return 0;
}

int w_Font_getFilter(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	const SamplerState &s = t->getSamplerState();
	const char *minstr;
	const char *magstr;
	SamplerState::getConstant(s.minFilter, minstr);
	SamplerState::getConstant(s.magFilter, magstr);
	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, s.maxAnisotropy);
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

int w_Font_getKerning(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	float kerning = 0.0f;

	luax_catchexcept(L, [&]() {
		if (lua_type(L, 2) == LUA_TSTRING)
		{
			std::string left = luax_checkstring(L, 2);
			std::string right = luax_checkstring(L, 3);
			kerning = t->getKerning(left, right);
		}
		else
		{
			uint32 left = (uint32) luaL_checknumber(L, 2);
			uint32 right = (uint32) luaL_checknumber(L, 3);
			kerning = t->getKerning(left, right);
		}
	});

	lua_pushnumber(L, kerning);
	return 1;
}

int w_Font_setFallbacks(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	std::vector<graphics::Font *> fallbacks;

	for (int i = 2; i <= lua_gettop(L); i++)
		fallbacks.push_back(luax_checkfont(L, i));

	luax_catchexcept(L, [&](){ t->setFallbacks(fallbacks); });
	return 0;
}

int w_Font_getDPIScale(lua_State *L)
{
	Font *t = luax_checkfont(L, 1);
	lua_pushnumber(L, t->getDPIScale());
	return 1;
}

static const luaL_Reg w_Font_functions[] =
{
	{ "getHeight", w_Font_getHeight },
	{ "getWidth", w_Font_getWidth },
	{ "getGlyphPosition", w_Font_getGlyphPosition },
	{ "getWrap", w_Font_getWrap },
	{ "setLineHeight", w_Font_setLineHeight },
	{ "getLineHeight", w_Font_getLineHeight },
	{ "setFilter", w_Font_setFilter },
	{ "getFilter", w_Font_getFilter },
	{ "getAscent", w_Font_getAscent },
	{ "getDescent", w_Font_getDescent },
	{ "getBaseline", w_Font_getBaseline },
	{ "hasGlyphs", w_Font_hasGlyphs },
	{ "getKerning", w_Font_getKerning },
	{ "setFallbacks", w_Font_setFallbacks },
	{ "getDPIScale", w_Font_getDPIScale },
	{ 0, 0 }
};

extern "C" int luaopen_font(lua_State *L)
{
	return luax_register_type(L, &Font::type, w_Font_functions, nullptr);
}

} // graphics
} // love
