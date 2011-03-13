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

#include "wrap_Shape.h"
#include <common/StringMap.h>

namespace love
{
namespace physics
{
namespace box2d
{
	Shape * luax_checkshape(lua_State * L, int idx)
	{
		return luax_checktype<Shape>(L, idx, "Shape", PHYSICS_SHAPE_T);
	}

	int w_Shape_getType(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		const char * type = "";
		Shape::getConstant(t->getType(), type);
		lua_pushstring(L, type);
		return 1;
	}

	int w_Shape_destroy(lua_State * L)
	{
		Proxy * p = (Proxy *)lua_touserdata(L, 1);
		p->own = false;

		Shape * t = (Shape *)p->data;
		t->release();
		return 0;
	}

	static const luaL_Reg functions[] = {
		{ "getType", w_Shape_getType },
		{ "destroy", w_Shape_destroy },
		{ 0, 0 }
	};

	int luaopen_shape(lua_State * L)
	{
		return luax_register_type(L, "Shape", functions);
	}

} // box2d
} // physics
} // love

