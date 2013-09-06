/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "wrap_Image.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Image *luax_checkimage(lua_State *L, int idx)
{
	return luax_checktype<Image>(L, idx, "Image", GRAPHICS_IMAGE_T);
}

int w_Image_getWidth(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	lua_pushnumber(L, t->getWidth());
	return 1;
}

int w_Image_getHeight(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	lua_pushnumber(L, t->getHeight());
	return 1;
}

int w_Image_getDimensions(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	lua_pushnumber(L, t->getWidth());
	lua_pushnumber(L, t->getHeight());
	return 2;
}

int w_Image_setFilter(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	Image::Filter f = t->getFilter();

	const char *minstr = luaL_checkstring(L, 2);
	const char *magstr = luaL_optstring(L, 3, minstr);

	if (!Image::getConstant(minstr, f.min))
		return luaL_error(L, "Invalid filter mode: %s", minstr);
	if (!Image::getConstant(magstr, f.mag))
		return luaL_error(L, "Invalid filter mode: %s", magstr);

	f.anisotropy = (float) luaL_optnumber(L, 4, 1.0);

	EXCEPT_GUARD(t->setFilter(f);)
	return 0;
}

int w_Image_getFilter(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	const Image::Filter f = t->getFilter();
	const char *minstr;
	const char *magstr;
	Image::getConstant(f.min, minstr);
	Image::getConstant(f.mag, magstr);
	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, f.anisotropy);
	return 3;
}

int w_Image_setMipmapFilter(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	Image::Filter f = t->getFilter();

	if (lua_isnoneornil(L, 2))
		f.mipmap = Image::FILTER_NONE; // mipmapping is disabled if no argument is given
	else
	{
		const char *mipmapstr = luaL_checkstring(L, 2);
		if (!Image::getConstant(mipmapstr, f.mipmap))
			return luaL_error(L, "Invalid filter mode: %s", mipmapstr);
	}

	EXCEPT_GUARD(t->setFilter(f);)

	float sharpness = luaL_optnumber(L, 3, 0);
	t->setMipmapSharpness(sharpness);

	return 0;
}

int w_Image_getMipmapFilter(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);

	const Image::Filter &f = t->getFilter();

	const char *mipmapstr;
	if (Image::getConstant(f.mipmap, mipmapstr))
		lua_pushstring(L, mipmapstr);
	else
		lua_pushnil(L); // only return a mipmap filter if mipmapping is enabled

	lua_pushnumber(L, t->getMipmapSharpness());
	return 2;
}

int w_Image_setWrap(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);

	Image::Wrap w;

	const char *sstr = luaL_checkstring(L, 2);
	const char *tstr = luaL_optstring(L, 3, sstr);

	if (!Image::getConstant(sstr, w.s))
		return luaL_error(L, "Invalid wrap mode: %s", sstr);
	if (!Image::getConstant(tstr, w.t))
		return luaL_error(L, "Invalid wrap mode, %s", tstr);

	i->setWrap(w);

	return 0;
}

int w_Image_getWrap(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	const Image::Wrap w = i->getWrap();
	const char *sstr;
	const char *tstr;
	Image::getConstant(w.s, sstr);
	Image::getConstant(w.t, tstr);
	lua_pushstring(L, sstr);
	lua_pushstring(L, tstr);
	return 2;
}

int w_Image_isCompressed(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	luax_pushboolean(L, i->isCompressed());
	return 1;
}

int w_Image_refresh(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	EXCEPT_GUARD(i->refresh();)
	return 0;
}

int w_Image_getData(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);

	if (i->isCompressed())
	{
		love::image::CompressedData *t = i->getCompressedData();
		if (t)
		{
			t->retain();
			luax_pushtype(L, "CompressedData", IMAGE_COMPRESSED_DATA_T, t);
		}
		else
			lua_pushnil(L);
	}
	else
	{
		love::image::ImageData *t = i->getImageData();
		if (t)
		{
			t->retain();
			luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, t);
		}
		else
			lua_pushnil(L);
	}

	return 1;
}

static const luaL_Reg functions[] =
{
	{ "getWidth", w_Image_getWidth },
	{ "getHeight", w_Image_getHeight },
	{ "getDimensions", w_Image_getDimensions },
	{ "setFilter", w_Image_setFilter },
	{ "getFilter", w_Image_getFilter },
	{ "setWrap", w_Image_setWrap },
	{ "getWrap", w_Image_getWrap },
	{ "setMipmapFilter", w_Image_setMipmapFilter },
	{ "getMipmapFilter", w_Image_getMipmapFilter },
	{ "isCompressed", w_Image_isCompressed },
	{ "refresh", w_Image_refresh },
	{ "getData", w_Image_getData },
	{ 0, 0 }
};

extern "C" int luaopen_image(lua_State *L)
{
	return luax_register_type(L, "Image", functions);
}

} // opengl
} // graphics
} // love
