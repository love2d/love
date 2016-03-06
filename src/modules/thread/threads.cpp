/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

Lock::Lock(Mutex *m)
	: mutex(m)
{
	mutex->lock();
}

Lock::Lock(Mutex &m)
	: mutex(&m)
{
	mutex->lock();
}

Lock::~Lock()
{
	mutex->unlock();
}

EmptyLock::EmptyLock()
	: mutex(nullptr)
{
}

EmptyLock::~EmptyLock()
{
	if (mutex)
		mutex->unlock();
}

void EmptyLock::setLock(Mutex *m)
{
	if (m)
		m->lock();

	if (mutex)
		mutex->unlock();

	mutex = m;
}

void EmptyLock::setLock(Mutex &m)
{
	m.lock();

	if (mutex)
		mutex->unlock();

	mutex = &m;
}

Threadable::Threadable()
{
	owner = newThread(this);
}

Threadable::~Threadable()
{
	delete owner;
}

bool Threadable::start()
{
	return owner->start();
}

void Threadable::wait()
{
	owner->wait();
}

bool Threadable::isRunning() const
{
	return owner->isRunning();
}

const char *Threadable::getThreadName() const
{
	return threadName.empty() ? nullptr : threadName.c_str();
}

MutexRef::MutexRef()
	: mutex(newMutex())
{
}

MutexRef::~MutexRef()
{
	delete mutex;
}

MutexRef::operator Mutex*() const
{
	return mutex;
}

Mutex *MutexRef::operator->() const
{
	return mutex;
}

ConditionalRef::ConditionalRef()
	: conditional(newConditional())
{
}

ConditionalRef::~ConditionalRef()
{
	delete conditional;
}

ConditionalRef::operator Conditional*() const
{
	return conditional;
}

Conditional *ConditionalRef::operator->() const
{
	return conditional;
}

} // thread
} // love
