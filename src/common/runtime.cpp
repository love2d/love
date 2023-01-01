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

#include "config.h"
#include "runtime.h"

// LOVE
#include "Module.h"
#include "Object.h"
#include "Reference.h"
#include "StringMap.h"

// C++
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstddef>
#include <cmath>
#include <sstream>

// VS2013 doesn't support alignof
#if defined(_MSC_VER) && _MSC_VER <= 1800
#define LOVE_ALIGNOF(x) __alignof(x)
#else
#define LOVE_ALIGNOF(x) alignof(x)
#endif

namespace love
{

/**
 * Called when an object is collected. The object is released
 * once in this function, possibly deleting it.
 **/
static int w__gc(lua_State *L)
{
	Proxy *p = (Proxy *) lua_touserdata(L, 1);
	if (p->object != nullptr)
	{
		p->object->release();
		p->object = nullptr;
	}
	return 0;
}

static int w__tostring(lua_State *L)
{
	Proxy *p = (Proxy *) lua_touserdata(L, 1);
	const char *typname = lua_tostring(L, lua_upvalueindex(1));
	lua_pushfstring(L, "%s: %p", typname, p->object);
	return 1;
}

static int w__type(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));
	return 1;
}

static int w__typeOf(lua_State *L)
{
	Proxy *p = (Proxy *)lua_touserdata(L, 1);
	Type *t = luax_type(L, 2);
	if (!t)
		luax_pushboolean(L, false);
	else
		luax_pushboolean(L, p->type->isa(*t));
	return 1;
}

static int w__eq(lua_State *L)
{
	Proxy *p1 = (Proxy *)lua_touserdata(L, 1);
	Proxy *p2 = (Proxy *)lua_touserdata(L, 2);
	luax_pushboolean(L, p1->object == p2->object && p1->object != nullptr);
	return 1;
}

typedef uint64 ObjectKey;

static bool luax_isfulllightuserdatasupported(lua_State *L)
{
	// LuaJIT prior to commit e9af1abec542e6f9851ff2368e7f196b6382a44c doesn't
	// support lightuserdata > 48 bits. This is not a problem with Android,
	// Windows, macOS, and iOS as they'll use updated LuaJIT or won't use
	// pointers > 48 bits, but this is not the case for Linux. So check for
	// this capability first!
	static bool checked = false;
	static bool supported = false;

	if (!checked)
	{
		lua_pushcclosure(L, [](lua_State *L) -> int
		{
			// Try to push pointer with all bits set.
			lua_pushlightuserdata(L, (void *) (~((size_t) 0)));
			return 1;
		}, 0);

		supported = lua_pcall(L, 0, 1, 0) == 0;
		checked = true;

		lua_pop(L, 1);
	}

	return supported;
}

// For use with the love object pointer -> Proxy pointer registry.
// Using the pointer directly via lightuserdata would be ideal, but LuaJIT
// (before a commit to 2.1 in 2020) cannot use lightuserdata with more than 47
// bits whereas some newer arm64 architectures allow pointers which use more
// than that.
static ObjectKey luax_computeloveobjectkey(lua_State *L, love::Object *object)
{
	// love objects should be allocated on the heap, and thus are subject
	// to the alignment rules of operator new / malloc. Lua numbers (doubles)
	// can store all possible integers up to 2^53. We can store pointers that
	// use more than 53 bits if their alignment is guaranteed to be more than 1.
	// For example an alignment requirement of 8 means we can shift the
	// pointer's bits by 3.
	const size_t minalign = LOVE_ALIGNOF(std::max_align_t);
	uintptr_t key = (uintptr_t) object;

	if ((key & (minalign - 1)) != 0)
	{
		luaL_error(L, "Cannot push love object to Lua: unexpected alignment "
				   "(pointer is %p but alignment should be %d)", object, minalign);
	}

	static const size_t shift = (size_t) log2(LOVE_ALIGNOF(std::max_align_t));

	key >>= shift;

	return (ObjectKey) key;
}

