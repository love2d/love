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

#include <common/config.h>

#include "sdl/Mouse.h"

#include "wrap_Mouse.h"

namespace love
{
namespace mouse
{
	static Mouse * instance = 0;

	int w_getX(lua_State * L)
	{
		lua_pushnumber(L, instance->getX());
		return 1;
	}

	int w_getY(lua_State * L)
	{
		lua_pushnumber(L, instance->getY());
		return 1;
	}

	int w_getPosition(lua_State * L)
	{
		int x, y;
		instance->getPosition(x, y);
		lua_pushinteger(L, x);
		lua_pushinteger(L, y);
		return 2;
	}

	int w_setPosition(lua_State * L)
	{
		int x = luaL_checkint(L, 1);
		int y = luaL_checkint(L, 2);
		instance->setPosition(x, y);
		return 0;
	}

	int w_isDown(lua_State * L)
	{
		Mouse::Button b;
		unsigned int num = lua_gettop(L);
		Mouse::Button * buttonlist = new Mouse::Button[num+1];
		unsigned int counter = 0;

		for (unsigned int i = 0; i < num; i++)
		{
			if (Mouse::getConstant(luaL_checkstring(L, i+1), b))
				buttonlist[counter++] = b;
		}
		buttonlist[counter] = Mouse::BUTTON_MAX_ENUM;

		luax_pushboolean(L, instance->isDown(buttonlist));
		delete[] buttonlist;
		return 1;
	}

	int w_setVisible(lua_State * L)
	{
		bool b = luax_toboolean(L, 1);
		instance->setVisible(b);
		return 0;
	}

	int w_isVisible(lua_State * L)
	{
		luax_pushboolean(L, instance->isVisible());
		return 1;
	}

	int w_setGrab(lua_State * L)
	{
		bool b = luax_toboolean(L, 1);
		instance->setGrab(b);
		return 0;
	}

	int w_isGrabbed(lua_State * L)
	{
		luax_pushboolean(L, instance->isGrabbed());
		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "getX", w_getX },
		{ "getY", w_getY },
		{ "setPosition", w_setPosition },
		{ "isDown", w_isDown },
		{ "setVisible", w_setVisible },
		{ "isVisible", w_isVisible },
		{ "getPosition", w_getPosition },
		{ "setGrab", w_setGrab },
		{ "isGrabbed", w_isGrabbed },
		{ 0, 0 }
	};

	int luaopen_love_mouse(lua_State * L)
	{
		if (instance == 0)
		{
			try
			{
				instance = new love::mouse::sdl::Mouse();
			}
			catch (Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}
		else
			instance->retain();

		WrappedModule w;
		w.module = instance;
		w.name = "mouse";
		w.flags = MODULE_T;
		w.functions = functions;
		w.types = 0;

		return luax_register_module(L, w);
	}

} // mouse
} // love
