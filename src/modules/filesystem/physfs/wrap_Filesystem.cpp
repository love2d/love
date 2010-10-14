/**
* Copyright (c) 2006-2010 LOVE Development Team
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

	int w_init(lua_State * L)
	{
		const char * arg0 = luaL_checkstring(L, 1);

		try
		{
			instance->init(arg0);
		}
		catch(Exception & e)
		{
			return luaL_error(L, e.what());
		}

		return 0;
	}

	int w_setIdentity(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);

		if(!instance->setIdentity(arg))
			return luaL_error(L, "Could not set write directory.");

		return 0;
	}

	int w_setSource(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);

		if(!instance->setSource(arg))
			return luaL_error(L, "Could not set source.");

		return 0;
	}

	int w_newFile(lua_State * L)
	{
		const char * filename = luaL_checkstring(L, 1);
		File * t;
		try
		{
			t = instance->newFile(filename);
		}
		catch(Exception e)
		{
			return luaL_error(L, e.what());
		}
		luax_newtype(L, "File", FILESYSTEM_FILE_T, (void*)t);
		return 1;
	}

	int w_newFileData(lua_State * L)
	{
		if(!lua_isstring(L, 1))
			return luaL_error(L, "String expected.");
		if(!lua_isstring(L, 2))
			return luaL_error(L, "String expected.");

		size_t length = 0;
		const char * str = lua_tolstring(L, 1, &length);
		const char * filename = lua_tostring(L, 2);
		const char * decstr = lua_isstring(L, 3) ? lua_tostring(L, 3) : 0;

		FileData::Decoder decoder = FileData::FILE;

		if(decstr)
			FileData::getConstant(decstr, decoder);

		FileData * t = 0;

		switch(decoder)
		{
		case FileData::FILE:
			t = instance->newFileData((void*)str, (int)length, filename);
			break;
		case FileData::BASE64:
			t = instance->newFileData(str, filename);
			break;
		default:
			return luaL_error(L, "Unrecognized FileData decoder: %s", decstr);
		}

		luax_newtype(L, "FileData", FILESYSTEM_FILE_DATA_T, (void*)t);
		return 1;
	}

	int w_getWorkingDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getWorkingDirectory());
		return 1;
	}

	int w_getUserDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getUserDirectory());
		return 1;
	}

	int w_getAppdataDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getAppdataDirectory());
		return 1;
	}

	int w_getSaveDirectory(lua_State * L)
	{
		lua_pushstring(L, instance->getSaveDirectory());
		return 1;
	}

	int w_exists(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->exists(arg) ? 1 : 0);
		return 1;
	}

	int w_isDirectory(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->isDirectory(arg) ? 1 : 0);
		return 1;
	}

	int w_isFile(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->isFile(arg) ? 1 : 0);
		return 1;
	}

	int w_mkdir(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->mkdir(arg) ? 1 : 0);
		return 1;
	}

	int w_remove(lua_State * L)
	{
		const char * arg = luaL_checkstring(L, 1);
		lua_pushboolean(L, instance->remove(arg) ? 1 : 0);
		return 1;
	}

	int w_read(lua_State * L)
	{
		try
		{
			return instance->read(L);
		}
		catch(Exception e)
		{
			return luaL_error(L, e.what());
		}
	}

	int w_write(lua_State * L)
	{
		try
		{
			return instance->write(L);
		}
		catch(Exception e)
		{
			return luaL_error(L, e.what());
		}
	}

	int w_enumerate(lua_State * L)
	{
		return instance->enumerate(L);
	}

	int w_lines(lua_State * L)
	{
		return instance->lines(L);
	}

	int w_load(lua_State * L)
	{
		return instance->load(L);
	}

	int loader(lua_State * L)
	{
		const char * filename = lua_tostring(L, -1);

		std::string tmp(filename);

		int size = tmp.size();

		if(size <= 4 || strcmp(filename + (size-4), ".lua") != 0)
		{
			tmp.append(".lua");
			size = tmp.size();
		}

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
	static const luaL_Reg functions[] = {
		{ "init",  w_init },
		{ "setIdentity",  w_setIdentity },
		{ "setSource",  w_setSource },
		{ "newFile",  w_newFile },
		{ "getWorkingDirectory",  w_getWorkingDirectory },
		{ "getUserDirectory",  w_getUserDirectory },
		{ "getAppdataDirectory",  w_getAppdataDirectory },
		{ "getSaveDirectory",  w_getSaveDirectory },
		{ "exists",  w_exists },
		{ "isDirectory",  w_isDirectory },
		{ "isFile",  w_isFile },
		{ "mkdir",  w_mkdir },
		{ "remove",  w_remove },
		{ "read",  w_read },
		{ "write",  w_write },
		{ "enumerate",  w_enumerate },
		{ "lines",  w_lines },
		{ "load",  w_load },
		{ "newFileData", w_newFileData },
		{ 0, 0 }
	};

	static const lua_CFunction types[] = {
		luaopen_file,
		luaopen_filedata,
		0
	};

	int luaopen_love_filesystem(lua_State * L)
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
		else
		{
			instance->retain();
			love::luax_register_searcher(L, loader);
		}

		WrappedModule w;
		w.module = instance;
		w.name = "filesystem";
		w.flags = MODULE_FILESYSTEM_T;
		w.functions = functions;
		w.types = types;

		return luax_register_module(L, w);
	}

} // physfs
} // filesystem
} // love
