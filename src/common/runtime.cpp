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

#include "runtime.h"

// LOVE
#include "Module.h"
#include "Object.h"
#include "Reference.h"

// STD
#include <iostream>

namespace love
{

	/**
	* Called when an object is collected. The object is released
	* once in this function, possibly deleting it.
	**/
	static int w__gc(lua_State * L)
	{
		Proxy * p = (Proxy *)lua_touserdata(L, 1);
		Object * t = (Object *)p->data;
		if(p->own)
			t->release();
		return 0;
	}

	static int w__tostring(lua_State * L)
	{
		lua_pushvalue(L, lua_upvalueindex(1));
		return 1;
	}

	Reference * luax_refif(lua_State * L, int type)
	{
		Reference * r = 0;

		// Create a reference only if the test succeeds.
		if(lua_type(L, -1) == type)
			r = new Reference(L); 
		else // Pop the value even if it fails (but also if it succeeds).
			lua_pop(L, 1);

		return r;
	}

	void luax_printstack(lua_State * L)
	{
		for(int i = 1;i<=lua_gettop(L);i++)
		{
			std::cout << i << " - " << luaL_typename(L, i) << std::endl;
		}
	}

	bool luax_toboolean(lua_State * L, int idx)
	{
		return (lua_toboolean(L, idx) == 1 ? true : false);
	}

	void luax_pushboolean(lua_State * L, bool b)
	{
		lua_pushboolean(L, b ? 1 : 0);
	}

	bool luax_optboolean(lua_State * L, int idx, bool b)
	{
		if(lua_isboolean(L, idx) == 1)
			return (lua_toboolean(L, idx) == 1 ? true : false);
		return b;
	}

	int luax_assert_argc(lua_State * L, int min)
	{
		int argc = lua_gettop(L);
		if( argc < min )
			return luaL_error(L, "Incorrect number of arguments. Got [%d], expected at least [%d]", argc, min);
		return 0;
	}

	int luax_assert_argc(lua_State * L, int min, int max)
	{
		int argc = lua_gettop(L);
		if( argc < min || argc > max)
			return luaL_error(L, "Incorrect number of arguments. Got [%d], expected [%d-%d]", argc, min, max);
		return 0;
	}

	int luax_assert_function(lua_State * L, int n)
	{
		if(!lua_isfunction(L, n))
			return luaL_error(L, "Argument must be of type \"function\".");
		return 0;
	}

	int luax_register_module(lua_State * L, const WrappedModule & m)
	{
		// Put a reference to the C++ module in Lua.
		luax_getregistry(L, REGISTRY_MODULES);

		Proxy * p = (Proxy *)lua_newuserdata(L, sizeof(Proxy));
		p->own = true;
		p->data = m.module;
		p->flags = m.flags;

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
		luaL_register(L, 0, m.functions);

		// Register types.
		if(m.types != 0)
			for(const lua_CFunction * t = m.types; *t != 0; t++)
				(*t)(L);

		lua_setfield(L, -2, m.name); // love.graphics = table
		lua_pop(L, 1); // love

		return 0;
	}

	int luax_preload(lua_State * L, lua_CFunction f, const char * name)
	{
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "preload");
		lua_pushcfunction(L, f);
		lua_setfield(L, -2, name);
		lua_pop(L, 2);
		return 0;
	}

	int luax_register_type(lua_State * L, const char * tname, const luaL_Reg * f)
	{
		luaL_newmetatable(L, tname);

		// m.__index = m
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		// setup gc
		lua_pushcfunction(L, w__gc);
		lua_setfield(L, -2, "__gc");

		// Add tostring function.
		lua_pushstring(L, tname);
		lua_pushcclosure(L, w__tostring, 1);
		lua_setfield(L, -2, "__tostring");

		if(f != 0)
			luaL_register(L, 0, f);

		lua_pop(L, 1); // Pops metatable.
		return 0;
	}

	int luax_register_searcher(lua_State * L, lua_CFunction f)
	{
		// Add the package loader to the package.loaders table.
		lua_getglobal(L, "package");

		if(lua_isnil(L, -1))
			return luaL_error(L, "Can't register searcher: package table does not exist.");
		
		lua_getfield(L, -1, "loaders");
		
		if(lua_isnil(L, -1))
			return luaL_error(L, "Can't register searcher: package.loaders table does not exist.");
		
		int len = lua_objlen(L, -1);
		lua_pushinteger(L, len+1);
		lua_pushcfunction(L, f);
		lua_settable(L, -3);
		lua_pop(L, 2);
		return 0;
	}

	void luax_newtype(lua_State * L, const char * name, bits flags, void * data, bool own)
	{
		Proxy * u = (Proxy *)lua_newuserdata(L, sizeof(Proxy));

		u->data = data;
		u->flags = flags;
		u->own = own;

		luaL_newmetatable(L, name);
		lua_setmetatable(L, -2);
	}

	bool luax_istype(lua_State * L, int idx, love::bits type)
	{
		if(lua_isuserdata(L, idx) == 0)
			return false;

		return ((((Proxy *)lua_touserdata(L, idx))->flags & type) == type);
	}

	int luax_getfunction(lua_State * L, const char * mod, const char * fn)
	{
		lua_getglobal(L, "love");
		if(lua_isnil(L, -1)) return luaL_error(L, "Could not find global love!");
		lua_getfield(L, -1, mod);
		if(lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s!", mod);
		lua_getfield(L, -1, fn);
		if(lua_isnil(L, -1)) return luaL_error(L, "Could not find love.%s.%s!", mod, fn);
		
		lua_remove(L, -2); // remove mod
		lua_remove(L, -2); // remove fn
		return 0;
	}

	int luax_convobj(lua_State * L, int idx, const char * mod, const char * fn)
	{
		// Convert string to a file.
		luax_getfunction(L, mod, fn);
		lua_pushvalue(L, idx); // The initial argument.
		lua_call(L, 1, 1); // Call the function, one arg, one return value.
		lua_replace(L, idx); // Replace the initial argument with the new object.
		return 0;
	}

	int luax_strtofile(lua_State * L, int idx)
	{
		return luax_convobj(L, idx, "filesystem", "newFile");
	}

	int luax_filetodata(lua_State * L, int idx)
	{
		return luax_convobj(L, idx, "filesystem", "read");
	}

	int luax_insist(lua_State * L, int idx, const char * k)
	{
		lua_getfield(L, idx, k);

		// Create if necessary.
		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1); // Pop the non-table.
			lua_newtable(L);
			lua_pushvalue(L, -1); // Duplicate the table to leave on top.
			lua_setfield(L, -3, k); // k[idx] = table
		}

		return 1;
	}

	int luax_insistglobal(lua_State * L, const char * k)
	{
		lua_getglobal(L, k);

		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1); // Pop the non-table.
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setglobal(L, k);
		}

		return 1;
	}

	int luax_insistlove(lua_State * L, const char * k)
	{
		luax_insistglobal(L, "love");
		luax_insist(L, -1, k);
		
		// The love table should be replaced with the top stack
		// item. Only the reqested table should remain on the stack.
		lua_replace(L, -2);

		return 1;
	}

	int luax_getregistry(lua_State * L, Registry r)
	{
		switch(r)
		{
		case REGISTRY_GC:
			return luax_insistlove(L, "_gc");
		case REGISTRY_MODULES:
			return luax_insistlove(L, "_modules");
		default:
			return luaL_error(L, "Attempted to use invalid registry.");
		}
	}


} // love
