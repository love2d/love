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

#include "wrap_Texture.h"

namespace love
{
namespace graphics
{

Texture *luax_checktexture(lua_State *L, int idx)
{
	return luax_checktype<Texture>(L, idx);
}

int w_Texture_getTextureType(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const char *tstr;
	if (!Texture::getConstant(t->getTextureType(), tstr))
		return luax_enumerror(L, "texture type", Texture::getConstants(TEXTURE_MAX_ENUM), tstr);
	lua_pushstring(L, tstr);
	return 1;
}

static int w__optMipmap(lua_State *L, Texture *t, int idx)
{
	int mipmap = 0;

	if (!lua_isnoneornil(L, idx))
	{
		mipmap = (int) luaL_checkinteger(L, idx) - 1;

		if (mipmap < 0 || mipmap >= t->getMipmapCount())
			luaL_error(L, "Invalid mipmap index: %d", mipmap + 1);
	}

	return mipmap;
}

int w_Texture_getWidth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getWidth(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getHeight(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getHeight(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getDimensions(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	int mipmap = w__optMipmap(L, t, 2);
	lua_pushnumber(L, t->getWidth(mipmap));
	lua_pushnumber(L, t->getHeight(mipmap));
	return 2;
}

int w_Texture_getDepth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getDepth(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getLayerCount(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getLayerCount());
	return 1;
}

int w_Texture_getMipmapCount(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getMipmapCount());
	return 1;
}

int w_Texture_getPixelWidth(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getPixelWidth(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getPixelHeight(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getPixelHeight(w__optMipmap(L, t, 2)));
	return 1;
}

int w_Texture_getPixelDimensions(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	int mipmap = w__optMipmap(L, t, 2);
	lua_pushnumber(L, t->getPixelWidth(mipmap));
	lua_pushnumber(L, t->getPixelHeight(mipmap));
	return 2;
}

int w_Texture_getDPIScale(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	lua_pushnumber(L, t->getDPIScale());
	return 1;
}

int w_Texture_setFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	Texture::Filter f = t->getFilter();

	const char *minstr = luaL_checkstring(L, 2);
	const char *magstr = luaL_optstring(L, 3, minstr);

	if (!Texture::getConstant(minstr, f.min))
		return luax_enumerror(L, "filter mode", Texture::getConstants(f.min), minstr);
	if (!Texture::getConstant(magstr, f.mag))
		return luax_enumerror(L, "filter mode", Texture::getConstants(f.mag), magstr);

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

int w_Texture_setMipmapFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	Texture::Filter f = t->getFilter();

	if (lua_isnoneornil(L, 2))
		f.mipmap = Texture::FILTER_NONE; // mipmapping is disabled if no argument is given
	else
	{
		const char *mipmapstr = luaL_checkstring(L, 2);
		if (!Texture::getConstant(mipmapstr, f.mipmap))
			return luax_enumerror(L, "filter mode", Texture::getConstants(f.mipmap), mipmapstr);
	}

	luax_catchexcept(L, [&](){ t->setFilter(f); });
	t->setMipmapSharpness((float) luaL_optnumber(L, 3, 0.0));

	return 0;
}

int w_Texture_getMipmapFilter(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);

	const Texture::Filter &f = t->getFilter();

	const char *mipmapstr;
	if (Texture::getConstant(f.mipmap, mipmapstr))
		lua_pushstring(L, mipmapstr);
	else
		lua_pushnil(L); // only return a mipmap filter if mipmapping is enabled

	lua_pushnumber(L, t->getMipmapSharpness());
	return 2;
}

int w_Texture_setWrap(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	Texture::Wrap w;

	const char *sstr = luaL_checkstring(L, 2);
	const char *tstr = luaL_optstring(L, 3, sstr);
	const char *rstr = luaL_optstring(L, 4, sstr);

	if (!Texture::getConstant(sstr, w.s))
		return luax_enumerror(L, "wrap mode", Texture::getConstants(w.s), sstr);
	if (!Texture::getConstant(tstr, w.t))
		return luax_enumerror(L, "wrap mode", Texture::getConstants(w.t), tstr);
	if (!Texture::getConstant(rstr, w.r))
		return luax_enumerror(L, "wrap mode", Texture::getConstants(w.r), rstr);

	luax_pushboolean(L, t->setWrap(w));
	return 1;
}

int w_Texture_getWrap(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	const Texture::Wrap w = t->getWrap();

	const char *sstr = nullptr;
	const char *tstr = nullptr;
	const char *rstr = nullptr;

	if (!Texture::getConstant(w.s, sstr))
		return luaL_error(L, "Unknown wrap mode.");
	if (!Texture::getConstant(w.t, tstr))
		return luaL_error(L, "Unknown wrap mode.");
	if (!Texture::getConstant(w.r, rstr))
		return luaL_error(L, "Unknown wrap mode.");

	lua_pushstring(L, sstr);
	lua_pushstring(L, tstr);
	lua_pushstring(L, rstr);
	return 3;
}

int w_Texture_getFormat(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	PixelFormat format = t->getPixelFormat();
	const char *str;
	if (!getConstant(format, str))
		return luaL_error(L, "Unknown pixel format.");

	lua_pushstring(L, str);
	return 1;
}

int w_Texture_isReadable(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	luax_pushboolean(L, t->isReadable());
	return 1;
}

int w_Texture_setDepthSampleMode(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);

	Optional<CompareMode> mode;
	if (!lua_isnoneornil(L, 2))
	{
		const char *str = luaL_checkstring(L, 2);

		mode.hasValue = true;
		if (!getConstant(str, mode.value))
			return luax_enumerror(L, "compare mode", getConstants(mode.value), str);
	}

	luax_catchexcept(L, [&]() { t->setDepthSampleMode(mode); });
	return 0;
}

int w_Texture_getDepthSampleMode(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);
	Optional<CompareMode> mode = t->getDepthSampleMode();

