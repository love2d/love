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

// LOVE
#include "wrap_Animation.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Animation * luax_checkanimation(lua_State * L, int idx)
	{
		return luax_checktype<Animation>(L, idx, "Animation", LOVE_GRAPHICS_ANIMATION_BITS);
	}

	int _wrap_Animation_addFrame(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		float w = (float)luaL_checknumber(L, 4);
		float h = (float)luaL_checknumber(L, 5);
		float d = (float)luaL_checknumber(L, 6);
		t->addFrame(x, y, w, h, d);
		return 0;
	}

	int _wrap_Animation_play(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		t->play();
		return 0;
	}

	int _wrap_Animation_stop(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		t->stop();
		return 0;
	}

	int _wrap_Animation_reset(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		t->reset();
		return 0;
	}

	int _wrap_Animation_seek(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		int frame = luaL_checkint(L, 2);
		t->seek(frame);
		return 0;
	}

	int _wrap_Animation_getCurrentFrame(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		lua_pushnumber(L, t->getCurrentFrame());
		return 1;
	}

	int _wrap_Animation_getSize(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		lua_pushnumber(L, t->getSize());
		return 1;
	}

	int _wrap_Animation_setDelay(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		int frame = luaL_checkint(L, 2);
		float delay = (float)luaL_checknumber(L, 3);
		t->setDelay(frame, delay);
		return 0;
	}

	int _wrap_Animation_setSpeed(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		float speed = (float)luaL_checknumber(L, 2);
		t->setSpeed(speed);
		return 0;
	}

	int _wrap_Animation_getSpeed(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		lua_pushnumber(L, t->getSpeed());
		return 1;
	}

	int _wrap_Animation_update(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		float dt = (float)luaL_checknumber(L, 2);
		t->update(dt);
		return 0;
	}

	int _wrap_Animation_getWidth(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		lua_pushnumber(L, t->getWidth());
		return 1;
	}

	int _wrap_Animation_getHeight(lua_State * L)
	{
		Animation * t = luax_checkanimation(L, 1);
		lua_pushnumber(L, t->getHeight());
		return 1;
	}

	static const luaL_Reg wrap_Animation_functions[] = {
		{ "addFrame", _wrap_Animation_addFrame },
		{ "play", _wrap_Animation_play },
		{ "stop", _wrap_Animation_stop },
		{ "reset", _wrap_Animation_reset },
		{ "seek", _wrap_Animation_seek },
		{ "getCurrentFrame", _wrap_Animation_getCurrentFrame },
		{ "getSize", _wrap_Animation_getSize },
		{ "setDelay", _wrap_Animation_setDelay },
		{ "setSpeed", _wrap_Animation_setSpeed },
		{ "getSpeed", _wrap_Animation_getSpeed },
		{ "update", _wrap_Animation_update },
		{ "getWidth", _wrap_Animation_getWidth },
		{ "getHeight", _wrap_Animation_getHeight },
		{ 0, 0 }
	};

	int wrap_Animation_open(lua_State * L)
	{
		luax_register_type(L, "Animation", wrap_Animation_functions);
		return 0;
	}

} // opengl
} // graphics
} // love
