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

#ifndef LOVE_THREAD_THREADS_H
#define LOVE_THREAD_THREADS_H

#include "common/config.h"

#define LOVE_THREADS_SDL	0
#define LOVE_THREADS_WIN32	1
#define LOVE_THREADS_POSIX	2

// Choose correct threads API.
// Headless love uses either windows threads or posix threads.
// Headed (standard) love uses SDL threads.
#ifdef LOVE_HEADLESS
#  ifdef WIN32
#    define LOVE_THREADS	LOVE_THREADS_WIN32
#  else
#    define LOVE_THREADS	LOVE_THREADS_POSIX
#  endif
#else
#  define LOVE_THREADS	LOVE_THREADS_SDL
#endif


// include the correct header
#if LOVE_THREADS == LOVE_THREADS_POSIX
#  include "posix/threads.h"
#elif LOVE_THREADS == LOVE_THREADS_WIN32
#  include "win32/threads.h"
#elif LOVE_THREADS == LOVE_THREADS_SDL
#  include "sdl/threads.h"
#endif


namespace love
{
namespace thread
{

const char *threadAPI();

class Lock
{
public:
	Lock(Mutex *m): mutex(m)
	{
		mutex->lock();
	}

	Lock(Mutex &m): mutex(&m)
	{
		mutex->lock();
	}

	~Lock()
	{
		mutex->unlock();
	}
private:
	Mutex *mutex;

	Lock(Lock &/* lock*/) {}
};

} // thread
} // love

#endif /* LOVE_THREAD_THREADS_H */
