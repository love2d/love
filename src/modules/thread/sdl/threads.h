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

#ifndef LOVE_THREAD_SDL_THREADS_H
#define LOVE_THREAD_SDL_THREADS_H

#include "SDL.h"
#include "common/config.h"

namespace love
{
namespace thread
{

class Mutex
{
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
private:
	SDL_mutex *mutex;
	Mutex(const Mutex &/* mutex*/) {}

	friend class Conditional;
};



class ThreadBase
{
public:
	ThreadBase();
	virtual ~ThreadBase();

	bool start();
	void wait();
	void kill(); // FIXME: not supported by SDL (SDL's kill is probably cancel)?

	static unsigned int threadId();

protected:
	virtual void main() = 0;

private:
	SDL_Thread *thread;
	ThreadBase(ThreadBase &/* thread*/) {}
	bool running;

	static int thread_runner(void *param);
};

class Semaphore
{
public:
	Semaphore(unsigned int initial_value);
	~Semaphore();

	unsigned int value();
	void post();
	bool wait(int timeout = -1);
	bool tryWait();

private:
	Semaphore(const Semaphore &/* sem*/) {}
	SDL_sem *semaphore;
};

// Should conditional inherit from mutex?
class Conditional
{
public:
	Conditional();
	~Conditional();

	void signal();
	void broadcast();
	bool wait(Mutex *mutex, int timeout=-1);

private:
	SDL_cond *cond;
};

} // thread
} // love


#endif // LOVE_THREAD_SDL_THREADS_H
