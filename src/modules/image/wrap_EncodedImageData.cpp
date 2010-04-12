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

#include "wrap_ImageData.h"

#include <common/wrap_Data.h>

namespace love
{
namespace image
{
	EncodedImageData * luax_checkencodedimagedata(lua_State * L, int idx)
	{
		return luax_checktype<EncodedImageData>(L, idx, "EncodedImageData", IMAGE_ENCODED_IMAGE_DATA_T);
	}
	
	int w_EncodedImageData_getFormat(lua_State * L) {
		EncodedImageData * e = luax_checkencodedimagedata(L, 1);
		EncodedImageData::Format f = e->getFormat();
		const char * fmt;
		EncodedImageData::getConstant(f, fmt);
		lua_pushstring(L, fmt);
		return 1;
	}

	static const luaL_Reg functions[] = {

		// Data
		{ "getPointer", w_Data_getPointer },
		{ "getSize", w_Data_getSize },

		{ "getFormat", w_EncodedImageData_getFormat },
		{ 0, 0 }
	};

	int luaopen_encodedimagedata(lua_State * L)
	{
		return luax_register_type(L, "EncodedImageData", functions);
	}

} // image
} // love