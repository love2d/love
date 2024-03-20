/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#pragma once

#include "common/runtime.h"
#include "SpriteBatch.h"
#include "math/wrap_Transform.h"

namespace love
{
namespace graphics
{

template <typename T>
static void luax_checkstandardtransform(lua_State *L, int idx, const T &func)
{
	math::Transform *tf = luax_totype<math::Transform>(L, idx);
	if (tf != nullptr)
	{
		func(tf->getMatrix());
	}
	else
	{
		int nargs = lua_gettop(L);
		float x  = (float) luaL_optnumber(L, idx + 0, 0.0);
		float y  = (float) luaL_optnumber(L, idx + 1, 0.0);
		float a  = (float) luaL_optnumber(L, idx + 2, 0.0);
		float sx = (float) luaL_optnumber(L, idx + 3, 1.0);
		float sy = (float) luaL_optnumber(L, idx + 4, sx);
		float ox = 0.0f;
		float oy = 0.0f;
		if (nargs >= idx + 5)
		{
			ox = (float) luaL_optnumber(L, idx + 5, 0.0);
			oy = (float) luaL_optnumber(L, idx + 6, 0.0);
		}
		float kx = 0.0f;
		float ky = 0.0f;
		if (nargs >= idx + 7)
		{
			kx = (float) luaL_optnumber(L, idx + 7, 0.0);
			ky = (float) luaL_optnumber(L, idx + 8, 0.0);
		}
		func(Matrix4(x, y, a, sx, sy, ox, oy, kx, ky));
	}
}

SpriteBatch *luax_checkspritebatch(lua_State *L, int idx);
extern "C" int luaopen_spritebatch(lua_State *L);

} // graphics
} // love
