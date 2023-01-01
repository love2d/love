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

#include "wrap_Image.h"
#include "data/wrap_Data.h"
#include "common/Data.h"
#include "common/StringMap.h"

#include "Image.h"

#include "filesystem/wrap_Filesystem.h"

namespace love
{
namespace image
{

#define instance() (Module::getInstance<Image>(Module::M_IMAGE))

int w_newImageData(lua_State *L)
{
	// Case 1: width & height.
	if (lua_isnumber(L, 1))
	{
		int w = (int) luaL_checkinteger(L, 1);
		int h = (int) luaL_checkinteger(L, 2);
		if (w <= 0 || h <= 0)
			return luaL_error(L, "Invalid image size.");

		PixelFormat format = PIXELFORMAT_RGBA8;

		if (!lua_isnoneornil(L, 3))
		{
			const char *fstr = luaL_checkstring(L, 3);
			if (!getConstant(fstr, format))
				return luax_enumerror(L, "pixel format", fstr);
		}

		size_t numbytes = 0;
		const char *bytes = nullptr;

		if (luax_istype(L, 4, Data::type))
		{
			Data *data = data::luax_checkdata(L, 4);
			bytes = (const char *) data->getData();
			numbytes = data->getSize();
		}
		else if (!lua_isnoneornil(L, 4))
			bytes = luaL_checklstring(L, 4, &numbytes);

		ImageData *t = nullptr;
		luax_catchexcept(L, [&](){ t = instance()->newImageData(w, h, format); });

		if (bytes)
		{
			if (numbytes != t->getSize())
			{
				t->release();
				return luaL_error(L, "The size of the raw byte string must match the ImageData's actual size in bytes.");
			}

			memcpy(t->getData(), bytes, t->getSize());
		}

		luax_pushtype(L, t);
		t->release();
		return 1;
	}
	else if (filesystem::luax_cangetdata(L, 1)) // Case 2: File(Data).
	{
		Data *data = love::filesystem::luax_getdata(L, 1);

		ImageData *t = nullptr;
		luax_catchexcept(L,
			[&]() { t = instance()->newImageData(data); },
			[&](bool) { data->release(); }
		);

		luax_pushtype(L, t);
		t->release();
		return 1;
	}
	else
	{
		return luax_typerror(L, 1, "value");
	}
}

int w_newCompressedData(lua_State *L)
{
	Data *data = love::filesystem::luax_getdata(L, 1);

	CompressedImageData *t = nullptr;
	luax_catchexcept(L,
		[&]() { t = instance()->newCompressedData(data); },
		[&](bool) { data->release(); }
	);

	luax_pushtype(L, CompressedImageData::type, t);
	t->release();
	return 1;
}

int w_isCompressed(lua_State *L)
{
	Data *data = love::filesystem::luax_getdata(L, 1);
	bool compressed = instance()->isCompressed(data);
	data->release();

	luax_pushboolean(L, compressed);
	return 1;
}

int w_newCubeFaces(lua_State *L)
{
	ImageData *id = luax_checkimagedata(L, 1);
	std::vector<StrongRef<ImageData>> faces;
	luax_catchexcept(L, [&](){ faces = instance()->newCubeFaces(id); });
	for (auto face : faces)
		luax_pushtype(L, face);
	return (int) faces.size();
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "newImageData",  w_newImageData },
	{ "newCompressedData", w_newCompressedData },
	{ "isCompressed", w_isCompressed },
	{ "newCubeFaces", w_newCubeFaces },
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
		luax_catchexcept(L, [&](){ instance = new love::image::Image(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "image";
	w.type = &Image::type;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // image
} // love
