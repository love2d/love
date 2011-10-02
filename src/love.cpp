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

// LOVE
#include <common/config.h>
#include <common/version.h>
#include <common/runtime.h>

#ifdef LOVE_WINDOWS
#include <windows.h>
#endif // LOVE_WINDOWS

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_BUILD_EXE

// SDL
#include <SDL.h>

// Modules
#include <audio/wrap_Audio.h>
#include <event/sdl/wrap_Event.h>
#include <filesystem/physfs/wrap_Filesystem.h>
#include <font/freetype/wrap_Font.h>
#include <graphics/opengl/wrap_Graphics.h>
#include <image/wrap_Image.h>
#include <joystick/sdl/wrap_Joystick.h>
#include <keyboard/sdl/wrap_Keyboard.h>
#include <mouse/sdl/wrap_Mouse.h>
#include <physics/box2d/wrap_Physics.h>
#include <sound/wrap_Sound.h>
#include <timer/sdl/wrap_Timer.h>
#include <thread/wrap_Thread.h>

// Libraries.
#include "libraries/luasocket/luasocket.h"

// Scripts
#include "scripts/boot.lua.h"

#endif // LOVE_BUILD_EXE

#ifdef LOVE_BUILD_STANDALONE

static const luaL_Reg modules[] = {
	{ "love.audio", love::audio::luaopen_love_audio },
	{ "love.event", love::event::sdl::luaopen_love_event },
	{ "love.filesystem", love::filesystem::physfs::luaopen_love_filesystem },
	{ "love.font", love::font::freetype::luaopen_love_font },
	{ "love.graphics", love::graphics::opengl::luaopen_love_graphics },
	{ "love.image", love::image::luaopen_love_image },
	{ "love.joystick", love::joystick::sdl::luaopen_love_joystick },
	{ "love.keyboard", love::keyboard::sdl::luaopen_love_keyboard },
	{ "love.mouse", love::mouse::sdl::luaopen_love_mouse },
	{ "love.physics", love::physics::box2d::luaopen_love_physics },
	{ "love.sound", love::sound::luaopen_love_sound },
	{ "love.timer", love::timer::sdl::luaopen_love_timer },
	{ "love.thread", love::thread::luaopen_love_thread },
	{ 0, 0 }
};

#endif // LOVE_BUILD_STANDALONE

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
int w__openConsole(lua_State * L);
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

extern "C" LOVE_EXPORT int luaopen_love(lua_State * L)
{
	love::luax_insistglobal(L, "love");

	// Set version information.
	lua_pushstring(L, love::VERSION);
	lua_setfield(L, -2, "_version");

	lua_pushnumber(L, love::VERSION_MAJOR);
	lua_setfield(L, -2, "_version_major");
	lua_pushnumber(L, love::VERSION_MINOR);
	lua_setfield(L, -2, "_version_minor");
	lua_pushnumber(L, love::VERSION_REV);
	lua_setfield(L, -2, "_version_revision");

	lua_pushstring(L, love::VERSION_CODENAME);
	lua_setfield(L, -2, "_version_codename");

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
	lua_pushcfunction(L, w__openConsole);
	lua_setfield(L, -2, "_openConsole");
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

	lua_newtable(L);

	for(int i = 0; love::VERSION_COMPATIBILITY[i] != 0; ++i)
	{
		lua_pushstring(L, love::VERSION_COMPATIBILITY[i]);
		lua_rawseti(L, -2, i+1);
	}

	lua_setfield(L, -2, "_version_compat");

	lua_pop(L, 1); // love

#ifdef LOVE_BUILD_STANDALONE

	// Preload module loaders.
	for(int i = 0; modules[i].name != 0; i++)
	{
		love::luax_preload(L, modules[i].func, modules[i].name);
	}

	love::luasocket::__open(L);

#endif // LOVE_BUILD_STANDALONE

	return 0;
}

#ifdef LOVE_LEGENDARY_UTF8_ARGV_HACK

