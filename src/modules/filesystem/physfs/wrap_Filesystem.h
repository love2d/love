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

#ifndef LOVE_FILESYSTEM_PHYSFS_WRAP_FILESYSTEM_H
#define LOVE_FILESYSTEM_PHYSFS_WRAP_FILESYSTEM_H

// LOVE
#include "Filesystem.h"
#include "wrap_File.h"
#include "wrap_FileData.h"

namespace love
{
namespace filesystem
{
namespace physfs
{
	bool hack_setupWriteDirectory();
	int _wrap_init(lua_State * L);
	int _wrap_setIdentity(lua_State * L);
	int _wrap_setSource(lua_State * L);
	int _wrap_newFile(lua_State * L);
	int _wrap_newFileData(lua_State * L);
	int _wrap_getWorkingDirectory(lua_State * L);
	int _wrap_getUserDirectory(lua_State * L);
	int _wrap_getAppdataDirectory(lua_State * L);
	int _wrap_getSaveDirectory(lua_State * L);
	int _wrap_exists(lua_State * L);
	int _wrap_isDirectory(lua_State * L);
	int _wrap_isFile(lua_State * L);
	int _wrap_mkdir(lua_State * L);
	int _wrap_remove(lua_State * L);
	int _wrap_open(lua_State * L);
	int _wrap_close(lua_State * L);
	int _wrap_read(lua_State * L);
	int _wrap_write(lua_State * L);
	int _wrap_eof(lua_State * L);
	int _wrap_tell(lua_State * L);
	int _wrap_seek(lua_State * L);
	int _wrap_enumerate(lua_State * L);
	int _wrap_lines(lua_State * L);
	int _wrap_load(lua_State * L);
	int loader(lua_State * L);
	int wrap_Filesystem_open(lua_State * L);

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_WRAP_FILESYSTEM_H
