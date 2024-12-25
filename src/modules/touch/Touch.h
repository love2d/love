/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#ifndef LOVE_TOUCH_TOUCH_H
#define LOVE_TOUCH_TOUCH_H

// LOVE
#include "common/int.h"
#include "common/Object.h"
#include "common/Module.h"
#include "common/StringMap.h"

// C++
#include <vector>
#include <limits>

namespace love
{
namespace touch
{

void setTrackpadTouch(bool enable);

class Touch : public Module
{
public:

	enum DeviceType
	{
		DEVICE_TOUCHSCREEN,
		DEVICE_TOUCHPAD,
		DEVICE_TOUCHPAD_RELATIVE,
		DEVICE_MAX_ENUM
	};

	struct TouchInfo
	{
		int64 id;  // Identifier. Only unique for the duration of the touch-press.
		double x;  // Position in pixels (for touchscreens) or normalized [0, 1] position (for touchpads) along the x-axis.
		double y;  // Position in pixels (for touchscreens) or normalized [0, 1] position (for touchpads) along the y-axis.
		double dx; // Amount moved along the x-axis.
		double dy; // Amount moved along the y-axis.
		double pressure;
		DeviceType deviceType;
		bool mouse;
	};

	virtual ~Touch() {}

	/**
	 * Gets all currently active touches.
	 **/
	virtual const std::vector<TouchInfo> &getTouches() const = 0;

	/**
	 * Gets a specific touch, using its ID.
	 **/
	virtual const TouchInfo &getTouch(int64 id) const = 0;

	STRINGMAP_CLASS_DECLARE(DeviceType);

protected:

	Touch(const char *name)
		: Module(M_TOUCH, name)
	{}

}; // Touch

} // touch
} // love

#endif // LOVE_TOUCH_TOUCH_H
