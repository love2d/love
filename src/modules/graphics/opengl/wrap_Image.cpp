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
#include "wrap_Image.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Image * luax_checkimage(lua_State * L, int idx)
	{
		return luax_checktype<Image>(L, idx, "Image", LOVE_GRAPHICS_IMAGE_BITS);
	}

	int _wrap_Image_getWidth(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1);
		lua_pushnumber(L, t->getWidth());
		return 1;
	}

	int _wrap_Image_getHeight(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1);
		lua_pushnumber(L, t->getHeight());
		return 1;
	}

	int _wrap_Image_setFilter(lua_State * L)
	{
		Image * t = luax_checkimage(L, 1);
		int min = luaL_checkint(L, 2);
		int mag = luaL_checkint(L, 3);
		t->setFilter(min, mag);
		return 1;
	}

	static const luaL_Reg wrap_Image_functions[] = {
		{ "getWidth", _wrap_Image_getWidth },
		{ "getHeight", _wrap_Image_getHeight },
		{ "setFilter", _wrap_Image_setFilter },
		{ 0, 0 }
	};

	int wrap_Image_open(lua_State * L)
	{
		luax_register_type(L, "Image", wrap_Image_functions);
		return 0;
	}

} // opengl
} // graphics
} // love
