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

#ifndef LOVE_SYSTEM_H
#define LOVE_SYSTEM_H

// LOVE
#include "common/config.h"
#include "common/Module.h"
#include "common/StringMap.h"

// stdlib
#include <string>

namespace love
{
namespace system
{

class System : public Module
{
public:

	enum PowerState
	{
		POWER_UNKNOWN,
		POWER_BATTERY,
		POWER_NO_BATTERY,
		POWER_CHARGING,
		POWER_CHARGED,
		POWER_MAX_ENUM
	};

	System();
	virtual ~System() {}

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_SYSTEM; }

	/**
	 * Gets the current operating system.
	 **/
	std::string getOS() const;

	/**
	 * Gets the number of reported CPU cores on the current system.
	 * Does not account for technologies such as Hyperthreading: a 4-core
	 * Hyperthreading-enabled Intel CPU will report 8, instead of 4.
	 **/
	virtual int getProcessorCount() const = 0;

	/**
	 * Replaces the contents of the system's text clipboard with a string.
	 * @param text The clipboard text to set.
	 **/
	virtual void setClipboardText(const std::string &text) const = 0;

	/**
	 * Gets the contents of the system's text clipboard.
	 **/
	virtual std::string getClipboardText() const = 0;

	/**
	 * Gets information about the system's power supply.
	 *
	 * @param[out] seconds Time in seconds of battery life left.
	 *             -1 if a value can't be determined.
	 * @param[out] percent The percentage of battery life left (0-100.)
	 *             -1 if a value can't be determined.
	 *
	 * @return The current state of the battery.
	 **/
	virtual PowerState getPowerInfo(int &seconds, int &percent) const = 0;

	/**
	 * Opens the specified URL with the user's default program to handle that
	 * particular URL type.
	 *
	 * @param url The URL to open.
	 *
	 * @return Whether the URL was opened successfully.
	 **/
	virtual bool openURL(const std::string &url) const;

	/**
	 * Vibrates for the specified amount of seconds.
	 *
	 * @param number of seconds to vibrate.
	 */
	virtual void vibrate(double seconds) const;

	static bool getConstant(const char *in, PowerState &out);
	static bool getConstant(PowerState in, const char *&out);

private:

	static StringMap<PowerState, POWER_MAX_ENUM>::Entry powerEntries[];
	static StringMap<PowerState, POWER_MAX_ENUM> powerStates;

}; // System

} // system
} // love

#endif // LOVE_SYSTEM_H
