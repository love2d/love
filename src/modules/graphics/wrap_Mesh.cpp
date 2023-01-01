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
#include "wrap_Mesh.h"
#include "Image.h"
#include "Canvas.h"
#include "wrap_Texture.h"

// C++
#include <algorithm>

namespace love
{
namespace graphics
{

Mesh *luax_checkmesh(lua_State *L, int idx)
{
	return luax_checktype<Mesh>(L, idx);
}

static inline size_t writeUnorm8Data(lua_State *L, int startidx, int components, char *data)
{
	uint8 *componentdata = (uint8 *) data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (uint8) (luax_optnumberclamped01(L, startidx + i, 1.0) * 255.0);

	return sizeof(uint8) * components;
}

static inline size_t writeUnorm16Data(lua_State *L, int startidx, int components, char *data)
{
	uint16 *componentdata = (uint16 *) data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (uint16) (luax_optnumberclamped01(L, startidx + i, 1.0) * 65535.0);

	return sizeof(uint16) * components;
}

static inline size_t writeFloatData(lua_State *L, int startidx, int components, char *data)
{
	float *componentdata = (float *) data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (float) luaL_optnumber(L, startidx + i, 0);

	return sizeof(float) * components;
}

char *luax_writeAttributeData(lua_State *L, int startidx, vertex::DataType type, int components, char *data)
{
	switch (type)
	{
	case vertex::DATA_UNORM8:
		return data + writeUnorm8Data(L, startidx, components, data);
	case vertex::DATA_UNORM16:
		return data + writeUnorm16Data(L, startidx, components, data);
	case vertex::DATA_FLOAT:
		return data + writeFloatData(L, startidx, components, data);
	default:
		return data;
	}
}

static inline size_t readUnorm8Data(lua_State *L, int components, const char *data)
{
	const uint8 *componentdata = (const uint8 *) data;

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) componentdata[i] / 255.0);

	return sizeof(uint8) * components;
}

static inline size_t readUnorm16Data(lua_State *L, int components, const char *data)
{
	const uint16 *componentdata = (const uint16 *) data;

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) componentdata[i] / 65535.0);

	return sizeof(uint16) * components;
}

static inline size_t readFloatData(lua_State *L, int components, const char *data)
{
	const float *componentdata = (const float *) data;

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, componentdata[i]);

	return sizeof(float) * components;
}

const char *luax_readAttributeData(lua_State *L, vertex::DataType type, int components, const char *data)
{
	switch (type)
	{
	case vertex::DATA_UNORM8:
		return data + readUnorm8Data(L, components, data);
	case vertex::DATA_UNORM16:
		return data + readUnorm16Data(L, components, data);
	case vertex::DATA_FLOAT:
		return data + readFloatData(L, components, data);
	default:
		return data;
	}
}

int w_Mesh_setVertices(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	int vertstart = (int) luaL_optnumber(L, 3, 1) - 1;

	int vertcount = -1;
	if (!lua_isnoneornil(L, 4))
	{
		vertcount = (int) luaL_checknumber(L, 4);
		if (vertcount <= 0)
			return luaL_error(L, "Vertex count must be greater than 0.");
	}

	size_t stride = t->getVertexStride();
	size_t byteoffset = vertstart * stride;
	int totalverts = (int) t->getVertexCount();

	if (vertstart >= totalverts)
		return luaL_error(L, "Invalid vertex start index (must be between 1 and %d)", totalverts);

	if (luax_istype(L, 2, Data::type))
	{
		Data *d = luax_checktype<Data>(L, 2);

		vertcount = vertcount >= 0 ? vertcount : (totalverts - vertstart);
		if (vertstart + vertcount > totalverts)
			return luaL_error(L, "Too many vertices (expected at most %d, got %d)", totalverts - vertstart, vertcount);

		size_t datasize = std::min(d->getSize(), vertcount * stride);
		char *bytedata = (char *) t->mapVertexData() + byteoffset;

		memcpy(bytedata, d->getData(), datasize);

		t->unmapVertexData(byteoffset, datasize);
		return 0;
	}

	luaL_checktype(L, 2, LUA_TTABLE);
	int tablelen = (int) luax_objlen(L, 2);

	vertcount = vertcount >= 0 ? std::min(vertcount, tablelen) : tablelen;
	if (vertstart + vertcount > totalverts)
		return luaL_error(L, "Too many vertices (expected at most %d, got %d)", totalverts - vertstart, vertcount);

	const std::vector<Mesh::AttribFormat> &vertexformat = t->getVertexFormat();

	int ncomponents = 0;
	for (const Mesh::AttribFormat &format : vertexformat)
		ncomponents += format.components;

	char *data = (char *) t->mapVertexData() + byteoffset;

	for (int i = 0; i < vertcount; i++)
	{
		// get vertices[vertindex]
		lua_rawgeti(L, 2, i + 1);
		luaL_checktype(L, -1, LUA_TTABLE);

		// get vertices[vertindex][j]
		for (int j = 1; j <= ncomponents; j++)
			lua_rawgeti(L, -j, j);

		int idx = -ncomponents;

		for (const Mesh::AttribFormat &format : vertexformat)
		{
			// Fetch the values from Lua and store them in data buffer.
			data = luax_writeAttributeData(L, idx, format.type, format.components, data);
			idx += format.components;
		}

		lua_pop(L, ncomponents + 1);
	}

	t->unmapVertexData(byteoffset, vertcount * stride);
	return 0;
}

