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

#include "types.h"
#include "Module.h"

// Lua
extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

namespace love
{
	class Module;

	void luax_printstack(lua_State * L);
	bool luax_toboolean(lua_State * L, int idx);
	void luax_pushboolean(lua_State * L, bool b);
	bool luax_optboolean(lua_State * L, int idx, bool b);
	int luax_assert_argc(lua_State * L, int lower);
	int luax_assert_argc(lua_State * L, int lower, int upper);
	int luax_assert_function(lua_State * L, int n);

	int luax_register_gc(lua_State * L, const char * mname, Module * module);

	int luax_register_info(lua_State * L, const char * name, 
		const char * provides, const char * desc, const char * author,
		lua_CFunction open);

	int luax_register_module(lua_State * L, const luaL_Reg * fn, const lua_CFunction * types);
	int luax_preload(lua_State * L, lua_CFunction f, const char * name);
	int luax_register_type(lua_State * L, const char * tname, const luaL_Reg * fn);

	int luax_register_searcher(lua_State * L, lua_CFunction f);
	

	void luax_newtype(lua_State * L, const char * tname, bits flags, void * data, bool own = true);

	bool luax_istype(lua_State * L, int idx, love::bits type);
	
	// Gets a function love.mod.fn.
	int luax_getfunction(lua_State * L, const char * mod, const char * fn);

	/**
	* Converts an object into another object
	* by the specified function mod.fn. The function must accept
	* a single file as a parameter, and return one value.
	* 
	* Note that the initial object is converted, i.e. replaced.
	**/
	int luax_convobj(lua_State * L, int idx, const char * mod, const char * fn);

	/**
	* Converts a string into a File object. Note that the 
	* string is replaced by the new File object at the
	* specified index, and NOT pushed onto the stack.
	**/
	int luax_strtofile(lua_State * L, int idx);

	template <typename T>
	T * luax_checktype(lua_State * L, int idx, const char * tname, love::bits type)
	{
		if(lua_isuserdata(L, idx) == 0)
			luaL_error(L, "Incorrect parameter type: expected userdata.");

		userdata * u = (userdata *)lua_touserdata(L, idx);

		if((u->flags & type) != type)
			luaL_error(L, "Incorrect parameter type: expected %s", tname);

		return (T *)u->data;	
	}

} // love

#endif // LOVE_RUNTIME_H
