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
		InitializeCriticalSection(&mutex);
	}



	Mutex::~Mutex() {
		DeleteCriticalSection(&mutex);
	}



	void Mutex::lock() {
		EnterCriticalSection(&mutex);
	}



	void Mutex::unlock() {
		LeaveCriticalSection(&mutex);
	}




	int ThreadBase::thread_runner(void* param) {
		ThreadBase* thread = (ThreadBase*)param;
		thread->main();
		return 0;
	}

	ThreadBase::ThreadBase() : running(false) {
		HANDLE thread;
	}



	ThreadBase::~ThreadBase() {
		if (running) {
			wait();
		}
	}



	bool ThreadBase::start() {
		thread = CreateThread(NULL, 0, run_thread, rd, NULL);
		if (thread == NULL) {
			return false;
		} else {
			running = true;
			return true;
		}
	}

	void ThreadBase::wait() {
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
		running = false;
	}

	void ThreadBase::kill() {
		TerminateThread(thread, FALSE);
		running = false;
	}

	unsigned int ThreadBase::threadId() {
		return (unsigned int)GetCurrentThreadId();
	}




	Semaphore::Semaphore(unsigned int initial_value)
	: count(initial_value) {
		semaphore = CreateSemaphore(NULL, initial_value, 65535, NULL);
	}

	Semaphore::~Semaphore() {
		CloseHandle(semaphore);
	}

	unsigned int Semaphore::value() {
		return count;
	}

	void Semaphore::post() {
		InterlockedIncrement(&count);
		if (ReleaseSemaphore(semaphore, 1, NULL) == FALSE) {
			InterlockedDecrement(&count);
		}
	}

	bool Semaphore::wait(int timeout)  {
		int result;

		result = WaitForSingleObject(semaphore, timeout < 0 ? INFINITE : timeout);
		if (result == WAIT_OBJECT_0) {
			InterlockedDecrement(&count);
			return true;
		} else if (result == WAIT_TIMEOUT) {
			return false;
		} else {
			// error
			return false;
		}
	}

	bool Semaphore::tryWait() {
		return wait(0);
	}


	// Conditional variable implementation based on semaphores.
	// Vista+ builds should probably use CONDITIONAL_VARIABLE.
	//
	// based on http://www.cs.wustl.edu/~schmidt/win32-cv-2.html and ACE


	Conditional::Conditional()
	: waiting(0), signals(0) {

	}

	Conditional::~Conditional() {

	}

	void Conditional::signal() {
		mutex.lock();
		if (waiting > signals) {
			signals++;
			sem.post();
			mutex.unlock();
			done.wait();
		} else {
			mutex.unlock();
		}
	}

	void Conditional::broadcast() {
		mutex.lock();
		if (waiting > signals) {
			int num = waiting - signals;
			signals = waiting;
			for(int i = 0; i < num; i++) sem.post();
			mutex.unlock();
			for(int i = 0; i < num; i++) done.wait();
		} else {
			mutex.unlock();
		}
	}

	bool Conditional::wait(Mutex* cmutex, int timeout) {
		mutex.lock();
		waiting++;
		mutex.unlock();

		cmutex->unlock();
		bool ret = sem.wait(timeout);


		mutex.lock();
		if (signals > 0) {
			if (!ret) sem.wait();
			done.post();
			signals--;
		}
		waiting--;
		mutex.unlock();
		cmutex->lock();
	}

} // namespace thread
} // namespace love