void get_utf8_arguments(int & argc, char **& argv)
{
	LPWSTR cmd = GetCommandLineW();

	if(!cmd)
		return;

	LPWSTR * argv_w = CommandLineToArgvW(cmd, &argc);

	argv = new char*[argc];

	for(int i = 0; i<argc; ++i)
	{
		// Size of wide char buffer (plus one for trailing '\0').
		size_t wide_len = wcslen(argv_w[i])+1;

		// Get size in UTF-8.
		int utf8_size = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], wide_len, argv[i], 0, 0, 0);

		argv[i] = new char[utf8_size];

		// Convert to UTF-8.
		int ok = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], wide_len, argv[i], utf8_size, 0, 0);

		int len = strlen(argv[i]);

		if(!ok)
			printf("Warning: could not convert to UTF8.\n");
	}

	LocalFree(argv_w);
}

#endif // LOVE_LEGENDARY_UTF8_ARGV_HACK

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK

int w__openConsole(lua_State * L)
{
	static bool is_open = false;

	if(is_open)
		return 0;

	static const int MAX_CONSOLE_LINES = 5000;
	long std_handle;
	int console_handle;
	CONSOLE_SCREEN_BUFFER_INFO console_info;
	FILE *fp;

	AllocConsole();

	// Set size.
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
	console_info.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), console_info.dwSize);

	SetConsoleTitle(TEXT("LOVE Console"));

	// Redirect stdout.
	std_handle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	console_handle = _open_osfhandle(std_handle, _O_TEXT);
	fp = _fdopen(console_handle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// Redirect stdin.
	std_handle = (long)GetStdHandle(STD_INPUT_HANDLE);
	console_handle = _open_osfhandle(std_handle, _O_TEXT);
	fp = _fdopen(console_handle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// Redirect stderr.
	std_handle = (long)GetStdHandle(STD_ERROR_HANDLE);
	console_handle = _open_osfhandle(std_handle, _O_TEXT);
	fp = _fdopen(console_handle, "w");
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);

	// Sync std::cout, std::cerr, etc.
	std::ios::sync_with_stdio();

	is_open = true;

	return 0;
}

#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_BUILD_EXE

int main(int argc, char ** argv)
{
#ifdef LOVE_LEGENDARY_UTF8_ARGV_HACK
	int hack_argc = 0;
	char ** hack_argv = 0;
	get_utf8_arguments(hack_argc, hack_argv);
	argc = hack_argc;
	argv = hack_argv;
#endif // LOVE_LEGENDARY_UTF8_ARGV_HACK

	// Oh, you just want the version? Okay!
	if(argc > 1 && strcmp(argv[1],"--version") == 0) {
		printf("LOVE %s (%s)\n", love::VERSION, love::VERSION_CODENAME);
		return 0;
	}

	// Create the virtual machine.
	lua_State * L = lua_open();
	luaL_openlibs(L);

	love::luax_preload(L, luaopen_love, "love");

	luaopen_love(L);

	// Add command line arguments to global arg (like stand-alone Lua).
	{
		lua_newtable(L);

		if(argc > 0)
		{
			lua_pushstring(L, argv[0]);
			lua_rawseti(L, -2, -2);
		}

		lua_pushstring(L, "embedded boot.lua");
		lua_rawseti(L, -2, -1);

		for(int i = 1; i<argc; i++)
		{
			lua_pushstring(L, argv[i]);
			lua_rawseti(L, -2, i);
		}

		lua_setglobal(L, "arg");
	}

	// Add love.__exe = true.
	// This indicates that we're running the
	// standalone version of love, and not the
	// DLL version.
	{
		lua_getglobal(L, "love");
		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "_exe");
		lua_pop(L, 1);
	}

	// Boot
	if (luaL_loadbuffer(L, (const char *)love::boot_lua, sizeof(love::boot_lua), "boot.lua") == 0)
		lua_call(L, 0, 0);

	lua_close(L);

#ifdef LOVE_LEGENDARY_UTF8_ARGV_HACK
	if(hack_argv)
	{
		for(int i = 0; i<hack_argc; ++i)
			delete [] hack_argv[i];
		delete [] hack_argv;
	}
#endif // LOVE_LEGENDARY_UTF8_ARGV_HACK
	return 0;
}

#endif // LOVE_BUILD_EXE
