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
#include "common/config.h"
#include "common/version.h"
#include "common/runtime.h"

#include "love.h"

#ifdef LOVE_WINDOWS
#include <windows.h>
#endif // LOVE_WINDOWS

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_LEGENDARY_LIBSTDCXX_HACK

#include <iostream>

// Workarounds for symbols that are missing from Leopard stdlibc++.dylib.
// http://stackoverflow.com/questions/3484043/os-x-program-runs-on-dev-machine-crashing-horribly-on-others
_GLIBCXX_BEGIN_NAMESPACE(std)
// From ostream_insert.h
template ostream& __ostream_insert(ostream&, const char*, streamsize);

#ifdef _GLIBCXX_USE_WCHAR_T
template wostream& __ostream_insert(wostream&, const wchar_t*, streamsize);
#endif

// From ostream.tcc
template ostream& ostream::_M_insert(long);
template ostream& ostream::_M_insert(unsigned long);
template ostream& ostream::_M_insert(bool);
#ifdef _GLIBCXX_USE_LONG_LONG
template ostream& ostream::_M_insert(long long);
template ostream& ostream::_M_insert(unsigned long long);
#endif
template ostream& ostream::_M_insert(double);
template ostream& ostream::_M_insert(long double);
template ostream& ostream::_M_insert(const void*);

#ifdef _GLIBCXX_USE_WCHAR_T
template wostream& wostream::_M_insert(long);
template wostream& wostream::_M_insert(unsigned long);
template wostream& wostream::_M_insert(bool);
#ifdef _GLIBCXX_USE_LONG_LONG
template wostream& wostream::_M_insert(long long);
template wostream& wostream::_M_insert(unsigned long long);
#endif
template wostream& wostream::_M_insert(double);
template wostream& wostream::_M_insert(long double);
template wostream& wostream::_M_insert(const void*);
#endif

_GLIBCXX_END_NAMESPACE

#endif // LOVE_LEGENDARY_LIBSTDCXX_HACK

// Libraries.
#include "libraries/luasocket/luasocket.h"

// Scripts
#include "scripts/boot.lua.h"

// All modules define a c-accessible luaopen
// so let's make use of those, instead
// of addressing implementations directly.
extern "C"
{
	extern int luaopen_love_audio(lua_State*);
	extern int luaopen_love_event(lua_State*);
	extern int luaopen_love_filesystem(lua_State*);
	extern int luaopen_love_font(lua_State*);
	extern int luaopen_love_graphics(lua_State*);
	extern int luaopen_love_image(lua_State*);
	extern int luaopen_love_joystick(lua_State*);
	extern int luaopen_love_keyboard(lua_State*);
	extern int luaopen_love_mouse(lua_State*);
	extern int luaopen_love_physics(lua_State*);
	extern int luaopen_love_sound(lua_State*);
	extern int luaopen_love_timer(lua_State*);
	extern int luaopen_love_thread(lua_State*);
	extern int luaopen_love_math(lua_State*);
	extern int luaopen_love_boot(lua_State*);
}

static const luaL_Reg modules[] = {
	{ "love.audio", luaopen_love_audio },
	{ "love.event", luaopen_love_event },
	{ "love.filesystem", luaopen_love_filesystem },
	{ "love.font", luaopen_love_font },
	{ "love.graphics", luaopen_love_graphics },
	{ "love.image", luaopen_love_image },
	{ "love.joystick", luaopen_love_joystick },
	{ "love.keyboard", luaopen_love_keyboard },
	{ "love.math", luaopen_love_math },
	{ "love.mouse", luaopen_love_mouse },
	{ "love.physics", luaopen_love_physics },
	{ "love.sound", luaopen_love_sound },
	{ "love.timer", luaopen_love_timer },
	{ "love.thread", luaopen_love_thread },
	{ "love.boot", luaopen_love_boot },
	{ 0, 0 }
};

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
int w__openConsole(lua_State * L);
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

const char *love_version()
{
	return love::VERSION;
}

const char *love_codename()
{
	return love::VERSION_CODENAME;
}

int luaopen_love(lua_State * L)
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

	for (int i = 0; love::VERSION_COMPATIBILITY[i] != 0; ++i)
	{
		lua_pushstring(L, love::VERSION_COMPATIBILITY[i]);
		lua_rawseti(L, -2, i+1);
	}

	lua_setfield(L, -2, "_version_compat");

#ifdef LOVE_WINDOWS
	lua_pushstring(L, "Windows");
#elif defined(LOVE_MACOSX)
	lua_pushstring(L, "OS X");
#elif defined(LOVE_LINUX)
	lua_pushstring(L, "Linux");
#else
	lua_pushstring(L, "Unknown");
#endif
	lua_setfield(L, -2, "_os");

	// Preload module loaders.
	for (int i = 0; modules[i].name != 0; i++)
	{
		love::luax_preload(L, modules[i].func, modules[i].name);
	}

	love::luasocket::__open(L);

	return 1;
}

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK

int w__openConsole(lua_State * L)
{
	static bool is_open = false;
	if (is_open)
		return 0;
	is_open = true;

	if (GetConsoleWindow() != NULL || AllocConsole() == 0)
		return 0;

	const int MAX_CONSOLE_LINES = 5000;
	CONSOLE_SCREEN_BUFFER_INFO console_info;

	// Set size.
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
	console_info.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), console_info.dwSize);

	SetConsoleTitle(TEXT("LOVE Console"));

	FILE * fp;

	// Redirect stdout.
	fp = freopen("CONOUT$", "w", stdout);
	if (L && fp == NULL)
		luaL_error(L, "Console redirection of stdout failed.");

	// Redirect stdin.
	fp = freopen("CONIN$", "r", stdin);
	if (L && fp == NULL)
		luaL_error(L, "Console redirection of stdin failed.");

	// Redirect stderr.
	fp = freopen("CONOUT$", "w", stderr);
	if (L && fp == NULL)
		luaL_error(L, "Console redirection of stderr failed.");

	return 0;
}

#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

int luaopen_love_boot(lua_State *L)
{
	if (luaL_loadbuffer(L, (const char *)love::boot_lua, sizeof(love::boot_lua), "boot.lua") == 0)
		lua_call(L, 0, 1);

	return 1;
}



