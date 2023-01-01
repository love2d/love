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

#ifndef LOVE_TIMER_TIMER_H
#define LOVE_TIMER_TIMER_H

// LOVE
#include "common/Module.h"

namespace love
{
namespace timer
{

class Timer : public Module
{
public:

	Timer();
	virtual ~Timer() {}

	// Implements Module.
	ModuleType getModuleType() const override { return M_TIMER; }
	const char *getName() const override { return "love.timer"; }

	/**
	 * Measures the time between this call and the previous call,
	 * and updates internal values accordingly.
	 **/
	double step();

	/**
	 * Tries to sleep for the specified amount of time. The precision is
	 * usually 1ms.
	 * @param seconds The number of seconds to sleep for.
	 **/
	void sleep(double seconds) const;

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
	int getFPS() const;

	/**
	 * Gets the average delta time (seconds per frame) over the last second.
	 **/
	double getAverageDelta() const;

	/**
	 * Gets the amount of time in seconds passed since its first invocation
	 * (which happens as part of the Timer constructor,
	 * which is called when the module is first opened).
	 * Useful for profiling code or measuring intervals.
	 * The time is microsecond-precise, and increases monotonically.
	 * @return The time (in seconds)
	 **/
	static double getTime();

private:

	// Frame delta vars.
	double currTime;
	double prevTime;
	double prevFpsUpdate;

	// Updated with a certain frequency.
	int fps;
	double averageDelta;

	// The frequency by which to update the FPS.
	double fpsUpdateFrequency;

	// Frames since last FPS update.
	int frames;

	// The current timestep.
	double dt;

}; // Timer

} // timer
} // love

#endif // LOVE_TIMER_TIMER_H
