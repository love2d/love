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

#include <SDL_version.h>

// C++
#include <algorithm>

namespace love
{
namespace touch
{
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
#if SDL_VERSION_ATLEAST(3, 0, 0)
	case SDL_EVENT_FINGER_DOWN:
#else
	case SDL_FINGERDOWN:
#endif
		touches.erase(std::remove_if(touches.begin(), touches.end(), compare), touches.end());
		touches.push_back(info);
		break;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	case SDL_EVENT_FINGER_MOTION:
#else
	case SDL_FINGERMOTION:
#endif
	{
		for (TouchInfo &touch : touches)
		{
			if (touch.id == info.id)
				touch = info;
		}
		break;
	}
#if SDL_VERSION_ATLEAST(3, 0, 0)
	case SDL_EVENT_FINGER_UP:
#else
	case SDL_FINGERUP:
#endif
		touches.erase(std::remove_if(touches.begin(), touches.end(), compare), touches.end());
		break;
	default:
		break;
	}
}

} // sdl
} // touch
} // love