static void luax_pushloveobjectkey(lua_State *L, ObjectKey key)
{
	// If full 64-bit lightuserdata is supported, always use that. Otherwise,
	// if the key is smaller than 2^53 (which is integer precision for double
	// datatype), then push number. Otherwise, throw error.
	if (luax_isfulllightuserdatasupported(L))
		lua_pushlightuserdata(L, (void *) key);
	else if (key > 0x20000000000000ULL) // 2^53
		luaL_error(L, "Cannot push love object to Lua: pointer value %p is too large", key);
	else
		lua_pushnumber(L, (lua_Number) key);
}

static int w__release(lua_State *L)
{
	Proxy *p = (Proxy *) lua_touserdata(L, 1);
	Object *object = p->object;

	if (object != nullptr)
	{
		p->object = nullptr;
		object->release();

		// Fetch the registry table of instantiated objects.
		luax_getregistry(L, REGISTRY_OBJECTS);

		if (lua_istable(L, -1))
		{
			// loveobjects[object] = nil
			ObjectKey objectkey = luax_computeloveobjectkey(L, object);
			luax_pushloveobjectkey(L, objectkey);
			lua_pushnil(L);
			lua_settable(L, -3);
		}

		lua_pop(L, 1);
	}

	luax_pushboolean(L, object != nullptr);
	return 1;
}

Reference *luax_refif(lua_State *L, int type)
{
	Reference *r = nullptr;

	// Create a reference only if the test succeeds.
	if (lua_type(L, -1) == type)
		r = new Reference(L);
	else // Pop the value manually if it fails (done by Reference if it succeeds).
		lua_pop(L, 1);

	return r;
}

void luax_printstack(lua_State *L)
{
	for (int i = 1; i <= lua_gettop(L); i++)
		std::cout << i << " - " << luaL_typename(L, i) << std::endl;
}

int luax_traceback(lua_State *L)
{
	if (!lua_isstring(L, 1))  // 'message' not a string?
		return 1; // keep it intact

	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}

	lua_pushvalue(L, 1); // pass error message
	lua_pushinteger(L, 2); // skip this function and traceback
	lua_call(L, 2, 1); // call debug.traceback
	return 1;
}

bool luax_isarrayoftables(lua_State *L, int idx)
{
	if (!lua_istable(L, idx))
		return false;

	lua_rawgeti(L, idx, 1);
	bool tableoftables = lua_istable(L, -1);
	lua_pop(L, 1);
	return tableoftables;
}

bool luax_toboolean(lua_State *L, int idx)
{
	return (lua_toboolean(L, idx) != 0);
}

bool luax_checkboolean(lua_State *L, int idx)
{
	luaL_checktype(L, idx, LUA_TBOOLEAN);
	return luax_toboolean(L, idx);
}

void luax_pushboolean(lua_State *L, bool b)
{
	lua_pushboolean(L, b ? 1 : 0);
}

bool luax_optboolean(lua_State *L, int idx, bool b)
{
	if (lua_isboolean(L, idx) == 1)
		return (lua_toboolean(L, idx) == 1 ? true : false);
	return b;
}

std::string luax_tostring(lua_State *L, int idx)
{
	size_t len;
	const char *str = lua_tolstring(L, idx, &len);
	return std::string(str, len);
}

std::string luax_checkstring(lua_State *L, int idx)
{
	size_t len;
	const char *str = luaL_checklstring(L, idx, &len);
	return std::string(str, len);
}

void luax_pushstring(lua_State *L, const std::string &str)
{
	lua_pushlstring(L, str.data(), str.size());
}

void luax_pushpointerasstring(lua_State *L, const void *pointer)
{
	char str[sizeof(void *)];
	memcpy(str, &pointer, sizeof(void *));
	lua_pushlstring(L, str, sizeof(void *));
}

bool luax_boolflag(lua_State *L, int table_index, const char *key, bool defaultValue)
{
	lua_getfield(L, table_index, key);

	bool retval;
	if (lua_isnoneornil(L, -1))
		retval = defaultValue;
	else
		retval = lua_toboolean(L, -1) != 0;

	lua_pop(L, 1);
	return retval;
}

