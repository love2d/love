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

#include "common/config.h"
#include "common/delay.h"

#ifdef LOVE_WINDOWS
#	include <windows.h>
#	include <time.h>
#else
#	include <sys/time.h>
#endif

#include "Timer.h"

namespace love
{
namespace timer
{
namespace sdl
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
	// Init the SDL timer system.
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
		throw Exception(SDL_GetError());
}

Timer::~Timer()
{
	// Quit SDL timer.
	SDL_QuitSubSystem(SDL_INIT_TIMER);
}

const char *Timer::getName() const
{
	return "love.timer.sdl";
}

void Timer::step()
{
	// Frames rendered
	frames++;

	// "Current" time is previous time by now.
	prevTime = currTime;

	// Get ticks from SDL
	currTime = SDL_GetTicks();

	// Convert to number of seconds
	dt = (currTime - prevTime)/1000.0;

	double timeSinceLast = (currTime - prevFpsUpdate)/1000.0;
	// Update FPS?
	if (timeSinceLast > fpsUpdateFrequency)
	{
		fps = int((frames/timeSinceLast) + 0.5);
		averageDelta = timeSinceLast/frames;
		prevFpsUpdate = currTime;
		frames = 0;
	}
}

void Timer::sleep(double seconds) const
{
	if (seconds > 0)
		delay((int)(seconds*1000));
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

double Timer::getTime() const
{
	return SDL_GetTicks()/1000.0;
}

double Timer::getMicroTime() const
{
#ifdef LOVE_WINDOWS
	static __int64 freq = 0;

	if (!freq)
	{
		LARGE_INTEGER temp;
		QueryPerformanceFrequency(&temp);

		freq = (__int64) temp.QuadPart;
	}

	LARGE_INTEGER microTime;
	QueryPerformanceCounter(&microTime);

	// The 64 to 32 bit integer conversion, assuming the fraction part down
	// to microseconds takes 20 bits, should not be a problem unless the
	// system has an uptime of a few decades.
	return (double) microTime.QuadPart / (double) freq;
#else
	timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + t.tv_usec/1000000.0;
#endif
}

} // sdl
} // timer
} // love
