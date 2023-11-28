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

// LOVE
#include "common/config.h"
#include "common/version.h"
#include "common/deprecation.h"
#include "common/runtime.h"
#include "modules/window/Window.h"

#include "love.h"

// C++
#include <string>
#include <sstream>

#ifdef LOVE_WINDOWS
#include <windows.h>

#if defined(_MSC_VER) && (_MSC_VER < 1900)
// VS 2013 and earlier doesn't have snprintf
#define snprintf sprintf_s
#endif // defined(_MSC_VER) && (_MSC_VER < 1900)

#endif // LOVE_WINDOWS

#ifdef LOVE_ANDROID
#include <SDL.h>
#endif // LOVE_ANDROID

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
#ifdef LOVE_ENABLE_LUA53
#	include "libraries/lua53/lutf8lib.h"
#endif

// For love::graphics::setGammaCorrect.
#ifdef LOVE_ENABLE_GRAPHICS
#	include "graphics/Graphics.h"
#endif

// For love::audio::Audio::setMixWithSystem.
#ifdef LOVE_ENABLE_AUDIO
#	include "audio/Audio.h"
#endif

// Scripts.
#include "scripts/nogame.lua.h"

// Put the Lua code directly into a raw string literal.
static const char arg_lua[] =
#include "arg.lua"
;

static const char callbacks_lua[] =
#include "callbacks.lua"
;

static const char boot_lua[] =
#include "boot.lua"
;

static const char jit_setup_lua[] =
#include "jitsetup.lua"
;

// All modules define a c-accessible luaopen
// so let's make use of those, instead
// of addressing implementations directly.
extern "C"
{
#if defined(LOVE_ENABLE_AUDIO)
	extern int luaopen_love_audio(lua_State*);
#endif
#if defined(LOVE_ENABLE_DATA)
	extern int luaopen_love_data(lua_State*);
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
	extern int luaopen_love_jitsetup(lua_State*);
	extern int luaopen_love_arg(lua_State*);
	extern int luaopen_love_callbacks(lua_State*);
	extern int luaopen_love_boot(lua_State*);
}

static const luaL_Reg modules[] = {
#if defined(LOVE_ENABLE_AUDIO)
	{ "love.audio", luaopen_love_audio },
#endif
#if defined(LOVE_ENABLE_DATA)
	{ "love.data", luaopen_love_data },
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
#if defined(LOVE_ENABLE_THREAD)
	{ "love.thread", luaopen_love_thread },
#endif
#if defined(LOVE_ENABLE_TIMER)
	{ "love.timer", luaopen_love_timer },
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
	{ "love.jitsetup", luaopen_love_jitsetup },
	{ "love.arg", luaopen_love_arg },
	{ "love.callbacks", luaopen_love_callbacks },
	{ "love.boot", luaopen_love_boot },
	{ 0, 0 }
};

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK
int w__openConsole(lua_State *L);
#endif // LOVE_LEGENDARY_CONSOLE_IO_HACK

#ifdef LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK
int w__setAccelerometerAsJoystick(lua_State *L);
#endif

