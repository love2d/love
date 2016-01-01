/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "wrap_Texture.h"

namespace love
{
namespace graphics
{

Texture *luax_checktexture(lua_State *L, int idx)
{
	return luax_checktype<Texture>(L, idx, GRAPHICS_TEXTURE_ID);
}

int w_Texture_getWidth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getWidth());
	return 1;
}

int w_Texture_getHeight(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getHeight());
	return 1;
}

int w_Texture_getDimensions(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getWidth());
	lua_pushnumber(L, t->getHeight());
	return 2;
}

int w_Texture_setFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	Texture::Filter f = t->getFilter();

	const char *minstr = luaL_checkstring(L, 2);
	const char *magstr = luaL_optstring(L, 3, minstr);

	if (!Texture::getConstant(minstr, f.min))
		return luaL_error(L, "Invalid filter mode: %s", minstr);
	if (!Texture::getConstant(magstr, f.mag))
		return luaL_error(L, "Invalid filter mode: %s", magstr);

	f.anisotropy = (float) luaL_optnumber(L, 4, 1.0);

	luax_catchexcept(L, [&](){ t->setFilter(f); });
	return 0;
}

int w_Texture_getFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const Texture::Filter f = t->getFilter();

	const char *minstr = nullptr;
	const char *magstr = nullptr;

	if (!Texture::getConstant(f.min, minstr))
		return luaL_error(L, "Unknown filter mode.");
	if (!Texture::getConstant(f.mag, magstr))
		return luaL_error(L, "Unknown filter mode.");

	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, f.anisotropy);
	return 3;
}

int w_Texture_setWrap(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	Texture::Wrap w;

	const char *sstr = luaL_checkstring(L, 2);
	const char *tstr = luaL_optstring(L, 3, sstr);

	if (!Texture::getConstant(sstr, w.s))
		return luaL_error(L, "Invalid wrap mode: %s", sstr);
	if (!Texture::getConstant(tstr, w.t))
		return luaL_error(L, "Invalid wrap mode, %s", tstr);

	luax_pushboolean(L, t->setWrap(w));
	return 1;
}

int w_Texture_getWrap(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const Texture::Wrap w = t->getWrap();

	const char *sstr = nullptr;
	const char *tstr = nullptr;

	if (!Texture::getConstant(w.s, sstr))
		return luaL_error(L, "Unknown wrap mode.");
	if (!Texture::getConstant(w.t, tstr))
		return luaL_error(L, "Unknown wrap mode.");

	lua_pushstring(L, sstr);
	lua_pushstring(L, tstr);
	return 2;
}

const luaL_Reg w_Texture_functions[] =
{
	{ "getWidth", w_Texture_getWidth },
	{ "getHeight", w_Texture_getHeight },
	{ "getDimensions", w_Texture_getDimensions },
	{ "setFilter", w_Texture_setFilter },
	{ "getFilter", w_Texture_getFilter },
	{ "setWrap", w_Texture_setWrap },
	{ "getWrap", w_Texture_getWrap },
	{ 0, 0 }
};

extern "C" int luaopen_texture(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_TEXTURE_ID, "Texture", w_Texture_functions, nullptr);
}

} // graphics
} // love
