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

#include "wrap_Image.h"

#include "common/Data.h"
#include "common/StringMap.h"

#include "magpie/Image.h"

namespace love
{
namespace image
{

static Image *instance = 0;

int w_newImageData(lua_State *L)
{
	// Case 1: Integers.
	if (lua_isnumber(L, 1))
	{
		int w = luaL_checkint(L, 1);
		int h = luaL_checkint(L, 2);
		if (w <= 0 || h <= 0)
			return luaL_error(L, "Invalid image size.");

		ImageData *t = 0;
		try
		{
			t = instance->newImageData(w, h);
		}
		catch(love::Exception &e)
		{
			return luaL_error(L, "%s", e.what());
		}
		luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, t);
		return 1;
	}

	// Case 2: File(Data).

	// Convert to FileData, if necessary.
	if (lua_isstring(L, 1) || luax_istype(L, 1, FILESYSTEM_FILE_T))
		luax_convobj(L, 1, "filesystem", "newFileData");

	love::filesystem::FileData *data = luax_checktype<love::filesystem::FileData>(L, 1, "FileData", FILESYSTEM_FILE_DATA_T);

	ImageData *t = 0;
	try
	{
		t = instance->newImageData(data);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, t);

	return 1;
}

int w_newCompressedData(lua_State *L)
{
	// Convert to FileData, if necessary.
	if (lua_isstring(L, 1) || luax_istype(L, 1, FILESYSTEM_FILE_T))
		luax_convobj(L, 1, "filesystem", "newFileData");

	love::filesystem::FileData *data = luax_checktype<love::filesystem::FileData>(L, 1, "FileData", FILESYSTEM_FILE_DATA_T);

	CompressedData *t = 0;
	try
	{
		t = instance->newCompressedData(data);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	luax_pushtype(L, "CompressedData", IMAGE_COMPRESSED_DATA_T, t);

	return 1;
}

int w_isCompressed(lua_State *L)
{
	// Convert to FileData, if necessary.
	if (lua_isstring(L, 1) || luax_istype(L, 1, FILESYSTEM_FILE_T))
		luax_convobj(L, 1, "filesystem", "newFileData");

	love::filesystem::FileData *data = luax_checktype<love::filesystem::FileData>(L, 1, "FileData", FILESYSTEM_FILE_DATA_T);

	bool compressed = false;
	try
	{
		compressed = instance->isCompressed(data);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
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
	if (instance == 0)
	{
		try
		{
			instance = new love::image::magpie::Image();
		}
		catch(Exception &e)
		{
			return luaL_error(L, "%s", e.what());
		}
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
