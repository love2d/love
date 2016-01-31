/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "common/wrap_Data.h"

#include "love.h"

// C++
#include <string>
#include <sstream>

#ifdef LOVE_WINDOWS
#include <windows.h>
#endif // LOVE_WINDOWS

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK
#include <SDL_hints.h>
#endif // LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK

// Libraries.
#ifdef LOVE_ENABLE_LUASOCKET
#	include "libraries/luasocket/luasocket.h"
#endif
#ifdef LOVE_ENABLE_ENET
#	include "libraries/enet/lua-enet.h"
#endif
#ifdef LOVE_ENABLE_LUAUTF8
#	include "libraries/luautf8/lutf8lib.h"
#endif

// For love::graphics::setGammaCorrect.
#ifdef LOVE_ENABLE_GRAPHICS
#	include "graphics/Graphics.h"
#endif

// Scripts
#include "scripts/nogame.lua.h"
#include "scripts/boot.lua.h"

// All modules define a c-accessible luaopen
// so let's make use of those, instead
// of addressing implementations directly.
extern "C"
{
#if defined(LOVE_ENABLE_AUDIO)
	extern int luaopen_love_audio(lua_State*);
#endif
#if defined(LOVE_ENABLE_EVENT)
	extern int luaopen_love_event(lua_State*);
#endif
#if defined(LOVE_ENABLE_FILESYSTEM)
	extern int luaopen_love_filesystem(lua_State*);
#endif
#if defined(LOVE_ENABLE_FONT)
	extern int luaopen_love_font(lua_State*);
#endif
#if defined(LOVE_ENABLE_GRAPHICS)
	extern int luaopen_love_graphics(lua_State*);
#endif
#if defined(LOVE_ENABLE_IMAGE)
	extern int luaopen_love_image(lua_State*);
#endif
#if defined(LOVE_ENABLE_JOYSTICK)
	extern int luaopen_love_joystick(lua_State*);
#endif
#if defined(LOVE_ENABLE_KEYBOARD)
	extern int luaopen_love_keyboard(lua_State*);
#endif
#if defined(LOVE_ENABLE_MATH)
	extern int luaopen_love_math(lua_State*);
#endif
#if defined(LOVE_ENABLE_MOUSE)
	extern int luaopen_love_mouse(lua_State*);
#endif
#if defined(LOVE_ENABLE_PHYSICS)
	extern int luaopen_love_physics(lua_State*);
#endif
#if defined(LOVE_ENABLE_SOUND)
	extern int luaopen_love_sound(lua_State*);
#endif
#if defined(LOVE_ENABLE_SYSTEM)
	extern int luaopen_love_system(lua_State*);
#endif
#if defined(LOVE_ENABLE_TIMER)
	extern int luaopen_love_timer(lua_State*);
#endif
#if defined(LOVE_ENABLE_THREAD)
	extern int luaopen_love_thread(lua_State*);
#endif
#if defined(LOVE_ENABLE_TOUCH)
	extern int luaopen_love_touch(lua_State*);
#endif
#if defined(LOVE_ENABLE_VIDEO)
	extern int luaopen_love_video(lua_State*);
#endif
#if defined(LOVE_ENABLE_WINDOW)
	extern int luaopen_love_window(lua_State*);
#endif
	extern int luaopen_love_nogame(lua_State*);
	extern int luaopen_love_boot(lua_State*);
}

