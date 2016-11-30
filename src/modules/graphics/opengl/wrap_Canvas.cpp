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

#include "wrap_Canvas.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Canvas *luax_checkcanvas(lua_State *L, int idx)
{
	return luax_checktype<Canvas>(L, idx);
}

int w_Canvas_getFormat(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	PixelFormat format = canvas->getPixelFormat();
	const char *str;
	if (!getConstant(format, str))
		return luaL_error(L, "Unknown pixel format.");

	lua_pushstring(L, str);
	return 1;
}

int w_Canvas_getMSAA(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	lua_pushinteger(L, canvas->getMSAA());
	return 1;
}

static const luaL_Reg w_Canvas_functions[] =
{
	{ "getFormat", w_Canvas_getFormat },
	{ "getMSAA", w_Canvas_getMSAA },
	{ 0, 0 }
};

extern "C" int luaopen_canvas(lua_State *L)
{
	return luax_register_type(L, &Canvas::type, w_Texture_functions, w_Canvas_functions, nullptr);
}

} // opengl
} // graphics
} // love