int w_Mesh_setVertex(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t index = (size_t) luaL_checkinteger(L, 2) - 1;

	bool istable = lua_istable(L, 3);

	const std::vector<Mesh::AttribFormat> &vertexformat = t->getVertexFormat();

	char *data = (char *) t->getVertexScratchBuffer();
	char *writtendata = data;

	int idx = istable ? 1 : 3;

	if (istable)
	{
		for (const Mesh::AttribFormat &format : vertexformat)
		{
			for (int i = idx; i < idx + format.components; i++)
				lua_rawgeti(L, 3, i);

			// Fetch the values from Lua and store them in data buffer.
			writtendata = luax_writeAttributeData(L, -format.components, format.type, format.components, writtendata);

			idx += format.components;
			lua_pop(L, format.components);
		}
	}
	else
	{
		for (const Mesh::AttribFormat &format : vertexformat)
		{
			// Fetch the values from Lua and store them in data buffer.
			writtendata = luax_writeAttributeData(L, idx, format.type, format.components, writtendata);
			idx += format.components;
		}
	}

	luax_catchexcept(L, [&](){ t->setVertex(index, data, t->getVertexStride()); });
	return 0;
}

int w_Mesh_getVertex(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t index = (size_t) luaL_checkinteger(L, 2) - 1;

	const std::vector<Mesh::AttribFormat> &vertexformat = t->getVertexFormat();

	char *data = (char *) t->getVertexScratchBuffer();
	const char *readdata = data;

	luax_catchexcept(L, [&](){ t->getVertex(index, data, t->getVertexStride()); });

	int n = 0;

	for (const Mesh::AttribFormat &format : vertexformat)
	{
		readdata = luax_readAttributeData(L, format.type, format.components, readdata);
		n += format.components;
	}

	return n;
}

int w_Mesh_setVertexAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t vertindex = (size_t) luaL_checkinteger(L, 2) - 1;
	int attribindex = (int) luaL_checkinteger(L, 3) - 1;

	vertex::DataType type;
	int components;
	luax_catchexcept(L, [&](){ type = t->getAttributeInfo(attribindex, components); });

	// Maximum possible size for a single vertex attribute.
	char data[sizeof(float) * 4];

	// Fetch the values from Lua and store them in the data buffer.
	luax_writeAttributeData(L, 4, type, components, data);

	luax_catchexcept(L, [&](){ t->setVertexAttribute(vertindex, attribindex, data, sizeof(float) * 4); });
	return 0;
}

int w_Mesh_getVertexAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t vertindex = (size_t) luaL_checkinteger(L, 2) - 1;
	int attribindex = (int) luaL_checkinteger(L, 3) - 1;

	vertex::DataType type;
	int components;
	luax_catchexcept(L, [&](){ type = t->getAttributeInfo(attribindex, components); });

	// Maximum possible size for a single vertex attribute.
	char data[sizeof(float) * 4];

	luax_catchexcept(L, [&](){ t->getVertexAttribute(vertindex, attribindex, data, sizeof(float) * 4); });

	luax_readAttributeData(L, type, components, data);
	return components;
}

