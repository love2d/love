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

// LOVE
#include "common/config.h"
#include "common/Exception.h"
#include "Touch.h"

// C++
#include <algorithm>

#include <SDL3/SDL_hints.h>

namespace love
{
namespace touch
{

// See src/modules/touch/Touch.cpp.
void setTrackpadTouchImplementation(bool enable)
{
	SDL_SetHint(SDL_HINT_TRACKPAD_IS_TOUCH_ONLY, enable ? "1" : "0");
}

namespace sdl
{

Touch::Touch()
	: love::touch::Touch("love.touch.sdl")
{
}

const std::vector<Touch::TouchInfo> &Touch::getTouches() const
{
	return touches;
}

const Touch::TouchInfo &Touch::getTouch(int64 id) const
{
	for (const auto &touch : touches)
	{
		if (touch.id == id)
			return touch;
	}

	throw love::Exception("Invalid active touch ID: %d", id);
}

void Touch::onEvent(Uint32 eventtype, const TouchInfo &info)
{
	auto compare = [&](const TouchInfo &touch) -> bool
	{
		return touch.id == info.id;
	};

	switch (eventtype)
	{
	case SDL_EVENT_FINGER_DOWN:
		touches.erase(std::remove_if(touches.begin(), touches.end(), compare), touches.end());
		touches.push_back(info);
		break;
	case SDL_EVENT_FINGER_MOTION:
	{
		for (TouchInfo &touch : touches)
		{
			if (touch.id == info.id)
				touch = info;
		}
		break;
	}
	case SDL_EVENT_FINGER_UP:
		touches.erase(std::remove_if(touches.begin(), touches.end(), compare), touches.end());
		break;
	default:
		break;
	}
}

Touch::DeviceType Touch::getDeviceType(SDL_TouchDeviceType sdltype)
{
	switch (sdltype)
	{
		case SDL_TOUCH_DEVICE_DIRECT: return DEVICE_TOUCHSCREEN;
		case SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE: return DEVICE_TOUCHPAD;
		case SDL_TOUCH_DEVICE_INDIRECT_RELATIVE: return DEVICE_TOUCHPAD_RELATIVE;
		default: return DEVICE_TOUCHSCREEN;
	}
}

} // sdl
} // touch
} // love
