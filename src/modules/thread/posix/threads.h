/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#ifndef LOVE_THREAD_POSIX_THREADS_H
#define LOVE_THREAD_POSIX_THREADS_H

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

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
	pthread_mutex_t mutex;
	Mutex(const Mutex &mutex) {}

	friend class Conditional;
};


class ThreadBase
{
public:
	ThreadBase();
	virtual ~ThreadBase();

	bool start();
	void wait();
	void kill();

	static unsigned int threadId();

protected:
	virtual void main() = 0;

private:
	pthread_t thread;
	ThreadBase(ThreadBase &thread) {}

	static void *thread_runner(void *param);
	bool running;
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
	Semaphore(const Semaphore &sem) {}
	sem_t sem;
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
	pthread_cond_t cond;
};

} // thread
} // love


#endif // LOVE_THREAD_POSIX_THREADS_H