int luax_intflag(lua_State *L, int table_index, const char *key, int defaultValue)
{
	lua_getfield(L, table_index, key);

	int retval;
	if (!lua_isnumber(L, -1))
		retval = defaultValue;
	else
		retval = (int) lua_tointeger(L, -1);

	lua_pop(L, 1);
	return retval;
}

double luax_numberflag(lua_State *L, int table_index, const char *key, double defaultValue)
{
	lua_getfield(L, table_index, key);

	double retval;
	if (!lua_isnumber(L, -1))
		retval = defaultValue;
	else
		retval = lua_tonumber(L, -1);

	lua_pop(L, 1);
	return retval;
}

int luax_checkintflag(lua_State *L, int table_index, const char *key)
{
	lua_getfield(L, table_index, key);

	int retval;
	if (!lua_isnumber(L, -1))
	{
		std::string err = "expected integer field " + std::string(key) + " in table";
		return luaL_argerror(L, table_index, err.c_str());
	}
	else
		retval = (int) luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	return retval;
}

int luax_assert_argc(lua_State *L, int min)
{
	int argc = lua_gettop(L);
	if (argc < min)
		return luaL_error(L, "Incorrect number of arguments. Got [%d], expected at least [%d]", argc, min);
	return 0;
}

int luax_assert_argc(lua_State *L, int min, int max)
{
	int argc = lua_gettop(L);
	if (argc < min || argc > max)
		return luaL_error(L, "Incorrect number of arguments. Got [%d], expected [%d-%d]", argc, min, max);
	return 0;
}

int luax_assert_function(lua_State *L, int idx)
{
	if (!lua_isfunction(L, idx))
		return luaL_error(L, "Argument must be of type \"function\".");
	return 0;
}

int luax_assert_nilerror(lua_State *L, int idx)
{
	if (lua_isnoneornil(L, idx))
	{
		if (lua_isstring(L, idx + 1))
			return luaL_error(L, lua_tostring(L, idx + 1));
		else
			return luaL_error(L, "assertion failed!");
	}
	return 0;
}

void luax_setfuncs(lua_State *L, const luaL_Reg *l)
{
	if (l == nullptr)
		return;

	for (; l->name != nullptr; l++)
	{
		lua_pushcfunction(L, l->func);
		lua_setfield(L, -2, l->name);
	}
}

int luax_require(lua_State *L, const char *name)
{
	lua_getglobal(L, "require");
	lua_pushstring(L, name);
	lua_call(L, 1, 1);
	return 1;
}

int luax_register_module(lua_State *L, const WrappedModule &m)
{
	m.type->init();

	// Put a reference to the C++ module in Lua.
	luax_insistregistry(L, REGISTRY_MODULES);

	Proxy *p = (Proxy *)lua_newuserdata(L, sizeof(Proxy));
	p->object = m.module;
	p->type = m.type;

	luaL_newmetatable(L, m.module->getName());
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, w__gc);
	lua_setfield(L, -2, "__gc");

	lua_setmetatable(L, -2);
	lua_setfield(L, -2, m.name); // _modules[name] = proxy
	lua_pop(L, 1);

	// Gets the love table.
	luax_insistglobal(L, "love");

	// Create new table for module.
	lua_newtable(L);

	// Register all the functions.
	if (m.functions != nullptr)
		luax_setfuncs(L, m.functions);

	// Register types.
	if (m.types != nullptr)
	{
		for (const lua_CFunction *t = m.types; *t != nullptr; t++)
			(*t)(L);
	}

	lua_pushvalue(L, -1);
	lua_setfield(L, -3, m.name); // love.graphics = table
	lua_remove(L, -2); // love

	// Register module instance
	Module::registerInstance(m.module);

	return 1;
}

int luax_preload(lua_State *L, lua_CFunction f, const char *name)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, f);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
	return 0;
}

