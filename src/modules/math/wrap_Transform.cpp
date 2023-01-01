/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "wrap_Transform.h"

namespace love
{
namespace math
{

Transform *luax_checktransform(lua_State *L, int idx)
{
	return luax_checktype<Transform>(L, idx, Transform::type);
}

int w_Transform_clone(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	Transform *newtransform = t->clone();
	luax_pushtype(L, newtransform);
	newtransform->release();
	return 1;
}

int w_Transform_inverse(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	Transform *inverse = t->inverse();
	luax_pushtype(L, inverse);
	inverse->release();
	return 1;
}

int w_Transform_apply(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	Transform *other = luax_checktransform(L, 2);
	t->apply(other);
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_isAffine2DTransform(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	luax_pushboolean(L, t->getMatrix().isAffine2DTransform());
	return 1;
}

int w_Transform_translate(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	float x = (float) luaL_checknumber(L, 2);
	float y = (float) luaL_checknumber(L, 3);
	t->translate(x, y);
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_rotate(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	float angle = (float) luaL_checknumber(L, 2);
	t->rotate(angle);
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_scale(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	float sx = (float) luaL_checknumber(L, 2);
	float sy = (float) luaL_optnumber(L, 3, sx);
	t->scale(sx, sy);
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_shear(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	float kx = (float) luaL_checknumber(L, 2);
	float ky = (float) luaL_checknumber(L, 3);
	t->shear(kx, ky);
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_reset(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	t->reset();
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_setTransformation(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	float x =  (float) luaL_optnumber(L, 2, 0.0);
	float y =  (float) luaL_optnumber(L, 3, 0.0);
	float a =  (float) luaL_optnumber(L, 4, 0.0);
	float sx = (float) luaL_optnumber(L, 5, 1.0);
	float sy = (float) luaL_optnumber(L, 6, sx);
	float ox = (float) luaL_optnumber(L, 7, 0.0);
	float oy = (float) luaL_optnumber(L, 8, 0.0);
	float kx = (float) luaL_optnumber(L, 9, 0.0);
	float ky = (float) luaL_optnumber(L, 10, 0.0);
	t->setTransformation(x, y, a, sx, sy, ox, oy, kx, ky);
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_setMatrix(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);

	bool columnmajor = false;

	int idx = 2;
	if (lua_type(L, idx) == LUA_TSTRING)
	{
		const char *layoutstr = lua_tostring(L, idx);
		Transform::MatrixLayout layout;
		if (!Transform::getConstant(layoutstr, layout))
			return luax_enumerror(L, "matrix layout", Transform::getConstants(layout), layoutstr);

		columnmajor = (layout == Transform::MATRIX_COLUMN_MAJOR);
		idx++;
	}

	float elements[16];

	if (lua_istable(L, idx))
	{
		lua_rawgeti(L, idx, 1);
		bool tableoftables = lua_istable(L, -1);
		lua_pop(L, 1);

		if (tableoftables)
		{
			if (columnmajor)
			{
				for (int column = 0; column < 4; column++)
				{
					lua_rawgeti(L, idx, column + 1);

					for (int row = 0; row < 4; row++)
					{
						lua_rawgeti(L, -(row + 1), row + 1);
						elements[column * 4 + row] = (float) luaL_checknumber(L, -1);
					}

					lua_pop(L, 4 + 1);
				}
			}
			else
			{
				for (int row = 0; row < 4; row++)
				{
					lua_rawgeti(L, idx, row + 1);

					for (int column = 0; column < 4; column++)
					{
						// The table has the matrix elements laid out in row-major
						// order, but we need to store them column-major in memory.
						lua_rawgeti(L, -(column + 1), column + 1);
						elements[column * 4 + row] = (float) luaL_checknumber(L, -1);
					}

					lua_pop(L, 4 + 1);
				}
			}
		}
		else
		{
			if (columnmajor)
			{
				for (int column = 0; column < 4; column++)
				{
					for (int row = 0; row < 4; row++)
					{
						lua_rawgeti(L, idx, column * 4 + row + 1);
						elements[column * 4 + row] = (float) luaL_checknumber(L, -1);
					}
				}
			}
			else
			{
				for (int column = 0; column < 4; column++)
				{
					for (int row = 0; row < 4; row++)
					{
						// The table has the matrix elements laid out in row-major
						// order, but we need to store them column-major in memory.
						lua_rawgeti(L, idx, row * 4 + column + 1);
						elements[column * 4 + row] = (float) luaL_checknumber(L, -1);
					}
				}
			}

			lua_pop(L, 16);
		}
	}
	else
	{
		if (columnmajor)
		{
			for (int i = 0; i < 16; i++)
				elements[i] = (float) luaL_checknumber(L, idx + i);
		}
		else
		{
			for (int column = 0; column < 4; column++)
			{
				for (int row = 0; row < 4; row++)
					elements[column * 4 + row] = (float) luaL_checknumber(L, row * 4 + column + idx);
			}
		}
	}

	t->setMatrix(Matrix4(elements));
	lua_pushvalue(L, 1);
	return 1;
}

int w_Transform_getMatrix(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	const float *elements = t->getMatrix().getElements();

	// We want to push elements in row-major order, but they're stored column-
	// major.
	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
			lua_pushnumber(L, elements[column * 4 + row]);
	}

	return 16;
}

int w_Transform_transformPoint(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	love::Vector2 p;
	p.x = (float) luaL_checknumber(L, 2);
	p.y = (float) luaL_checknumber(L, 3);
	p = t->transformPoint(p);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	return 2;
}

int w_Transform_inverseTransformPoint(lua_State *L)
{
	Transform *t = luax_checktransform(L, 1);
	love::Vector2 p;
	p.x = (float) luaL_checknumber(L, 2);
	p.y = (float) luaL_checknumber(L, 3);
	p = t->inverseTransformPoint(p);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	return 2;
}

int w_Transform__mul(lua_State *L)
{
	Transform *t1 = luax_checktransform(L, 1);
	Transform *t2 = luax_checktransform(L, 2);
	Transform *t3 = new Transform(t1->getMatrix() * t2->getMatrix());
	luax_pushtype(L, t3);
	t3->release();
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "clone", w_Transform_clone },
	{ "inverse", w_Transform_inverse },
	{ "apply", w_Transform_apply },
	{ "isAffine2DTransform", w_Transform_isAffine2DTransform },
	{ "translate", w_Transform_translate },
	{ "rotate", w_Transform_rotate },
	{ "scale", w_Transform_scale },
	{ "shear", w_Transform_shear },
	{ "reset", w_Transform_reset },
	{ "setTransformation", w_Transform_setTransformation },
	{ "setMatrix", w_Transform_setMatrix },
	{ "getMatrix", w_Transform_getMatrix },
	{ "transformPoint", w_Transform_transformPoint },
	{ "inverseTransformPoint", w_Transform_inverseTransformPoint },
	{ "__mul", w_Transform__mul },
	{ 0, 0 }
};

extern "C" int luaopen_transform(lua_State *L)
{
	return luax_register_type(L, &Transform::type, functions, nullptr);
}

} // math
} // love
