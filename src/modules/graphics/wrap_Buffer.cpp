/**
* Copyright (c) 2006-2024 LOVE Development Team
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

#include "wrap_Buffer.h"
#include "Buffer.h"
#include "common/Data.h"

#include <limits>

namespace love
{
namespace graphics
{

static const double defaultComponents[] = {0.0, 0.0, 0.0, 1.0};

template <typename T>
static inline size_t writeData(lua_State *L, int startidx, int components, char *data)
{
	auto componentdata = (T *) data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (T) (luaL_optnumber(L, startidx + i, defaultComponents[i]));

	return sizeof(T) * components;
}

template <typename T>
static inline size_t writeSNormData(lua_State *L, int startidx, int components, char *data)
{
	auto componentdata = (T *) data;
	constexpr auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		componentdata[i] = (T) (luax_optnumberclamped(L, startidx + i, -1.0, 1.0, defaultComponents[i]) * maxval);

	return sizeof(T) * components;
}

template <typename T>
static inline size_t writeUNormData(lua_State *L, int startidx, int components, char *data)
{
	auto componentdata = (T *) data;
	constexpr auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		componentdata[i] = (T) (luax_optnumberclamped01(L, startidx + i, 1.0) * maxval);

	return sizeof(T) * components;
}

template <typename T>
static inline size_t writeDataRequired(lua_State *L, int startidx, int components, char *data)
{
	auto componentdata = (T*)data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (T)(luaL_checknumber(L, startidx + i));

	return sizeof(T) * components;
}

void luax_writebufferdata(lua_State *L, int startidx, DataFormat format, char *data)
{
	switch (format)
	{
		case DATAFORMAT_FLOAT:      writeData<float>(L, startidx, 1, data); break;
		case DATAFORMAT_FLOAT_VEC2: writeData<float>(L, startidx, 2, data); break;
		case DATAFORMAT_FLOAT_VEC3: writeData<float>(L, startidx, 3, data); break;
		case DATAFORMAT_FLOAT_VEC4: writeData<float>(L, startidx, 4, data); break;

		case DATAFORMAT_FLOAT_MAT2X2: writeDataRequired<float>(L, startidx, 4, data); break;
		case DATAFORMAT_FLOAT_MAT2X3: writeDataRequired<float>(L, startidx, 6, data); break;
		case DATAFORMAT_FLOAT_MAT2X4: writeDataRequired<float>(L, startidx, 8, data); break;

		case DATAFORMAT_FLOAT_MAT3X2: writeDataRequired<float>(L, startidx, 6, data); break;
		case DATAFORMAT_FLOAT_MAT3X3: writeDataRequired<float>(L, startidx, 9, data); break;
		case DATAFORMAT_FLOAT_MAT3X4: writeDataRequired<float>(L, startidx, 12, data); break;

		case DATAFORMAT_FLOAT_MAT4X2: writeDataRequired<float>(L, startidx, 8, data); break;
		case DATAFORMAT_FLOAT_MAT4X3: writeDataRequired<float>(L, startidx, 12, data); break;
		case DATAFORMAT_FLOAT_MAT4X4: writeDataRequired<float>(L, startidx, 16, data); break;

		case DATAFORMAT_INT32:      writeData<int32>(L, startidx, 1, data); break;
		case DATAFORMAT_INT32_VEC2: writeData<int32>(L, startidx, 2, data); break;
		case DATAFORMAT_INT32_VEC3: writeData<int32>(L, startidx, 3, data); break;
		case DATAFORMAT_INT32_VEC4: writeData<int32>(L, startidx, 4, data); break;

		case DATAFORMAT_UINT32:      writeData<uint32>(L, startidx, 1, data); break;
		case DATAFORMAT_UINT32_VEC2: writeData<uint32>(L, startidx, 2, data); break;
		case DATAFORMAT_UINT32_VEC3: writeData<uint32>(L, startidx, 3, data); break;
		case DATAFORMAT_UINT32_VEC4: writeData<uint32>(L, startidx, 4, data); break;

		case DATAFORMAT_SNORM8_VEC4: writeSNormData<int8>(L, startidx, 4, data); break;
		case DATAFORMAT_UNORM8_VEC4: writeUNormData<uint8>(L, startidx, 4, data); break;
		case DATAFORMAT_INT8_VEC4:   writeData<int8>(L, startidx, 4, data); break;
		case DATAFORMAT_UINT8_VEC4:  writeData<uint8>(L, startidx, 4, data); break;

		case DATAFORMAT_SNORM16_VEC2: writeSNormData<int16>(L, startidx, 2, data); break;
		case DATAFORMAT_SNORM16_VEC4: writeSNormData<int16>(L, startidx, 4, data); break;

		case DATAFORMAT_UNORM16_VEC2: writeUNormData<uint16>(L, startidx, 2, data); break;
		case DATAFORMAT_UNORM16_VEC4: writeUNormData<uint16>(L, startidx, 4, data); break;

		case DATAFORMAT_INT16_VEC2: writeData<int16>(L, startidx, 2, data); break;
		case DATAFORMAT_INT16_VEC4: writeData<int16>(L, startidx, 4, data); break;

		case DATAFORMAT_UINT16:      writeData<uint16>(L, startidx, 1, data); break;
		case DATAFORMAT_UINT16_VEC2: writeData<uint16>(L, startidx, 2, data); break;
		case DATAFORMAT_UINT16_VEC4: writeData<uint16>(L, startidx, 4, data); break;

		default: break;
	}
}

template <typename T>
static inline size_t readData(lua_State *L, int components, const char *data)
{
	const auto componentdata = (const T *) data;

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) componentdata[i]);

	return sizeof(T) * components;
}

template <typename T>
static inline size_t readSNormData(lua_State *L, int components, const char *data)
{
	const auto componentdata = (const T *) data;
	constexpr auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, std::max(-1.0, (lua_Number) componentdata[i] / (lua_Number)maxval));

	return sizeof(T) * components;
}

template <typename T>
static inline size_t readUNormData(lua_State *L, int components, const char *data)
{
	const auto componentdata = (const T *) data;
	constexpr auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) componentdata[i] / (lua_Number)maxval);

	return sizeof(T) * components;
}

void luax_readbufferdata(lua_State *L, DataFormat format, const char *data)
{
	switch (format)
	{
		case DATAFORMAT_FLOAT:      readData<float>(L, 1, data); break;
		case DATAFORMAT_FLOAT_VEC2: readData<float>(L, 2, data); break;
		case DATAFORMAT_FLOAT_VEC3: readData<float>(L, 3, data); break;
		case DATAFORMAT_FLOAT_VEC4: readData<float>(L, 4, data); break;

		case DATAFORMAT_INT32:      readData<int32>(L, 1, data); break;
		case DATAFORMAT_INT32_VEC2: readData<int32>(L, 2, data); break;
		case DATAFORMAT_INT32_VEC3: readData<int32>(L, 3, data); break;
		case DATAFORMAT_INT32_VEC4: readData<int32>(L, 4, data); break;

		case DATAFORMAT_UINT32:      readData<uint32>(L, 1, data); break;
		case DATAFORMAT_UINT32_VEC2: readData<uint32>(L, 2, data); break;
		case DATAFORMAT_UINT32_VEC3: readData<uint32>(L, 3, data); break;
		case DATAFORMAT_UINT32_VEC4: readData<uint32>(L, 4, data); break;

		case DATAFORMAT_SNORM8_VEC4: readSNormData<int8>(L, 4, data); break;
		case DATAFORMAT_UNORM8_VEC4: readUNormData<uint8>(L, 4, data); break;
		case DATAFORMAT_INT8_VEC4:   readData<int8>(L, 4, data); break;
		case DATAFORMAT_UINT8_VEC4:  readData<uint8>(L, 4, data); break;

		case DATAFORMAT_SNORM16_VEC2: readSNormData<int16>(L, 2, data); break;
		case DATAFORMAT_SNORM16_VEC4: readSNormData<int16>(L, 4, data); break;

		case DATAFORMAT_UNORM16_VEC2: readUNormData<uint16>(L, 2, data); break;
		case DATAFORMAT_UNORM16_VEC4: readUNormData<uint16>(L, 4, data); break;

		case DATAFORMAT_INT16_VEC2: readData<int16>(L, 2, data); break;
		case DATAFORMAT_INT16_VEC4: readData<int16>(L, 4, data); break;

		case DATAFORMAT_UINT16:      readData<uint16>(L, 1, data); break;
		case DATAFORMAT_UINT16_VEC2: readData<uint16>(L, 2, data); break;
		case DATAFORMAT_UINT16_VEC4: readData<uint16>(L, 4, data); break;

		default: break;
	}
}

Buffer *luax_checkbuffer(lua_State *L, int idx)
{
	return luax_checktype<Buffer>(L, idx);
}

static int w_Buffer_setArrayData(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);

	int sourceindex = (int) luaL_optnumber(L, 3, 1) - 1;
	int destindex = (int) luaL_optnumber(L, 4, 1) - 1;

	if (sourceindex < 0)
		return luaL_error(L, "Source start index must be at least 1.");

	int count = -1;
	if (!lua_isnoneornil(L, 5))
	{
		count = (int) luaL_checknumber(L, 5);
		if (count <= 0)
			return luaL_error(L, "Element count must be greater than 0.");
	}

	size_t stride = t->getArrayStride();
	size_t bufferoffset = destindex * stride;
	int arraylength = (int) t->getArrayLength();

	if (destindex >= arraylength || destindex < 0)
		return luaL_error(L, "Invalid buffer start index (must be between 1 and %d)", arraylength);

	if (luax_istype(L, 2, Data::type))
	{
		Data *d = luax_checktype<Data>(L, 2);

		int dataarraylength = d->getSize() / stride;

		if (sourceindex >= dataarraylength)
			return luaL_error(L, "Invalid data start index (must be between 1 and %d)", dataarraylength);

		int maxcount = std::min(dataarraylength - sourceindex, arraylength - destindex);

		if (count < 0)
			count = maxcount;

		if (count > maxcount)
			return luaL_error(L, "Too many array elements (expected at most %d, got %d)", maxcount, count);

		size_t dataoffset = sourceindex * stride;
		size_t datasize = std::min(d->getSize() - dataoffset, count * stride);
		const void *sourcedata = (const uint8 *) d->getData() + dataoffset;

		t->fill(bufferoffset, datasize, sourcedata);
		return 0;
	}

	const std::vector<Buffer::DataMember> &members = t->getDataMembers();

	int ncomponents = 0;
	for (const Buffer::DataMember &member : members)
		ncomponents += member.info.components;

	luaL_checktype(L, 2, LUA_TTABLE);
	int tablelen = (int) luax_objlen(L, 2);

	lua_rawgeti(L, 2, 1);
	bool tableoftables = lua_istable(L, -1);
	lua_pop(L, 1);

	if (!tableoftables)
	{
		if (tablelen % ncomponents != 0)
			return luaL_error(L, "Array length in flat array variant of Buffer:setArrayData must be a multiple of the total number of components (%d)", ncomponents);
		tablelen /= ncomponents;
	}

	if (sourceindex >= tablelen)
		return luaL_error(L, "Invalid data start index (must be between 1 and %d)", tablelen);

	count = count >= 0 ? std::min(count, tablelen - sourceindex) : tablelen - sourceindex;

	if (destindex + count > arraylength)
		return luaL_error(L, "Too many array elements (expected at most %d, got %d)", arraylength - destindex, count);

	char *data = (char *) t->map(Buffer::MAP_WRITE_INVALIDATE, bufferoffset, count * stride);

	if (tableoftables)
	{
		for (int i = sourceindex; i < count; i++)
		{
			// get arraydata[index]
			lua_rawgeti(L, 2, i + 1);
			luaL_checktype(L, -1, LUA_TTABLE);

			// get arraydata[index][j]
			for (int j = 1; j <= ncomponents; j++)
				lua_rawgeti(L, -j, j);

			int idx = -ncomponents;

			for (const Buffer::DataMember &member : members)
			{
				luax_writebufferdata(L, idx, member.decl.format, data + member.offset);
				idx += member.info.components;
			}

			lua_pop(L, ncomponents + 1);
			data += stride;
		}
	}
	else // Flat array
	{
		for (int i = sourceindex; i < count; i++)
		{
			// get arraydata[arrayindex * ncomponents + componentindex]
			for (int componentindex = 1; componentindex <= ncomponents; componentindex++)
				lua_rawgeti(L, 2, i * ncomponents + componentindex);

			int idx = -ncomponents;

			for (const Buffer::DataMember &member : members)
			{
				luax_writebufferdata(L, idx, member.decl.format, data + member.offset);
				idx += member.info.components;
			}

			lua_pop(L, ncomponents);
			data += stride;
		}
	}

	t->unmap(bufferoffset, count * stride);

	return 0;
}

static int w_Buffer_clear(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	size_t offset = 0;
	size_t size = t->getSize();
	if (!lua_isnoneornil(L, 2))
	{
		lua_Number offsetp = luaL_checknumber(L, 2);
		lua_Number sizep = luaL_checknumber(L, 3);
		if (offsetp < 0 || sizep < 0)
			return luaL_error(L, "Offset and size parameters cannot be negative.");
		offset = (size_t) offsetp;
		size = (size_t) sizep;
	}
	luax_catchexcept(L, [&]() { t->clear(offset, size); });
	return 0;
}

static int w_Buffer_getElementCount(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	lua_pushinteger(L, t->getArrayLength());
	return 1;
}

static int w_Buffer_getElementStride(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	lua_pushinteger(L, t->getArrayStride());
	return 1;
}

static int w_Buffer_getSize(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	lua_pushinteger(L, t->getSize());
	return 1;
}

static int w_Buffer_getFormat(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	const auto &members = t->getDataMembers();

	lua_createtable(L, (int) members.size(), 0);

	for (size_t i = 0; i < members.size(); i++)
	{
		const Buffer::DataMember &member = members[i];

		lua_createtable(L, 0, 4);

		lua_pushstring(L, member.decl.name.c_str());
		lua_setfield(L, -2, "name");

		const char *formatstr = "unknown";
		getConstant(member.decl.format, formatstr);
		lua_pushstring(L, formatstr);
		lua_setfield(L, -2, "format");

		lua_pushinteger(L, member.decl.arrayLength);
		lua_setfield(L, -2, "arraylength");

		lua_pushinteger(L, member.decl.bindingLocation);
		lua_setfield(L, -2, "location");

		lua_pushinteger(L, member.offset);
		lua_setfield(L, -2, "offset");

		lua_pushinteger(L, member.size);
		lua_setfield(L, -2, "size");

		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

static int w_Buffer_isBufferType(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	BufferUsage bufferusage = BUFFERUSAGE_MAX_ENUM;
	const char *typestr = luaL_checkstring(L, 2);
	if (!getConstant(typestr, bufferusage))
		return luax_enumerror(L, "buffer type", getConstants(bufferusage), typestr);
	luax_pushboolean(L, (t->getUsageFlags() & (1 << bufferusage)) != 0);
	return 1;
}

static int w_Buffer_getDebugName(lua_State *L)
{
	Buffer *t = luax_checkbuffer(L, 1);
	const std::string &debugName = t->getDebugName();
	if (debugName.empty())
		lua_pushnil(L);
	else
		luax_pushstring(L, debugName);
	return 1;
}

static const luaL_Reg w_Buffer_functions[] =
{
	{ "setArrayData", w_Buffer_setArrayData },
	{ "clear", w_Buffer_clear },
	{ "getElementCount", w_Buffer_getElementCount },
	{ "getElementStride", w_Buffer_getElementStride },
	{ "getSize", w_Buffer_getSize },
	{ "getFormat", w_Buffer_getFormat },
	{ "isBufferType", w_Buffer_isBufferType },
	{ "getDebugName", w_Buffer_getDebugName },
	{ 0, 0 }
};

extern "C" int luaopen_graphicsbuffer(lua_State *L)
{
	return luax_register_type(L, &Buffer::type, w_Buffer_functions, nullptr);
}

} // graphics
} // love
