/**
 * Copyright (c) 2006-2026 LOVE Development Team
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
#include "wrap_Speech.h"

namespace love
{
namespace speech
{

#define instance() (Module::getInstance<Speech>(Module::M_SPEECH))

int w_speak(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);
	const bool interrupt = luax_optboolean(L, 2, false);
	luax_pushboolean(L, instance()->speak(text, interrupt));
	return 1;
}

int w_stop(lua_State *L)
{
	instance()->stop();
	return 0;
}

int w_setRoute(lua_State *L)
{
	Speech::Route route;
	const char *routestr = luaL_checkstring(L, 1);
	if (!Speech::getConstant(routestr, route))
		return luax_enumerror(L, "speech route", Speech::getConstants(route), routestr);

	instance()->setRoute(route);

	return 0;
}

int w_getOutput(lua_State *L)
{
	const char *outputstr = nullptr;
	Speech::getConstant(instance()->getOutput(), outputstr);

	lua_pushstring(L, outputstr);
	return 1;
}

int w_getBackendName(lua_State *L)
{
	const char *backend = instance()->getBackendName();
	if (backend != nullptr)
		lua_pushstring(L, backend);
	else
		lua_pushnil(L);
	return 1;
}

int w_setRate(lua_State *L)
{
	float rate = (float) luaL_checknumber(L, 1);
	instance()->setRate(rate);
	return 0;
}

int w_setVolume(lua_State *L)
{
	float volume = (float) luaL_checknumber(L, 1);
	instance()->setVolume(volume);
	return 0;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "speak", w_speak },
	{ "stop", w_stop },
	{ "setRoute", w_setRoute },
	{ "getOutput", w_getOutput },
	{ "getBackendName", w_getBackendName },
	{ "setRate", w_setRate },
	{ "setVolume", w_setVolume },
	{ 0, 0 }
};


extern "C" int luaopen_love_speech(lua_State *L)
{
	Speech *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::speech::Speech(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "speech";
	w.type = &Module::type;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // speech
} // love