static const luaL_Reg modules[] = {
#if defined(LOVE_ENABLE_AUDIO)
	{ "love.audio", luaopen_love_audio },
#endif
#if defined(LOVE_ENABLE_EVENT)
	{ "love.event", luaopen_love_event },
#endif
#if defined(LOVE_ENABLE_FILESYSTEM)
	{ "love.filesystem", luaopen_love_filesystem },
#endif
#if defined(LOVE_ENABLE_FONT)
	{ "love.font", luaopen_love_font },
#endif
#if defined(LOVE_ENABLE_GRAPHICS)
	{ "love.graphics", luaopen_love_graphics },
#endif
#if defined(LOVE_ENABLE_IMAGE)
	{ "love.image", luaopen_love_image },
#endif
#if defined(LOVE_ENABLE_JOYSTICK)
	{ "love.joystick", luaopen_love_joystick },
#endif
#if defined(LOVE_ENABLE_KEYBOARD)
	{ "love.keyboard", luaopen_love_keyboard },
#endif
#if defined(LOVE_ENABLE_MATH)
	{ "love.math", luaopen_love_math },
#endif
#if defined(LOVE_ENABLE_MOUSE)
	{ "love.mouse", luaopen_love_mouse },
#endif
#if defined(LOVE_ENABLE_PHYSICS)
	{ "love.physics", luaopen_love_physics },
#endif
#if defined(LOVE_ENABLE_SOUND)
	{ "love.sound", luaopen_love_sound },
#endif
#if defined(LOVE_ENABLE_SYSTEM)
	{ "love.system", luaopen_love_system },
#endif
#if defined(LOVE_ENABLE_TIMER)
	{ "love.timer", luaopen_love_timer },
#endif
#if defined(LOVE_ENABLE_THREAD)
	{ "love.thread", luaopen_love_thread },
#endif
#if defined(LOVE_ENABLE_TOUCH)
	{ "love.touch", luaopen_love_touch },
#endif
#if defined(LOVE_ENABLE_VIDEO)
	{ "love.video", luaopen_love_video },
#endif
#if defined(LOVE_ENABLE_WINDOW)
	{ "love.window", luaopen_love_window },
#endif
	{ "love.nogame", luaopen_love_nogame },
	{ "love.boot", luaopen_love_boot },
	{ 0, 0 }
};

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
int w__openConsole(lua_State *L);
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK
int w__setAccelerometerAsJoystick(lua_State *L);
#endif

const char *love_version()
{
	// Do not refer to love::VERSION here, the linker
	// will patch it back up to the executable's one..
	return LOVE_VERSION_STRING;
}

const char *love_codename()
{
	return love::VERSION_CODENAME;
}

static int w_love_getVersion(lua_State *L)
{
	lua_pushinteger(L, love::VERSION_MAJOR);
	lua_pushinteger(L, love::VERSION_MINOR);
	lua_pushinteger(L, love::VERSION_REV);
	lua_pushstring(L, love::VERSION_CODENAME);
	return 4;
}

static int w_love_isVersionCompatible(lua_State *L)
{
	std::string version;

	if (lua_type(L, 1) == LUA_TSTRING)
		version = luaL_checkstring(L, 1);
	else
	{
		int major = (int) luaL_checknumber(L, 1);
		int minor = (int) luaL_checknumber(L, 2);
		int rev   = (int) luaL_checknumber(L, 3);

		// Convert the numbers to a string, since VERSION_COMPATIBILITY is an
		// array of version strings.
		std::stringstream ss;
		ss << major << "." << minor << "." << rev;

		version = ss.str();
	}

	for (int i = 0; love::VERSION_COMPATIBILITY[i] != nullptr; i++)
	{
		if (version.compare(love::VERSION_COMPATIBILITY[i]) != 0)
			continue;

		lua_pushboolean(L, true);
		return 1;
	}

	lua_pushboolean(L, false);
	return 1;
}

static int w__setGammaCorrect(lua_State *L)
{
#ifdef LOVE_ENABLE_GRAPHICS
	love::graphics::setGammaCorrect((bool) lua_toboolean(L, 1));
#endif
	return 0;
}

