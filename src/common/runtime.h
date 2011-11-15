/**
* Copyright (c) 2006-2011 LOVE Development Team
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
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

namespace love
{
	// Forward declarations.
	class Module;
	class Reference;

	// Exposed mutex of the GC
	extern void *_gcmutex;
	extern unsigned int _gcthread;

	/**
	* Registries represent special tables which can be accessed with
	* luax_getregistry.
	**/
	enum Registry
	{
		REGISTRY_GC = 1,
		REGISTRY_MODULES,
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
		bits flags;

		// The light userdata.
		void * data;

		// True if Lua should delete on GC.
		bool own;
	};

	/**
	* A Module with Lua wrapper functions and other data.
	**/
	struct WrappedModule
	{
		// The module containing the functions.
		Module * module;

		// The name for the table to put the functions in, without the 'love'-prefix.
		const char * name;

		// The type flags of this module.
		love::bits flags;

		// The functions of the module (last element {0,0}).
		const luaL_Reg * functions;

		// A list of functions which expose the types of the modules (last element 0).
		const lua_CFunction * types;

	};

	/**
	* Returns a reference to the top stack element (-1) if the value
	* is of the specified type. If the value is incorrect, zero is returned.
	*
	* In any case, the top stack element is popped, regardless of its type.
	**/
	Reference * luax_refif(lua_State * L, int type);

	/**
	* Prints the current contents of the stack. Only useful for debugging.
	* @param L The Lua state.
	**/
	void luax_printstack(lua_State * L);

	/**
	* Converts the value at idx to a bool. It follow the same rules
	* as lua_toboolean, but returns a bool instead of an int.
	* @param L The Lua state.
	* @param idx The index on the Lua stack.
	* @return True if the value evaluates to true, false otherwise.
	**/
	bool luax_toboolean(lua_State * L, int idx);

	/**
	* Pushes a bool onto the stack. It's the same as lua_pushboolean,
	* but with bool instead of int.
	* @param L The Lua state.
	* @param b The bool to push.
	**/
	void luax_pushboolean(lua_State * L, bool b);

	/**
	* Converts the value at idx to a bool, or if not present, b is returned.
	* @param L The Lua state.
	* @param idx The index of the Lua stack.
	* @param b The value to return if no value exist at the specified index.
	* @return True if the value evaluates to true, false otherwise.
	**/
	bool luax_optboolean(lua_State * L, int idx, bool b);

	/**
	* Converts the value at idx to a std::string. It takes care of the string
	* size and possible embedded nulls.
	* @param L The Lua state.
	* @param idx The index on the Lua stack.
	* @return Copy of the string at the specified index.
	**/
	std::string luax_checkstring(lua_State * L, int idx);

	/**
	* Pushes a std::string onto the stack. It uses the length of the string
	* for lua_pushlstring's len argument.
	* @param L The Lua state.
	* @param str The string to push.
	**/
	void luax_pushstring(lua_State * L, std::string str);
	
	/**
	* Require at least 'min' number of items on the stack.
	* @param L The Lua state.
	* @param min The minimum number of items on the stack.
	* @return Zero if conditions are met, otherwise a Lua error (longjmp).
	**/
	int luax_assert_argc(lua_State * L, int min);

	/**
	* Require at least 'min', but more than 'max' items on the stack.
	* @param L The Lua state.
	* @param min The minimum number of items on the stack.
	* @param max The maximum number of items on the stack.
	* @return Zero if conditions are met, otherwise a Lua error (longjmp).
	**/
	int luax_assert_argc(lua_State * L, int min, int max);

	/**
	* Require that the value at idx is a function.
	* @param L The Lua state.
	 *@param idx The index on the stack.
	**/
	int luax_assert_function(lua_State * L, int idx);

	/**
	* Register a module in the love table. The love table will be created if it does not exist.
	* @param L The Lua state.
	**/
	int luax_register_module(lua_State * L, const WrappedModule & m);

	/**
	* Inserts a module with 'name' into the package.preloaded table.
	* @param f The function to be called when the module is opened.
	* @param name The name of the module, with 'love'-prefix, for instance 'love.graphics'.
	**/
	int luax_preload(lua_State * L, lua_CFunction f, const char * name);

	/**
	* Register a new type.
	* @param tname The name of the type. This must not conflict with other type names,
	* even from other modules.
	* @param f The list of member functions for the type.
	**/
	int luax_register_type(lua_State * L, const char * tname, const luaL_Reg * f = 0);

	/**
	 * Do a table.insert from C
	 * @param L the state
	 * @param tindex the stack index of the table
	 * @param vindex the stack index of the value
	 * @param pos the position to insert it in
	 **/
	int luax_table_insert(lua_State * L, int tindex, int vindex, int pos = -1);

	/**
	* Register a new searcher function for package.loaders. This can for instance enable
	* loading of files through love.filesystem using standard require.
	* @param L The Lua state.
	* @param f The searcher function.
	* @param pos The position to insert the loader in.
	**/
	int luax_register_searcher(lua_State * L, lua_CFunction f, int pos = -1);

	/**
	* Creates a new Lua-accessible object of the given type, and put it on the stack.
	* @param L The Lua state.
	* @param name The name of the type. This must match the used earlier with luax_register_type.
	* @param flags The type information.
	* @param data The pointer to the actual object.
	* @own Set this to true (default) if the object should be released upon garbage collection.
	**/
	void luax_newtype(lua_State * L, const char * name, bits flags, void * data, bool own = true);

	/**
	* Checks whether the value at idx is a certain type.
	* @param L The Lua state.
	* @param idx The index on the stack.
	* @param type The type to check for.
	* @return True if the value is Proxy of the specified type, false otherwise.
	**/
	bool luax_istype(lua_State * L, int idx, love::bits type);

	/**
	* Gets the function love.module.function and puts it on top of the stack (alone). If the
	* love table, the module, or the function does not exist, an error is returned.
	* @return An error if nonexistent, or 1 if successful.
	**/
	int luax_getfunction(lua_State * L, const char * module, const char * function);

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
	int luax_convobj(lua_State * L, int idx, const char * module, const char * function);

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
	int luax_convobj(lua_State * L, int idxs[], int n, const char * module, const char * function);

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
	int luax_insist(lua_State * L, int idx, const char * k);

	/**
	* Insist that a global table 'k' exists. See luax_insist.
	* @param k The name of the table we are insisting exist.
	**/
	int luax_insistglobal(lua_State * L, const char * k);

	/**
	* Insists that a table 'k' exists inside the 'love' table. See luax_insist.
	* @param k The name of the table we are insisting exist.
	**/
	int luax_insistlove(lua_State * L, const char * k);

	/**
	* Gets (creates if needed) the specified Registry, and puts it on top
	* of the stack.
	* @param L The Lua state.
	* @param r The Registry to get.
	**/
	int luax_getregistry(lua_State * L, Registry r);

	Type luax_type(lua_State * L, int idx);

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
	* Converts the value at idx to the specified type without checking that
	* this conversion is valid. If the type has been previously verified with
	* luax_istype, then this can be safely used. Otherwise, use luax_checktype.
	* @param L The Lua state.
	* @param idx The index on the stack.
	* @param name The name of the type.
	* @param type The type bit.
	**/
	template <typename T>
	T * luax_totype(lua_State * L, int idx, const char *, love::bits)
	{
		return (T*)(((Proxy *)lua_touserdata(L, idx))->data);
	}

	/**
	* Like luax_totype, but causes an error if the value at idx is not Proxy,
	* or is not the specified type.
	* @param L The Lua state.
	* @param idx The index on the stack.
	* @param name The name of the type.
	* @param type The type bit.
	**/
	template <typename T>
	T * luax_checktype(lua_State * L, int idx, const char * name, love::bits type)
	{
		if(lua_isuserdata(L, idx) == 0)
			luaL_error(L, "Incorrect parameter type: expected userdata.");

		Proxy * u = (Proxy *)lua_touserdata(L, idx);

		if((u->flags & type) != type)
			luaL_error(L, "Incorrect parameter type: expected %s", name);

		return (T *)u->data;
	}

	template <typename T>
	T * luax_getmodule(lua_State * L, const char * k, love::bits type)
	{
		luax_getregistry(L, REGISTRY_MODULES);
		lua_getfield(L, -1, k);

		if(!lua_isuserdata(L, -1))
			luaL_error(L, "Tried to get nonexisting module %s.", k);

		Proxy * u = (Proxy *)lua_touserdata(L, -1);

		if((u->flags & type) != type)
			luaL_error(L, "Incorrect module %s", k);

		lua_pop(L, 2);

		return (T*)u->data;
	}

} // love

#endif // LOVE_RUNTIME_H
