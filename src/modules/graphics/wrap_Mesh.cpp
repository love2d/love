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

// LOVE
#include "wrap_Mesh.h"
#include "wrap_Buffer.h"
#include "Texture.h"
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

	if (vertstart >= totalverts || vertstart < 0)
		return luaL_error(L, "Invalid vertex start index (must be between 1 and %d)", totalverts);

	if (luax_istype(L, 2, Data::type))
	{
		Data *d = luax_checktype<Data>(L, 2);

		vertcount = vertcount >= 0 ? vertcount : (totalverts - vertstart);
		if (vertstart + vertcount > totalverts)
			return luaL_error(L, "Too many vertices (expected at most %d, got %d)", totalverts - vertstart, vertcount);

		size_t datasize = std::min(d->getSize(), vertcount * stride);
		char *bytedata = (char *) t->getVertexData() + byteoffset;

		memcpy(bytedata, d->getData(), datasize);

		t->setVertexDataModified(byteoffset, datasize);
		t->flush();

		return 0;
	}

	luaL_checktype(L, 2, LUA_TTABLE);
	int tablelen = (int) luax_objlen(L, 2);

	vertcount = vertcount >= 0 ? std::min(vertcount, tablelen) : tablelen;
	if (vertstart + vertcount > totalverts)
		return luaL_error(L, "Too many vertices (expected at most %d, got %d)", totalverts - vertstart, vertcount);

	const std::vector<Buffer::DataMember> &vertexformat = t->getVertexFormat();

	int ncomponents = 0;
	for (const Buffer::DataMember &member : vertexformat)
		ncomponents += member.info.components;

	char *data = (char *) t->getVertexData() + byteoffset;

	for (int i = 0; i < vertcount; i++)
	{
		// get vertices[vertindex]
		lua_rawgeti(L, 2, i + 1);
		luaL_checktype(L, -1, LUA_TTABLE);

		// get vertices[vertindex][j]
		for (int j = 1; j <= ncomponents; j++)
			lua_rawgeti(L, -j, j);

		int idx = -ncomponents;

		for (const Buffer::DataMember &member : vertexformat)
		{
			// Fetch the values from Lua and store them in data buffer.
			luax_writebufferdata(L, idx, member.decl.format, data + member.offset);
			idx += member.info.components;
		}

		lua_pop(L, ncomponents + 1);

		data += stride;
	}

	t->setVertexDataModified(byteoffset, vertcount * stride);
	t->flush();

	return 0;
}

int w_Mesh_setVertex(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t index = (size_t) luaL_checkinteger(L, 2) - 1;

	bool istable = lua_istable(L, 3);

	const std::vector<Buffer::DataMember> &vertexformat = t->getVertexFormat();

	char *data = nullptr;
	size_t offset = 0;
	luax_catchexcept(L, [&](){ data = (char *) t->checkVertexDataOffset(index, &offset); });

	int idx = istable ? 1 : 3;

	if (istable)
	{
		for (const Buffer::DataMember &member : vertexformat)
		{
			int components = member.info.components;

			for (int i = idx; i < idx + components; i++)
				lua_rawgeti(L, 3, i);

			// Fetch the values from Lua and store them in data buffer.
			luax_writebufferdata(L, -components, member.decl.format, data + member.offset);

			idx += components;
			lua_pop(L, components);
		}
	}
	else
	{
		for (const Buffer::DataMember &member : vertexformat)
		{
			// Fetch the values from Lua and store them in data buffer.
			luax_writebufferdata(L, idx, member.decl.format, data + member.offset);
			idx += member.info.components;
		}
	}

	t->setVertexDataModified(offset, t->getVertexStride());
	return 0;
}

int w_Mesh_getVertex(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t index = (size_t) luaL_checkinteger(L, 2) - 1;

	const std::vector<Buffer::DataMember> &vertexformat = t->getVertexFormat();

	const char *data = nullptr;
	luax_catchexcept(L, [&](){ data = (const char *) t->checkVertexDataOffset(index, nullptr); });

	int n = 0;

	for (const Buffer::DataMember &member : vertexformat)
	{
		luax_readbufferdata(L, member.decl.format, data + member.offset);
		n += member.info.components;
	}

	return n;
}

