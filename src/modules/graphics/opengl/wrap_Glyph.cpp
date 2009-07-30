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
#include "wrap_Glyph.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Glyph * luax_checkglyph(lua_State * L, int idx)
	{
		return luax_checktype<Glyph>(L, idx, "Glyph", LOVE_GRAPHICS_GLYPH_BITS);
	}

	static const luaL_Reg wrap_Glyph_functions[] = {
		{ 0, 0 }
	};

	int wrap_Glyph_open(lua_State * L)
	{
		luax_register_type(L, "Glyph", wrap_Glyph_functions);
		return 0;
	}

} // opengl
} // graphics
} // love
