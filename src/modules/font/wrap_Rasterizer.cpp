/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "wrap_Rasterizer.h"

#include "data/wrap_Data.h"

namespace love
{
namespace font
{

Rasterizer *luax_checkrasterizer(lua_State *L, int idx)
{
	return luax_checktype<Rasterizer>(L, idx);
}

int w_Rasterizer_getHeight(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	lua_pushinteger(L, t->getHeight());
	return 1;
}

int w_Rasterizer_getAdvance(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	lua_pushinteger(L, t->getAdvance());
	return 1;
}

int w_Rasterizer_getAscent(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	lua_pushinteger(L, t->getAscent());
	return 1;
}

int w_Rasterizer_getDescent(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	lua_pushinteger(L, t->getDescent());
	return 1;
}

int w_Rasterizer_getLineHeight(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	lua_pushinteger(L, t->getLineHeight());
	return 1;
}

int w_Rasterizer_getGlyphData(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	GlyphData *g = 0;

	luax_catchexcept(L, [&]() {
		// getGlyphData accepts a unicode character or a codepoint number.
		if (lua_type(L, 2) == LUA_TSTRING)
		{
			std::string glyph = luax_checkstring(L, 2);
			g = t->getGlyphData(glyph);
		}
		else
		{
			uint32 glyph = (uint32) luaL_checknumber(L, 2);
			g = t->getGlyphData(glyph);
		}
	});

	luax_pushtype(L, g);
	g->release();
	return 1;
}

int w_Rasterizer_getGlyphCount(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);
	lua_pushinteger(L, t->getGlyphCount());
	return 1;
}

int w_Rasterizer_hasGlyphs(lua_State *L)
{
	Rasterizer *t = luax_checkrasterizer(L, 1);

	bool hasglyph = false;

	int count = lua_gettop(L) - 1;
	count = count < 1 ? 1 : count;

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

const luaL_Reg w_Rasterizer_functions[] =
{
	{ "getHeight", w_Rasterizer_getHeight },
	{ "getAdvance", w_Rasterizer_getAdvance },
	{ "getAscent", w_Rasterizer_getAscent },
	{ "getDescent", w_Rasterizer_getDescent },
	{ "getLineHeight", w_Rasterizer_getLineHeight },
	{ "getGlyphData", w_Rasterizer_getGlyphData },
	{ "getGlyphCount", w_Rasterizer_getGlyphCount },
	{ "hasGlyphs", w_Rasterizer_hasGlyphs },
	{ 0, 0 }
};

extern "C" int luaopen_rasterizer(lua_State *L)
{
	return luax_register_type(L, &Rasterizer::type, w_Rasterizer_functions, nullptr);
}

} // font
} // love
