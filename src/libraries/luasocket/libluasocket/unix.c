/*=========================================================================*\
* Unix domain socket
* LuaSocket toolkit
\*=========================================================================*/
#include "lua.h"
#include "lauxlib.h"

#include "unixtcp.h"
#include "unixudp.h"

/*-------------------------------------------------------------------------*\
* Modules and functions
\*-------------------------------------------------------------------------*/
static const luaL_Reg mod[] = {
    {"tcp", unixtcp_open},
    {"udp", unixudp_open},
    {NULL, NULL}
};

/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int luaopen_socket_unix(lua_State *L)
{
	int i;
	lua_newtable(L);
    for (i = 0; mod[i].name; i++) mod[i].func(L);
	return 1;
}

