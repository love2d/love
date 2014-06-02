/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

static Image *instance = nullptr;

int w_newImageData(lua_State *L)
{
	// Case 1: Integers.
	if (lua_isnumber(L, 1))
	{
		int w = luaL_checkint(L, 1);
		int h = luaL_checkint(L, 2);
		if (w <= 0 || h <= 0)
			return luaL_error(L, "Invalid image size.");

		ImageData *t = nullptr;
		EXCEPT_GUARD(t = instance->newImageData(w, h);)

		luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, t);
		return 1;
	}

	// Case 2: File(Data).
	love::filesystem::FileData *data = love::filesystem::luax_getFileData(L, 1);

	ImageData *t = nullptr;
	EXCEPT_GUARD_FINALLY(t = instance->newImageData(data);, data->release();)

	luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, t);
	return 1;
}

int w_newCompressedData(lua_State *L)
{
	love::filesystem::FileData *data = love::filesystem::luax_getFileData(L, 1);

	CompressedData *t = nullptr;
	EXCEPT_GUARD_FINALLY(t = instance->newCompressedData(data);, data->release();)

	luax_pushtype(L, "CompressedData", IMAGE_COMPRESSED_DATA_T, t);
	return 1;
}

int w_isCompressed(lua_State *L)
{
	love::filesystem::FileData *data = love::filesystem::luax_getFileData(L, 1);
	bool compressed = instance->isCompressed(data);
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
	luaopen_compresseddata,
	0
};

extern "C" int luaopen_love_image(lua_State *L)
{
	if (instance == nullptr)
	{
		EXCEPT_GUARD(instance = new love::image::magpie::Image();)
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "image";
	w.flags = MODULE_IMAGE_T;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // image
} // love
