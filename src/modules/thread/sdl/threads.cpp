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

	Mutex::Mutex() {
		mutex = SDL_CreateMutex();
	}

	Mutex::~Mutex() {
		SDL_DestroyMutex(mutex);
	}

	void Mutex::lock() {
		SDL_mutexP(mutex);
	}

	void Mutex::unlock() {
		SDL_mutexV(mutex);
	}



	int ThreadBase::thread_runner(void* param) {
		ThreadBase* thread = (ThreadBase*)param;
		thread->main();
		return 0;
	}

	ThreadBase::ThreadBase() : running(false) {
		SDL_Thread* thread;
	}

	ThreadBase::~ThreadBase() {
		if (running) {
			wait();
		}
	}

	bool ThreadBase::start() {
		thread = SDL_CreateThread(thread_runner, this);
		if (thread == NULL) {
			return false;
		} else {
			running = true;
			return true;
		}
	}

	void ThreadBase::wait() {
		SDL_WaitThread(thread, NULL);
		running = false;
	}

	void ThreadBase::kill() {
		SDL_KillThread(thread);
		running = false;
	}

	unsigned int ThreadBase::threadId() {
		return (unsigned int)SDL_ThreadID();
	}

	Semaphore::Semaphore(unsigned int initial_value) {
		semaphore = SDL_CreateSemaphore(initial_value);
	}

	Semaphore::~Semaphore() {
		SDL_DestroySemaphore(semaphore);
	}

	unsigned int Semaphore::value() {
		return SDL_SemValue(semaphore);
	}

	void Semaphore::post() {
		SDL_SemPost(semaphore);
	}

	bool Semaphore::wait(int timeout) {
		if (timeout < 0) {
			return SDL_SemWait(semaphore) ? false : true;
		} else if (timeout == 0) {
			return SDL_SemTryWait(semaphore) ? false : true;
		} else {
			int ret = SDL_SemWaitTimeout(semaphore, timeout);
			if (ret == SDL_MUTEX_TIMEDOUT) {
				return false;
			} else if (ret == 0) {
				return true;
			} else {
				// some nasty error
				return false;
			}
		}
	}

	bool Semaphore::tryWait() {
		return SDL_SemTryWait(semaphore) ? false : true;
	}



	Conditional::Conditional() {
		cond = SDL_CreateCond();
	}

	Conditional::~Conditional() {
		SDL_DestroyCond(cond);
	}

	void Conditional::signal() {
		SDL_CondSignal(cond);
	}

	void Conditional::broadcast()  {
		SDL_CondBroadcast(cond);
	}

	bool Conditional::wait(Mutex* mutex, int timeout) {
		if (timeout < 0) {
			if (SDL_CondWait(cond, mutex->mutex)) {
				// error
				return false;
			} else {
				return true;
			}
		} else {
			int ret = SDL_CondWaitTimeout(cond, mutex->mutex, timeout);
			if (ret == SDL_MUTEX_TIMEDOUT) {
				return false;
			} else if (ret == 0) {
				return true;
			} else {
				// some bad error
				return false;
			}
		}
	}

} // namespace thread
} // namespace love