int w_Mesh_getVertexCount(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	lua_pushinteger(L, t->getVertexCount());
	return 1;
}

int w_Mesh_getVertexFormat(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	const std::vector<Mesh::AttribFormat> &vertexformat = t->getVertexFormat();
	lua_createtable(L, (int) vertexformat.size(), 0);

	const char *tname = nullptr;

	for (size_t i = 0; i < vertexformat.size(); i++)
	{
		if (!vertex::getConstant(vertexformat[i].type, tname))
			return luax_enumerror(L, "vertex attribute data type", vertex::getConstants(vertexformat[i].type), tname);

		lua_createtable(L, 3, 0);

		lua_pushstring(L, vertexformat[i].name.c_str());
		lua_rawseti(L, -2, 1);

		lua_pushstring(L, tname);
		lua_rawseti(L, -2, 2);

		lua_pushinteger(L, vertexformat[i].components);
		lua_rawseti(L, -2, 3);

		// format[i] = {name, type, components}
		lua_rawseti(L, -2, (int) i + 1);
	}

	return 1;
}

int w_Mesh_setAttributeEnabled(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *name = luaL_checkstring(L, 2);
	bool enable = luax_checkboolean(L, 3);
	luax_catchexcept(L, [&](){ t->setAttributeEnabled(name, enable); });
	return 0;
}

int w_Mesh_isAttributeEnabled(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *name = luaL_checkstring(L, 2);
	bool enabled = false;
	luax_catchexcept(L, [&](){ enabled = t->isAttributeEnabled(name); });
	lua_pushboolean(L, enabled);
	return 1;
}

int w_Mesh_attachAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *name = luaL_checkstring(L, 2);
	Mesh *mesh = luax_checkmesh(L, 3);

	AttributeStep step = STEP_PER_VERTEX;
	const char *stepstr = lua_isnoneornil(L, 4) ? nullptr : luaL_checkstring(L, 4);
	if (stepstr != nullptr && !vertex::getConstant(stepstr, step))
		return luax_enumerror(L, "vertex attribute step", vertex::getConstants(step), stepstr);

	const char *attachname = luaL_optstring(L, 5, name);

	luax_catchexcept(L, [&](){ t->attachAttribute(name, mesh, attachname, step); });
	return 0;
}

int w_Mesh_detachAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *name = luaL_checkstring(L, 2);
	bool success = false;
	luax_catchexcept(L, [&](){ success = t->detachAttribute(name); });
	luax_pushboolean(L, success);
	return 1;
}

int w_Mesh_flush(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	t->flush();
	return 0;
}

int w_Mesh_setVertexMap(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	if (lua_isnoneornil(L, 2))
	{
		// Disable the vertex map / index buffer.
		luax_catchexcept(L, [&](){ t->setVertexMap(); });
		return 0;
	}

	if (luax_istype(L, 2, Data::type))
	{
		Data *d = luax_totype<Data>(L, 2, Data::type);

		const char *indextypestr = luaL_checkstring(L, 3);
		IndexDataType indextype;
		if (!vertex::getConstant(indextypestr, indextype))
			return luax_enumerror(L, "index data type", vertex::getConstants(indextype), indextypestr);

		size_t datatypesize = vertex::getIndexDataSize(indextype);

		int indexcount = (int) luaL_optinteger(L, 4, d->getSize() / datatypesize);

		if (indexcount < 1 || indexcount * datatypesize > d->getSize())
			return luaL_error(L, "Invalid index count: %d", indexcount);

		luax_catchexcept(L, [&]() { t->setVertexMap(indextype, d->getData(), indexcount * datatypesize); });
		return 0;
	}

	bool is_table = lua_istable(L, 2);
	int nargs = is_table ? (int) luax_objlen(L, 2) : lua_gettop(L) - 1;

	std::vector<uint32> vertexmap;
	vertexmap.reserve(nargs);

	if (is_table)
	{
		for (int i = 0; i < nargs; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			vertexmap.push_back(uint32(luaL_checkinteger(L, -1) - 1));
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < nargs; i++)
			vertexmap.push_back(uint32(luaL_checkinteger(L, i + 2) - 1));
	}

	luax_catchexcept(L, [&](){ t->setVertexMap(vertexmap); });
	return 0;
}