int w_Mesh_setVertexAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t vertindex = (size_t) luaL_checkinteger(L, 2) - 1;
	int attribindex = (int) luaL_checkinteger(L, 3) - 1;

	const auto &vertexformat = t->getVertexFormat();

	if (attribindex < 0 || attribindex >= (int) vertexformat.size())
		return luaL_error(L, "Invalid vertex attribute index: %d", attribindex + 1);

	const Buffer::DataMember &member = vertexformat[attribindex];

	char *data = nullptr;
	size_t offset = 0;
	luax_catchexcept(L, [&](){ data = (char *) t->checkVertexDataOffset(vertindex, &offset); });

	// Fetch the values from Lua and store them in the data buffer.
	luax_writebufferdata(L, 4, member.decl.format, data + member.offset);

	t->setVertexDataModified(offset + member.offset, member.size);
	return 0;
}

int w_Mesh_getVertexAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	size_t vertindex = (size_t) luaL_checkinteger(L, 2) - 1;
	int attribindex = (int) luaL_checkinteger(L, 3) - 1;

	const auto &vertexformat = t->getVertexFormat();

	if (attribindex < 0 || attribindex >= (int) vertexformat.size())
		return luaL_error(L, "Invalid vertex attribute index: %d", attribindex + 1);

	const Buffer::DataMember &member = vertexformat[attribindex];

	const char *data = nullptr;
	luax_catchexcept(L, [&](){ data = (const char *) t->checkVertexDataOffset(vertindex, nullptr); });

	luax_readbufferdata(L, member.decl.format, data + member.offset);
	return member.info.components;
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

	const std::vector<Buffer::DataMember> &vertexformat = t->getVertexFormat();
	lua_createtable(L, (int) vertexformat.size(), 0);

	const char *tname = nullptr;

	for (size_t i = 0; i < vertexformat.size(); i++)
	{
		const auto &member = vertexformat[i];

		if (!getConstant(member.decl.format, tname))
			return luax_enumerror(L, "vertex attribute data type", getConstants(member.decl.format), tname);

		lua_createtable(L, 0, 4);

		lua_pushstring(L, member.decl.name.c_str());
		lua_setfield(L, -2, "name");

		lua_pushnumber(L, member.decl.bindingLocation);
		lua_setfield(L, -2, "location");

		const char *formatstr = "unknown";
		getConstant(member.decl.format, formatstr);
		lua_pushstring(L, formatstr);
		lua_setfield(L, -2, "format");

		lua_pushinteger(L, member.decl.arrayLength);
		lua_setfield(L, -2, "arraylength");

		lua_pushinteger(L, member.offset);
		lua_setfield(L, -2, "offset");

		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int w_Mesh_setAttributeEnabled(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	bool enable = luax_checkboolean(L, 3);
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		const char *name = luaL_checkstring(L, 2);
		luax_catchexcept(L, [&](){ t->setAttributeEnabled(name, enable); });
	}
	else
	{
		int location = luaL_checkint(L, 2);
		luax_catchexcept(L, [&](){ t->setAttributeEnabled(location, enable); });
	}
	return 0;
}

int w_Mesh_isAttributeEnabled(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	bool enabled = false;
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		const char *name = luaL_checkstring(L, 2);
		luax_catchexcept(L, [&](){ enabled = t->isAttributeEnabled(name); });
	}
	else
	{
		int location = luaL_checkint(L, 2);
		luax_catchexcept(L, [&](){ enabled = t->isAttributeEnabled(location); });
	}
	lua_pushboolean(L, enabled);
	return 1;
}

