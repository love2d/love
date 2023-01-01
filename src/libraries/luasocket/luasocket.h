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

#ifndef LOVE_LUASOCKET_LUASOCKET_H
#define LOVE_LUASOCKET_LUASOCKET_H

// LOVE
#include <common/runtime.h>

namespace love
{
namespace luasocket
{

int __open(lua_State * L);

// Loaders for all lua files. We want to be able
// to load these dynamically. (Identical in the LuaSocket 
// documentation. These functions wrap the parsing of code, etc).
int __open_luasocket_socket(lua_State * L);
int __open_luasocket_ftp(lua_State * L);
int __open_luasocket_http(lua_State * L);
int __open_luasocket_ltn12(lua_State * L);
int __open_luasocket_mime(lua_State * L);
int __open_luasocket_smtp(lua_State * L);
int __open_luasocket_tp(lua_State * L);
int __open_luasocket_url(lua_State * L);
int __open_luasocket_headers(lua_State * L);
int __open_luasocket_mbox(lua_State * L);

} // luasocket
} // love

#endif // LOVE_LUASOCKET_LUASOCKET_H
