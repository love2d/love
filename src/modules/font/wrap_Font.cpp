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

#include "wrap_Font.h"

#include "Font.h"
#include "freetype/Font.h"

#include "wrap_GlyphData.h"
#include "wrap_Rasterizer.h"

#include "filesystem/wrap_Filesystem.h"

namespace love
{
namespace font
{

#define instance() (Module::getInstance<Font>(Module::M_FONT))

int w_newRasterizer(lua_State *L)
{
	if (lua_type(L, 1) == LUA_TNUMBER || lua_type(L, 2) == LUA_TNUMBER || lua_isnone(L, 1))
	{
		// First or second argument is a number: call newTrueTypeRasterizer.
		return w_newTrueTypeRasterizer(L);
	}
	else if (lua_isnoneornil(L, 2))
	{
		// Single argument of another type: call Font::newRasterizer.
		Rasterizer *t = nullptr;
		filesystem::FileData *d = filesystem::luax_getfiledata(L, 1);

		luax_catchexcept(L,
			[&]() { t = instance()->newRasterizer(d); },
			[&](bool) { d->release(); }
		);

		luax_pushtype(L, t);
		t->release();
		return 1;
	}
	else
	{
		// Otherwise call newBMFontRasterizer.
		return w_newBMFontRasterizer(L);
	}
}

int w_newTrueTypeRasterizer(lua_State *L)
{
	Rasterizer *t = nullptr;
	TrueTypeRasterizer::Hinting hinting = TrueTypeRasterizer::HINTING_NORMAL;

	if (lua_type(L, 1) == LUA_TNUMBER || lua_isnone(L, 1))
	{
		// First argument is a number: use the default TrueType font.
		int size = (int) luaL_optinteger(L, 1, 12);

		const char *hintstr = lua_isnoneornil(L, 2) ? nullptr : luaL_checkstring(L, 2);
		if (hintstr && !TrueTypeRasterizer::getConstant(hintstr, hinting))
			return luax_enumerror(L, "TrueType font hinting mode", TrueTypeRasterizer::getConstants(hinting), hintstr);

		if (lua_isnoneornil(L, 3))
			luax_catchexcept(L, [&](){ t = instance()->newTrueTypeRasterizer(size, hinting); });
		else
		{
			float dpiscale = (float) luaL_checknumber(L, 3);
			luax_catchexcept(L, [&](){ t = instance()->newTrueTypeRasterizer(size, dpiscale, hinting); });
		}
	}
	else
	{
		love::Data *d = nullptr;

		if (luax_istype(L, 1, love::Data::type))
		{
			d = data::luax_checkdata(L, 1);
			d->retain();
		}
		else
			d = filesystem::luax_getfiledata(L, 1);

		int size = (int) luaL_optinteger(L, 2, 12);

		const char *hintstr = lua_isnoneornil(L, 3) ? nullptr : luaL_checkstring(L, 3);
		if (hintstr && !TrueTypeRasterizer::getConstant(hintstr, hinting))
			return luax_enumerror(L, "TrueType font hinting mode", TrueTypeRasterizer::getConstants(hinting), hintstr);

		if (lua_isnoneornil(L, 4))
		{
			luax_catchexcept(L,
				[&]() { t = instance()->newTrueTypeRasterizer(d, size, hinting); },
				[&](bool) { d->release(); }
			);
		}
		else
		{
			float dpiscale = (float) luaL_checknumber(L, 4);
			luax_catchexcept(L,
				[&]() { t = instance()->newTrueTypeRasterizer(d, size, dpiscale, hinting); },
				[&](bool) { d->release(); }
			);
		}
	}

	luax_pushtype(L, t);
	t->release();
	return 1;
}

static void convimagedata(lua_State *L, int idx)
{
	if (lua_type(L, 1) == LUA_TSTRING || luax_istype(L, idx, love::filesystem::File::type) || luax_istype(L, idx, love::filesystem::FileData::type))
		luax_convobj(L, idx, "image", "newImageData");
}

int w_newBMFontRasterizer(lua_State *L)
{
	Rasterizer *t = nullptr;

	filesystem::FileData *d = filesystem::luax_getfiledata(L, 1);
	std::vector<image::ImageData *> images;
	float dpiscale = (float) luaL_optnumber(L, 3, 1.0);

	if (lua_istable(L, 2))
	{
		for (int i = 1; i <= (int) luax_objlen(L, 2); i++)
		{
			lua_rawgeti(L, 2, i);

			convimagedata(L, -1);
			image::ImageData *id = luax_checktype<image::ImageData>(L, -1);
			images.push_back(id);
			id->retain();

			lua_pop(L, 1);
		}
	}
	else
	{
		convimagedata(L, 2);
		image::ImageData *id = luax_checktype<image::ImageData>(L, 2);
		images.push_back(id);
		id->retain();
	}

	luax_catchexcept(L,
		[&]() { t = instance()->newBMFontRasterizer(d, images, dpiscale); },
		[&](bool) { d->release(); for (auto id : images) id->release(); }
	);

	luax_pushtype(L, t);
	t->release();
	return 1;
}

int w_newImageRasterizer(lua_State *L)
{
	Rasterizer *t = nullptr;

	convimagedata(L, 1);

	image::ImageData *d = luax_checktype<image::ImageData>(L, 1);
	std::string glyphs = luax_checkstring(L, 2);
	int extraspacing = (int) luaL_optinteger(L, 3, 0);
	float dpiscale = (float) luaL_optnumber(L, 4, 1.0);

	luax_catchexcept(L, [&](){ t = instance()->newImageRasterizer(d, glyphs, extraspacing, dpiscale); });

	luax_pushtype(L, t);
	t->release();
	return 1;
}

int w_newGlyphData(lua_State *L)
{
	Rasterizer *r = luax_checkrasterizer(L, 1);
	GlyphData *t = nullptr;

	// newGlyphData accepts a unicode character or a codepoint number.
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		std::string glyph = luax_checkstring(L, 2);
		luax_catchexcept(L, [&](){ t = instance()->newGlyphData(r, glyph); });
	}
	else
	{
		uint32 g = (uint32) luaL_checknumber(L, 2);
		t = instance()->newGlyphData(r, g);
	}

	luax_pushtype(L, t);
	t->release();
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "newRasterizer",  w_newRasterizer },
	{ "newTrueTypeRasterizer", w_newTrueTypeRasterizer },
	{ "newBMFontRasterizer", w_newBMFontRasterizer },
	{ "newImageRasterizer", w_newImageRasterizer },
	{ "newGlyphData",  w_newGlyphData },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_glyphdata,
	luaopen_rasterizer,
	0
};

extern "C" int luaopen_love_font(lua_State *L)
{
	Font *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new freetype::Font(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "font";
	w.type = &Module::type;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // font
} // love
