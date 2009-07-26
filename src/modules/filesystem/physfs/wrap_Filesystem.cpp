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
#include "wrap_Filesystem.h"

namespace love
{
namespace filesystem
{
namespace physfs
{
	static Filesystem * instance = 0;

	bool hack_setupWriteDirectory()
	{
		if(instance != 0)
			return instance->setupWriteDirectory();
		return false;
	}

	int _wrap_setIdentity(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);

		if(!instance->setIdentity(arg))
			return luaL_error(L, "Could not set write directory.");

		return 0;
	}

	int _wrap_setSource(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);

		if(!instance->setSource(arg))
			return luaL_error(L, "Could not set source.");

		return 0;
	}

	int _wrap_newFile(lua_State * L)
	{
		const char * filename = luaL_checkstring(L, 1);
		File * t = instance->newFile(filename);
		luax_newtype(L, "File", LOVE_FILESYSTEM_FILE_BITS, (void*)t);
		return 1;
	}

	int _wrap_newFileData(lua_State * L)
	{
		if(!lua_isstring(L, 1))
			return luaL_error(L, "String expected.");
		if(!lua_isstring(L, 2))
			return luaL_error(L, "String expected.");

		size_t length = 0;
		const char * str = lua_tolstring(L, 1, &length);
		const char * filename = lua_tostring(L, 2);

		FileData * t = instance->newFileData((void*)str, (int)length, filename);

		luax_newtype(L, "FileData", LOVE_FILESYSTEM_FILE_DATA_BITS, (void*)t);
		return 1;
	}

	int _wrap_getWorkingDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getWorkingDirectory());
		return 1;
	}

	int _wrap_getUserDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getUserDirectory());
		return 1;
	}

	int _wrap_getAppdataDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getAppdataDirectory());
		return 1;
	}

	int _wrap_getSaveDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getSaveDirectory());
		return 1;
	}

	int _wrap_exists(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->exists(arg) ? 1 : 0);
		return 1;
	}

	int _wrap_isDirectory(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->isDirectory(arg) ? 1 : 0);
		return 1;
	}

	int _wrap_isFile(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->isFile(arg) ? 1 : 0);
		return 1;
	}

	int _wrap_mkdir(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->mkdir(arg) ? 1 : 0);
		return 1;
	}

	int _wrap_remove(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->remove(arg) ? 1 : 0);
		return 1;
	}

	int _wrap_read(lua_State * L)
	{
		return instance->read(L);
	}

	int _wrap_write(lua_State * L)
	{
		return instance->write(L);
	}	

	int _wrap_enumerate(lua_State * L)
	{
		return instance->enumerate(L);
	}

	int _wrap_lines(lua_State * L)
	{
		return instance->lines(L);
	}

	int _wrap_load(lua_State * L)
	{
		return instance->load(L);
	}

	int loader(lua_State * L)
	{
		const char * filename = lua_tostring(L, -1);

		std::string tmp(filename);

		int size = tmp.size();

		if(size <= 4 || strcmp(filename + (size-4), ".lua") != 0)
			tmp.append(".lua");

		for(int i=0;i<size-4;i++)
		{
			if(tmp[i] == '.')
			{
				tmp[i] = '/';
			}
		}

		// Check whether file exists.
		if(!instance->exists(tmp.c_str()))
		{
			lua_pushfstring(L, "\n\tno file \"%s\" in LOVE game directories.\n", tmp.c_str());
			return 1;
		}

		lua_pop(L, 1);
		lua_pushstring(L, tmp.c_str());

		// Ok, load it.
		return instance->load(L);
	}

	// List of functions to wrap.
	const luaL_Reg wrap_Filesystem_functions[] = {
		{ "setIdentity",  _wrap_setIdentity },
		{ "setSource",  _wrap_setSource },
		{ "newFile",  _wrap_newFile },
		{ "getWorkingDirectory",  _wrap_getWorkingDirectory },
		{ "getUserDirectory",  _wrap_getUserDirectory },
		{ "getAppdataDirectory",  _wrap_getAppdataDirectory },
		{ "getSaveDirectory",  _wrap_getSaveDirectory },
		{ "exists",  _wrap_exists },
		{ "isDirectory",  _wrap_isDirectory },
		{ "isFile",  _wrap_isFile },
		{ "mkdir",  _wrap_mkdir },
		{ "remove",  _wrap_remove },
		{ "read",  _wrap_read },
		{ "write",  _wrap_write },
		{ "enumerate",  _wrap_enumerate },
		{ "lines",  _wrap_lines },
		{ "load",  _wrap_load },
		{ 0, 0 }
	};

	const lua_CFunction wrap_Filesystem_types[] = {
		wrap_File_open,
		wrap_FileData_open,
		0
	};

	int wrap_Filesystem_open(lua_State * L)
	{
		if(instance == 0)
		{
			try 
			{
				instance = new Filesystem();
				love::luax_register_searcher(L, loader);
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, "love.filesystem", instance);

		return luax_register_module(L, wrap_Filesystem_functions, wrap_Filesystem_types);
	}

} // physfs
} // filesystem
} // love

