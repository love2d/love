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
#include "wrap_System.h"
#include "sdl/System.h"

namespace love
{
namespace system
{

static System *instance = 0;

int w_getOS(lua_State *L)
{
	luax_pushstring(L, instance->getOS());
	return 1;
}

int w_getProcessorCount(lua_State *L)
{
	lua_pushinteger(L, instance->getProcessorCount());
	return 1;
}

int w_setClipboardText(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);
	instance->setClipboardText(text);
	return 0;
}

int w_getClipboardText(lua_State *L)
{
	luax_pushstring(L, instance->getClipboardText());
	return 1;
}

int w_getPowerInfo(lua_State *L)
{
	int seconds = -1, percent = -1;
	const char *str;

	System::PowerState state = instance->getPowerInfo(seconds, percent);

	if (!System::getConstant(state, str))
		str = "unknown";

	lua_pushstring(L, str);

	if (percent >= 0)
		lua_pushinteger(L, percent);
	else
		lua_pushnil(L);

	if (seconds >= 0)
		lua_pushinteger(L, seconds);
	else
		lua_pushnil(L);

	return 3;
}

static const luaL_Reg functions[] =
{
	{ "getOS", w_getOS },
	{ "getProcessorCount", w_getProcessorCount },
	{ "setClipboardText", w_setClipboardText },
	{ "getClipboardText", w_getClipboardText },
	{ "getPowerInfo", w_getPowerInfo },
	{ 0, 0 }
};

extern "C" int luaopen_love_system(lua_State *L)
{
	if (instance == 0)
	{
		instance = new love::system::sdl::System();
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "system";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // system
} // love