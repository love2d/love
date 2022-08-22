/**
 * Copyright (c) 2006-2022 LOVE Development Team
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

#define instance() (Module::getInstance<System>(Module::M_SYSTEM))

int w_getOS(lua_State *L)
{
	luax_pushstring(L, instance()->getOS());
	return 1;
}

int w_getProcessorCount(lua_State *L)
{
	lua_pushinteger(L, instance()->getProcessorCount());
	return 1;
}

int w_setClipboardText(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);
	luax_catchexcept(L, [&]() { instance()->setClipboardText(text); });
	return 0;
}

int w_getClipboardText(lua_State *L)
{
	std::string text;
	luax_catchexcept(L, [&]() { text = instance()->getClipboardText(); });
	luax_pushstring(L, text);
	return 1;
}

int w_getPowerInfo(lua_State *L)
{
	int seconds = -1, percent = -1;
	const char *str;

	System::PowerState state = instance()->getPowerInfo(seconds, percent);

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

int w_openURL(lua_State *L)
{
	std::string url = luax_checkstring(L, 1);
	luax_pushboolean(L, instance()->openURL(url));
	return 1;
}

int w_vibrate(lua_State *L)
{
	double seconds = luaL_optnumber(L, 1, 0.5);
	instance()->vibrate(seconds);
	return 0;
}

int w_hasBackgroundMusic(lua_State *L)
{
	lua_pushboolean(L, instance()->hasBackgroundMusic());
	return 1;
}

int w_getPreferredLocales(lua_State* L)
{
	int i = 1;
	std::vector<std::string> locales = instance()->getPreferredLocales();

	lua_createtable(L, locales.size(), 0);

	for (const std::string& str: locales)
	{
		luax_pushstring(L, str);
		lua_rawseti(L, -2, i++);
	}

	return 1;
}

int w_getLibraryVersions(lua_State *L)
{
	bool istable = lua_istable(L, 1);
	int num = istable ? (int) luax_objlen(L, 1) : lua_gettop(L);

	System::Library lib;
	std::vector<System::Library> libraries;
	libraries.reserve(num);

	if (num == 0) {
		libraries.reserve(System::LIBRARY_MAX_ENUM);
		libraries = {
			System::LIBRARY_LOVE,
			System::LIBRARY_FREETYPE,
			System::LIBRARY_LUA,
			System::LIBRARY_SDL,
			System::LIBRARY_THEORA,
			System::LIBRARY_ZLIB,
		};
	}

	const char *name;
	for (int i = 0; i < num; i++)
	{
		if (istable)
		{
			lua_rawgeti(L, 1, i + 1);
			name = luaL_checkstring(L, -1);
			lua_pop(L, 1);
		}
		else
			name = luaL_checkstring(L, i + 1);
		if (!System::getConstant(name, lib))
			return luax_enumerror(L, "library name", name);

		libraries.push_back(lib);
	}


	std::map<System::Library,std::string> versions = instance()->getLibraryVersions(libraries);
	if (num == 1)
		luax_pushstring(L, versions.begin()->second);
	else
	{
		std::map<System::Library,std::string>::iterator i;
		lua_createtable(L, 0, num);
		for (i = versions.begin(); i != versions.end(); i++)
		{
			if (!System::getConstant(i->first, name))
				return luaL_error(L, "Internal error: unknown library name");
			luax_pushstring(L, i->second);
			lua_setfield(L, -2, name);
		}
	}

	return 1;
}

static const luaL_Reg functions[] =
{
	{ "getOS", w_getOS },
	{ "getProcessorCount", w_getProcessorCount },
	{ "setClipboardText", w_setClipboardText },
	{ "getClipboardText", w_getClipboardText },
	{ "getPowerInfo", w_getPowerInfo },
	{ "openURL", w_openURL },
	{ "vibrate", w_vibrate },
	{ "hasBackgroundMusic", w_hasBackgroundMusic },
	{ "getPreferredLocales", w_getPreferredLocales },
	{ "getLibraryVersions", w_getLibraryVersions },
	{ 0, 0 }
};

extern "C" int luaopen_love_system(lua_State *L)
{
	System *instance = instance();
	if (instance == nullptr)
	{
		instance = new love::system::sdl::System();
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "system";
	w.type = &Module::type;
	w.functions = functions;
	w.types = nullptr;

	return luax_register_module(L, w);
}

} // system
} // love
