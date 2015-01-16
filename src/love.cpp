/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "common/version.h"
#include "modules/love/love.h"
#include <SDL.h>

#ifdef LOVE_BUILD_EXE

// Lua
extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

#ifdef LOVE_WINDOWS
#include <windows.h>
#endif // LOVE_WINDOWS

#ifdef LOVE_MACOSX
#include "OSX.h"
#endif // LOVE_MACOSX

#ifdef LOVE_LEGENDARY_UTF8_ARGV_HACK

void get_utf8_arguments(int &argc, char **&argv)
{
	LPWSTR cmd = GetCommandLineW();

	if (!cmd)
		return;

	LPWSTR *argv_w = CommandLineToArgvW(cmd, &argc);

	argv = new char *[argc];

	for (int i = 0; i < argc; ++i)
	{
		// Size of wide char buffer (plus one for trailing '\0').
		size_t wide_len = wcslen(argv_w[i]) + 1;

		// Get size in UTF-8.
		int utf8_size = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], wide_len, argv[i], 0, 0, 0);

		argv[i] = new char[utf8_size];

		// Convert to UTF-8.
		int ok = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], wide_len, argv[i], utf8_size, 0, 0);

		int len = strlen(argv[i]);

		if (!ok)
			printf("Warning: could not convert to UTF8.\n");
	}

	LocalFree(argv_w);
}

#endif // LOVE_LEGENDARY_UTF8_ARGV_HACK

#ifdef LOVE_LEGENDARY_APP_ARGV_HACK

#include <vector>

static void get_app_arguments(int argc, char **argv, int &new_argc, char **&new_argv)
{
	std::vector<std::string> temp_argv;
	for (int i = 0; i < argc; i++)
	{
		// Don't copy -psn_xxx argument from argv.
		if (i == 0 || strncmp(argv[i], "-psn_", 5) != 0)
			temp_argv.push_back(std::string(argv[i]));
	}

	// Check for a drop file string.
	std::string dropfilestr = love::osx::checkDropEvents();
	if (!dropfilestr.empty())
	{
		temp_argv.insert(temp_argv.begin() + 1, dropfilestr);
	}
	else
	{
		// If it exists, add the love file in love.app/Contents/Resources/ to argv.
		std::string loveResourcesPath = love::osx::getLoveInResources();
		if (!loveResourcesPath.empty())
		{
			// Run in pseudo-fused mode.
			std::vector<std::string>::iterator it = temp_argv.begin();
			it = temp_argv.insert(it + 1, loveResourcesPath);
			temp_argv.insert(it + 1, std::string("--fused"));
		}
	}

	// Copy temp argv vector to new argv array.
	new_argc = (int) temp_argv.size();
	new_argv = new char *[new_argc+1];

	for (int i = 0; i < new_argc; i++)
	{
		new_argv[i] = new char[temp_argv[i].length() + 1];
		strcpy(new_argv[i], temp_argv[i].c_str());
	}

	new_argv[new_argc] = NULL;
}

#endif // LOVE_LEGENDARY_APP_ARGV_HACK

static int love_preload(lua_State *L, lua_CFunction f, const char *name)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, f);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
	return 0;
}

int main(int argc, char **argv)
{
#ifdef LOVE_LEGENDARY_UTF8_ARGV_HACK
	int hack_argc = 0;	char **hack_argv = 0;
	get_utf8_arguments(hack_argc, hack_argv);
	argc = hack_argc;
	argv = hack_argv;
#endif // LOVE_LEGENDARY_UTF8_ARGV_HACK

#ifdef LOVE_LEGENDARY_APP_ARGV_HACK
	int hack_argc = 0;
	char **hack_argv = 0;
	get_app_arguments(argc, argv, hack_argc, hack_argv);
	argc = hack_argc;
	argv = hack_argv;
#endif // LOVE_LEGENDARY_APP_ARGV_HACK

	if (strcmp(love::VERSION, love_version()) != 0)
	{
		printf("Version mismatch detected!\nLOVE binary is version %s\n"
				"LOVE library is version %s\n", love::VERSION, love_version());
		return 1;
	}

	// Oh, you just want the version? Okay!
	if (argc > 1 && strcmp(argv[1], "--version") == 0)
	{
		printf("LOVE %s (%s)\n", love_version(), love_codename());
		return 0;
	}

	// Create the virtual machine.
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// Add love to package.preload for easy requiring.
	love_preload(L, luaopen_love, "love");

	// Add command line arguments to global arg (like stand-alone Lua).
	{
		lua_newtable(L);

		if (argc > 0)
		{
			lua_pushstring(L, argv[0]);
			lua_rawseti(L, -2, -2);
		}

		lua_pushstring(L, "embedded boot.lua");
		lua_rawseti(L, -2, -1);

		for (int i = 1; i < argc; i++)
		{
			lua_pushstring(L, argv[i]);
			lua_rawseti(L, -2, i);
		}

		lua_setglobal(L, "arg");
	}

	// require "love"
	lua_getglobal(L, "require");
	lua_pushstring(L, "love");
	lua_call(L, 1, 1); // leave the returned table on the stack.

	// Add love._exe = true.
	// This indicates that we're running the standalone version of love, and not
	// the library version.
	{
		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "_exe");
	}

	// Pop the love table returned by require "love".
	lua_pop(L, 1);

	// require "love.boot" (preloaded when love was required.)
	lua_getglobal(L, "require");
	lua_pushstring(L, "love.boot");
	lua_call(L, 1, 1);

	// Call the returned boot function.
	lua_call(L, 0, 1);

	int retval = 0;
	if (lua_isnumber(L, -1))
		retval = (int) lua_tonumber(L, -1);

	lua_close(L);

#if defined(LOVE_LEGENDARY_UTF8_ARGV_HACK) || defined(LOVE_LEGENDARY_APP_ARGV_HACK)
	if (hack_argv)
	{
		for (int i = 0; i<hack_argc; ++i)
			delete [] hack_argv[i];
		delete [] hack_argv;
	}
#endif // LOVE_LEGENDARY_UTF8_ARGV_HACK || LOVE_LEGENDARY_APP_ARGV_HACK
	return retval;
}

#endif // LOVE_BUILD_EXE
