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
	: mutex(0)
{
}

EmptyLock::~EmptyLock()
{
	if (mutex)
		mutex->unlock();
}

void EmptyLock::setLock(Mutex *m)
{
	if (mutex)
		mutex->unlock();

	mutex = m;

	if (mutex)
		mutex->lock();
}

void EmptyLock::setLock(Mutex &m)
{
	if (mutex)
		mutex->unlock();

	mutex = &m;
	mutex->lock();
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

} // thread
} // love
