/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "event/Event.h"
#include "common/config.h"

#ifdef LOVE_BUILD_STANDALONE
extern "C" int luaopen_love(lua_State * L);
#endif // LOVE_BUILD_STANDALONE

namespace love
{
namespace thread
{
LuaThread::LuaThread(const std::string &name, love::Data *code)
	: code(code)
	, name(name)
{
	threadName = name;
}

LuaThread::~LuaThread()
{
}

void LuaThread::threadFunction()
{
	error.clear();

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

#ifdef LOVE_BUILD_STANDALONE
	luax_preload(L, luaopen_love, "love");
	luax_require(L, "love");
	lua_pop(L, 1);
#endif // LOVE_BUILD_STANDALONE

	luax_require(L, "love.thread");
	lua_pop(L, 1);

	// We load love.filesystem by default, since require still exists without it
	// but won't load files from the proper paths. love.filesystem also must be
	// loaded before using any love function that can take a filepath argument.
	luax_require(L, "love.filesystem");
	lua_pop(L, 1);

	if (luaL_loadbuffer(L, (const char *) code->getData(), code->getSize(), name.c_str()) != 0)
		error = luax_tostring(L, -1);
	else
	{
		int pushedargs = (int) args.size();

		for (int i = 0; i < pushedargs; i++)
			args[i].toLua(L);

		args.clear();

		if (lua_pcall(L, pushedargs, 0, 0) != 0)
			error = luax_tostring(L, -1);
	}

	lua_close(L);

	if (!error.empty())
		onError();
}

bool LuaThread::start(const std::vector<Variant> &args)
{
	this->args = args;
	return Threadable::start();
}

const std::string &LuaThread::getError() const
{
	return error;
}

void LuaThread::onError()
{
	if (error.empty())
		return;

	auto eventmodule = Module::getInstance<event::Event>(Module::M_EVENT);
	if (!eventmodule)
		return;

	Proxy p;
	p.type = THREAD_THREAD_ID;
	p.object = this;

	std::vector<Variant> vargs = {
		Variant(p.type, &p),
		Variant(error.c_str(), error.length())
	};

	StrongRef<event::Message> msg(new event::Message("threaderror", vargs), Acquire::NORETAIN);
	eventmodule->push(msg);
}

} // thread
} // love
