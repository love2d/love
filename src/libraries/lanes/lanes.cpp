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

#include "lanes.h"

// Lanes open function.
extern "C"
{
	int luaopen_lanes( lua_State *L );
}

namespace love
{
namespace lanes
{

	// Opens the lanes.lua file.
	static int open_lanes(lua_State * L)
	{
		#include "lanes/lanes.lua.h"
		lua_getglobal(L, "lanes");
		return 1;
	}

	int open(lua_State * L)
	{
		luax_preload(L, open_lanes, "lanes");
		luax_preload(L, luaopen_lanes, "lua51-lanes");
		return 0;
	}

} // lanes
} // love