int luax_register_type(lua_State *L, love::Type *type, ...)
{
	type->init();

	// Get the place for storing and re-using instantiated love types.
	luax_getregistry(L, REGISTRY_OBJECTS);

	// Create registry._loveobjects if it doesn't exist yet.
	if (!lua_istable(L, -1))
	{
		lua_newtable(L);
		lua_replace(L, -2);

		// Create a metatable.
		lua_newtable(L);

		// metatable.__mode = "v". Weak userdata values.
		lua_pushliteral(L, "v");
		lua_setfield(L, -2, "__mode");

		// setmetatable(newtable, metatable)
		lua_setmetatable(L, -2);

		// registry._loveobjects = newtable
		lua_setfield(L, LUA_REGISTRYINDEX, "_loveobjects");
	}
	else
		lua_pop(L, 1);

	luaL_newmetatable(L, type->getName());

	// m.__index = m
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// setup gc
	lua_pushcfunction(L, w__gc);
	lua_setfield(L, -2, "__gc");

	// Add equality
	lua_pushcfunction(L, w__eq);
	lua_setfield(L, -2, "__eq");

	// Add tostring function.
	lua_pushstring(L, type->getName());
	lua_pushcclosure(L, w__tostring, 1);
	lua_setfield(L, -2, "__tostring");

	// Add type
	lua_pushstring(L, type->getName());
	lua_pushcclosure(L, w__type, 1);
	lua_setfield(L, -2, "type");

	// Add typeOf
	lua_pushcfunction(L, w__typeOf);
	lua_setfield(L, -2, "typeOf");

	// Add release
	lua_pushcfunction(L, w__release);
	lua_setfield(L, -2, "release");

	va_list fs;
	va_start(fs, type);
	for (const luaL_Reg *f = va_arg(fs, const luaL_Reg *); f; f = va_arg(fs, const luaL_Reg *))
		luax_setfuncs(L, f);
	va_end(fs);

	lua_pop(L, 1); // Pops metatable.
	return 0;
}

void luax_gettypemetatable(lua_State *L, const love::Type &type)
{
	const char *name = type.getName();
	lua_getfield(L, LUA_REGISTRYINDEX, name);
}

int luax_table_insert(lua_State *L, int tindex, int vindex, int pos)
{
	if (tindex < 0)
		tindex = lua_gettop(L)+1+tindex;
	if (vindex < 0)
		vindex = lua_gettop(L)+1+vindex;

	if (pos == -1)
	{
		lua_pushvalue(L, vindex);
		lua_rawseti(L, tindex, (int) luax_objlen(L, tindex)+1);
		return 0;
	}
	else if (pos < 0)
		pos = (int) luax_objlen(L, tindex)+1+pos;

	for (int i = (int) luax_objlen(L, tindex)+1; i > pos; i--)
	{
		lua_rawgeti(L, tindex, i-1);
		lua_rawseti(L, tindex, i);
	}

	lua_pushvalue(L, vindex);
	lua_rawseti(L, tindex, pos);
	return 0;
}

int luax_register_searcher(lua_State *L, lua_CFunction f, int pos)
{
	// Add the package loader to the package.loaders table.
	lua_getglobal(L, "package");

	if (lua_isnil(L, -1))
		return luaL_error(L, "Can't register searcher: package table does not exist.");

	lua_getfield(L, -1, "loaders");

	// Lua 5.2 renamed package.loaders to package.searchers.
	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		lua_getfield(L, -1, "searchers");
	}

	if (lua_isnil(L, -1))
		return luaL_error(L, "Can't register searcher: package.loaders table does not exist.");

	lua_pushcfunction(L, f);
	luax_table_insert(L, -2, -1, pos);
	lua_pop(L, 3);
	return 0;
}

void luax_rawnewtype(lua_State *L, love::Type &type, love::Object *object)
{
	Proxy *u = (Proxy *)lua_newuserdata(L, sizeof(Proxy));

	object->retain();

	u->object = object;
	u->type = &type;

	const char *name = type.getName();
	luaL_newmetatable(L, name);

	lua_getfield(L, -1, "__gc");
	bool has_gc = !lua_isnoneornil(L, -1);
	lua_pop(L, 1);

	// Make sure mt.__gc exists, so Lua states which don't have the object's
	// module loaded will still clean the object up when it's collected.
	if (!has_gc)
	{
		lua_pushcfunction(L, w__gc);
		lua_setfield(L, -2, "__gc");
	}

	lua_setmetatable(L, -2);
}

