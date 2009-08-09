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
#include "wrap_Frame.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Frame * luax_checkframe(lua_State * L, int idx)
	{
		return luax_checktype<Frame>(L, idx, "Frame", GRAPHICS_FRAME_T);
	}
	
	int w_Frame_flip(lua_State *L)
	{
		Frame *frame = luax_checktype<Frame>(L, 1, "Frame", GRAPHICS_FRAME_T);
		frame->flip(luax_toboolean(L, 2), luax_toboolean(L, 3));
		return 0;
	}

	static const luaL_Reg w_Frame_functions[] = {
		{ "flip", w_Frame_flip },
		{ 0, 0 }
	};

	int w_Frame_open(lua_State * L)
	{
		luax_register_type(L, "Frame", w_Frame_functions);
		return 0;
	}

} // opengl
} // graphics
} // love
