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

#ifndef LOVE_TIMER_SDL_TIMER_H
#define LOVE_TIMER_SDL_TIMER_H

// SDL
#include <SDL.h>

// LOVE
#include <common/Module.h>

namespace love
{
namespace timer
{
namespace sdl
{
	/**
	* An SDL timer module. Can keep track of time between certain function
	* calls, and provides access to a FPS metric which updates once each second.
	**/
	class Timer : public Module
	{
	private:

		// Frame delta vars.
		Uint32 currTime;
		Uint32 prevTime;
		Uint32 prevFpsUpdate;

		// Updated with a certain frequency.
		double fps;

		// The frequency by which to update the FPS.
		double fpsUpdateFrequency;

		// Frames since last FPS update.
		int frames;

		// The current timestep.
		double dt;

	public:

		/**
		* Constructor. Initializes the SDL/timer subsystem.
		**/
		Timer();

		/**
		* Destructor.
		**/
		~Timer();

		/**
		* Gets the name of the module.
		* @return Always returns "love.timer.sdl".
		**/
		const char * getName() const;

		/**
		* Measures the time between this call and the previous call,
		* and updates internal values accordinly.
		**/
		void step();

		/**
		* Tries to sleep for the specified amount of time. The precision is
		* usually 1ms.
		* @param seconds The number of seconds to sleep for.
		**/
		void sleep(double seconds);

		/**
		* Gets the time between the last two frames, assuming step is called
		* each frame.
		**/
		double getDelta() const;

		/**
		* Gets the average FPS over the last second. Beucase the value is only updated
		* once per second, it does not look erratic when displayed on screen.
		* @return The "current" FPS.
		**/
		double getFPS() const;

		/**
		* Gets the amount of time since the program started. Only useful for timing
		* code or measuring intervals.
		* @return The time (in seconds) since the program started.
		**/
		double getTime() const;

		/**
		 * Gets the amount of time passed since an unspecified time. The time is accurate
		 * to the microsecond, and is limited to 24 hours.
		 * @return The time (in seconds)
		 **/
		double getMicroTime() const;

	}; // Timer

} // sdl
} // timer
} // love

#endif // LOVE_TIMER_SDL_TIMER_H