void luax_pushtype(lua_State *L, love::Type &type, love::Object *object)
{
	if (object == nullptr)
	{
		lua_pushnil(L);
		return;
	}

	// Fetch the registry table of instantiated objects.
	luax_getregistry(L, REGISTRY_OBJECTS);

	// The table might not exist - it should be insisted in luax_register_type.
	if (lua_isnoneornil(L, -1))
	{
		lua_pop(L, 1);
		return luax_rawnewtype(L, type, object);
	}

	ObjectKey objectkey = luax_computeloveobjectkey(L, object);

	// Get the value of loveobjects[object] on the stack.
	luax_pushloveobjectkey(L, objectkey);
	lua_gettable(L, -2);

	// If the Proxy userdata isn't in the instantiated types table yet, add it.
	if (lua_type(L, -1) != LUA_TUSERDATA)
	{
		lua_pop(L, 1);

		luax_rawnewtype(L, type, object);

		luax_pushloveobjectkey(L, objectkey);
		lua_pushvalue(L, -2);

		// loveobjects[object] = Proxy.
		lua_settable(L, -4);
	}

	// Remove the loveobjects table from the stack.
	lua_remove(L, -2);

	// Keep the Proxy userdata on the stack.
}

bool luax_istype(lua_State *L, int idx, love::Type &type)
{
	if (lua_type(L, idx) != LUA_TUSERDATA)
		return false;

	Proxy *p = (Proxy *) lua_touserdata(L, idx);

	if (p->type != nullptr)
		return p->type->isa(type);
	else
		return false;
}

int luax_getfunction(lua_State *L, const char *mod, const char *fn)
{
	lua_getglobal(L, "love");
	if (lua_isnil(L, -1)) return luaL_error(L, "Could not find global love!");
	lua_getfield(L, -1, mod);
	if (lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s!", mod);
	lua_getfield(L, -1, fn);
	if (lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s.%s!", mod, fn);

	lua_remove(L, -2); // remove mod
	lua_remove(L, -2); // remove fn
	return 0;
}

int luax_convobj(lua_State *L, int idx, const char *mod, const char *fn)
{
	// Convert to absolute index if necessary.
	if (idx < 0 && idx > LUA_REGISTRYINDEX)
		idx += lua_gettop(L) + 1;

	// Convert string to a file.
	luax_getfunction(L, mod, fn);
	lua_pushvalue(L, idx); // The initial argument.
	lua_call(L, 1, 2); // Call the function, one arg, one return value (plus optional errstring.)
	luax_assert_nilerror(L, -2); // Make sure the function returned something.
	lua_pop(L, 1); // Pop the second return value now that we don't need it.
	lua_replace(L, idx); // Replace the initial argument with the new object.
	return 0;
}

int luax_convobj(lua_State *L, const int idxs[], int n, const char *mod, const char *fn)
{
	luax_getfunction(L, mod, fn);
	for (int i = 0; i < n; i++)
	{
		lua_pushvalue(L, idxs[i]); // The arguments.
	}
	lua_call(L, n, 2); // Call the function, n args, one return value (plus optional errstring.)
	luax_assert_nilerror(L, -2); // Make sure the function returned something.
	lua_pop(L, 1); // Pop the second return value now that we don't need it.
	if (n > 0)
		lua_replace(L, idxs[0]); // Replace the initial argument with the new object.
	return 0;
}

int luax_convobj(lua_State *L, const std::vector<int>& idxs, const char *module, const char *function)
{
	const int *idxPtr = idxs.size() > 0 ? &idxs[0] : nullptr;
	return luax_convobj(L, idxPtr, (int) idxs.size(), module, function);
}

int luax_pconvobj(lua_State *L, int idx, const char *mod, const char *fn)
{
	// Convert string to a file.
	luax_getfunction(L, mod, fn);
	lua_pushvalue(L, idx); // The initial argument.
	int ret = lua_pcall(L, 1, 1, 0); // Call the function, one arg, one return value.
	if (ret == 0)
		lua_replace(L, idx); // Replace the initial argument with the new object.
	return ret;
}

int luax_pconvobj(lua_State *L, const int idxs[], int n, const char *mod, const char *fn)
{
	luax_getfunction(L, mod, fn);
	for (int i = 0; i < n; i++)
	{
		lua_pushvalue(L, idxs[i]); // The arguments.
	}
	int ret = lua_pcall(L, n, 1, 0); // Call the function, n args, one return value.
	if (ret == 0)
		lua_replace(L, idxs[0]); // Replace the initial argument with the new object.
	return ret;
}

int luax_pconvobj(lua_State *L, const std::vector<int>& idxs, const char *module, const char *function)
{
	const int *idxPtr = idxs.size() > 0 ? &idxs[0] : nullptr;
	return luax_pconvobj(L, idxPtr, (int) idxs.size(), module, function);
}

int luax_insist(lua_State *L, int idx, const char *k)
{
	// Convert to absolute index if necessary.
	if (idx < 0 && idx > LUA_REGISTRYINDEX)
		idx += lua_gettop(L) + 1;

	lua_getfield(L, idx, k);

	// Create if necessary.
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1); // Pop the non-table.
		lua_newtable(L);
		lua_pushvalue(L, -1); // Duplicate the table to leave on top.
		lua_setfield(L, idx, k); // lua_stack[idx][k] = lua_stack[-1] (table)
	}

	return 1;
}

