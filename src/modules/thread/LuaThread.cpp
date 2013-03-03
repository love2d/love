/**
* Copyright (c) 2006-2012 LOVE Development Team
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

#include "LuaThread.h"
#include <common/config.h>

#ifdef LOVE_BUILD_STANDALONE
extern "C" int luaopen_love(lua_State * L);
#endif // LOVE_BUILD_STANDALONE
extern "C" int luaopen_love_thread(lua_State *L);

namespace love
{
namespace thread
{
LuaThread::LuaThread(const std::string &name, love::Data *code)
	: code(code)
	, name(name)
{
	code->retain();
}

LuaThread::~LuaThread()
{
	code->release();
}

void LuaThread::threadFunction()
{
	this->retain();
	lua_State * L = lua_open();
	luaL_openlibs(L);
#ifdef LOVE_BUILD_STANDALONE
	love::luax_preload(L, luaopen_love, "love");
	luaopen_love(L);
#endif // LOVE_BUILD_STANDALONE
	luaopen_love_thread(L);
	if (luaL_loadbuffer(L, (const char *) code->getData(), code->getSize(), name.c_str()) != 0)
		error = luax_tostring(L, -1);
	else
		if (lua_pcall(L, 0, 0, 0) != 0)
			error = luax_tostring(L, -1);
	lua_close(L);
	this->release();
}

const std::string &LuaThread::getError()
{
	return error;
}
} // thread
} // love
