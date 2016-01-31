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

#include "wrap_GlyphData.h"

namespace love
{
namespace font
{

GlyphData *luax_checkglyphdata(lua_State *L, int idx)
{
	return luax_checktype<GlyphData>(L, idx, FONT_GLYPH_DATA_ID);
}

int w_GlyphData_getWidth(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);
	lua_pushinteger(L, t->getWidth());
	return 1;
}

int w_GlyphData_getHeight(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);
	lua_pushinteger(L, t->getHeight());
	return 1;
}

int w_GlyphData_getDimensions(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);
	lua_pushinteger(L, t->getWidth());
	lua_pushinteger(L, t->getHeight());
	return 2;
}

int w_GlyphData_getGlyph(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);
	uint32 glyph = t->getGlyph();
	lua_pushnumber(L, (lua_Number) glyph);
	return 1;
}

int w_GlyphData_getGlyphString(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);

	luax_catchexcept(L, [&](){ luax_pushstring(L, t->getGlyphString()); });
	return 1;
}

int w_GlyphData_getAdvance(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);
	lua_pushinteger(L, t->getAdvance());
	return 1;
}

int w_GlyphData_getBearing(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);
	lua_pushinteger(L, t->getBearingX());
	lua_pushinteger(L, t->getBearingY());
	return 2;
}

int w_GlyphData_getBoundingBox(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);

	int minX = t->getMinX();
	int minY = t->getMinY();
	int maxX = t->getMaxX();
	int maxY = t->getMaxY();

	int width = maxX - minX;
	int height = maxY - minY;

	lua_pushinteger(L, minX);
	lua_pushinteger(L, minY);
	lua_pushinteger(L, width);
	lua_pushinteger(L, height);

	return 4;
}

int w_GlyphData_getFormat(lua_State *L)
{
	GlyphData *t = luax_checkglyphdata(L, 1);

	const char *str;
	if (!GlyphData::getConstant(t->getFormat(), str))
		return luaL_error(L, "unknown GlyphData format.");

	lua_pushstring(L, str);
	return 1;
}

const luaL_Reg w_GlyphData_functions[] =
{
	{ "getWidth", w_GlyphData_getWidth },
	{ "getHeight", w_GlyphData_getHeight },
	{ "getDimensions", w_GlyphData_getDimensions },
	{ "getGlyph", w_GlyphData_getGlyph },
	{ "getGlyphString", w_GlyphData_getGlyphString },
	{ "getAdvance", w_GlyphData_getAdvance },
	{ "getBearing", w_GlyphData_getBearing },
	{ "getBoundingBox", w_GlyphData_getBoundingBox },
	{ "getFormat", w_GlyphData_getFormat },
	{ 0, 0 }
};

extern "C" int luaopen_glyphdata(lua_State *L)
{
	return luax_register_type(L, FONT_GLYPH_DATA_ID, "GlyphData", w_Data_functions, w_GlyphData_functions, nullptr);
}

} // font
} // love