int luax_insistglobal(lua_State *L, const char *k)
{
	lua_getglobal(L, k);

	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1); // Pop the non-table.
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, k);
	}

	return 1;
}

int luax_c_insistglobal(lua_State *L, const char *k)
{
	return luax_insistglobal(L, k);
}

int luax_insistlove(lua_State *L, const char *k)
{
	luax_insistglobal(L, "love");
	luax_insist(L, -1, k);

	// The love table should be replaced with the top stack
	// item. Only the reqested table should remain on the stack.
	lua_replace(L, -2);

	return 1;
}

int luax_getlove(lua_State *L, const char *k)
{
	lua_getglobal(L, "love");

	if (!lua_isnil(L, -1))
	{
		lua_getfield(L, -1, k);
		lua_replace(L, -2);
	}

	return 1;
}

int luax_insistregistry(lua_State *L, Registry r)
{
	switch (r)
	{
	case REGISTRY_MODULES:
		return luax_insistlove(L, "_modules");
	case REGISTRY_OBJECTS:
		return luax_insist(L, LUA_REGISTRYINDEX, "_loveobjects");
	default:
		return luaL_error(L, "Attempted to use invalid registry.");
	}
}

int luax_getregistry(lua_State *L, Registry r)
{
	switch (r)
	{
	case REGISTRY_MODULES:
		return luax_getlove(L, "_modules");
	case REGISTRY_OBJECTS:
		lua_getfield(L, LUA_REGISTRYINDEX, "_loveobjects");
		return 1;
	default:
		return luaL_error(L, "Attempted to use invalid registry.");
	}
}

static const char *MAIN_THREAD_KEY = "_love_mainthread";

lua_State *luax_insistpinnedthread(lua_State *L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);

	if (lua_isnoneornil(L, -1))
	{
		lua_pop(L, 1);

		// lua_pushthread returns 1 if it's actually the main thread, but we
		// can't actually get the real main thread if lua_pushthread doesn't
		// return it (in Lua 5.1 at least), so we ignore that for now...
		// We do store a strong reference to the current thread/coroutine in
		// the registry, however.
		lua_pushthread(L);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);
	}

	lua_State *thread = lua_tothread(L, -1);
	lua_pop(L, 1);
	return thread;
}

lua_State *luax_getpinnedthread(lua_State *L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, MAIN_THREAD_KEY);
	lua_State *thread = lua_tothread(L, -1);
	lua_pop(L, 1);
	return thread;
}

