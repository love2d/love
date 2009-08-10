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

// LOVE
#include "wrap_Native.h"

#include <filesystem/File.h>

#include <cstring>

namespace love
{
namespace native
{
namespace tcc
{
	static Native * instance = 0;

	int w_compile(lua_State * L)
	{
		int argn = lua_gettop(L);
		
		// We need argn strings.
		char ** strs = new char*[argn];

		for(int i = 1; i<=argn; i++)
		{
			// Convert to File, if necessary.
			if(lua_isstring(L, 1))
				luax_convobj(L, 1, "filesystem", "newFile");

			filesystem::File * file = luax_checktype<filesystem::File>(L, 1, "File", FILESYSTEM_FILE_T);
			
			int size = file->getSize();

			// Allocate some new data. Need an extra char for the
			// trailing 0. 
			char * str = new char[size+1];
			str[size] = 0;
			file->read((void*)str, size);

			// Put it in the list of strings.
			strs[i-1] = str;
		}

		bool r = instance->compile((const char **)strs, argn);

		luax_pushboolean(L, r);

		// Cleanup.
		for(int i = 0; i<argn; i++)
			delete strs[i];
		delete [] strs;

		return 1;
	}

	int w_getSymbol(lua_State * L)
	{
		const char * sym = luaL_checkstring(L, 1);
		void * ptr = instance->getSymbol(sym);
		lua_CFunction fn = (lua_CFunction)ptr;

		if(fn == 0)
			lua_pushnil(L);
		else
			lua_pushcfunction(L, fn);

		return 1;
	}

	/**
	* Searcher function for modules compiled with love.native.
	**/
	static int searcher(lua_State * L)
	{
		const char * src = lua_tostring(L, 1);

		int length = strlen(src);

		// luaopen_ needs 8 chars.
		char * dst = new char[length+8+1];
		memcpy(dst, "luaopen_", 8); 

		for(int i = 0; i<=length; i++)
			dst[i+8] = (src[i] == '.') ? '_' : src[i];

		void * sym = instance->getSymbol(dst);

		if(sym == 0)
			lua_pushfstring(L, "\tno symbol \"%s\" in love.native.\n", dst);
		else
			lua_pushcfunction(L, (lua_CFunction)sym);

		delete [] dst;

		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "compile", w_compile },
		{ "getSymbol", w_getSymbol },
		{ 0, 0 }
	};

	int luaopen_love_native(lua_State * L)
	{
		if(instance == 0)
		{
			try 
			{
				instance = new Native();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_searcher(L, searcher);

		luax_register_gc(L, instance);

		return luax_register_module(L, functions, 0, 0, "native");
	}

} // tcc
} // native
} // love
