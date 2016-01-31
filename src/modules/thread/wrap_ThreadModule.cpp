/**
* Copyright (c) 2006-2010 LOVE Development Team
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

// LOVE
#include "wrap_ThreadModule.h"
#include "wrap_LuaThread.h"
#include "wrap_Channel.h"
#include "ThreadModule.h"

#include "filesystem/File.h"
#include "filesystem/FileData.h"

// C
#include <cstring>

namespace love
{
namespace thread
{

#define instance() (Module::getInstance<ThreadModule>(Module::M_THREAD))

int w_newThread(lua_State *L)
{
	std::string name = "Thread code";
	love::Data *data = nullptr;

	if (lua_isstring(L, 1))
	{
		size_t slen = 0;
		const char *str = lua_tolstring(L, 1, &slen);

		// Treat the string as Lua code if it's long or has a newline.
		if (slen >= 1024 || memchr(str, '\n', slen))
		{
			// Construct a FileData from the string.
			lua_pushvalue(L, 1);
			lua_pushstring(L, "string");
			int idxs[] = {lua_gettop(L) - 1, lua_gettop(L)};
			luax_convobj(L, idxs, 2, "filesystem", "newFileData");
			lua_pop(L, 1);
			lua_replace(L, 1);
		}
		else
			luax_convobj(L, 1, "filesystem", "newFileData");
	}
	else if (luax_istype(L, 1, FILESYSTEM_FILE_ID))
		luax_convobj(L, 1, "filesystem", "newFileData");

	if (luax_istype(L, 1, FILESYSTEM_FILE_DATA_ID))
	{
		love::filesystem::FileData *fdata = luax_checktype<love::filesystem::FileData>(L, 1, FILESYSTEM_FILE_DATA_ID);
		name = std::string("@") + fdata->getFilename();
		data = fdata;
	}
	else
	{
		data = luax_checktype<love::Data>(L, 1, DATA_ID);
	}

	LuaThread *t = instance()->newThread(name, data);
	luax_pushtype(L, THREAD_THREAD_ID, t);
	t->release();
	return 1;
}

int w_newChannel(lua_State *L)
{
	Channel *c = instance()->newChannel();
	luax_pushtype(L, THREAD_CHANNEL_ID, c);
	c->release();
	return 1;
}

int w_getChannel(lua_State *L)
{
	std::string name = luax_checkstring(L, 1);
	Channel *c = instance()->getChannel(name);
	luax_pushtype(L, THREAD_CHANNEL_ID, c);
	c->release();
	return 1;
}

// List of functions to wrap.
static const luaL_Reg module_functions[] =
{
	{ "newThread", w_newThread },
	{ "newChannel", w_newChannel },
	{ "getChannel", w_getChannel },
	{ 0, 0 }
};

static const lua_CFunction types[] = {
	luaopen_thread,
	luaopen_channel,
	0
};

extern "C" int luaopen_love_thread(lua_State *L)
{
	ThreadModule *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::thread::ThreadModule(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "thread";
	w.type = MODULE_ID;
	w.functions = module_functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // thread
} // love
