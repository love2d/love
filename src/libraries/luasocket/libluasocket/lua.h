#ifndef LUA_WRAP_H
#define LUA_WRAP_H

#define LUA_COMPAT_ALL
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM > 501
#	define luaL_reg luaL_Reg
#	define luaL_putchar(B, c) luaL_addchar(B, c)
#	define luaL_typerror(L, n, t) luax_typerror(L, n, t)

extern int luax_typerror(lua_State *L, int narg, const char *type);
#endif

#endif // LUA_WRAP_H
