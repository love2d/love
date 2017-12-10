
#ifndef LUA53_LSTRLIB_H
#define LUA53_LSTRLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"

typedef struct luaL_Buffer_53 {
	luaL_Buffer b; /* make incorrect code crash! */
	char *ptr;
	size_t nelems;
	size_t capacity;
	lua_State *L2;
} luaL_Buffer_53;

void lua53_pushresult (luaL_Buffer_53 *B);
void lua53_cleanupbuffer (luaL_Buffer_53 *B);

void lua53_str_pack (lua_State *L, const char *fmt, int startidx, luaL_Buffer_53 *b);
int lua53_str_packsize (lua_State *L);
int lua53_str_unpack (lua_State *L, const char *fmt, const char *data, size_t ld, int dataidx, int posidx);

#ifdef __cplusplus
}
#endif

#endif /* LUA53_LSTRLIB_H */