int w_Mesh_attachAttribute(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);

	const char *name = nullptr;
	int location = -1;
	if (lua_type(L, 2) == LUA_TSTRING)
		name = luaL_checkstring(L, 2);
	else
		location = luaL_checkint(L, 2);

	Buffer *buffer = nullptr;
	Mesh *mesh = nullptr;
	if (luax_istype(L, 3, Buffer::type))
	{
		buffer = luax_checktype<Buffer>(L, 3);
	}
	else
	{
		mesh = luax_checkmesh(L, 3);
		buffer = mesh->getVertexBuffer();
		if (buffer == nullptr)
			return luaL_error(L, "Mesh does not have its own vertex buffer.");
	}

	AttributeStep step = STEP_PER_VERTEX;
	const char *stepstr = lua_isnoneornil(L, 4) ? nullptr : luaL_checkstring(L, 4);
	if (stepstr != nullptr && !getConstant(stepstr, step))
		return luax_enumerror(L, "vertex attribute step", getConstants(step), stepstr);

	const char *attachname = name;
	int attachlocation = location;
	if (name != nullptr)
		attachname = luaL_optstring(L, 5, name);
	else
		attachlocation = luaL_optint(L, 5, location);

	int startindex = (int) luaL_optinteger(L, 6, 1) - 1;

	if (name != nullptr)
		luax_catchexcept(L, [&](){ t->attachAttribute(name, buffer, mesh, attachname, startindex, step); });
	else
		luax_catchexcept(L, [&]() { t->attachAttribute(location, buffer, mesh, attachlocation, startindex, step); });
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

int w_Mesh_getAttachedAttributes(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const auto &attributes = t->getAttachedAttributes();

	lua_createtable(L, (int) attributes.size(), 0);

	for (int i = 0; i < (int) attributes.size(); i++)
	{
		const auto &attrib = attributes[i];

		lua_createtable(L, 0, 7);

		luax_pushstring(L, attrib.name);
		lua_setfield(L, -2, "name");

		lua_pushnumber(L, attrib.bindingLocation);
		lua_setfield(L, -2, "location");

		luax_pushtype(L, attrib.buffer.get());
		lua_setfield(L, -2, "buffer");

		const char *stepstr = nullptr;
		if (!getConstant(attrib.step, stepstr))
			return luaL_error(L, "Invalid vertex attribute step.");
		lua_pushstring(L, stepstr);
		lua_setfield(L, -2, "step");

		const Buffer::DataMember &member = attrib.buffer->getDataMember(attrib.indexInBuffer);
		luax_pushstring(L, member.decl.name);
		lua_setfield(L, -2, "nameinbuffer");

		lua_pushnumber(L, member.decl.bindingLocation);
		lua_setfield(L, -2, "locationinbuffer");

		lua_pushinteger(L, attrib.startArrayIndex + 1);
		lua_setfield(L, -2, "startindex");

		lua_rawseti(L, -1, i + 1);
	}

	return 1;
}

int w_Mesh_getVertexBuffer(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	luax_pushtype(L, t->getVertexBuffer());
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
		if (!getConstant(indextypestr, indextype))
			return luax_enumerror(L, "index data type", getConstants(indextype), indextypestr);

		size_t datatypesize = getIndexDataSize(indextype);

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

int w_Mesh_setIndexBuffer(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	Buffer *b = nullptr;
	if (!lua_isnoneornil(L, 2))
		b = luax_checkbuffer(L, 2);
	luax_catchexcept(L, [&]() { t->setIndexBuffer(b); });
	return 0;
}

int w_Mesh_getIndexBuffer(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	luax_pushtype(L, t->getIndexBuffer());
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

	luax_pushtype(L, tex);
	return 1;
}

int w_Mesh_setDrawMode(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	const char *str = luaL_checkstring(L, 2);
	PrimitiveType mode;

	if (!getConstant(str, mode))
		return luax_enumerror(L, "mesh draw mode", getConstants(mode), str);

	t->setDrawMode(mode);
	return 0;
}

int w_Mesh_getDrawMode(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	PrimitiveType mode = t->getDrawMode();
	const char *str;

	if (!getConstant(mode, str))
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
	{ "getAttachedAttributes", w_Mesh_getAttachedAttributes },
	{ "getVertexBuffer", w_Mesh_getVertexBuffer },
	{ "flush", w_Mesh_flush },
	{ "setVertexMap", w_Mesh_setVertexMap },
	{ "getVertexMap", w_Mesh_getVertexMap },
	{ "setIndexBuffer", w_Mesh_setIndexBuffer },
	{ "getIndexBuffer", w_Mesh_getIndexBuffer },
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
