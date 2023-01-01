/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "wrap_Cursor.h"

namespace love
{
namespace mouse
{

Cursor *luax_checkcursor(lua_State *L, int idx)
{
	return luax_checktype<Cursor>(L, idx);
}

int w_Cursor_getType(lua_State *L)
{
	Cursor *cursor = luax_checkcursor(L, 1);

	Cursor::CursorType ctype = cursor->getType();
	const char *typestr = nullptr;

	if (ctype == Cursor::CURSORTYPE_IMAGE)
		mouse::Cursor::getConstant(ctype, typestr);
	else if (ctype == Cursor::CURSORTYPE_SYSTEM)
	{
		Cursor::SystemCursor systype = cursor->getSystemType();
		mouse::Cursor::getConstant(systype, typestr);
	}

	if (!typestr)
		return luaL_error(L, "Unknown cursor type.");

	lua_pushstring(L, typestr);
	return 1;
}

static const luaL_Reg w_Cursor_functions[] =
{
	{ "getType", w_Cursor_getType },
	{ 0, 0 },
};

extern "C" int luaopen_cursor(lua_State *L)
{
	return luax_register_type(L, &Cursor::type, w_Cursor_functions, nullptr);
}

} // mouse
} // love
