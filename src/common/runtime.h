/**
* Copyright (c) 2006-2009 LOVE Development Team
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

	/**
	* Registries represent special tables which can be accessed with
	* luax_getregistry.
	**/
	enum Registry
	{
		REGISTRY_GC = 1,
	};

	/**
	* This structure wraps all Lua-exposed objects. It exists in the
	* Lua state as a full UserData (so we can catch __gc "events"), 
	* though the Object it refers to is light UserData in the sense 
	* that it is not allocated by the Lua VM. 
	**/
	struct UserData
	{
		bits flags; // Holds type information (see types.h).
		void * data;
		bool own; // True if Lua should delete on GC.
	};

	
	/**
	* Used to store constants in arrays.
	**/
	struct LuaConstant
	{
		const char * name;
		int value;
	};

	/**
	* Prints the current contents of the stack. Only useful for debugging.
	* @param L The Lua state.
	**/
	void luax_printstack(lua_State * L);

	/**
	* Converts the value at idx to a bool. It follow the same rules
	* as lua_toboolean, but returns a bool instead of an int.
	* @param L The Lua state.
	* @param idx The index on the Lua state.
	* @return True if the value evaluates to true, false otherwise.
	**/
	bool luax_toboolean(lua_State * L, int idx);

	/**
	* Pushes a bool onto the stack. It's the same as lua_pushboolean, 
	* but with bool instead of int.
	* @param L The Lua state.
	* @paarm b The bool to push.
	**/
	void luax_pushboolean(lua_State * L, bool b);

	/**
	* Converts the value at idx to a bool, or if not present, b is returned.
	* @param L The Lua state.
	* @param idx The index of the Lua state.
	* @param b The value to return if no value exist at the specified index.
	* @return True if the value evaluates to true, false otherwise.
	**/
	bool luax_optboolean(lua_State * L, int idx, bool b);

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
	* Register a dummy UserData (full) for a module. This enables us to catch the
	* call to __gc, and properly call the destructor on the Module in question.
	* @param L The Lua state.
	* @param module The Module to register the dummy UserData for.
	**/
	int luax_register_gc(lua_State * L, Module * module);

	/**
	* Register a module in the love table. The love table will created if it does not exist.
	* @param L The Lua state.
	* @param f The functions of the module (last element {0,0}).
	* @param t A list of functions which expose the types of the modules (last element 0).
	* @param c A list of constants (last element 0).
	* @param name The name for the table to put the functions in, without the 'love'-prefix.
	**/
	int luax_register_module(lua_State * L, const luaL_Reg * f, const lua_CFunction * t, const LuaConstant * c, const char * name);
	
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
	int luax_register_type(lua_State * L, const char * tname, const luaL_Reg * f);

	/**
	* Register a new searcher function for package.loaders. This can for instance enable
	* loading of files through love.filesystem using standard require. 
	* @param L The Lua state.
	* @param f The searcher function.
	**/
	int luax_register_searcher(lua_State * L, lua_CFunction f);

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
	* @return True if the value is UserData of the specified type, false otherwise.
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
	T * luax_totype(lua_State * L, int idx, const char * name, love::bits type)
	{
		return (T*)(((UserData *)lua_touserdata(L, idx))->data);
	}

	/**
	* Like luax_totype, but causes an error if the value at idx is not UserData, 
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

		UserData * u = (UserData *)lua_touserdata(L, idx);

		if((u->flags & type) != type)
			luaL_error(L, "Incorrect parameter type: expected %s", name);

		return (T *)u->data;	
	}

} // love

#endif // LOVE_RUNTIME_H
