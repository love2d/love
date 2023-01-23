#include "libretro.h"
#include "common/version.h"
#include "common/runtime.h"
#include "modules/love/love.h"

// Lua
extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

static lua_State *L = nullptr;

RETRO_API void retro_set_environment(retro_environment_t env) {
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t sendFrame)
{
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t sendAudioSample)
{
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t audioSampleBatch)
{
}

RETRO_API void retro_set_input_poll(retro_input_poll_t pollInput)
{
}

RETRO_API void retro_set_input_state(retro_input_state_t getInputState)
{
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

RETRO_API void retro_get_system_info(struct retro_system_info *info)
{
    info->library_name = "Love";
    info->library_version = love_version();
    info->need_fullpath = false;
    info->valid_extensions = "lua";
    info->block_extract = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->geometry.base_width = 256;
    info->geometry.base_height = 256;
    info->geometry.max_width = 256;
    info->geometry.max_height = 256;
    info->timing.fps = 60.0;
    info->timing.sample_rate = 48000;
}

RETRO_API size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

RETRO_API void *retro_get_memory_data(unsigned id)
{
    return nullptr;
}

RETRO_API void retro_reset()
{
}

static int love_preload(lua_State *L, lua_CFunction f, const char *name)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, f);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
	return 0;
}

RETRO_API void retro_init() {
	// Create the virtual machine.
	L = luaL_newstate();
	luaL_openlibs(L);

	// LuaJIT-specific setup needs to be done as early as possible - before
	// get_app_arguments because that loads external library code. This is also
	// loaded inside require("love"). Note that it doesn't use the love table.
	love_preload(L, luaopen_love_jitsetup, "love.jitsetup");
	lua_getglobal(L, "require");
	lua_pushstring(L, "love.jitsetup");
	lua_call(L, 1, 0);
}

RETRO_API void retro_deinit() {
    lua_close(L);
    L = nullptr;
}

RETRO_API bool retro_load_game(const struct retro_game_info *game)
{
	int argc = 0;
	char **argv = nullptr;

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

    return true;
}

RETRO_API void retro_unload_game()
{
}

RETRO_API void retro_run() {
    if (!L) {
        return;
    }

	int stackpos = lua_gettop(L);
	int nres;
	love::luax_resume(L, 0, &nres);
#if LUA_VERSION_NUM >= 504
    lua_pop(L, nres);
#else
    lua_pop(L, lua_gettop(L) - stackpos);
#endif
}
