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

#include "wrap_DroppedFile.h"
#include "wrap_File.h"

namespace love
{
namespace filesystem
{

DroppedFile *luax_checkdroppedfile(lua_State *L, int idx)
{
	return luax_checktype<DroppedFile>(L, idx, "DroppedFile", FILESYSTEM_DROPPED_FILE_T);
}

static const luaL_Reg functions[] =
{
	// Inherits from File.
	{ "getSize", w_File_getSize },
	{ "open", w_File_open },
	{ "close", w_File_close },
	{ "isOpen", w_File_isOpen },
	{ "read", w_File_read },
	{ "write", w_File_write },
	{ "flush", w_File_flush },
	{ "eof", w_File_eof },
	{ "tell", w_File_tell },
	{ "seek", w_File_seek },
	{ "lines", w_File_lines },
	{ "setBuffer", w_File_setBuffer },
	{ "getBuffer", w_File_getBuffer },
	{ "getMode", w_File_getMode },
	{ "getFilename", w_File_getFilename },
	{ "getExtension", w_File_getExtension },
	{ 0, 0 }
};

extern "C" int luaopen_droppedfile(lua_State *L)
{
	return luax_register_type(L, "DroppedFile", functions);
}

} // filesystem
} // love
