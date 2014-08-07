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

#include "LuaThread.h"
#include "event/Event.h"
#include "common/config.h"

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
	, args(0)
	, nargs(0)
{
	threadName = name;
}

LuaThread::~LuaThread()
{
	// No args should still exist at this point,
	// but you never know.
	for (int i = 0; i < nargs; ++i)
		args[i]->release();
}

void LuaThread::threadFunction()
{
	this->retain();
	error.clear();
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
#ifdef LOVE_BUILD_STANDALONE
	love::luax_preload(L, luaopen_love, "love");
	luaopen_love(L);
#endif // LOVE_BUILD_STANDALONE
	luaopen_love_thread(L);
	if (luaL_loadbuffer(L, (const char *) code->getData(), code->getSize(), name.c_str()) != 0)
		error = luax_tostring(L, -1);
	else
	{
		int pushedargs = nargs;
		for (int i = 0; i < nargs; ++i)
		{
			args[i]->toLua(L);
			args[i]->release();
		}
		// Set both args and nargs to nil, prevents the deconstructor from
		// accessing it again.
		nargs = 0;
		args = 0;

		if (lua_pcall(L, pushedargs, 0, 0) != 0)
			error = luax_tostring(L, -1);
	}
	lua_close(L);
	if (!error.empty())
		onError();
	this->release();
}

bool LuaThread::start(Variant **args, int nargs)
{
	for (int i = 0; i < this->nargs; ++i)
		this->args[i]->release();

	this->args = args;
	this->nargs = nargs;

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

	event::Event *event = Module::getInstance<event::Event>(Module::M_EVENT);
	if (!event)
		return;

	Proxy p;
	p.flags = THREAD_THREAD_T;
	p.data = this;

	Variant *arg1 = new Variant(THREAD_THREAD_ID, &p);
	Variant *arg2 = new Variant(error.c_str(), error.length());
	event::Message *msg = new event::Message("threaderror", arg1, arg2);
	arg1->release();
	arg2->release();

	event->push(msg);
	msg->release();
}

} // thread
} // love
