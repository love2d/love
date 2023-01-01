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

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "common/delay.h"
#include "Timer.h"

#include <iostream>
#if defined(LOVE_WINDOWS)
#include <windows.h>
#elif defined(LOVE_MACOSX) || defined(LOVE_IOS)
#include <mach/mach_time.h>
#include <sys/time.h>
#elif defined(LOVE_LINUX)
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#endif

namespace love
{
namespace timer
{

Timer::Timer()
	: currTime(0)
	, prevFpsUpdate(0)
	, fps(0)
	, averageDelta(0)
	, fpsUpdateFrequency(1)
	, frames(0)
	, dt(0)
{
	prevFpsUpdate = currTime = getTime();
}

double Timer::step()
{
	// Frames rendered
	frames++;

	// "Current" time is previous time by now.
	prevTime = currTime;

	// Get time from system.
	currTime = getTime();

	// Convert to number of seconds.
	dt = currTime - prevTime;

	double timeSinceLast = currTime - prevFpsUpdate;
	// Update FPS?
	if (timeSinceLast > fpsUpdateFrequency)
	{
		fps = int((frames/timeSinceLast) + 0.5);
		averageDelta = timeSinceLast/frames;
		prevFpsUpdate = currTime;
		frames = 0;
	}

	return dt;
}

void Timer::sleep(double seconds) const
{
	if (seconds >= 0)
		love::sleep((unsigned int)(seconds*1000));
}

double Timer::getDelta() const
{
	return dt;
}

int Timer::getFPS() const
{
	return fps;
}

double Timer::getAverageDelta() const
{
	return averageDelta;
}

#if defined(LOVE_LINUX)

static inline timespec getTimeOfDay()
{
	timeval t;
	gettimeofday(&t, NULL);
	return timespec { t.tv_sec, t.tv_usec * 1000 };
}

static timespec getTimeAbsolute()
{
	// Check for POSIX timers and monotonic clocks. If not supported, use the gettimeofday fallback.
#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK) \
&& (defined(CLOCK_MONOTONIC_RAW) || defined(CLOCK_MONOTONIC))

#ifdef CLOCK_MONOTONIC_RAW
	clockid_t clk_id = CLOCK_MONOTONIC_RAW;
#else
	clockid_t clk_id = CLOCK_MONOTONIC;
#endif

	timespec t;
	if (clock_gettime(clk_id, &t) == 0)
		return t;
	else
		return getTimeOfDay();
#endif
	return getTimeOfDay();
}

double Timer::getTime()
{
	static const timespec start = getTimeAbsolute();
	const timespec now = getTimeAbsolute();
	// tv_sec and tv_nsec should be signed on POSIX, so we are fine in just subtracting here.
	const long sec = now.tv_sec - start.tv_sec;
	const long nsec = now.tv_nsec - start.tv_nsec;
	return (double) sec + (double) nsec / 1.0e9;
}

#elif defined(LOVE_MACOSX) || defined(LOVE_IOS)

static mach_timebase_info_data_t getTimebaseInfo()
{
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return info;
}

double Timer::getTime()
{
	static const mach_timebase_info_data_t info = getTimebaseInfo();
	static const uint64_t start = mach_absolute_time();
	const uint64_t rel = mach_absolute_time() - start;
	return ((double) rel * 1.0e-9) * (double) info.numer / (double) info.denom;
}

#elif defined(LOVE_WINDOWS)

static LARGE_INTEGER getTimeAbsolute()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t;
}

static LARGE_INTEGER getFrequency()
{
	LARGE_INTEGER freq;
	// "On systems that run Windows XP or later, the function will always succeed and will thus never return zero."
	QueryPerformanceFrequency(&freq);
	return freq;
}

double Timer::getTime()
{
	static const LARGE_INTEGER freq = getFrequency();
	static const LARGE_INTEGER start = getTimeAbsolute();
	const LARGE_INTEGER now = getTimeAbsolute();
	LARGE_INTEGER rel;
	rel.QuadPart = now.QuadPart - start.QuadPart;
	return (double) rel.QuadPart / (double) freq.QuadPart;
}

#endif

} // timer
} // love
