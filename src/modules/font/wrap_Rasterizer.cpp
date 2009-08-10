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

#include "wrap_Rasterizer.h"

#include <common/wrap_Data.h>

namespace love
{
namespace font
{
	Rasterizer * luax_checkrasterizer(lua_State * L, int idx)
	{
		return luax_checktype<Rasterizer>(L, idx, "Rasterizer", FONT_RASTERIZER_T);
	}

	static const luaL_Reg functions[] = {
		// Data
		{ "getPointer", w_Data_getPointer },
		{ "getSize", w_Data_getSize },
		{ 0, 0 }
	};

	int luaopen_rasterizer(lua_State * L)
	{
		return luax_register_type(L, "Rasterizer", functions);
	}

} // font
} // love
