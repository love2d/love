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

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "Timer.h"

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

#if defined(LOVE_LINUX)
static inline double getTimeOfDay()
{
	timeval t;
	gettimeofday(&t, NULL);
	return (double) t.tv_sec + (double) t.tv_usec / 1000000.0;
}
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

void Timer::step()
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

double Timer::getTimerPeriod()
{
#if defined(LOVE_MACOSX) || defined(LOVE_IOS)
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double) info.numer / (double) info.denom / 1000000000.0;
#elif defined(LOVE_WINDOWS)
	LARGE_INTEGER temp;
	if (QueryPerformanceFrequency(&temp) != 0 && temp.QuadPart != 0)
		return 1.0 / (double) temp.QuadPart;
#endif
	return 0;
}

double Timer::getTimeSinceEpoch()
{
	// The timer period (reciprocal of the frequency.)
	static const double timerPeriod = getTimerPeriod();

#if defined(LOVE_LINUX)
	double mt;
	// Check for POSIX timers and monotonic clocks. If not supported, use the gettimeofday fallback.
#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK) \
&& (defined(CLOCK_MONOTONIC_RAW) || defined(CLOCK_MONOTONIC))
	timespec t;
#ifdef CLOCK_MONOTONIC_RAW
	clockid_t clk_id = CLOCK_MONOTONIC_RAW;
#else
	clockid_t clk_id = CLOCK_MONOTONIC;
#endif
	if (clock_gettime(clk_id, &t) == 0)
		mt = (double) t.tv_sec + (double) t.tv_nsec / 1000000000.0;
	else
#endif
		mt = getTimeOfDay();
	return mt;
#elif defined(LOVE_MACOSX) || defined(LOVE_IOS)
	return (double) mach_absolute_time() * timerPeriod;
#elif defined(LOVE_WINDOWS)
	LARGE_INTEGER microTime;
	QueryPerformanceCounter(&microTime);
	return (double) microTime.QuadPart * timerPeriod;
#endif
}

double Timer::getTime() const
{
	return getTimeSinceEpoch();
}

} // timer
} // love
