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

#include <common/config.h>

#include "Thread.h"

#ifdef LOVE_BUILD_STANDALONE
extern "C" int luaopen_love(lua_State * L);
#endif // LOVE_BUILD_STANDALONE


namespace love
{
namespace thread
{
namespace sdl
{
	int threadfunc(ThreadData *comm)
	{
		lua_State * L = lua_open();
		luaL_openlibs(L);
	#ifdef LOVE_BUILD_STANDALONE
		love::luax_preload(L, luaopen_love, "love");
		luaopen_love(L);
	#endif // LOVE_BUILD_STANDALONE
		luaL_dostring(L, comm->getCode());
		lua_close(L);
		return 0;
	}

	ThreadData::ThreadData(const char *code)
		: code(code)
	{
	}

	const char *ThreadData::getCode()
	{
		return code;
	}

	Thread::Thread(ThreadModuleRegistrar *reg, std::string name, love::Data *data)
		: reg(reg), name(name), handle(0)
	{
		unsigned int len = data->getSize();
		this->data = new char[len];
		memcpy(this->data, data->getData(), len);
		comm = new ThreadData(this->data);
	}

	Thread::~Thread()
	{
		delete[] data;
		if (handle)
			SDL_KillThread(handle);
		reg->unregister(name);
	}

	void Thread::start()
	{
		if (!handle)
			SDL_CreateThread((int (*)(void*)) threadfunc, (void*) comm);
	}

	void Thread::kill()
	{
		if (handle)
			SDL_KillThread(handle);
	}

	std::string Thread::getName()
	{
		return name;
	}

	ThreadModule::~ThreadModule()
	{
		for (threadlist_t::iterator i = threads.begin(); i != threads.end(); i++)
		{
			i->second->kill();
		}
	}

	Thread *ThreadModule::newThread(std::string name, love::Data *data)
	{
		if (threads.count(name) != 0)
			return 0;
		Thread *t = new Thread(this, name, data);
		threads[name] = t;
		return t;
	}

	Thread *ThreadModule::getThread(std::string name)
	{
		if (threads.count(name) == 0)
			return 0;
		threadlist_t::iterator i = threads.find(name);
		return i->second;
	}

	Thread **ThreadModule::getThreads()  //THIS FUNCTION IS BROKEN
	//DO NOT USE IT
	//IT EVEN CONTAINS MEMORY LEAKS!!
	{
		Thread **list = new Thread*[threads.size()];
		int c = 0;
		for (threadlist_t::iterator i = threads.begin(); i != threads.end(); i++, c++)
		{
			list[c] = i->second;
		}
		return 0;
	}

	void ThreadModule::unregister(std::string name)
	{
		if (threads.count(name) == 0)
			return;
		threadlist_t::iterator i = threads.find(name);
		threads.erase(i);
	}

	const char *ThreadModule::getName() const
	{
		return "love.thread.sdl";
	}
} // sdl
} // thread
} // love
