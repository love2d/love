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

#ifndef LOVE_RUNTIME_H
#define LOVE_RUNTIME_H

// LOVE
#include "types.h"

// Lua
extern "C" {
	#define LUA_COMPAT_ALL
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

// C++
#include <exception>

namespace love
{

// Forward declarations.
class Object;
class Module;
class Reference;

/**
 * Registries represent special tables which can be accessed with
 * luax_insistregistry and luax_getregistry.
 **/
enum Registry
{
	REGISTRY_MODULES,
	REGISTRY_OBJECTS
};

/**
 * This structure wraps all Lua-exposed objects. It exists in the
 * Lua state as a full userdata (so we can catch __gc "events"),
 * though the Object it refers to is light userdata in the sense
 * that it is not allocated by the Lua VM.
 **/
struct Proxy
{
	// Holds type information (see types.h).
	Type type;

	// Pointer to the actual object.
	Object *object;
};

/**
 * A Module with Lua wrapper functions and other data.
 **/
struct WrappedModule
{
	// The module containing the functions.
	Module *module;

	// The name for the table to put the functions in, without the 'love'-prefix.
	const char *name;

	// The type of this module.
	love::Type type;

	// The functions of the module (last element {0,0}).
	const luaL_Reg *functions;

	// A list of functions which expose the types of the modules (last element 0).
	const lua_CFunction *types;
};

/**
 * Returns a reference to the top stack element (-1) if the value
 * is of the specified type. If the value is incorrect, zero is returned.
 *
 * In any case, the top stack element is popped, regardless of its type.
 **/
Reference *luax_refif(lua_State *L, int type);

/**
 * Prints the current contents of the stack. Only useful for debugging.
 * @param L The Lua state.
 **/
void luax_printstack(lua_State *L);

/**
 * Converts the value at idx to a bool. It follow the same rules
 * as lua_toboolean, but returns a bool instead of an int.
 * @param L The Lua state.
 * @param idx The index on the Lua stack.
 * @return True if the value evaluates to true, false otherwise.
 **/
bool luax_toboolean(lua_State *L, int idx);

/**
 * Pushes a bool onto the stack. It's the same as lua_pushboolean,
 * but with bool instead of int.
 * @param L The Lua state.
 * @param b The bool to push.
 **/
void luax_pushboolean(lua_State *L, bool b);

/**
 * Converts the value at idx to a bool, or if not present, b is returned.
 * @param L The Lua state.
 * @param idx The index of the Lua stack.
 * @param b The value to return if no value exist at the specified index.
 * @return True if the value evaluates to true, false otherwise.
 **/
bool luax_optboolean(lua_State *L, int idx, bool b);

/**
 * Converts the value at idx to a std::string. It takes care of the string
 * size and possible embedded nulls.
 * @param L The Lua state.
 * @param idx The index on the Lua stack.
 * @return Copy of the string at the specified index.
 **/
std::string luax_tostring(lua_State *L, int idx);

/**
 * Converts the value at idx to a std::string. It takes care of the string
 * size and possible embedded nulls.
 * @param L The Lua state.
 * @param idx The index on the Lua stack.
 * @return Copy of the string at the specified index.
 **/
std::string luax_checkstring(lua_State *L, int idx);

/**
 * Pushes a std::string onto the stack. It uses the length of the string
 * for lua_pushlstring's len argument.
 * @param L The Lua state.
 * @param str The string to push.
 **/
void luax_pushstring(lua_State *L, const std::string &str);


bool luax_boolflag(lua_State *L, int table_index, const char *key, bool defaultValue);
int luax_intflag(lua_State *L, int table_index, const char *key, int defaultValue);

/**
 * Convert the value at the specified index to an Lua number, and then
 * convert to a float.
 *
 * @param L The Lua state.
 * @param idx The index on the stack.
 */
inline float luax_tofloat(lua_State *L, int idx)
{
	return static_cast<float>(lua_tonumber(L, idx));
}

/**
 * Like luax_tofloat, but checks that the value is a number.
 *
 * @see luax_tofloat
 */
inline float luax_checkfloat(lua_State *L, int idx)
{
	return static_cast<float>(luaL_checknumber(L, idx));
}

/**
 * Require at least 'min' number of items on the stack.
 * @param L The Lua state.
 * @param min The minimum number of items on the stack.
 * @return Zero if conditions are met, otherwise a Lua error (longjmp).
 **/
int luax_assert_argc(lua_State *L, int min);

/**
 * Require at least 'min', but more than 'max' items on the stack.
 * @param L The Lua state.
 * @param min The minimum number of items on the stack.
 * @param max The maximum number of items on the stack.
 * @return Zero if conditions are met, otherwise a Lua error (longjmp).
 **/
int luax_assert_argc(lua_State *L, int min, int max);

/**
 * Require that the value at idx is a function.
 * @param L The Lua state.
 *@param idx The index on the stack.
 **/
int luax_assert_function(lua_State *L, int idx);

/**
 * Require that the value at idx is not nil. If it is, the function throws an
 * error using an optional error string at idx+1.
 * @param L The Lua state.
 * @param idx The index on the stack.
 **/
int luax_assert_nilerror(lua_State *L, int idx);

/**
 * Registers all functions in the array l (see luaL_Reg) into the table at the
 * top of the stack.
 * Similar to Lua 5.2's luaL_setfuncs without the upvalues, and to Lua 5.1's
 * luaL_register without the library name.
 **/
void luax_setfuncs(lua_State *L, const luaL_Reg *l);

/**
 * Loads a Lua module using the 'require' function. Leaves the return result on
 * the stack.
 * @param name The name of the module to require.
 **/
int luax_require(lua_State *L, const char *name);

/**
 * Register a module in the love table. The love table will be created if it does not exist.
 * NOTE: The module-object is expected to have a +1 reference count before calling
 * this function, as it doesn't retain the object itself but Lua will release it
 * upon garbage collection.
 * @param L The Lua state.
 **/
int luax_register_module(lua_State *L, const WrappedModule &m);

/**
 * Inserts a module with 'name' into the package.preloaded table.
 * @param f The function to be called when the module is opened.
 * @param name The name of the module, with 'love'-prefix, for instance 'love.graphics'.
 **/
int luax_preload(lua_State *L, lua_CFunction f, const char *name);

/**
 * Register a new type.
 * @param type The type.
 * @param name The type's human-readable name
 * @param ... The list of lists of member functions for the type. (of type luaL_Reg*)
 **/
int luax_register_type(lua_State *L, love::Type type, const char *name, ...);

/**
 * Pushes the metatable of the specified type onto the stack.
**/
void luax_gettypemetatable(lua_State *L, love::Type type);

/**
 * Do a table.insert from C
 * @param L the state
 * @param tindex the stack index of the table
 * @param vindex the stack index of the value
 * @param pos the position to insert it in
 **/
int luax_table_insert(lua_State *L, int tindex, int vindex, int pos = -1);

/**
 * Register a new searcher function for package.loaders. This can for instance enable
 * loading of files through love.filesystem using standard require.
 * @param L The Lua state.
 * @param f The searcher function.
 * @param pos The position to insert the loader in.
 **/
int luax_register_searcher(lua_State *L, lua_CFunction f, int pos = -1);

/**
 * Pushes a Lua representation of the given object onto the stack, creating and
 * storing the Lua representation in a weak table if it doesn't exist yet.
 * NOTE: The object will be retained by Lua and released upon garbage collection.
 * @param L The Lua state.
 * @param type The type information of the object.
 * @param object The pointer to the actual object.
 **/
void luax_pushtype(lua_State *L, const love::Type type, love::Object *object);

/**
 * Creates a new Lua representation of the given object *without* checking if it
 * exists yet, and *without* storing it in a weak table.
 * This should only be used when performance is an extreme concern and the
 * object is not ever expected to be pushed to Lua again, as it prevents the
 * Lua-side objects from working in some cases when used as keys in tables.
 * NOTE: The object will be retained by Lua and released upon garbage collection.
 * @param L The Lua state.
 * @param type The type information of the object.
 * @param object The pointer to the actual object.
 **/
void luax_rawnewtype(lua_State *L, love::Type type, love::Object *object);

/**
 * Checks whether the value at idx is a certain type.
 * @param L The Lua state.
 * @param idx The index on the stack.
 * @param type The type to check for.
 * @return True if the value is Proxy of the specified type, false otherwise.
 **/
bool luax_istype(lua_State *L, int idx, love::Type type);

/**
 * Gets the function love.module.function and puts it on top of the stack (alone). If the
 * love table, the module, or the function does not exist, an error is returned.
 * @return An error if nonexistent, or 1 if successful.
 **/
int luax_getfunction(lua_State *L, const char *module, const char *function);

/**
 * Converts an object into another object by the specified function love.module.function.
 * The conversion function must accept a single object of the relevant type as a parameter,
 * and returnone value. If the function does not exist (see luax_getfunction), an error is returned.
 *
 * Note that the initial object at idx is replaced by the new object.
 *
 * @param L The Lua state.
 * @param idx The index on the stack.
 * @param module The module in the love table.
 * @param function The function in the module.
 **/
int luax_convobj(lua_State *L, int idx, const char *module, const char *function);

/**
 * Converts an object into another object by the specified function love.module.function.
 * The conversion function must accept a single object of the relevant type as its first parameter,
 * and return one value. If the function does not exist (see luax_getfunction), an error is returned.
 *
 * Note that the initial object at idx is replaced by the new object.
 *
 * @param L The Lua state.
 * @param idxs An array of indices on the stack.
 * @param n How many arguments are being passed.
 * @param module The module in the love table.
 * @param function The function in the module.
 **/
int luax_convobj(lua_State *L, int idxs[], int n, const char *module, const char *function);

// pcall versions of the above
int luax_pconvobj(lua_State *L, int idx, const char *module, const char *function);
int luax_pconvobj(lua_State *L, int idxs[], int n, const char *module, const char *function);

/**
 * 'Insist' that a table 'k' exists in the table at idx. Insistence involves that the
 * table (k) is created if it does not exist in the table at idx. The table at idx must
 * pre-exist, however. Also note that if the a non-table value exists at the specified
 * location, it will be overwritten with a new table. The insisted table, and only the
 * insisted table, will be placed on top of the stack.
 *
 * @param idx The index on the stack containing a table.
 * @param k The name of the table we are insisting exist.
 **/
int luax_insist(lua_State *L, int idx, const char *k);

/**
 * Insist that a global table 'k' exists. See luax_insist.
 * @param k The name of the table we are insisting exist.
 **/
int luax_insistglobal(lua_State *L, const char *k);

/**
 * Insists that a table 'k' exists inside the 'love' table. See luax_insist.
 * @param k The name of the table we are insisting exist.
 **/
int luax_insistlove(lua_State *L, const char *k);

/**
 * Pushes the table 'k' in the love table onto the stack. Pushes nil if the
 * table doesn't exist.
 * @param k The name of the table we want to get.
 **/
int luax_getlove(lua_State *L, const char *k);

/**
 * Gets (creates if needed) the specified Registry, and pushes it into the
 * stack.
 * @param L The Lua state.
 * @param r The Registry to get.
 **/
int luax_insistregistry(lua_State *L, Registry r);

/**
 * Gets the specified Registry, and pushes it onto the stack. Pushes nil if the
 * registry hasn't been created (see luax_insistregistry.)
 * @param L The Lua state.
 * @param r The Registry to get.
 **/
int luax_getregistry(lua_State *L, Registry r);

/**
 * Gets (and pins if needed) a "pinned" Lua thread (coroutine) in the specified
 * Lua state. This will usually be the main Lua thread, unless the first call
 * to this function for a specific Lua state is made from within a coroutine.
 * NOTE: This does not push anything to the stack.
 **/
lua_State *luax_insistpinnedthread(lua_State *L);

/**
 * Gets a "pinned" Lua thread (coroutine) in the specified Lua state. This will
 * usually be the main Lua thread. This can be used to access global variables
 * in a specific Lua state without needing another alive lua_State value.
 * PRECONDITION: luax_insistpinnedthread must have been called on a lua_State
 * value corresponding to the Lua state which will be used with this function.
 * NOTE: This does not push anything to the stack.
 **/
lua_State *luax_getpinnedthread(lua_State *L);

extern "C" { // Also called from luasocket
	int luax_typerror(lua_State *L, int narg, const char *tname);
}

/**
 * Calls luax_objlen/lua_rawlen depending on version
 **/
size_t luax_objlen(lua_State *L, int ndx);

extern "C" { // Called by enet and luasocket
	void luax_register(lua_State *L, const char *name, const luaL_Reg *l);
	int luax_c_insistglobal(lua_State *L, const char *k);
}

/**
 * Like luax_totype, but causes an error if the value at idx is not Proxy,
 * or is not the specified type.
 * @param L The Lua state.
 * @param idx The index on the stack.
 * @param type The type bit.
 **/
template <typename T>
T *luax_checktype(lua_State *L, int idx, love::Type type)
{
	if (lua_type(L, idx) != LUA_TUSERDATA)
	{
		const char *name = "Invalid";
		getTypeName(type, name);
		luax_typerror(L, idx, name);
	}

	Proxy *u = (Proxy *)lua_touserdata(L, idx);

	if (!typeFlags[u->type][type])
	{
		const char *name = "Invalid";
		getTypeName(type, name);
		luax_typerror(L, idx, name);
	}

	return (T *)u->object;
}

template <typename T>
T *luax_getmodule(lua_State *L, love::Type type)
{
	const char *name = "Invalid";
	getTypeName(type, name);

	luax_insistregistry(L, REGISTRY_MODULES);
	lua_getfield(L, -1, name);

	if (!lua_isuserdata(L, -1))
		luaL_error(L, "Tried to get nonexistant module %s.", name);

	Proxy *u = (Proxy *)lua_touserdata(L, -1);

	if (!typeFlags[u->type][type])
		luaL_error(L, "Incorrect module %s", name);

	lua_pop(L, 2);

	return (T *)u->object;
}

template <typename T>
T *luax_optmodule(lua_State *L, love::Type type)
{
	const char *name = "Invalid";
	getTypeName(type, name);

	luax_insistregistry(L, REGISTRY_MODULES);
	lua_getfield(L, -1, name);

	if (!lua_isuserdata(L, -1))
	{
		lua_pop(L, 2);
		return 0;
	}

	Proxy *u = (Proxy *)lua_touserdata(L, -1);

	if (!typeFlags[u->type][type])
		luaL_error(L, "Incorrect module %s", name);

	lua_pop(L, 2);

	return (T *) u->object;
}

/**
 * Converts the value at idx to the specified type without checking that
 * this conversion is valid. If the type has been previously verified with
 * luax_istype, then this can be safely used. Otherwise, use luax_checktype.
 * @param L The Lua state.
 * @param idx The index on the stack.
 * @param type The type of the object.
 **/
template <typename T>
T *luax_totype(lua_State *L, int idx, love::Type /*type*/)
{
	return (T *)(((Proxy *)lua_touserdata(L, idx))->object);
}

Type luax_type(lua_State *L, int idx);

/**
 * Converts any exceptions thrown by the passed lambda function into a Lua error.
 * lua_error (and luaL_error) cannot be called from inside the exception handler
 * because they use longjmp, which causes undefined behaviour when the
 * destructor of the exception would have been called.
 **/
template <typename T>
int luax_catchexcept(lua_State *L, const T& func)
{
	bool should_error = false;

	try
	{
		func();
	}
	catch (const std::exception &e)
	{
		should_error = true;
		lua_pushstring(L, e.what());
	}

	if (should_error)
		return luaL_error(L, "%s", lua_tostring(L, -1));

	return 0;
}

template <typename T, typename F>
int luax_catchexcept(lua_State *L, const T& func, const F& finallyfunc)
{
	bool should_error = false;

	try
	{
		func();
	}
	catch (const std::exception &e)
	{
		should_error = true;
		lua_pushstring(L, e.what());
	}

	finallyfunc(should_error);

	if (should_error)
		return luaL_error(L, "%s", lua_tostring(L, -1));

	return 0;
}

} // love

#endif // LOVE_RUNTIME_H
