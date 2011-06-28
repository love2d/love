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

#include "threads.h"

namespace love
{
namespace thread
{
	Mutex::Mutex()
	{
		pthread_mutex_init(&mutex, NULL);
	}

	Mutex::~Mutex()
	{
		pthread_mutex_destroy(&mutex);
	}

	void Mutex::lock()
	{
		pthread_mutex_lock(&mutex);
	}

	void Mutex::unlock()
	{
		pthread_mutex_unlock(&mutex);
	}

	void* ThreadBase::thread_runner(void* param)
	{
		ThreadBase* thread = (ThreadBase*)param;
		thread->main();
		return NULL;
	}

	ThreadBase::ThreadBase()
		: running(false)
	{
		pthread_t thread;
	}

	ThreadBase::~ThreadBase()
	{
		if (running)
		{
			wait();
		}
	}

	bool ThreadBase::start()
	{
		if (pthread_create(&thread, NULL, thread_runner, this))
			return false;
		return (running = true);
	}

	void ThreadBase::wait()
	{
		pthread_join(thread, NULL);
		running = false;
	}

	void ThreadBase::kill()
	{
		pthread_kill(thread, 9);
		running = false;
	}

	unsigned int ThreadBase::threadId()
	{
		return (unsigned int)((size_t)pthread_self());
	}

	Semaphore::Semaphore(unsigned int initial_value)
	{
		sem_init(&sem, 0, initial_value);
	}

	Semaphore::~Semaphore()
	{
		sem_destroy(&sem);
	}

	unsigned int Semaphore::value()
	{
		int val = 0;
		if (sem_getvalue(&sem, &val)) {
			return 0;
		} else {
			return val;
		}
	}

	void Semaphore::post()
	{
		sem_post(&sem);
	}

	bool Semaphore::wait(int timeout)
	{
		if (timeout < 0)
			return !sem_wait(&sem);
		else if (timeout == 0)
			return !sem_trywait(&sem);
		else
		{
			struct timespec ts;
			ts.tv_sec = timeout/1000;
			ts.tv_nsec = (timeout % 1000) * 1000000;
			return !sem_timedwait(&sem, &ts);
		}
	}

	bool Semaphore::tryWait()
	{
		return !sem_trywait(&sem);
	}

	Conditional::Conditional()
	{
		pthread_cond_init(&cond, NULL);
	}

	Conditional::~Conditional()
	{
		pthread_cond_destroy(&cond);
	}

	void Conditional::signal()
	{
		pthread_cond_signal(&cond);
	}

	void Conditional::broadcast()
	{
		pthread_cond_broadcast(&cond);
	}

	bool Conditional::wait(Mutex* mutex, int timeout)
	{
		if (timeout < 0)
			return !pthread_cond_wait(&cond, &mutex->mutex);
		else
		{
			struct timespec ts;
			int ret;

			ts.tv_sec = timeout / 1000;
			ts.tv_nsec = (timeout % 1000) * 1000000;

			ret = pthread_cond_timedwait(&cond, &mutex->mutex, &ts);
			return (ret == 0);
		}
	}
} // thread
} // love