	if (mode.hasValue)
	{
		const char *str = nullptr;
		if (!getConstant(mode.value, str))
			return luaL_error(L, "Unknown compare mode.");
		lua_pushstring(L, str);
	}
	else
		lua_pushnil(L);

	return 1;
}

const luaL_Reg w_Texture_functions[] =
{
	{ "getTextureType", w_Texture_getTextureType },
	{ "getWidth", w_Texture_getWidth },
	{ "getHeight", w_Texture_getHeight },
	{ "getDimensions", w_Texture_getDimensions },
	{ "getDepth", w_Texture_getDepth },
	{ "getLayerCount", w_Texture_getLayerCount },
	{ "getMipmapCount", w_Texture_getMipmapCount },
	{ "getPixelWidth", w_Texture_getPixelWidth },
	{ "getPixelHeight", w_Texture_getPixelHeight },
	{ "getPixelDimensions", w_Texture_getPixelDimensions },
	{ "getDPIScale", w_Texture_getDPIScale },
	{ "setFilter", w_Texture_setFilter },
	{ "getFilter", w_Texture_getFilter },
	{ "setMipmapFilter", w_Texture_setMipmapFilter },
	{ "getMipmapFilter", w_Texture_getMipmapFilter },
	{ "setWrap", w_Texture_setWrap },
	{ "getWrap", w_Texture_getWrap },
	{ "getFormat", w_Texture_getFormat },
	{ "isReadable", w_Texture_isReadable },
	{ "getDepthSampleMode", w_Texture_getDepthSampleMode },
	{ "setDepthSampleMode", w_Texture_setDepthSampleMode },
	{ 0, 0 }
};

extern "C" int luaopen_texture(lua_State *L)
{
	return luax_register_type(L, &Texture::type, w_Texture_functions, nullptr);
}

} // graphics
} // love