int w_Mesh_getVertexMap(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	std::vector<uint32> vertex_map;
	bool has_vertex_map = false;
	luax_catchexcept(L, [&](){ has_vertex_map = t->getVertexMap(vertex_map); });

	if (!has_vertex_map)
	{
		lua_pushnil(L);
		return 1;
	}

	int element_count = (int) vertex_map.size();

	lua_createtable(L, element_count, 0);

	for (int i = 0; i < element_count; i++)
	{
		lua_pushinteger(L, lua_Integer(vertex_map[i]) + 1);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int w_Mesh_setTexture(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	if (lua_isnoneornil(L, 2))
		t->setTexture();
	else
	{
		Texture *tex = luax_checktexture(L, 2);
		luax_catchexcept(L, [&](){ t->setTexture(tex); });
	}

	return 0;
}

int w_Mesh_getTexture(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	Texture *tex = t->getTexture();

	if (tex == nullptr)
		return 0;

	// FIXME: big hack right here.
	if (dynamic_cast<Image *>(tex) != nullptr)
		luax_pushtype(L, Image::type, tex);
	else if (dynamic_cast<Canvas *>(tex) != nullptr)
		luax_pushtype(L, Canvas::type, tex);
	else
		return luaL_error(L, "Unable to determine texture type.");

	return 1;
}

int w_Mesh_setDrawMode(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *str = luaL_checkstring(L, 2);
	PrimitiveType mode;

	if (!vertex::getConstant(str, mode))
		return luax_enumerror(L, "mesh draw mode", vertex::getConstants(mode), str);

	t->setDrawMode(mode);
	return 0;
}

int w_Mesh_getDrawMode(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	PrimitiveType mode = t->getDrawMode();
	const char *str;

	if (!vertex::getConstant(mode, str))
		return luaL_error(L, "Unknown mesh draw mode.");

	lua_pushstring(L, str);
	return 1;
}

int w_Mesh_setDrawRange(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	if (lua_isnoneornil(L, 2))
		t->setDrawRange();
	else
	{
		int start = (int) luaL_checkinteger(L, 2) - 1;
		int count = (int) luaL_checkinteger(L, 3);
		luax_catchexcept(L, [&](){ t->setDrawRange(start, count); });
	}

	return 0;
}

int w_Mesh_getDrawRange(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	int start = 0;
	int count = 1;
	if (!t->getDrawRange(start, count))
		return 0;

	lua_pushinteger(L, start + 1);
	lua_pushinteger(L, count);
	return 2;
}

static const luaL_Reg w_Mesh_functions[] =
{
	{ "setVertices", w_Mesh_setVertices },
	{ "setVertex", w_Mesh_setVertex },
	{ "getVertex", w_Mesh_getVertex },
	{ "setVertexAttribute", w_Mesh_setVertexAttribute },
	{ "getVertexAttribute", w_Mesh_getVertexAttribute },
	{ "getVertexCount", w_Mesh_getVertexCount },
	{ "getVertexFormat", w_Mesh_getVertexFormat },
	{ "setAttributeEnabled", w_Mesh_setAttributeEnabled },
	{ "isAttributeEnabled", w_Mesh_isAttributeEnabled },
	{ "attachAttribute", w_Mesh_attachAttribute },
	{ "detachAttribute", w_Mesh_detachAttribute },
	{ "flush", w_Mesh_flush },
	{ "setVertexMap", w_Mesh_setVertexMap },
	{ "getVertexMap", w_Mesh_getVertexMap },
	{ "setTexture", w_Mesh_setTexture },
	{ "getTexture", w_Mesh_getTexture },
	{ "setDrawMode", w_Mesh_setDrawMode },
	{ "getDrawMode", w_Mesh_getDrawMode },
	{ "setDrawRange", w_Mesh_setDrawRange },
	{ "getDrawRange", w_Mesh_getDrawRange },
	{ 0, 0 }
};

extern "C" int luaopen_mesh(lua_State *L)
{
	return luax_register_type(L, &Mesh::type, w_Mesh_functions, nullptr);
}

} // graphics
} // love
