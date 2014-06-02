/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#ifndef LOVE_FILESYSTEM_WRAP_FILE_H
#define LOVE_FILESYSTEM_WRAP_FILE_H

// LOVE
#include "common/runtime.h"
#include "File.h"

namespace love
{
namespace filesystem
{

// Does not use lua_error, so it's safe to call in exception handling code.
int luax_ioError(lua_State *L, const char *fmt, ...);

File *luax_checkfile(lua_State *L, int idx);
int w_File_getSize(lua_State *L);
int w_File_open(lua_State *L);
int w_File_close(lua_State *L);
int w_File_isOpen(lua_State *L);
int w_File_read(lua_State *L);
int w_File_write(lua_State *L);
int w_File_flush(lua_State *L);
int w_File_eof(lua_State *L);
int w_File_tell(lua_State *L);
int w_File_seek(lua_State *L);
int w_File_lines(lua_State *L);
int w_File_setBuffer(lua_State *L);
int w_File_getBuffer(lua_State *L);
int w_File_getMode(lua_State *L);
extern "C" int luaopen_file(lua_State *L);

} // filesystem
} // love

#endif // LOVE_FILESYSTEM_WRAP_FILE_H
