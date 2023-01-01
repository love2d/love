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

#include "wrap_Text.h"
#include "wrap_Font.h"
#include "math/wrap_Transform.h"

namespace love
{
namespace graphics
{

Text *luax_checktext(lua_State *L, int idx)
{
	return luax_checktype<Text>(L, idx);
}

int w_Text_set(lua_State *L)
{
	Text *t = luax_checktext(L, 1);

	std::vector<Font::ColoredString> newtext;
	luax_checkcoloredstring(L, 2, newtext);

	luax_catchexcept(L, [&](){ t->set(newtext); });
	return 0;
}

int w_Text_setf(lua_State *L)
{
	Text *t = luax_checktext(L, 1);

	float wraplimit = (float) luaL_checknumber(L, 3);

	Font::AlignMode align;
	const char *alignstr = luaL_checkstring(L, 4);
	if (!Font::getConstant(alignstr, align))
		return luax_enumerror(L, "align mode", Font::getConstants(align), alignstr);

	std::vector<Font::ColoredString> newtext;
	luax_checkcoloredstring(L, 2, newtext);

	luax_catchexcept(L, [&](){ t->set(newtext, wraplimit, align); });

	return 0;
}

int w_Text_add(lua_State *L)
{
	Text *t = luax_checktext(L, 1);

	int index = 0;

	std::vector<Font::ColoredString> text;
	luax_checkcoloredstring(L, 2, text);

	if (luax_istype(L, 3, math::Transform::type))
	{
		math::Transform *tf = luax_totype<math::Transform>(L, 3);
		luax_catchexcept(L, [&](){ index = t->add(text, tf->getMatrix()); });
	}
	else
	{
		float x  = (float) luaL_optnumber(L, 3, 0.0);
		float y  = (float) luaL_optnumber(L, 4, 0.0);
		float a  = (float) luaL_optnumber(L, 5, 0.0);
		float sx = (float) luaL_optnumber(L, 6, 1.0);
		float sy = (float) luaL_optnumber(L, 7, sx);
		float ox = (float) luaL_optnumber(L, 8, 0.0);
		float oy = (float) luaL_optnumber(L, 9, 0.0);
		float kx = (float) luaL_optnumber(L, 10, 0.0);
		float ky = (float) luaL_optnumber(L, 11, 0.0);

		Matrix4 m(x, y, a, sx, sy, ox, oy, kx, ky);
		luax_catchexcept(L, [&](){ index = t->add(text, m); });
	}

	lua_pushnumber(L, index + 1);
	return 1;
}

int w_Text_addf(lua_State *L)
{
	Text *t = luax_checktext(L, 1);

	int index = 0;

	std::vector<Font::ColoredString> text;
	luax_checkcoloredstring(L, 2, text);

	float wrap = (float) luaL_checknumber(L, 3);

	Font::AlignMode align = Font::ALIGN_MAX_ENUM;
	const char *alignstr = luaL_checkstring(L, 4);

	if (!Font::getConstant(alignstr, align))
		return luax_enumerror(L, "align mode", Font::getConstants(align), alignstr);

	if (luax_istype(L, 5, math::Transform::type))
	{
		math::Transform *tf = luax_totype<math::Transform>(L, 5);
		luax_catchexcept(L, [&](){ index = t->addf(text, wrap, align, tf->getMatrix()); });
	}
	else
	{
		float x  = (float) luaL_optnumber(L, 5, 0.0);
		float y  = (float) luaL_optnumber(L, 6, 0.0);
		float a  = (float) luaL_optnumber(L, 7, 0.0);
		float sx = (float) luaL_optnumber(L, 8, 1.0);
		float sy = (float) luaL_optnumber(L, 9, sx);
		float ox = (float) luaL_optnumber(L, 10, 0.0);
		float oy = (float) luaL_optnumber(L, 11, 0.0);
		float kx = (float) luaL_optnumber(L, 12, 0.0);
		float ky = (float) luaL_optnumber(L, 13, 0.0);

		Matrix4 m(x, y, a, sx, sy, ox, oy, kx, ky);
		luax_catchexcept(L, [&](){ index = t->addf(text, wrap, align, m); });
	}

	lua_pushnumber(L, index + 1);
	return 1;
}

int w_Text_clear(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	luax_catchexcept(L, [&](){ t->clear(); });
	return 0;
}

int w_Text_setFont(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	Font *f = luax_checktype<Font>(L, 2);
	luax_catchexcept(L, [&](){ t->setFont(f); });
	return 0;
}

int w_Text_getFont(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	Font *f = t->getFont();
	luax_pushtype(L, f);
	return 1;
}

int w_Text_getWidth(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	int index = (int) luaL_optinteger(L, 2, 0) - 1;
	lua_pushnumber(L, t->getWidth(index));
	return 1;
}

int w_Text_getHeight(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	int index = (int) luaL_optinteger(L, 2, 0) - 1;
	lua_pushnumber(L, t->getHeight(index));
	return 1;
}

int w_Text_getDimensions(lua_State *L)
{
	Text *t = luax_checktext(L, 1);
	int index = (int) luaL_optinteger(L, 2, 0) - 1;
	lua_pushnumber(L, t->getWidth(index));
	lua_pushnumber(L, t->getHeight(index));
	return 2;
}

static const luaL_Reg w_Text_functions[] =
{
	{ "set", w_Text_set },
	{ "setf", w_Text_setf },
	{ "add", w_Text_add },
	{ "addf", w_Text_addf },
	{ "clear", w_Text_clear },
	{ "setFont", w_Text_setFont },
	{ "getFont", w_Text_getFont },
	{ "getWidth", w_Text_getWidth },
	{ "getHeight", w_Text_getHeight },
	{ "getDimensions", w_Text_getDimensions },
	{ 0, 0 }
};

extern "C" int luaopen_text(lua_State *L)
{
	return luax_register_type(L, &Text::type, w_Text_functions, nullptr);
}

} // graphics
} // love
