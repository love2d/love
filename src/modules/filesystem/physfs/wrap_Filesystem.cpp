/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

static Filesystem *instance = 0;

bool hack_setupWriteDirectory()
{
	if (instance != 0)
		return instance->setupWriteDirectory();
	return false;
}

int w_init(lua_State *L)
{
	const char *arg0 = luaL_checkstring(L, 1);

	try
	{
		instance->init(arg0);
	}
	catch(Exception &e)
	{
		return luaL_error(L, e.what());
	}

	return 0;
}

int w_setRelease(lua_State *L)
{
	// no error checking needed, everything, even nothing
	// can be converted to a boolean
	instance->setRelease(luax_toboolean(L, 1));
	return 0;
}

int w_setIdentity(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);

	if (!instance->setIdentity(arg))
		return luaL_error(L, "Could not set write directory.");

	return 0;
}

int w_getIdentity(lua_State *L)
{
	lua_pushstring(L, instance->getIdentity());
	return 1;
}

int w_setSource(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);

	if (!instance->setSource(arg))
		return luaL_error(L, "Could not set source.");

	return 0;
}

int w_newFile(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);
	File *t;
	try
	{
		t = instance->newFile(filename);
	}
	catch(Exception e)
	{
		return luaL_error(L, e.what());
	}
	luax_newtype(L, "File", FILESYSTEM_FILE_T, (void *)t);
	return 1;
}

int w_newFileData(lua_State *L)
{
	if (!lua_isstring(L, 1))
		return luaL_error(L, "String expected.");
	if (!lua_isstring(L, 2))
		return luaL_error(L, "String expected.");

	size_t length = 0;
	const char *str = lua_tolstring(L, 1, &length);
	const char *filename = lua_tostring(L, 2);
	const char *decstr = lua_isstring(L, 3) ? lua_tostring(L, 3) : 0;

	FileData::Decoder decoder = FileData::FILE;

	if (decstr)
		FileData::getConstant(decstr, decoder);

	FileData *t = 0;

	switch (decoder)
	{
	case FileData::FILE:
		t = instance->newFileData((void *)str, (int)length, filename);
		break;
	case FileData::BASE64:
		t = instance->newFileData(str, filename);
		break;
	default:
		return luaL_error(L, "Unrecognized FileData decoder: %s", decstr);
	}

	luax_newtype(L, "FileData", FILESYSTEM_FILE_DATA_T, (void *)t);
	return 1;
}

int w_getWorkingDirectory(lua_State *L)
{
	lua_pushstring(L, instance->getWorkingDirectory());
	return 1;
}

int w_getUserDirectory(lua_State *L)
{
	lua_pushstring(L, instance->getUserDirectory());
	return 1;
}

int w_getAppdataDirectory(lua_State *L)
{
	lua_pushstring(L, instance->getAppdataDirectory());
	return 1;
}

int w_getSaveDirectory(lua_State *L)
{
	lua_pushstring(L, instance->getSaveDirectory());
	return 1;
}

int w_exists(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);
	lua_pushboolean(L, instance->exists(arg) ? 1 : 0);
	return 1;
}

int w_isDirectory(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);
	lua_pushboolean(L, instance->isDirectory(arg) ? 1 : 0);
	return 1;
}

int w_isFile(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);
	lua_pushboolean(L, instance->isFile(arg) ? 1 : 0);
	return 1;
}

int w_mkdir(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);
	lua_pushboolean(L, instance->mkdir(arg) ? 1 : 0);
	return 1;
}

int w_remove(lua_State *L)
{
	const char *arg = luaL_checkstring(L, 1);
	lua_pushboolean(L, instance->remove(arg) ? 1 : 0);
	return 1;
}

int w_read(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);
	int64 len = (int64) luaL_optinteger(L, 2, File::ALL);

	Data *data = 0;
	try
	{
		data = instance->read(filename, len);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	if (data == 0)
		return luaL_error(L, "File could not be read.");

	// Push the string.
	lua_pushlstring(L, (const char *) data->getData(), data->getSize());

	// Push the size.
	lua_pushinteger(L, data->getSize());

	// Lua has a copy now, so we can free it.
	data->release();

	return 2;
}

static int w_write_or_append(lua_State *L, File::Mode mode)
{
	const char *filename = luaL_checkstring(L, 1);

	const char *input = 0;
	size_t len = 0;

	if (luax_istype(L, 2, DATA_T))
	{
		love::Data *data = luax_totype<love::Data>(L, 2, "Data", DATA_T);
		input = (const char *) data->getData();
		len = data->getSize();
	}
	else if (lua_isstring(L, 2))
		input = lua_tolstring(L, 2, &len);
	else
		return luaL_argerror(L, 2, "string or Data expected");

	// Get how much we should write. Length of string default.
	len = luaL_optinteger(L, 3, len);

	try
	{
		if (mode == File::APPEND)
			instance->append(filename, (const void *) input, len);
		else
			instance->write(filename, (const void *) input, len);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	luax_pushboolean(L, true);

	return 1;
}

int w_write(lua_State *L)
{
	return w_write_or_append(L, File::WRITE);
}

int w_append(lua_State *L)
{
	return w_write_or_append(L, File::APPEND);
}

int w_enumerate(lua_State *L)
{
	return instance->enumerate(L);
}

int w_lines(lua_State *L)
{
	File *file;

	if (lua_isstring(L, 1))
	{
		file = instance->newFile(lua_tostring(L, 1));
		try
		{
			if (!file->open(File::READ))
				return luaL_error(L, "Could not open file.");
		}
		catch(love::Exception &e)
		{
			return luaL_error(L, "%s", e.what());
		}
		luax_newtype(L, "File", FILESYSTEM_FILE_T, file);
	}
	else
		return luaL_error(L, "Expected filename.");

	lua_pushcclosure(L, Filesystem::lines_i, 1);
	return 1;
}

int w_load(lua_State *L)
{
	try
	{
		return instance->load(L);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, e.what());
	}
}

