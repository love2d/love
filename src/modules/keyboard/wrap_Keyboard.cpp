/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "common/config.h"

#include "wrap_Keyboard.h"

#include "sdl/Keyboard.h"

namespace love
{
namespace keyboard
{

#define instance() (Module::getInstance<Keyboard>(Module::M_KEYBOARD))

int w_setKeyRepeat(lua_State *L)
{
	instance()->setKeyRepeat(luax_toboolean(L, 1));
	return 0;
}

int w_hasKeyRepeat(lua_State *L)
{
	luax_pushboolean(L, instance()->hasKeyRepeat());
	return 1;
}

int w_isDown(lua_State *L)
{
	Keyboard::Key k;
	unsigned int num = lua_gettop(L);
	Keyboard::Key *keylist = new Keyboard::Key[num+1];
	unsigned int counter = 0;

	for (unsigned int i = 0; i < num; i++)
	{
		if (Keyboard::getConstant(luaL_checkstring(L, i+1), k))
			keylist[counter++] = k;
	}
	keylist[counter] = Keyboard::KEY_MAX_ENUM;

	luax_pushboolean(L, instance()->isDown(keylist));
	delete[] keylist;
	return 1;
}

int w_setTextInput(lua_State *L)
{
	instance()->setTextInput(luax_toboolean(L, 1));
	return 0;
}

int w_hasTextInput(lua_State *L)
{
	luax_pushboolean(L, instance()->hasTextInput());
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "setKeyRepeat", w_setKeyRepeat },
	{ "hasKeyRepeat", w_hasKeyRepeat },
	{ "setTextInput", w_setTextInput },
	{ "hasTextInput", w_hasTextInput },
	{ "isDown", w_isDown },
	{ 0, 0 }
};

extern "C" int luaopen_love_keyboard(lua_State *L)
{
	Keyboard *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::keyboard::sdl::Keyboard(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "keyboard";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // keyboard
} // love
