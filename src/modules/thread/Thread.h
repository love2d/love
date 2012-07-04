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

#ifndef LOVE_THREAD_SDL_THREAD_H
#define LOVE_THREAD_SDL_THREAD_H

// STL
#include <map>
#include <string>
#include <vector>
#include <cstring>

// LOVE
#include "filesystem/File.h"
#include "common/runtime.h"
#include "common/Module.h"
#include "common/Variant.h"
#include "thread/threads.h"

namespace love
{
namespace thread
{

class ThreadModule;

class ThreadData
{
public:
	ThreadData(const char *name, size_t len, const char *code, void *mutex, void *cond);
	~ThreadData();
	const char *getCode();
	const char *getName(size_t *len = 0);
	Variant *getValue(const std::string &name);
	void clearValue(const std::string &name);
	void setValue(const std::string &name, Variant *v);
	std::vector<std::string> getKeys();

	void *mutex;
	void *cond;
private:
	char *code;
	char *name;
	std::map<std::string, Variant *> shared;
	size_t len;
};

class Thread : public love::Object
{
public:
	Thread(love::thread::ThreadModule *module, const std::string &name, love::Data *data);
	Thread(love::thread::ThreadModule *module, const std::string &name);
	virtual ~Thread();
	void start();
	void kill();
	void wait();
	std::string getName();
	Variant *get(const std::string &name);
	std::vector<std::string> getKeys();
	Variant *demand(const std::string &name);
	void clear(const std::string &name);
	void set(const std::string &name, Variant *v);
	void lock();
	void unlock();
private:
	class ThreadThread: public ThreadBase
	{
	private:
		ThreadData *comm;

	protected:
		virtual void main();

	public:
		ThreadThread(ThreadData *comm);
	};

	ThreadThread *handle;

	ThreadModule *module;
	ThreadData *comm;
	std::string name;
	char *data;
	Mutex *mutex;
	Conditional *cond;
	bool isThread;
}; // Thread

typedef std::map<std::string, Thread *> threadlist_t;

class ThreadModule : public love::Module
{
public:
	ThreadModule();
	virtual ~ThreadModule();
	Thread *newThread(const std::string &name, love::Data *data);
	void getThreads(Thread **list);
	Thread *getThread(const std::string &name);
	unsigned getThreadCount() const;
	void unregister(const std::string &name);
	const char *getName() const;
private:
	threadlist_t threads;
}; // ThreadModule
} // thread
} // love

#endif // LOVE_THREAD_SDL_THREAD_H
