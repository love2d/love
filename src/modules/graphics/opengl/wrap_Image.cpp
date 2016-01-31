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

	int xoffset = (int) luaL_optnumber(L, 2, 0);
	int yoffset = (int) luaL_optnumber(L, 3, 0);
	int w = (int) luaL_optnumber(L, 4, i->getWidth());
	int h = (int) luaL_optnumber(L, 5, i->getHeight());

	luax_catchexcept(L, [&](){ i->refresh(xoffset, yoffset, w, h); });
	return 0;
}

int w_Image_getData(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	int n = 0;

	if (i->isCompressed())
	{
		for (const auto &cdata : i->getCompressedData())
		{
			luax_pushtype(L, IMAGE_COMPRESSED_IMAGE_DATA_ID, cdata.get());
			n++;
		}
	}
	else
	{
		for (const auto &data : i->getImageData())
		{
			luax_pushtype(L, IMAGE_IMAGE_DATA_ID, data.get());
			n++;
		}
	}

	return n;
}

static const char *imageFlagName(Image::FlagType flagtype)
{
	const char *name = nullptr;
	Image::getConstant(flagtype, name);
	return name;
}

int w_Image_getFlags(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	Image::Flags flags = i->getFlags();

	lua_createtable(L, 0, 2);

	lua_pushboolean(L, flags.mipmaps);
	lua_setfield(L, -2, imageFlagName(Image::FLAG_TYPE_MIPMAPS));

	lua_pushboolean(L, flags.linear);
	lua_setfield(L, -2, imageFlagName(Image::FLAG_TYPE_LINEAR));

	return 1;
}

static const luaL_Reg w_Image_functions[] =
{
	{ "setMipmapFilter", w_Image_setMipmapFilter },
	{ "getMipmapFilter", w_Image_getMipmapFilter },
	{ "isCompressed", w_Image_isCompressed },
	{ "refresh", w_Image_refresh },
	{ "getData", w_Image_getData },
	{ "getFlags", w_Image_getFlags },
	{ 0, 0 }
};

extern "C" int luaopen_image(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_IMAGE_ID, "Image", w_Texture_functions, w_Image_functions, nullptr);
}

} // opengl
} // graphics
} // love
