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
	return luax_checktype<Image>(L, idx, GRAPHICS_IMAGE_ID);
}

int w_Image_setMipmapFilter(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);
	Texture::Filter f = t->getFilter();

	if (lua_isnoneornil(L, 2))
		f.mipmap = Texture::FILTER_NONE; // mipmapping is disabled if no argument is given
	else
	{
		const char *mipmapstr = luaL_checkstring(L, 2);
		if (!Texture::getConstant(mipmapstr, f.mipmap))
			return luaL_error(L, "Invalid filter mode: %s", mipmapstr);
	}

	luax_catchexcept(L, [&](){ t->setFilter(f); });
	t->setMipmapSharpness((float) luaL_optnumber(L, 3, 0.0));

	return 0;
}

int w_Image_getMipmapFilter(lua_State *L)
{
	Image *t = luax_checkimage(L, 1);

	const Texture::Filter &f = t->getFilter();

	const char *mipmapstr;
	if (Texture::getConstant(f.mipmap, mipmapstr))
		lua_pushstring(L, mipmapstr);
	else
		lua_pushnil(L); // only return a mipmap filter if mipmapping is enabled

	lua_pushnumber(L, t->getMipmapSharpness());
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

	int xoffset = luaL_optint(L, 2, 0);
	int yoffset = luaL_optint(L, 3, 0);
	int w = luaL_optint(L, 4, i->getWidth());
	int h = luaL_optint(L, 5, i->getHeight());

	luax_catchexcept(L, [&](){ i->refresh(xoffset, yoffset, w, h); });
	return 0;
}

int w_Image_getData(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);

	if (i->isCompressed())
	{
		love::image::CompressedData *t = i->getCompressedData();
		luax_pushtype(L, IMAGE_COMPRESSED_DATA_ID, t);
	}
	else
	{
		love::image::ImageData *t = i->getImageData();
		luax_pushtype(L, IMAGE_IMAGE_DATA_ID, t);
	}

	return 1;
}

static const luaL_Reg functions[] =
{
	// From wrap_Texture.
	{ "getWidth", w_Texture_getWidth },
	{ "getHeight", w_Texture_getHeight },
	{ "getDimensions", w_Texture_getDimensions },
	{ "setFilter", w_Texture_setFilter },
	{ "getFilter", w_Texture_getFilter },
	{ "setWrap", w_Texture_setWrap },
	{ "getWrap", w_Texture_getWrap },

	{ "setMipmapFilter", w_Image_setMipmapFilter },
	{ "getMipmapFilter", w_Image_getMipmapFilter },
	{ "isCompressed", w_Image_isCompressed },
	{ "refresh", w_Image_refresh },
	{ "getData", w_Image_getData },
	{ 0, 0 }
};

extern "C" int luaopen_image(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_IMAGE_ID, functions);
}

} // opengl
} // graphics
} // love