void luax_markdeprecated(lua_State *L, const char *name, APIType api)
{
	luax_markdeprecated(L, name, api, DEPRECATED_NO_REPLACEMENT, nullptr);
}

void luax_markdeprecated(lua_State *L, const char *name, APIType api, DeprecationType type, const char *replacement)
{
	MarkDeprecated deprecated(name, api, type, replacement);

	if (deprecated.info != nullptr && deprecated.info->uses == 1)
	{
		luaL_where(L, 1);
		const char *where = lua_tostring(L, -1);
		if (where != nullptr)
			deprecated.info->where = where;
		lua_pop(L, 1);
	}
}

extern "C" int luax_typerror(lua_State *L, int narg, const char *tname)
{
	int argtype = lua_type(L, narg);
	const char *argtname = nullptr;

	// We want to use the love type name for userdata, if possible.
	if (argtype == LUA_TUSERDATA && luaL_getmetafield(L, narg, "type") != 0)
	{
		lua_pushvalue(L, narg);
		if (lua_pcall(L, 1, 1, 0) == 0 && lua_type(L, -1) == LUA_TSTRING)
		{
			argtname = lua_tostring(L, -1);

			// Non-love userdata might have a type metamethod which doesn't
			// describe its type properly, so we only use it for love types.
			if (!Type::byName(argtname))
				argtname = nullptr;
		}
	}

	if (argtname == nullptr)
		argtname = lua_typename(L, argtype);

	const char *msg = lua_pushfstring(L, "%s expected, got %s", tname, argtname);
	return luaL_argerror(L, narg, msg);
}

int luax_enumerror(lua_State *L, const char *enumName, const char *value)
{
	return luaL_error(L, "Invalid %s: %s", enumName, value);
}

int luax_enumerror(lua_State *L, const char *enumName, const std::vector<std::string> &values, const char *value)
{
	std::stringstream valueStream;
	bool first = true;
	for (auto value : values)
	{
		valueStream << (first ? "'" : ", '") << value << "'";
		first = false;
	}

	std::string valueString = valueStream.str();
	return luaL_error(L, "Invalid %s '%s', expected one of: %s", enumName, value, valueString.c_str());
}

size_t luax_objlen(lua_State *L, int ndx)
{
#if LUA_VERSION_NUM == 501
	return lua_objlen(L, ndx);
#else
	return lua_rawlen(L, ndx);
#endif
}

void luax_register(lua_State *L, const char *name, const luaL_Reg *l)
{
	if (name)
		lua_newtable(L);

	luax_setfuncs(L, l);
	if (name)
	{
		lua_pushvalue(L, -1);
		lua_setglobal(L, name);
	}
}

void luax_runwrapper(lua_State *L, const char *filedata, size_t datalen, const char *filename, const love::Type &type, void *ffifuncs)
{
	luax_gettypemetatable(L, type);

	// Load and execute the given Lua file, sending the metatable and the ffi
	// functions struct pointer as arguments.
	if (lua_istable(L, -1))
	{
		std::string chunkname = std::string("=[love \"") + std::string(filename) + std::string("\"]");

		luaL_loadbuffer(L, filedata, datalen, chunkname.c_str());
		lua_pushvalue(L, -2);
		if (ffifuncs != nullptr)
			luax_pushpointerasstring(L, ffifuncs);
		else
			lua_pushnil(L);
		lua_call(L, 2, 0);
	}

	// Pop the metatable.
	lua_pop(L, 1);
}

Type *luax_type(lua_State *L, int idx)
{
	return Type::byName(luaL_checkstring(L, idx));
}

int luax_resume(lua_State *L, int nargs, int* nres)
{
#if LUA_VERSION_NUM >= 504
	return lua_resume(L, nullptr, nargs, nres);
#elif LUA_VERSION_NUM >= 502
	LOVE_UNUSED(nres);
	return lua_resume(L, nullptr, nargs);
#else
	LOVE_UNUSED(nres);
	return lua_resume(L, nargs);
#endif
}

} // love
