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

#include "common/Exception.h"
#include "wrap_BezierCurve.h"

#include <cmath>

namespace love
{
namespace math
{

BezierCurve *luax_checkbeziercurve(lua_State *L, int idx)
{
	return luax_checktype<BezierCurve>(L, idx);
}

int w_BezierCurve_getDegree(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	lua_pushnumber(L, curve->getDegree());
	return 1;
}

int w_BezierCurve_getDerivative(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	BezierCurve *deriv = new BezierCurve(curve->getDerivative());
	luax_pushtype(L, deriv);
	deriv->release();
	return 1;
}

int w_BezierCurve_getControlPoint(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	int idx = (int) luaL_checkinteger(L, 2);

	if (idx > 0) // 1-indexing
		idx--;

	luax_catchexcept(L, [&]() {
		Vector2 v = curve->getControlPoint(idx);
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
	});

	return 2;
}

int w_BezierCurve_setControlPoint(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	int idx = (int) luaL_checkinteger(L, 2);
	float vx = (float) luaL_checknumber(L, 3);
	float vy = (float) luaL_checknumber(L, 4);

	if (idx > 0) // 1-indexing
		idx--;

	luax_catchexcept(L, [&](){ curve->setControlPoint(idx, Vector2(vx,vy)); });
	return 0;
}

int w_BezierCurve_insertControlPoint(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	float vx = (float) luaL_checknumber(L, 2);
	float vy = (float) luaL_checknumber(L, 3);
	int idx = (int) luaL_optinteger(L, 4, -1);

	if (idx > 0) // 1-indexing
		idx--;

	luax_catchexcept(L, [&](){ curve->insertControlPoint(Vector2(vx,vy), idx); });
	return 0;
}

int w_BezierCurve_removeControlPoint(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	int idx = (int) luaL_checkinteger(L, 2);

	if (idx > 0) // 1-indexing
		idx--;

	luax_catchexcept(L, [&](){ curve->removeControlPoint(idx); });
	return 0;
}

int w_BezierCurve_getControlPointCount(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	lua_pushinteger(L, curve->getControlPointCount());
	return 1;
}

int w_BezierCurve_translate(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	float dx = (float) luaL_checknumber(L, 2);
	float dy = (float) luaL_checknumber(L, 3);
	curve->translate(Vector2(dx,dy));
	return 0;
}

int w_BezierCurve_rotate(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	double phi = luaL_checknumber(L, 2);
	float ox = (float) luaL_optnumber(L, 3, 0);
	float oy = (float) luaL_optnumber(L, 4, 0);
	curve->rotate(phi, Vector2(ox,oy));
	return 0;
}

int w_BezierCurve_scale(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	double s = luaL_checknumber(L, 2);
	float ox = (float) luaL_optnumber(L, 3, 0);
	float oy = (float) luaL_optnumber(L, 4, 0);
	curve->scale(s, Vector2(ox,oy));
	return 0;
}

int w_BezierCurve_evaluate(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	double t = luaL_checknumber(L, 2);

	luax_catchexcept(L, [&]() {
		Vector2 v = curve->evaluate(t);
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
	});

	return 2;

}

int w_BezierCurve_getSegment(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	double t1 = luaL_checknumber(L, 2);
	double t2 = luaL_checknumber(L, 3);

	BezierCurve *segment;
	luax_catchexcept(L, [&](){ segment = curve->getSegment(t1, t2); });
	luax_pushtype(L, segment);
	segment->release();

	return 1;
}

int w_BezierCurve_render(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	int accuracy = (int) luaL_optinteger(L, 2, 5);

	std::vector<Vector2> points;
	luax_catchexcept(L, [&](){ points = curve->render(accuracy); });

	lua_createtable(L, (int) points.size() * 2, 0);
	for (int i = 0; i < (int) points.size(); ++i)
	{
		lua_pushnumber(L, points[i].x);
		lua_rawseti(L, -2, 2*i+1);
		lua_pushnumber(L, points[i].y);
		lua_rawseti(L, -2, 2*i+2);
	}

	return 1;
}

int w_BezierCurve_renderSegment(lua_State *L)
{
	BezierCurve *curve = luax_checkbeziercurve(L, 1);
	double start = luaL_checknumber(L, 2);
	double end = luaL_checknumber(L, 3);
	int accuracy = (int) luaL_optinteger(L, 4, 5);

	std::vector<Vector2> points;
	luax_catchexcept(L, [&](){ points = curve->renderSegment(start, end, accuracy); });

	lua_createtable(L, (int) points.size() * 2, 0);
	for (int i = 0; i < (int) points.size(); ++i)
	{
		lua_pushnumber(L, points[i].x);
		lua_rawseti(L, -2, 2*i+1);
		lua_pushnumber(L, points[i].y);
		lua_rawseti(L, -2, 2*i+2);
	}

	return 1;
}

static const luaL_Reg w_BezierCurve_functions[] =
{
	{"getDegree", w_BezierCurve_getDegree},
	{"getDerivative", w_BezierCurve_getDerivative},
	{"getControlPoint", w_BezierCurve_getControlPoint},
	{"setControlPoint", w_BezierCurve_setControlPoint},
	{"insertControlPoint", w_BezierCurve_insertControlPoint},
	{"removeControlPoint", w_BezierCurve_removeControlPoint},
	{"getControlPointCount", w_BezierCurve_getControlPointCount},
	{"translate", w_BezierCurve_translate},
	{"rotate", w_BezierCurve_rotate},
	{"scale", w_BezierCurve_scale},
	{"evaluate", w_BezierCurve_evaluate},
	{"getSegment", w_BezierCurve_getSegment},
	{"render", w_BezierCurve_render},
	{"renderSegment", w_BezierCurve_renderSegment},
	{ 0, 0 }
};

extern "C" int luaopen_beziercurve(lua_State *L)
{
	return luax_register_type(L, &BezierCurve::type, w_BezierCurve_functions, nullptr);
}

} // math
} // love
