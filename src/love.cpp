/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "common/runtime.h"
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
#include "common/macosx.h"
#include <unistd.h>
#endif // LOVE_MACOSX

#ifdef LOVE_IOS
#include "common/ios.h"
#endif

#ifdef LOVE_WINDOWS
extern "C"
{

// Prefer the higher performance GPU on Windows systems that use nvidia Optimus.
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
// TODO: Re-evaluate in the future when the average integrated GPU in Optimus
// systems is less mediocre?
LOVE_EXPORT DWORD NvOptimusEnablement = 1;

// Same with AMD GPUs.
// https://community.amd.com/thread/169965
LOVE_EXPORT DWORD AmdPowerXpressRequestHighPerformance = 1;
}
#endif // LOVE_WINDOWS

#ifdef LOVE_LEGENDARY_APP_ARGV_HACK

#include <vector>

// Explicitly instantiate std::vector<std::string> to work around linker issues
// with libc++ when symbols are hidden-by-default.
// https://stackoverflow.com/a/48273604
template class std::vector<std::string>;

static void get_app_arguments(int argc, char **argv, int &new_argc, char **&new_argv)
{
	std::vector<std::string> temp_argv;
	for (int i = 0; i < argc; i++)
	{
		// Don't copy -psn_xxx argument from argv.
		if (i == 0 || strncmp(argv[i], "-psn_", 5) != 0)
			temp_argv.push_back(std::string(argv[i]));
	}

	// If it exists, add the love file in love.app/Contents/Resources/ to argv.
	std::string loveResourcesPath;
	bool fused = true;
#if defined(LOVE_MACOSX)
	loveResourcesPath = love::macosx::getLoveInResources();
#elif defined(LOVE_IOS)
	loveResourcesPath = love::ios::getLoveInResources(fused);
#endif
	if (!loveResourcesPath.empty())
	{
		std::vector<std::string>::iterator it = temp_argv.begin();
		it = temp_argv.insert(it + 1, loveResourcesPath);

		// Run in pseudo-fused mode.
		if (fused)
			temp_argv.insert(it + 1, std::string("--fused"));
	}
#ifdef LOVE_MACOSX
	else
	{
		// Check for a drop file string, if the app wasn't launched in a
		// terminal. Checking for the terminal is a pretty big hack, but works
		// around an issue where OS X will switch Spaces if the terminal
		// launching love is in its own full-screen Space.
		if (!isatty(STDIN_FILENO))
		{
			// Static to keep the same value after love.event.equit("restart").
			static std::string dropfilestr = love::macosx::checkDropEvents();
			if (!dropfilestr.empty())
				temp_argv.insert(temp_argv.begin() + 1, dropfilestr);
		}
	}
#endif

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

enum DoneAction
{
	DONE_QUIT,
	DONE_RESTART,
};

static DoneAction runlove(int argc, char **argv, int &retval)
{
	// Oh, you just want the version? Okay!
	if (argc > 1 && strcmp(argv[1], "--version") == 0)
	{
#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
		const char *err = nullptr;
		love_openConsole(err);
#endif
		printf("LOVE %s (%s)\n", love_version(), love_codename());
		retval = 0;
		return DONE_QUIT;
	}

	// Create the virtual machine.
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// LuaJIT-specific setup needs to be done as early as possible - before
	// get_app_arguments because that loads external library code. This is also
	// loaded inside require("love"). Note that it doesn't use the love table.
	love_preload(L, luaopen_love_jitsetup, "love.jitsetup");
	lua_getglobal(L, "require");
	lua_pushstring(L, "love.jitsetup");
	lua_call(L, 1, 0);

#ifdef LOVE_LEGENDARY_APP_ARGV_HACK
	int hack_argc = 0;
	char **hack_argv = nullptr;
	get_app_arguments(argc, argv, hack_argc, hack_argv);
	argc = hack_argc;
	argv = hack_argv;
#endif // LOVE_LEGENDARY_APP_ARGV_HACK

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

	// Turn the returned boot function into a coroutine and call it until done.
	lua_newthread(L);
	lua_pushvalue(L, -2);
	int stackpos = lua_gettop(L);
	int nres;
	while (love::luax_resume(L, 0, &nres) == LUA_YIELD)
#if LUA_VERSION_NUM >= 504
		lua_pop(L, nres);
#else
		lua_pop(L, lua_gettop(L) - stackpos);
#endif

	retval = 0;
	DoneAction done = DONE_QUIT;

	// if love.boot() returns "restart", we'll start up again after closing this
	// Lua state.
	if (lua_type(L, -1) == LUA_TSTRING && strcmp(lua_tostring(L, -1), "restart") == 0)
		done = DONE_RESTART;
	if (lua_isnumber(L, -1))
		retval = (int) lua_tonumber(L, -1);

	lua_close(L);

#if defined(LOVE_LEGENDARY_APP_ARGV_HACK) && !defined(LOVE_IOS)
	if (hack_argv)
	{
		for (int i = 0; i<hack_argc; ++i)
			delete [] hack_argv[i];
		delete [] hack_argv;
	}
#endif // LOVE_LEGENDARY_APP_ARGV_HACK

	return done;
}

int main(int argc, char **argv)
{
	if (strcmp(LOVE_VERSION_STRING, love_version()) != 0)
	{
		printf("Version mismatch detected!\nLOVE binary is version %s\n"
			   "LOVE library is version %s\n", LOVE_VERSION_STRING, love_version());
		return 1;
	}

	int retval = 0;
	DoneAction done = DONE_QUIT;

	do
	{
		done = runlove(argc, argv, retval);

#ifdef LOVE_IOS
		// on iOS we should never programmatically exit the app, so we'll just
		// "restart" when that is attempted. Games which use threads might cause
		// some issues if the threads aren't cleaned up properly...
		done = DONE_RESTART;
#endif
	} while (done != DONE_QUIT);

#ifdef LOVE_ANDROID
	SDL_Quit();
#endif

	return retval;
}

#endif // LOVE_BUILD_EXE
