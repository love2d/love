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

#include "wrap_Image.h"

#include "common/Data.h"
#include "common/StringMap.h"

#include "magpie/Image.h"

#include "filesystem/wrap_Filesystem.h"

namespace love
{
namespace image
{

#define instance() (Module::getInstance<Image>(Module::M_IMAGE))

int w_newImageData(lua_State *L)
{
	// Case 1: Integers.
	if (lua_isnumber(L, 1))
	{
		int w = (int) luaL_checknumber(L, 1);
		int h = (int) luaL_checknumber(L, 2);
		if (w <= 0 || h <= 0)
			return luaL_error(L, "Invalid image size.");

		size_t numbytes = 0;
		const char *bytes = nullptr;

		if (!lua_isnoneornil(L, 3))
			bytes = luaL_checklstring(L, 3, &numbytes);

		ImageData *t = nullptr;
		luax_catchexcept(L, [&](){ t = instance()->newImageData(w, h); });

		if (bytes)
		{
			if (numbytes != t->getSize())
			{
				t->release();
				return luaL_error(L, "The size of the raw byte string must match the ImageData's actual size in bytes.");
			}

			memcpy(t->getData(), bytes, t->getSize());
		}

		luax_pushtype(L, IMAGE_IMAGE_DATA_ID, t);
		t->release();
		return 1;
	}

	// Case 2: File(Data).
	love::filesystem::FileData *data = love::filesystem::luax_getfiledata(L, 1);

	ImageData *t = nullptr;
	luax_catchexcept(L,
		[&]() { t = instance()->newImageData(data); },
		[&](bool) { data->release(); }
	);

	luax_pushtype(L, IMAGE_IMAGE_DATA_ID, t);
	t->release();
	return 1;
}

int w_newCompressedData(lua_State *L)
{
	love::filesystem::FileData *data = love::filesystem::luax_getfiledata(L, 1);

	CompressedImageData *t = nullptr;
	luax_catchexcept(L,
		[&]() { t = instance()->newCompressedData(data); },
		[&](bool) { data->release(); }
	);

	luax_pushtype(L, IMAGE_COMPRESSED_IMAGE_DATA_ID, t);
	t->release();
	return 1;
}

int w_isCompressed(lua_State *L)
{
	love::filesystem::FileData *data = love::filesystem::luax_getfiledata(L, 1);
	bool compressed = instance()->isCompressed(data);
	data->release();

	luax_pushboolean(L, compressed);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "newImageData",  w_newImageData },
	{ "newCompressedData", w_newCompressedData },
	{ "isCompressed", w_isCompressed },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_imagedata,
	luaopen_compressedimagedata,
	0
};

extern "C" int luaopen_love_image(lua_State *L)
{
	Image *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::image::magpie::Image(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "image";
	w.type = MODULE_IMAGE_ID;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // image
} // love