int luaopen_love(lua_State *L)
{
	love::luax_insistpinnedthread(L);

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

#ifdef LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK
	lua_pushcfunction(L, w__setAccelerometerAsJoystick);
	lua_setfield(L, -2, "_setAccelerometerAsJoystick");
#endif

	lua_pushcfunction(L, w__setGammaCorrect);
	lua_setfield(L, -2, "_setGammaCorrect");

	lua_newtable(L);

	for (int i = 0; love::VERSION_COMPATIBILITY[i] != nullptr; i++)
	{
		lua_pushstring(L, love::VERSION_COMPATIBILITY[i]);
		lua_rawseti(L, -2, i+1);
	}

	lua_setfield(L, -2, "_version_compat");

	lua_pushcfunction(L, w_love_getVersion);
	lua_setfield(L, -2, "getVersion");

	lua_pushcfunction(L, w_love_isVersionCompatible);
	lua_setfield(L, -2, "isVersionCompatible");

#ifdef LOVE_WINDOWS
	lua_pushstring(L, "Windows");
#elif defined(LOVE_MACOSX)
	lua_pushstring(L, "OS X");
#elif defined(LOVE_IOS)
	lua_pushstring(L, "iOS");
#elif defined(LOVE_ANDROID)
	lua_pushstring(L, "Android");
#elif defined(LOVE_LINUX)
	lua_pushstring(L, "Linux");
#else
	lua_pushstring(L, "Unknown");
#endif
	lua_setfield(L, -2, "_os");

	// Preload module loaders.
	for (int i = 0; modules[i].name != nullptr; i++)
		love::luax_preload(L, modules[i].func, modules[i].name);

	// Load "common" types.
	love::w_Data_open(L);

#ifdef LOVE_ENABLE_LUASOCKET
	love::luasocket::__open(L);
#endif
#ifdef LOVE_ENABLE_ENET
	love::luax_preload(L, luaopen_enet, "enet");
#endif
#ifdef LOVE_ENABLE_LUAUTF8
	love::luax_preload(L, luaopen_luautf8, "utf8");
#endif

	return 1;
}

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK

// Mostly taken from the Windows 8.1 SDK's VersionHelpers.h.
static bool IsWindowsVistaOrGreater()
{
	OSVERSIONINFOEXW osvi = {sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0};

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
	osvi.wServicePackMajor = 0;

	DWORDLONG majorversionmask = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
	DWORDLONG versionmask = VerSetConditionMask(majorversionmask, VER_MINORVERSION, VER_GREATER_EQUAL);
	DWORDLONG mask = VerSetConditionMask(versionmask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, mask) != FALSE;
}

int w__openConsole(lua_State *L)
{
	static bool is_open = false;

	if (is_open)
	{
		love::luax_pushboolean(L, is_open);
		return 1;
	}

	is_open = true;

	// FIXME: we don't call AttachConsole in Windows XP because it seems to
	// break later AllocConsole calls if it fails. A better workaround might be
	// possible, but it's hard to find a WinXP system to test on these days...
	if (!IsWindowsVistaOrGreater() || !AttachConsole(ATTACH_PARENT_PROCESS))
	{
		// Create our own console if we can't attach to an existing one.
		if (!AllocConsole())
		{
			is_open = false;
			love::luax_pushboolean(L, is_open);
			return 1;
		}

		SetConsoleTitle(TEXT("LOVE Console"));

		const int MAX_CONSOLE_LINES = 5000;
		CONSOLE_SCREEN_BUFFER_INFO console_info;

		// Set size.
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
		console_info.dwSize.Y = MAX_CONSOLE_LINES;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), console_info.dwSize);
	}

	FILE *fp;

	// Redirect stdout.
	fp = freopen("CONOUT$", "w", stdout);
	if (L && fp == NULL)
		return luaL_error(L, "Console redirection of stdout failed.");

	// Redirect stdin.
	fp = freopen("CONIN$", "r", stdin);
	if (L && fp == NULL)
		return luaL_error(L, "Console redirection of stdin failed.");

	// Redirect stderr.
	fp = freopen("CONOUT$", "w", stderr);
	if (L && fp == NULL)
		return luaL_error(L, "Console redirection of stderr failed.");

	love::luax_pushboolean(L, is_open);
	return 1;
}

#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK
int w__setAccelerometerAsJoystick(lua_State *L)
{
	bool enable = (bool) lua_toboolean(L, 1);
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, enable ? "1" : "0");
	return 0;
}
#endif // LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK

int luaopen_love_nogame(lua_State *L)
{
	if (luaL_loadbuffer(L, (const char *)love::nogame_lua, sizeof(love::nogame_lua), "nogame.lua") == 0)
		lua_call(L, 0, 1);

	return 1;
}

int luaopen_love_boot(lua_State *L)
{
	if (luaL_loadbuffer(L, (const char *)love::boot_lua, sizeof(love::boot_lua), "boot.lua") == 0)
		lua_call(L, 0, 1);

	return 1;
}