int w_getLastModified(lua_State *L)
{
	return instance->getLastModified(L);
}

int w_getSize(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);

	int64 size = -1;
	try
	{
		size = instance->getSize(filename);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	// Error on failure or if size does not fit into a double precision floating-point number.
	if (size == -1)
		return luaL_error(L, "Could not determine file size.");
	else if (size >= 0x20000000000000LL)
		return luaL_error(L, "Size too large to fit into a Lua number!");


	lua_pushnumber(L, (lua_Number) size);
	return 1;
}

int loader(lua_State *L)
{
	const char *filename = lua_tostring(L, -1);

	std::string tmp(filename);
	tmp += ".lua";

	int size = tmp.size();

	for (int i=0; i<size-4; i++)
	{
		if (tmp[i] == '.')
		{
			tmp[i] = '/';
		}
	}

	// Check whether file exists.
	if (instance->exists(tmp.c_str()))
	{
		lua_pop(L, 1);
		lua_pushstring(L, tmp.c_str());
		// Ok, load it.
		return instance->load(L);
	}

	tmp = filename;
	size = tmp.size();
	for (int i=0; i<size; i++)
	{
		if (tmp[i] == '.')
		{
			tmp[i] = '/';
		}
	}

	if (instance->isDirectory(tmp.c_str()))
	{
		tmp += "/init.lua";
		if (instance->exists(tmp.c_str()))
		{
			lua_pop(L, 1);
			lua_pushstring(L, tmp.c_str());
			// Ok, load it.
			return instance->load(L);
		}
	}

	lua_pushfstring(L, "\n\tno file \"%s\" in LOVE game directories.\n", (tmp + ".lua").c_str());
	return 1;
}

inline const char *library_extension()
{
#ifdef LOVE_WINDOWS
	return ".dll";
#else
	return ".so";
#endif
}

int extloader(lua_State *L)
{
	const char *filename = lua_tostring(L, -1);
	std::string tokenized_name(filename);
	std::string tokenized_function(filename);

	for (unsigned int i = 0; i < tokenized_name.size(); i++)
	{
		if (tokenized_name[i] == '.')
		{
			tokenized_name[i] = '/';
			tokenized_function[i] = '_';
		}
	}

	tokenized_name += library_extension();

	void *handle = SDL_LoadObject((std::string(instance->getAppdataDirectory()) + LOVE_PATH_SEPARATOR LOVE_APPDATA_FOLDER LOVE_PATH_SEPARATOR + tokenized_name).c_str());
	if (!handle && instance->isRelease())
		handle = SDL_LoadObject((std::string(instance->getSaveDirectory()) + LOVE_PATH_SEPARATOR + tokenized_name).c_str());

	if (!handle)
	{
		lua_pushfstring(L, "\n\tno extension \"%s\" in LOVE paths.\n", filename);
		return 1;
	}

	void *func = SDL_LoadFunction(handle, ("loveopen_" + tokenized_function).c_str());
	if (!func)
		func = SDL_LoadFunction(handle, ("luaopen_" + tokenized_function).c_str());

	if (!func)
	{
		SDL_UnloadObject(handle);
		lua_pushfstring(L, "\n\textension \"%s\" is incompatible.\n", filename);
		return 1;
	}

	lua_pushcfunction(L, (lua_CFunction) func);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "init",  w_init },
	{ "setRelease", w_setRelease },
	{ "setIdentity",  w_setIdentity },
	{ "getIdentity", w_getIdentity },
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
	{ "append", w_append },
	{ "enumerate",  w_enumerate },
	{ "lines",  w_lines },
	{ "load",  w_load },
	{ "getLastModified", w_getLastModified },
	{ "getSize", w_getSize },
	{ "newFileData", w_newFileData },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_file,
	luaopen_filedata,
	0
};

extern "C" int luaopen_love_filesystem(lua_State *L)
{
	if (instance == 0)
	{
		try
		{
			instance = new Filesystem();
			love::luax_register_searcher(L, loader, 1);
			love::luax_register_searcher(L, extloader, 2);
		}
		catch(Exception &e)
		{
			return luaL_error(L, e.what());
		}
	}
	else
	{
		instance->retain();
		love::luax_register_searcher(L, loader, 1);
		love::luax_register_searcher(L, extloader, 2);
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
