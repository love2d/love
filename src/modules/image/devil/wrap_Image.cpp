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

#include "wrap_Image.h"
#include "image/EncodedImageData.h"
#include "image/wrap_EncodedImageData.h"

namespace love
{
namespace image
{
	extern Image * instance;
	
namespace devil
{
	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "newImageData",  w_newImageData },
		{ "newEncodedImageData", w_newEncodedImageData },
		{ 0, 0 }
	};

	static const lua_CFunction types[] = {
		luaopen_imagedata,
		luaopen_encodedimagedata,
		0
	};
	
	int luaopen_love_image(lua_State * L)
	{
		if(instance == 0)
		{
			try
			{
				instance = new Image();
			}
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}
		else
			instance->retain();

		WrappedModule w;
		w.module = instance;
		w.name = "image";
		w.flags = MODULE_IMAGE_T;
		w.functions = functions;
		w.types = types;

		return luax_register_module(L, w);
	}

} // devil
} // image
} // love
