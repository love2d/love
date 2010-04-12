/**
* Copyright (c) 2006-2010 LOVE Development Team
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
#include "wrap_Image.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Image * luax_checkimage(lua_State * L, int idx)
	{
		return luax_checktype<Image>(L, idx, "Image", GRAPHICS_IMAGE_T);
	}

	int w_Image_getWidth(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1);
		lua_pushnumber(L, t->getWidth());
		return 1;
	}

	int w_Image_getHeight(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1);
		lua_pushnumber(L, t->getHeight());
		return 1;
	}

	int w_Image_setFilter(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1);
		Image::Filter f;
		Image::FilterMode min;
		Image::FilterMode mag;
		const char * minstr = luaL_checkstring(L, 2);
		const char * magstr = luaL_checkstring(L, 3);
		if (!Image::getConstant(minstr, min))
			return luaL_error(L, "Invalid filter mode: %s", minstr);
		if (!Image::getConstant(magstr, mag))
			return luaL_error(L, "Invalid filter mode: %s", magstr);
		
		f.min = min;
		f.mag = mag;
		t->setFilter(f);
		return 0;
	}

	int w_Image_getFilter(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1); 
		Image::Filter f = t->getFilter();
		Image::FilterMode min = f.min;
		Image::FilterMode mag = f.mag;
		const char * minstr;
		const char * magstr;
		Image::getConstant(min, minstr);
		Image::getConstant(mag, magstr);
		lua_pushstring(L, minstr);
		lua_pushstring(L, magstr);
		return 2;
	}

	int w_Image_setWrap(lua_State * L)
	{
		Image * i = luax_checkimage(L, 1); 
		Image::Wrap w;
		Image::WrapMode s;
		Image::WrapMode t;
		const char * sstr = luaL_checkstring(L, 2);
		const char * tstr = luaL_checkstring(L, 3);
		if (!Image::getConstant(sstr, s))
			return luaL_error(L, "Invalid wrap mode: %s", sstr);
		if (!Image::getConstant(tstr, t))
			return luaL_error(L, "Invalid wrap mode, %s", tstr);
		
		w.s = s;
		w.t = t;
		i->setWrap(w);
		return 0;
	}

	int w_Image_getWrap(lua_State * L)
	{
		Image * i = luax_checkimage(L, 1); 
		Image::Wrap w = i->getWrap();
		Image::WrapMode s = w.s;
		Image::WrapMode t = w.t;
		const char * sstr;
		const char * tstr;
		Image::getConstant(s, sstr);
		Image::getConstant(t, tstr);
		lua_pushstring(L, sstr);
		lua_pushstring(L, tstr);
		return 2;
	}

	static const luaL_Reg functions[] = {
		{ "getWidth", w_Image_getWidth },
		{ "getHeight", w_Image_getHeight },
		{ "setFilter", w_Image_setFilter },
		{ "getFilter", w_Image_getFilter },
		{ "setWrap", w_Image_setWrap },
		{ "getWrap", w_Image_getWrap },
		{ 0, 0 }
	};

	int luaopen_image(lua_State * L)
	{
		return luax_register_type(L, "Image", functions);
	}

} // opengl
} // graphics
} // love
