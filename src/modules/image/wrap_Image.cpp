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

#include "wrap_Image.h"

#include "devil/Image.h"

namespace love
{
namespace image
{
	static Image * instance = 0;

	int _wrap_newImageData(lua_State * L)
	{

		// Case 1: Integers.
		if(lua_isnumber(L, 1))
		{
			int w = luaL_checkint(L, 1);
			int h = luaL_checkint(L, 2);
			ImageData * t = instance->newImageData(w, h);
			luax_newtype(L, "ImageData", LOVE_IMAGE_IMAGE_DATA_BITS, (void*)t);
			return 1;
		}

		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_strtofile(L, 1);

		// Case 2: String/File.
		love::filesystem::File * file = luax_checktype<love::filesystem::File>(L, 1, "File", LOVE_FILESYSTEM_FILE_BITS);
		ImageData * t = instance->newImageData(file);
		luax_newtype(L, "ImageData", LOVE_IMAGE_IMAGE_DATA_BITS, (void*)t);
		return 1;
	}

	// List of functions to wrap.
	const luaL_Reg wrap_Image_functions[] = {
		{ "newImageData",  _wrap_newImageData },
		{ 0, 0 }
	};

	static const lua_CFunction wrap_Image_types[] = {
		wrap_ImageData_open,
		0
	};

	int wrap_Image_open(lua_State * L)
	{

		if(instance == 0)
		{
			try 
			{
				instance = new love::image::devil::Image();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, "love.image", instance);

		return luax_register_module(L, wrap_Image_functions, wrap_Image_types, "image");
	}

} // image
} // love
