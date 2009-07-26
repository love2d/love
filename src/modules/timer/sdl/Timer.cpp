/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#include "Timer.h"

namespace love
{
namespace timer
{
namespace sdl
{
	Timer::Timer()
		: time_init(0), currTime(0), prevFpsUpdate(0), fps(0), fpsUpdateFrequency(1), 
		frames(0), dt(0)
	{
		// Init the SDL timer system.
		if(SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
			throw Exception(SDL_GetError());
	}

	Timer::~Timer()
	{
		// Quit SDL timer.
		SDL_QuitSubSystem(SDL_INIT_TIMER);
	}

	const char * Timer::getName() const
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
		dt = (currTime - prevTime)/1000.0f;

		// Update FPS?
		if((currTime - prevFpsUpdate)/1000.0f > fpsUpdateFrequency)
		{
			fps = frames/fpsUpdateFrequency;
			prevFpsUpdate = currTime;
			frames = 0;
		}
	}

	void Timer::sleep(unsigned int ms)
	{
		SDL_Delay(ms);
	}

	float Timer::getDelta() const
	{
		return dt;
	}

	float Timer::getFPS() const
	{
		return fps;
	}

	float Timer::getTime() const
	{
		return (SDL_GetTicks() - time_init)/1000.0f;
	}

} // sdl
} // timer
} // love
