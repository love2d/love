/**
* Copyright (c) 2006-2020 LOVE Development Team
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
	const auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		componentdata[i] = (T) (luax_optnumberclamped(L, startidx + i, -1.0, 1.0, defaultComponents[i]) * maxval);

	return sizeof(T) * components;
}

template <typename T>
static inline size_t writeUNormData(lua_State *L, int startidx, int components, char *data)
{
	auto componentdata = (T *) data;
	const auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		componentdata[i] = (T) (luax_optnumberclamped01(L, startidx + i, 1.0) * maxval);

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
	const auto maxval = std::numeric_limits<T>::max();

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, std::max(-1.0, (lua_Number) componentdata[i] / (lua_Number)maxval));

	return sizeof(T) * components;
}

template <typename T>
static inline size_t readUNormData(lua_State *L, int components, const char *data)
{
	const auto componentdata = (const T *) data;
	const auto maxval = std::numeric_limits<T>::max();

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

static const luaL_Reg w_Buffer_functions[] =
{
	{ 0, 0 }
};

extern "C" int luaopen_graphicsbuffer(lua_State *L)
{
	return luax_register_type(L, &Buffer::type, w_Buffer_functions, nullptr);
}

} // graphics
} // love
