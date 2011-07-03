/**
* Copyright (c) 2006-2011 LOVE Development Team
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
extern "C" int luaopen_love_thread(lua_State *L);

namespace love
{
namespace thread
{

	Thread::ThreadThread::ThreadThread(ThreadData* comm)
		: comm(comm)
	{
	}

	void Thread::ThreadThread::main()
	{
		lua_State * L = lua_open();
		luaL_openlibs(L);
	#ifdef LOVE_BUILD_STANDALONE
		love::luax_preload(L, luaopen_love, "love");
		luaopen_love(L);
	#endif // LOVE_BUILD_STANDALONE
		luaopen_love_thread(L);
		{
			size_t len;
			const char *name = comm->getName(&len);
			lua_pushlstring(L, name, len);
		}
 		luax_convobj(L, lua_gettop(L), "thread", "getThread");
		lua_getglobal(L, "love");
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "_curthread");
		if(luaL_dostring(L, comm->getCode()) == 1)
		{
			{
				Lock lock((Mutex*) comm->mutex);
				ThreadVariant *v = new ThreadVariant(lua_tostring(L, -1), lua_strlen(L, -1));
				comm->setValue("error", v);
				v->release();
			}
			((Conditional*) comm->cond)->broadcast();
		}
		lua_close(L);
	}


	ThreadVariant::ThreadVariant(bool boolean)
	{
		type = BOOLEAN;
		data.boolean = boolean;
	}

	ThreadVariant::ThreadVariant(double number)
	{
		type = NUMBER;
		data.number = number;
	}

	ThreadVariant::ThreadVariant(const char *string, size_t len)
	{
		type = STRING;
		char *buf = new char[len+1];
		memset(buf, 0, len+1);
		memcpy(buf, string, len);
		data.string.str = buf;
		data.string.len = len;
	}

	ThreadVariant::ThreadVariant(void *userdata)
	{
		type = LUSERDATA;
		data.userdata = userdata;
	}

	ThreadVariant::ThreadVariant(Type udatatype, void *userdata)
	{
		type = FUSERDATA;
		this->udatatype = udatatype;
		if (udatatype != INVALID_ID)
		{
			Proxy *p = (Proxy *) userdata;
			flags = p->flags;
			data.userdata = p->data;
			((love::Object *) data.userdata)->retain();
		}
		else
			data.userdata = userdata;
	}

	ThreadVariant::~ThreadVariant()
	{
		switch(type)
		{
			case STRING:
				delete[] data.string.str;
				break;
			case FUSERDATA:
				((love::Object *) data.userdata)->release();
				break;
			default:
				break;
		}
	}

	ThreadData::ThreadData(const char *name, size_t len, const char *code, void *mutex, void *cond)
		: len(len), mutex(mutex), cond(cond)
	{
		this->name = new char[len+1];
		memset(this->name, 0, len+1);
		memcpy(this->name, name, len);
		if (code)
		{
			len = strlen(code);
			this->code = new char[len+1];
			memset(this->code, 0, len+1);
			memcpy(this->code, code, len);
		}
		else
			this->code = 0;
	}

	ThreadData::~ThreadData()
	{
		delete[] name;
		delete[] code;
	}

	const char *ThreadData::getCode()
	{
		return code;
	}

	const char *ThreadData::getName(size_t *len)
	{
		if (len)
			*len = this->len;
		return name;
	}

	ThreadVariant* ThreadData::getValue(const std::string & name)
	{
		if (shared.count(name) == 0)
			return 0;
		return shared[name];
	}

	void ThreadData::clearValue(const std::string & name)
	{
		if (shared.count(name) == 0)
			return;
		shared[name]->release();
		shared.erase(name);
	}

	void ThreadData::setValue(const std::string & name, ThreadVariant *v)
	{
		if (shared.count(name) != 0)
			shared[name]->release();
		v->retain();
		shared[name] = v;
	}

	Thread::Thread(love::thread::ThreadModule *module, const std::string & name, love::Data *data)
		: handle(0), module(module), name(name), isThread(true)
	{
		module->retain();
		unsigned int len = data->getSize();
		this->data = new char[len+1];
		memset(this->data, 0, len+1);
		memcpy(this->data, data->getData(), len);
		mutex = new Mutex();
		cond = new Conditional();
		comm = new ThreadData(name.c_str(), name.length(), this->data, mutex, cond);
	}

	Thread::Thread(love::thread::ThreadModule *module, const std::string & name)
		: handle(0), module(module), name(name), data(0), isThread(false)
	{
		module->retain();
		mutex = new Mutex();
		cond = new Conditional();
		comm = new ThreadData(name.c_str(), name.length(), NULL, mutex, cond);
	}

	Thread::~Thread()
	{
		if (data)
			delete[] data;
		delete comm;
		module->unregister(name);
		delete mutex;
		delete cond;
		module->release();
	}

	void Thread::start()
	{
		if (!handle && isThread) {
			handle = new ThreadThread(comm);
			handle->start();
		}
	}

	void Thread::kill()
	{
		if (handle)
		{
			Lock lock((Mutex *) _gcmutex);
			handle->kill();
			delete handle;
			handle = 0;
		}
	}

	void Thread::wait()
	{
		if (handle)
		{
			handle->wait();
			delete handle;
			handle = 0;
		}
	}

	void Thread::lock()
	{
		mutex->lock();
	}

	void Thread::unlock()
	{
		mutex->unlock();
	}

	std::string Thread::getName()
	{
		return name;
	}

	ThreadVariant *Thread::get(const std::string & name)
	{
		ThreadVariant *v = comm->getValue(name);
		if (v)
			v->retain();
		return v;
	}

	ThreadVariant *Thread::demand(const std::string & name)
	{
		ThreadVariant *v = comm->getValue(name);
		while (!v)
		{
			if (comm->getValue("error"))
				return 0;
			cond->wait(mutex);
			v = comm->getValue(name);
		}
		v->retain();
		return v;
	}

	void Thread::clear(const std::string & name)
	{
		comm->clearValue(name);
	}

	void Thread::set(const std::string & name, ThreadVariant *v)
	{
		lock(); //this function explicitly locks
		comm->setValue(name, v); //because we need
		unlock(); //it to unlock here for the cond
		cond->broadcast();
	}

	ThreadModule::ThreadModule()
	{
		threads["main"] = new Thread(this, "main");
	}

	ThreadModule::~ThreadModule()
	{
		for (threadlist_t::iterator i = threads.begin(); i != threads.end(); i++)
		{
			i->second->kill();
			delete i->second;
		}
	}

	Thread *ThreadModule::newThread(const std::string & name, love::Data *data)
	{
		if (threads.count(name) != 0)
			return 0;
		Thread *t = new Thread(this, name, data);
		threads[name] = t;
		return t;
	}

	Thread *ThreadModule::getThread(const std::string & name)
	{
		if (threads.count(name) == 0)
			return 0;
		threadlist_t::iterator i = threads.find(name);
		return i->second;
	}

	void ThreadModule::getThreads(Thread ** list)
	{
		int c = 0;
		for (threadlist_t::iterator i = threads.begin(); i != threads.end(); i++, c++)
		{
			list[c] = i->second;
		}
	}

	unsigned ThreadModule::getThreadCount() const
	{
		return threads.size();
	}

	void ThreadModule::unregister(const std::string & name)
	{
		if (threads.count(name) == 0)
			return;
		threadlist_t::iterator i = threads.find(name);
		// FIXME: shouldn't the thread be deleted?
		threads.erase(i);
	}

	const char *ThreadModule::getName() const
	{
		return "love.thread.sdl";
	}

} // thread
} // love
