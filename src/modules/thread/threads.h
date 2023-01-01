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

#ifndef LOVE_THREAD_THREADS_H
#define LOVE_THREAD_THREADS_H

// LOVE
#include "common/config.h"
#include "Thread.h"

// C++
#include <string>

namespace love
{
namespace thread
{

class Mutex
{
public:
	virtual ~Mutex() {}

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class Conditional
{
public:
	virtual ~Conditional() {}

	virtual void signal() = 0;
	virtual void broadcast() = 0;
	virtual bool wait(Mutex *mutex, int timeout=-1) = 0;
};

class Lock
{
public:
	Lock(Mutex *m);
	Lock(Mutex &m);
	Lock(Lock &&other);
	~Lock();

private:
	Mutex *mutex;
};

class EmptyLock
{
public:
	EmptyLock();
	~EmptyLock();

	void setLock(Mutex *m);
	void setLock(Mutex &m);

private:
	Mutex *mutex;
};

class Threadable : public love::Object
{
public:
	static love::Type type;

	Threadable();
	virtual ~Threadable();

	virtual void threadFunction() = 0;

	bool start();
	void wait();
	bool isRunning() const;
	const char *getThreadName() const;

protected:

	Thread *owner;
	std::string threadName;

};

class MutexRef
{
public:
	MutexRef();
	~MutexRef();

	operator Mutex*() const;
	Mutex *operator->() const;

private:
	Mutex *mutex;
};

class ConditionalRef
{
public:
	ConditionalRef();
	~ConditionalRef();

	operator Conditional*() const;
	Conditional *operator->() const;

private:
	Conditional *conditional;
};

Mutex *newMutex();
Conditional *newConditional();
Thread *newThread(Threadable *t);

#if defined(LOVE_LINUX)
void disableSignals();
void reenableSignals();

struct ScopedDisableSignals
{
	ScopedDisableSignals() { disableSignals(); }
	~ScopedDisableSignals() { reenableSignals(); }
};
#endif

} // thread
} // love

#endif /* LOVE_THREAD_THREADS_H */
