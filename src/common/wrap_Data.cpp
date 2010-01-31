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

#include "wrap_Data.h"

namespace love
{
	Data * luax_checkdata(lua_State * L, int idx)
	{
		return luax_checktype<Data>(L, idx, "Data", DATA_T);
	}

	int w_Data_getPointer(lua_State * L)
	{
		Data * t = luax_checkdata(L, 1);
		lua_pushlightuserdata(L, t->getData());
		return 1;
	}

	int w_Data_getSize(lua_State * L)
	{
		Data * t = luax_checkdata(L, 1);
		lua_pushinteger(L, t->getSize());
		return 1;
	}

	const luaL_Reg w_Data_functions[] = {
		{ "getPointer", w_Data_getPointer },
		{ "getSize", w_Data_getSize },
		{ 0, 0 }
	};

	int w_Data_open(lua_State * L)
	{
		luax_register_type(L, "Data", w_Data_functions);
		return 0;
	}
	
} // love