#ifdef LOVE_ANDROID
static int w_print_sdl_log(lua_State *L)
{
	int nargs = lua_gettop(L);

	lua_getglobal(L, "tostring");

	std::string outstring;

	for (int i = 1; i <= nargs; i++)
	{
		// Call tostring(arg) and leave the result on the top of the stack.
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);

		const char *s = lua_tostring(L, -1);
		if (s == nullptr)
			return luaL_error(L, "'tostring' must return a string to 'print'");

		if (i > 1)
			outstring += "\t";

		outstring += s;

		lua_pop(L, 1); // Pop the result of tostring(arg).
	}

	SDL_Log("[LOVE] %s", outstring.c_str());
	return 0;
}
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
	{
		version = luaL_checkstring(L, 1);

		// Convert major.minor to major.minor.revision.
		if (std::count(version.begin(), version.end(), '.') < 2)
			version.append(".0");
	}
	else
	{
		int major = (int) luaL_checkinteger(L, 1);
		int minor = (int) luaL_checkinteger(L, 2);
		int rev   = (int) luaL_optinteger(L, 3, 0);

		// Convert the numbers to a string, since VERSION_COMPATIBILITY is an
		// array of version strings.
		std::stringstream ss;
		ss << major << "." << minor << "." << rev;

		version = ss.str();
	}

	for (int i = 0; love::VERSION_COMPATIBILITY[i] != nullptr; i++)
	{
		std::string v(love::VERSION_COMPATIBILITY[i]);

		// Convert major.minor to major.minor.revision.
		if (std::count(v.begin(), v.end(), '.') < 2)
			v.append(".0");

		if (version.compare(v) != 0)
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

static int w__setAudioMixWithSystem(lua_State *L)
{
	bool success = false;

#ifdef LOVE_ENABLE_AUDIO
	bool mix = love::luax_checkboolean(L, 1);
	success = love::audio::Audio::setMixWithSystem(mix);
#endif

	love::luax_pushboolean(L, success);
	return 1;
}

static int w__requestRecordingPermission(lua_State *L)
{
#ifdef LOVE_ENABLE_AUDIO
	love::audio::setRequestRecordingPermission((bool) lua_toboolean(L, 1));
#endif
	return 0;
}

static int w_love_setDeprecationOutput(lua_State *L)
{
	bool enable = love::luax_checkboolean(L, 1);
	love::setDeprecationOutputEnabled(enable);
	return 0;
}

static int w_love_hasDeprecationOutput(lua_State *L)
{
	love::luax_pushboolean(L, love::isDeprecationOutputEnabled());
	return 1;
}

static int w_deprecation__gc(lua_State *)
{
	love::deinitDeprecation();
	return 0;
}

static void luax_addcompatibilityalias(lua_State *L, const char *module, const char *name, const char *alias)
{
	lua_getglobal(L, module);
	if (lua_istable(L, -1))
	{
		lua_getfield(L, -1, alias);
		bool hasalias = !lua_isnoneornil(L, -1);
		lua_pop(L, 1);
		if (!hasalias)
		{
			lua_getfield(L, -1, name);
			lua_setfield(L, -2, alias);
		}
	}
	lua_pop(L, 1);
}

int luaopen_love(lua_State *L)
{
	// Preload module loaders.
	for (int i = 0; modules[i].name != nullptr; i++)
		love::luax_preload(L, modules[i].func, modules[i].name);

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

#ifdef LOVE_ANDROID
	lua_register(L, "print", w_print_sdl_log);
#endif

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

	// Exposed here because we need to be able to call it before the audio
	// module is initialized.
	lua_pushcfunction(L, w__setAudioMixWithSystem);
	lua_setfield(L, -2, "_setAudioMixWithSystem");
	lua_pushcfunction(L, w__requestRecordingPermission);
	lua_setfield(L, -2, "_requestRecordingPermission");

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

#ifdef LOVE_WINDOWS_UWP
	lua_pushstring(L, "UWP");
#elif LOVE_WINDOWS
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

	{
		love::initDeprecation();

		// Any old data that we can attach a metatable to, for __gc. We want to
		// call deinitDeprecation when love is garbage collected.
		lua_newuserdata(L, sizeof(int));

		luaL_newmetatable(L, "love_deprecation");
		lua_pushcfunction(L, w_deprecation__gc);
		lua_setfield(L, -2, "__gc");
		lua_setmetatable(L, -2);

		lua_setfield(L, -2, "_deprecation");

		lua_pushcfunction(L, w_love_setDeprecationOutput);
		lua_setfield(L, -2, "setDeprecationOutput");

		lua_pushcfunction(L, w_love_hasDeprecationOutput);
		lua_setfield(L, -2, "hasDeprecationOutput");
	}

	// Necessary for Data-creating methods to work properly in Data subclasses.
	love::luax_require(L, "love.data");
	lua_pop(L, 1);

#if LUA_VERSION_NUM <= 501
	// These are deprecated in Lua 5.1. LuaJIT 2.1 removes them, but code
	// written assuming LuaJIT 2.0 or Lua 5.1 is used might still rely on them.
	luax_addcompatibilityalias(L, "math", "fmod", "mod");
	luax_addcompatibilityalias(L, "string", "gmatch", "gfind");
#endif

#ifdef LOVE_ENABLE_LUASOCKET
	love::luasocket::__open(L);
#endif
#ifdef LOVE_ENABLE_ENET
	love::luax_preload(L, luaopen_enet, "enet");
#endif
#ifdef LOVE_ENABLE_LUA53
	love::luax_preload(L, luaopen_luautf8, "utf8");
#endif

#ifdef LOVE_ENABLE_WINDOW
	// In some environments, LuaJIT is limited to 2GB and LuaJIT sometimes panic when it
	// reaches OOM and closes the whole program, leaving the user confused about what's
	// going on.
	// We can't recover the state at this point, but it's better to inform user that
	// something very bad happening instead of silently exiting.
	// Note that this is not foolproof. In some cases, the whole process crashes by
	// uncaught exception that LuaJIT throws or simply exit as if calling
	// love.event.quit("not enough memory")
	lua_atpanic(L, [](lua_State *L)
	{
		using namespace love;
		using namespace love::window;

		char message[128];
		Window* windowModule = Module::getInstance<Window>(Module::M_WINDOW);

		snprintf(message, sizeof(message), "PANIC: unprotected error in call to Lua API (%s)", lua_tostring(L, -1));

		if (windowModule)
			windowModule->showMessageBox("Lua Fatal Error", message, Window::MESSAGEBOX_ERROR, windowModule->isOpen());

		fprintf(stderr, "%s\n", message);
		return 0;
	});
#endif

	return 1;
}

#ifdef LOVE_LEGENDARY_CONSOLE_IO_HACK

bool love_openConsole(const char *&err)
{
	static bool is_open = false;
	err = nullptr;

	if (is_open)
		return true;

	is_open = true;

	if (!AttachConsole(ATTACH_PARENT_PROCESS))
	{
		DWORD winerr = GetLastError();

		if (winerr == ERROR_ACCESS_DENIED)
		{
			// The process is already attached to a console. We'll assume stdout
			// and friends are already being directed there.
			is_open = true;
			return is_open;
		}

		// Create our own console if we can't attach to an existing one.
		if (!AllocConsole())
		{
			is_open = false;
			err = "Could not create console.";
			return is_open;
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
	if (fp == NULL)
	{
		err = "Console redirection of stdout failed.";
		return is_open;
	}

	// Redirect stdin.
	fp = freopen("CONIN$", "r", stdin);
	if (fp == NULL)
	{
		err = "Console redirection of stdin failed.";
		return is_open;
	}

	// Redirect stderr.
	fp = freopen("CONOUT$", "w", stderr);
	if (fp == NULL)
	{
		err = "Console redirection of stderr failed.";
		return is_open;
	}

	return is_open;
}

int w__openConsole(lua_State *L)
{
	const char *err = nullptr;
	bool isopen = love_openConsole(err);
	if (err != nullptr)
		return luaL_error(L, err);
	love::luax_pushboolean(L, isopen);
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
	if (luaL_loadbuffer(L, (const char *)love::nogame_lua, sizeof(love::nogame_lua), "=[love \"nogame.lua\"]") == 0)
		lua_call(L, 0, 1);

	return 1;
}

int luaopen_love_jitsetup(lua_State *L)
{
	if (luaL_loadbuffer(L, jit_setup_lua, sizeof(jit_setup_lua), "=[love \"jitsetup.lua\"]") == 0)
		lua_call(L, 0, 1);

	return 1;
}

int luaopen_love_arg(lua_State *L)
{
	if (luaL_loadbuffer(L, arg_lua, sizeof(arg_lua), "=[love \"arg.lua\"]") == 0)
		lua_call(L, 0, 1);

	return 1;
}

int luaopen_love_callbacks(lua_State *L)
{
	if (luaL_loadbuffer(L, callbacks_lua, sizeof(callbacks_lua), "=[love \"callbacks.lua\"]") == 0)
		lua_call(L, 0, 1);

	return 1;
}

int luaopen_love_boot(lua_State *L)
{
	if (luaL_loadbuffer(L, boot_lua, sizeof(boot_lua), "=[love \"boot.lua\"]") == 0)
		lua_call(L, 0, 1);

	return 1;
}
