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

#include "luasocket.h"

// LuaSocket
extern "C" {
#include "libluasocket/luasocket.h"
#include "libluasocket/mime.h"
}

// Quick macro for adding functions to 
// the preloder.
#define PRELOAD(name, function) \
	lua_getglobal(L, "package"); \
	lua_getfield(L, -1, "preload"); \
	lua_pushcfunction(L, function); \
	lua_setfield(L, -2, name); \
	lua_pop(L, 2);	

namespace love
{
namespace luasocket
{

int __open(lua_State * L)
{

	// Preload code from LuaSocket.
	PRELOAD("socket.core", luaopen_socket_core);
	PRELOAD("mime.core", luaopen_mime_core);

	PRELOAD("socket", __open_luasocket_socket);
	PRELOAD("socket.ftp", __open_luasocket_ftp)
	PRELOAD("socket.http", __open_luasocket_http);
	PRELOAD("ltn12", __open_luasocket_ltn12);
	PRELOAD("mime", __open_luasocket_mime)
	PRELOAD("socket.smtp", __open_luasocket_smtp);
	PRELOAD("socket.tp", __open_luasocket_tp)
	PRELOAD("socket.url", __open_luasocket_url)
	PRELOAD("socket.headers", __open_luasocket_headers)
	PRELOAD("mbox", __open_luasocket_mbox)

	// No need to register garbage collector function.

	return 0;
}

int __open_luasocket_socket(lua_State * L)
{
	#include "libluasocket/socket.lua.h"
	return 1;
}

int __open_luasocket_ftp(lua_State * L)
{
	#include "libluasocket/ftp.lua.h"
	return 1;
}

int __open_luasocket_http(lua_State * L)
{
	#include "libluasocket/http.lua.h"
	return 1;
}

int __open_luasocket_ltn12(lua_State * L)
{
	#include "libluasocket/ltn12.lua.h"
	return 1;
}

int __open_luasocket_mime(lua_State * L)
{
	#include "libluasocket/mime.lua.h"
	return 1;
}

int __open_luasocket_smtp(lua_State * L)
{
	#include "libluasocket/smtp.lua.h"
	return 1;
}

int __open_luasocket_tp(lua_State * L)
{
	#include "libluasocket/tp.lua.h"
	return 1;
}

int __open_luasocket_url(lua_State * L)
{
	#include "libluasocket/url.lua.h"
	return 1;
}

int __open_luasocket_headers(lua_State * L)
{
	#include "libluasocket/headers.lua.h"
	return 1;
}

int __open_luasocket_mbox(lua_State * L)
{
	#include "libluasocket/mbox.lua.h"
	return 1;
}

} // luasocket
} // love
