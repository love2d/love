/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "libluasocket/unix.h"
}

// Lua files
#include "libluasocket/ftp.lua.h"
#include "libluasocket/headers.lua.h"
#include "libluasocket/http.lua.h"
#include "libluasocket/ltn12.lua.h"
#include "libluasocket/mbox.lua.h"
#include "libluasocket/mime.lua.h"
#include "libluasocket/smtp.lua.h"
#include "libluasocket/socket.lua.h"
#include "libluasocket/tp.lua.h"
#include "libluasocket/url.lua.h"

static void preload(lua_State* L, const char* name, lua_CFunction func)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
}

static void preload(lua_State *L, const char *name, const char *chunkname, const void *lua, size_t size)
{
	if (luaL_loadbuffer(L, (const char *) lua, size, name) != 0)
	{
		luaL_loadstring(L, "local name, msg = ... return function() error(name..\": \"..msg) end");
		lua_pushstring(L, name);
		lua_pushvalue(L, -3);
		// Before:
		// -1: error message
		// -2: module name
		// -3: loadstring function
		// -4: error message
		lua_call(L, 2, 1);
		// After:
		// -1: function
		// -2: error message
		lua_remove(L, -2);
	}

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, name);
	lua_pop(L, 3);
}

namespace love
{

namespace luasocket
{

int preload(lua_State * L)
{

	// Preload code from LuaSocket.
	preload(L, "socket.core", luaopen_socket_core);
	preload(L, "socket.unix", luaopen_socket_unix);
	preload(L, "mime.core",   luaopen_mime_core);

	preload(L, "socket",         "=[socket \"socket.lua\"]",  socket_lua,  sizeof(socket_lua));
	preload(L, "socket.ftp",     "=[socket \"ftp.lua\"]",     ftp_lua,     sizeof(ftp_lua));
	preload(L, "socket.http",    "=[socket \"http.lua\"]",    http_lua,    sizeof(http_lua));
	preload(L, "ltn12",          "=[socket \"ltn12.lua\"]",   ltn12_lua,   sizeof(ltn12_lua));
	preload(L, "mime",           "=[socket \"mime.lua\"]",    mime_lua,    sizeof(mime_lua));
	preload(L, "socket.smtp",    "=[socket \"smtp.lua\"]",    smtp_lua,    sizeof(smtp_lua));
	preload(L, "socket.tp",      "=[socket \"tp.lua\"]",      tp_lua,      sizeof(tp_lua));
	preload(L, "socket.url",     "=[socket \"url.lua\"]",     url_lua,     sizeof(url_lua));
	preload(L, "socket.headers", "=[socket \"headers.lua\"]", headers_lua, sizeof(headers_lua));
	preload(L, "mbox",           "=[socket \"mbox.lua\"]",    mbox_lua,    sizeof(mbox_lua));

	// No need to register garbage collector function.

	return 0;
}

} // luasocket
} // love
