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

// LOVE
#include "wrap_Image.h"

namespace love
{
namespace graphics
{

Image *luax_checkimage(lua_State *L, int idx)
{
	return luax_checktype<Image>(L, idx);
}

int w_Image_isFormatLinear(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	luax_pushboolean(L, i->isFormatLinear());
	return 1;
}

int w_Image_isCompressed(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	luax_pushboolean(L, i->isCompressed());
	return 1;
}

int w_Image_replacePixels(lua_State *L)
{
	Image *i = luax_checkimage(L, 1);
	love::image::ImageData *id = luax_checktype<love::image::ImageData>(L, 2);

	int slice = 0;
	int mipmap = 0;
	int x = 0;
	int y = 0;
	bool reloadmipmaps = i->getMipmapsType() == Image::MIPMAPS_GENERATED;

	if (i->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 3) - 1;

	mipmap = (int) luaL_optinteger(L, 4, 1) - 1;

	if (!lua_isnoneornil(L, 5))
	{
		x = (int) luaL_checkinteger(L, 5);
		y = (int) luaL_checkinteger(L, 6);

		if (reloadmipmaps)
			reloadmipmaps = luax_optboolean(L, 7, reloadmipmaps);
	}

	luax_catchexcept(L, [&](){ i->replacePixels(id, slice, mipmap, x, y, reloadmipmaps); });
	return 0;
}

static const luaL_Reg w_Image_functions[] =
{
	{ "isFormatLinear", w_Image_isFormatLinear },
	{ "isCompressed", w_Image_isCompressed },
	{ "replacePixels", w_Image_replacePixels },
	{ 0, 0 }
};

extern "C" int luaopen_image(lua_State *L)
{
	return luax_register_type(L, &Image::type, w_Texture_functions, w_Image_functions, nullptr);
}

} // graphics
} // love
