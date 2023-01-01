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
#include "wrap_DataModule.h"
#include "wrap_Data.h"
#include "wrap_ByteData.h"
#include "wrap_DataView.h"
#include "wrap_CompressedData.h"
#include "DataModule.h"
#include "common/b64.h"

// Lua 5.3
#include "libraries/lua53/lstrlib.h"

// C
#include <cmath>
#include <iostream>
#include <algorithm>
#include <limits>

namespace love
{
namespace data
{

#define instance() (Module::getInstance<DataModule>(Module::M_DATA))

ContainerType luax_checkcontainertype(lua_State *L, int idx)
{
	const char *str = luaL_checkstring(L, idx);
	ContainerType ctype = CONTAINER_STRING;
	if (!getConstant(str, ctype))
		luax_enumerror(L, "container type", getConstants(ctype), str);
	return ctype;
}

int w_newDataView(lua_State *L)
{
	Data *data = luax_checkdata(L, 1);

	lua_Integer offset = luaL_checkinteger(L, 2);
	lua_Integer size = luaL_checkinteger(L, 3);

	if (offset < 0 || size < 0)
		return luaL_error(L, "DataView offset and size must not be negative.");

	DataView *d;
	luax_catchexcept(L, [&]() { d = instance()->newDataView(data, (size_t) offset, (size_t) size); });
	luax_pushtype(L, d);
	d->release();

	return 1;
}

int w_newByteData(lua_State *L)
{
	ByteData *d = nullptr;

	if (luax_istype(L, 1, Data::type))
	{
		Data *data = luax_checkdata(L, 1);

		if (data->getSize() > std::numeric_limits<lua_Integer>::max())
			return luaL_error(L, "Data's size is too large!");

		lua_Integer offset = luaL_optinteger(L, 2, 0);
		if (offset < 0)
			return luaL_error(L, "Offset argument must not be negative.");

		lua_Integer size = luaL_optinteger(L, 3, data->getSize() - offset);
		if (size <= 0)
			return luaL_error(L, "Size argument must be greater than zero.");
		else if ((size_t)(offset + size) > data->getSize())
			return luaL_error(L, "Offset and size arguments must fit within the given Data's size.");

		const char *bytes = (const char *) data->getData() + offset;
		luax_catchexcept(L, [&]() { d = instance()->newByteData(bytes, (size_t) size); });
	}
	else if (lua_type(L, 1) == LUA_TSTRING)
	{
		size_t size = 0;
		const char *data = luaL_checklstring(L, 1, &size);
		luax_catchexcept(L, [&]() { d = instance()->newByteData(data, size); });
	}
	else
	{
		lua_Integer size = luaL_checkinteger(L, 1);
		if (size <= 0)
			return luaL_error(L, "Data size must be a positive number.");
		luax_catchexcept(L, [&]() { d = instance()->newByteData((size_t) size); });
	}

	luax_pushtype(L, d);
	d->release();
	return 1;
}

int w_compress(lua_State *L)
{
	ContainerType ctype = luax_checkcontainertype(L, 1);

	const char *fstr = luaL_checkstring(L, 2);
	Compressor::Format format = Compressor::FORMAT_LZ4;

	if (!Compressor::getConstant(fstr, format))
		return luax_enumerror(L, "compressed data format", Compressor::getConstants(format), fstr);

	int level = (int) luaL_optinteger(L, 4, -1);
	size_t rawsize = 0;
	const char *rawbytes = nullptr;

	if (lua_isstring(L, 3))
		rawbytes = luaL_checklstring(L, 3, &rawsize);
	else
	{
		Data *rawdata = luax_checktype<Data>(L, 3);
		rawsize = rawdata->getSize();
		rawbytes = (const char *) rawdata->getData();
	}

	CompressedData *cdata = nullptr;
	luax_catchexcept(L, [&](){ cdata = compress(format, rawbytes, rawsize, level); });

	if (ctype == CONTAINER_DATA)
		luax_pushtype(L, cdata);
	else
		lua_pushlstring(L, (const char *) cdata->getData(), cdata->getSize());

	cdata->release();
	return 1;
}

int w_decompress(lua_State *L)
{
	ContainerType ctype = luax_checkcontainertype(L, 1);

	char *rawbytes = nullptr;
	size_t rawsize = 0;

	if (luax_istype(L, 2, CompressedData::type))
	{
		CompressedData *data = luax_checkcompresseddata(L, 2);
		rawsize = data->getDecompressedSize();
		luax_catchexcept(L, [&](){ rawbytes = decompress(data, rawsize); });
	}
	else
	{
		Compressor::Format format = Compressor::FORMAT_LZ4;
		const char *fstr = luaL_checkstring(L, 2);

		if (!Compressor::getConstant(fstr, format))
			return luax_enumerror(L, "compressed data format", Compressor::getConstants(format), fstr);

		size_t compressedsize = 0;
		const char *cbytes = nullptr;

		if (luax_istype(L, 3, Data::type))
		{
			Data *data = luax_checktype<Data>(L, 3);
			cbytes = (const char *) data->getData();
			compressedsize = data->getSize();
		}
		else
			cbytes = luaL_checklstring(L, 3, &compressedsize);

		luax_catchexcept(L, [&](){ rawbytes = decompress(format, cbytes, compressedsize, rawsize); });
	}

	if (ctype == CONTAINER_DATA)
	{
		ByteData *data = nullptr;
		luax_catchexcept(L, [&]() { data = instance()->newByteData(rawbytes, rawsize, true); });
		luax_pushtype(L, Data::type, data);
		data->release();
	}
	else
	{
		lua_pushlstring(L, rawbytes, rawsize);
		delete[] rawbytes;
	}

	return 1;
}

int w_encode(lua_State *L)
{
	ContainerType ctype = luax_checkcontainertype(L, 1);

	const char *formatstr = luaL_checkstring(L, 2);
	EncodeFormat format;
	if (!getConstant(formatstr, format))
		return luax_enumerror(L, "encode format", getConstants(format), formatstr);

	size_t srclen = 0;
	const char *src = nullptr;

	if (luax_istype(L, 3, Data::type))
	{
		Data *data = luax_totype<Data>(L, 3);
		src = (const char *) data->getData();
		srclen = data->getSize();
	}
	else
		src = luaL_checklstring(L, 3, &srclen);

	size_t linelen = (size_t) luaL_optinteger(L, 4, 0);

	size_t dstlen = 0;
	char *dst = nullptr;
	luax_catchexcept(L, [&](){ dst = encode(format, src, srclen, dstlen, linelen); });

	if (ctype == CONTAINER_DATA)
	{
		ByteData *data = nullptr;
		if (dst != nullptr)
			luax_catchexcept(L, [&]() { data = instance()->newByteData(dst, dstlen, true); });
		else
			luax_catchexcept(L, [&]() { data = instance()->newByteData(0); });

		luax_pushtype(L, Data::type, data);
		data->release();
	}
	else
	{
		if (dst != nullptr)
			lua_pushlstring(L, dst, dstlen);
		else
			lua_pushstring(L, "");

		delete[] dst;
	}

	return 1;
}

int w_decode(lua_State *L)
{
	ContainerType ctype = luax_checkcontainertype(L, 1);

	const char *formatstr = luaL_checkstring(L, 2);
	EncodeFormat format;
	if (!getConstant(formatstr, format))
		return luax_enumerror(L, "decode format", getConstants(format), formatstr);

	size_t srclen = 0;
	const char *src = nullptr;

	if (luax_istype(L, 3, Data::type))
	{
		Data *data = luax_totype<Data>(L, 3);
		src = (const char *) data->getData();
		srclen = data->getSize();
	}
	else
		src = luaL_checklstring(L, 3, &srclen);

	size_t dstlen = 0;
	char *dst = nullptr;
	luax_catchexcept(L, [&](){ dst = decode(format, src, srclen, dstlen); });

	if (ctype == CONTAINER_DATA)
	{
		ByteData *data = nullptr;
		if (dst != nullptr)
			luax_catchexcept(L, [&]() { data = instance()->newByteData(dst, dstlen, true); });
		else
			luax_catchexcept(L, [&]() { data = instance()->newByteData(0); });

		luax_pushtype(L, Data::type, data);
		data->release();
	}
	else
	{
		if (dst != nullptr)
			lua_pushlstring(L, dst, dstlen);
		else
			lua_pushstring(L, "");

		delete[] dst;
	}

	return 1;
}

int w_hash(lua_State *L)
{
	const char *fstr = luaL_checkstring(L, 1);
	HashFunction::Function function;
	if (!HashFunction::getConstant(fstr, function))
		return luax_enumerror(L, "hash function", HashFunction::getConstants(function), fstr);

	HashFunction::Value hashvalue;
	if (lua_isstring(L, 2))
	{
		size_t rawsize = 0;
		const char *rawbytes = luaL_checklstring(L, 2, &rawsize);
		luax_catchexcept(L, [&](){ love::data::hash(function, rawbytes, rawsize, hashvalue); });
	}
	else
	{
		Data *rawdata = luax_checktype<Data>(L, 2);
		luax_catchexcept(L, [&](){ love::data::hash(function, rawdata, hashvalue); });
	}

	lua_pushlstring(L, hashvalue.data, hashvalue.size);
	return 1;
}

int w_pack(lua_State *L)
{
	ContainerType ctype = luax_checkcontainertype(L, 1);
	const char *fmt = luaL_checkstring(L, 2);
	luaL_Buffer_53 b;
	lua53_str_pack(L, fmt, 3, &b);

	if (ctype == CONTAINER_DATA)
	{
		Data *d = nullptr;
		luax_catchexcept(L, [&]() { d = instance()->newByteData(b.nelems); });
		memcpy(d->getData(), b.ptr, d->getSize());

		lua53_cleanupbuffer(&b);
		luax_pushtype(L, Data::type, d);
		d->release();
	}
	else
		lua53_pushresult(&b);

	return 1;
}

int w_unpack(lua_State *L)
{
	const char *fmt = luaL_checkstring(L, 1);

	const char *data = nullptr;
	size_t datasize = 0;

	if (luax_istype(L, 2, Data::type))
	{
		Data *d = luax_checkdata(L, 2);
		data = (const char *) d->getData();
		datasize = d->getSize();
	}
	else
		data = luaL_checklstring(L, 2, &datasize);

	return lua53_str_unpack(L, fmt, data, datasize, 2, 3);
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "newDataView", w_newDataView },
	{ "newByteData", w_newByteData },
	{ "compress", w_compress },
	{ "decompress", w_decompress },
	{ "encode", w_encode },
	{ "decode", w_decode },
	{ "hash", w_hash },

	{ "pack", w_pack },
	{ "unpack", w_unpack },
	{ "getPackedSize", lua53_str_packsize },

	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_data,
	luaopen_bytedata,
	luaopen_dataview,
	luaopen_compresseddata,
	nullptr
};

extern "C" int luaopen_love_data(lua_State *L)
{
	DataModule *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new DataModule(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "data";
	w.type = &Module::type;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);
	return n;
}

} // data
} // love
