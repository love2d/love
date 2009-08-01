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

// STD

// SDL
#include <SDL.h>

// LOVE
#include <common/config.h>
#include <common/runtime.h>
#include <common/constants.h>
#include <common/MemoryData.h>

// Modules
#include <audio/wrap_Audio.h>
#include <event/sdl/wrap_Event.h>
#include <filesystem/physfs/wrap_Filesystem.h>
#include <graphics/opengl/wrap_Graphics.h>
#include <image/wrap_Image.h>
#include <joystick/sdl/wrap_Joystick.h>
#include <keyboard/sdl/wrap_Keyboard.h>
#include <mouse/sdl/wrap_Mouse.h>
#include <native/tcc/wrap_Native.h>
#include <physics/box2d/wrap_Physics.h>
#include <sound/wrap_Sound.h>
#include <timer/sdl/wrap_Timer.h>
#include <font/freetype/wrap_Font.h>

// Libraries.
#include "libraries/luasocket/luasocket.h"
#include "libraries/lanes/lanes.h"

// Resources
#include "resources/resources.h"

DECLSPEC int luaopen_love(lua_State * L)
{
	// Create the love table.
	lua_newtable(L);

	// Install constants.
	for(int i = 0; love::lua_constants[i].name != 0; i++)
	{
		lua_pushinteger(L, love::lua_constants[i].value);
		lua_setfield(L, -2, love::lua_constants[i].name);
	}

	// Create the _fin table.
	lua_newtable(L);
	lua_setfield(L, -2, "_fin");

	// Resources.
	love::luax_newtype(L, "Data", love::LOVE_DATA_BITS, new love::MemoryData((void*)love::Vera_ttf_data, love::Vera_ttf_size));
	lua_setfield(L, -2, "_vera");

	// Set the love table.
	lua_setglobal(L, "love");

	love::luax_preload(L, love::audio::wrap_Audio_open, "love.audio");
	love::luax_preload(L, love::filesystem::physfs::wrap_Filesystem_open, "love.filesystem");
	love::luax_preload(L, love::event::sdl::wrap_Event_open, "love.event");
	love::luax_preload(L, love::keyboard::sdl::wrap_Keyboard_open, "love.keyboard");
	love::luax_preload(L, love::mouse::sdl::wrap_Mouse_open, "love.mouse");
	love::luax_preload(L, love::native::tcc::wrap_Native_open, "love.native");
	love::luax_preload(L, love::timer::sdl::wrap_Timer_open, "love.timer");
	love::luax_preload(L, love::joystick::sdl::wrap_Joystick_open, "love.joystick");
	love::luax_preload(L, love::graphics::opengl::wrap_Graphics_open, "love.graphics");
	love::luax_preload(L, love::image::wrap_Image_open, "love.image");
	love::luax_preload(L, love::physics::box2d::wrap_Physics_open, "love.physics");
	love::luax_preload(L, love::sound::wrap_Sound_open, "love.sound");
	love::luax_preload(L, love::font::freetype::wrap_Font_open, "love.font");

	love::luasocket::__open(L);
	love::lanes::open(L);

	return 0;
}

#if LOVE_BUILD_EXE

int main(int argc, char ** argv)
{
	// Create the virtual machine.
	lua_State * L = lua_open();
	luaL_openlibs(L);

	love::luax_preload(L, luaopen_love, "love");

	luaopen_love(L);	

	// Add command line arguments to love.__args
	{
		lua_getglobal(L, "love");
		lua_newtable(L);
		for(int i = 0;i<argc;i++)
		{
			lua_pushstring(L, argv[i]);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, -2, "_args");
		lua_pop(L, 1);
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

	// This is where we should run the built-in Lua code
	// which gets everything started.

	// TODO: This is obviously test code.
	luaL_dofile(L, "../../src/scripts/boot.lua");
//#	include "scripts/boot.lua.h"

	lua_close(L);

#if defined(LOVE_DEBUG) && defined(LOVE_WINDOWS)
	printf("(press key)\n");
	getchar();
#endif
	printf("Done. This was: %s (%s)\n", LOVE_VERSION_STR.c_str(), LOVE_VERSION_CODENAME.c_str());
	return 0;
}

#endif // LOVE_BUILD_EXE
