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

#include "wrap_FileData.h"

#include "data/wrap_Data.h"

namespace love
{
namespace filesystem
{

FileData *luax_checkfiledata(lua_State *L, int idx)
{
	return luax_checktype<FileData>(L, idx);
}

int w_FileData_clone(lua_State *L)
{
	FileData *t = luax_checkfiledata(L, 1), *c = nullptr;
	luax_catchexcept(L, [&](){ c = t->clone(); });
	luax_pushtype(L, c);
	c->release();
	return 1;
}

int w_FileData_getFilename(lua_State *L)
{
	FileData *t = luax_checkfiledata(L, 1);
	lua_pushstring(L, t->getFilename().c_str());
	return 1;
}

int w_FileData_getExtension(lua_State *L)
{
	FileData *t = luax_checkfiledata(L, 1);
	lua_pushstring(L, t->getExtension().c_str());
	return 1;
}

static const luaL_Reg w_FileData_functions[] =
{
	{ "clone", w_FileData_clone },
	{ "getFilename", w_FileData_getFilename },
	{ "getExtension", w_FileData_getExtension },

	{ 0, 0 }
};

extern "C" int luaopen_filedata(lua_State *L)
{
	int ret = luax_register_type(L, &FileData::type, data::w_Data_functions, w_FileData_functions, nullptr);
	love::data::luax_rundatawrapper(L, FileData::type);
	return ret;
}

} // filesystem
} // love
