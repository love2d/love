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
#include "wrap_Mesh.h"
#include "Image.h"
#include "Canvas.h"
#include "graphics/wrap_Texture.h"

// C++
#include <typeinfo>
#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

Mesh *luax_checkmesh(lua_State *L, int idx)
{
	return luax_checktype<Mesh>(L, idx, GRAPHICS_MESH_ID);
}

static inline size_t writeByteData(lua_State *L, int startidx, int components, char *data)
{
	uint8 *componentdata = (uint8 *) data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (uint8) luaL_optnumber(L, startidx + i, 255);

	return sizeof(uint8) * components;
}

static inline size_t writeFloatData(lua_State *L, int startidx, int components, char *data)
{
	float *componentdata = (float *) data;

	for (int i = 0; i < components; i++)
		componentdata[i] = (float) luaL_optnumber(L, startidx + i, 0);

	return sizeof(float) * components;
}

char *luax_writeAttributeData(lua_State *L, int startidx, Mesh::DataType type, int components, char *data)
{
	switch (type)
	{
	case Mesh::DATA_BYTE:
		return data + writeByteData(L, startidx, components, data);
	case Mesh::DATA_FLOAT:
		return data + writeFloatData(L, startidx, components, data);
	default:
		return data;
	}
}

static inline size_t readByteData(lua_State *L, int components, const char *data)
{
	const uint8 *componentdata = (const uint8 *) data;

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, (lua_Number) componentdata[i]);

	return sizeof(uint8) * components;
}

static inline size_t readFloatData(lua_State *L, int components, const char *data)
{
	const float *componentdata = (const float *) data;

	for (int i = 0; i < components; i++)
		lua_pushnumber(L, componentdata[i]);

	return sizeof(float) * components;
}

const char *luax_readAttributeData(lua_State *L, Mesh::DataType type, int components, const char *data)
{
	switch (type)
	{
	case Mesh::DATA_BYTE:
		return data + readByteData(L, components, data);
	case Mesh::DATA_FLOAT:
		return data + readFloatData(L, components, data);
	default:
		return data;
	}
}

int w_Mesh_setVertices(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t vertoffset = (size_t) luaL_optnumber(L, 3, 1) - 1;

	if (vertoffset >= t->getVertexCount())
		return luaL_error(L, "Invalid vertex start index (must be between 1 and %d)", (int) t->getVertexCount());

	size_t stride = t->getVertexStride();
	size_t byteoffset = vertoffset * stride;

	if (luax_istype(L, 2, DATA_ID))
	{
		Data *d = luax_checktype<Data>(L, 2, DATA_ID);

		size_t datasize = std::min(d->getSize(), (t->getVertexCount() - vertoffset) * stride);
		char *bytedata = (char *) t->mapVertexData() + byteoffset;

		memcpy(bytedata, d->getData(), datasize);

		t->unmapVertexData(byteoffset, datasize);
		return 0;
	}

	luaL_checktype(L, 2, LUA_TTABLE);
	size_t nvertices = luax_objlen(L, 2);

	if (vertoffset + nvertices > t->getVertexCount())
		return luaL_error(L, "Too many vertices (expected at most %d, got %d)", (int) t->getVertexCount() - (int) vertoffset, (int) nvertices);

	const std::vector<Mesh::AttribFormat> &vertexformat = t->getVertexFormat();

	int ncomponents = 0;
	for (const Mesh::AttribFormat &format : vertexformat)
		ncomponents += format.components;

	char *data = (char *) t->mapVertexData() + byteoffset;

	for (size_t i = 0; i < nvertices; i++)
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

	t->unmapVertexData(byteoffset, nvertices * stride);
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

	Mesh::DataType type;
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

	Mesh::DataType type;
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
		if (!Mesh::getConstant(vertexformat[i].type, tname))
			return luaL_error(L, "Unknown vertex attribute data type.");

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
	bool enable = luax_toboolean(L, 3);
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
	luax_catchexcept(L, [&](){ t->attachAttribute(name, mesh); });
	return 0;
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
		t->setTexture(tex);
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
	if (typeid(*tex) == typeid(Image))
		luax_pushtype(L, GRAPHICS_IMAGE_ID, tex);
	else if (typeid(*tex) == typeid(Canvas))
		luax_pushtype(L, GRAPHICS_CANVAS_ID, tex);
	else
		return luaL_error(L, "Unable to determine texture type.");

	return 1;
}

int w_Mesh_setDrawMode(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *str = luaL_checkstring(L, 2);
	Mesh::DrawMode mode;

	if (!Mesh::getConstant(str, mode))
		return luaL_error(L, "Invalid mesh draw mode: %s", str);

	t->setDrawMode(mode);
	return 0;
}

int w_Mesh_getDrawMode(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	Mesh::DrawMode mode = t->getDrawMode();
	const char *str;

	if (!Mesh::getConstant(mode, str))
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
		int rangemin = (int) luaL_checknumber(L, 2) - 1;
		int rangemax = (int) luaL_checknumber(L, 3) - 1;
		luax_catchexcept(L, [&](){ t->setDrawRange(rangemin, rangemax); });
	}

	return 0;
}

int w_Mesh_getDrawRange(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	int rangemin = -1;
	int rangemax = -1;
	t->getDrawRange(rangemin, rangemax);

	if (rangemin < 0 || rangemax < 0)
		return 0;

	lua_pushinteger(L, rangemin + 1);
	lua_pushinteger(L, rangemax + 1);
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
	return luax_register_type(L, GRAPHICS_MESH_ID, "Mesh", w_Mesh_functions, nullptr);
}

} // opengl
} // graphics
} // love
