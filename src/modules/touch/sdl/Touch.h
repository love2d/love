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

#ifndef LOVE_TOUCH_SDL_TOUCH_H
#define LOVE_TOUCH_SDL_TOUCH_H

// LOVE
#include "touch/Touch.h"

// SDL
#include <SDL_events.h>

namespace love
{
namespace touch
{
namespace sdl
{

class Touch : public love::touch::Touch
{
public:

	virtual ~Touch() {}

	const std::vector<TouchInfo> &getTouches() const override;
	const TouchInfo &getTouch(int64 id) const override;

	// Implements Module.
	const char *getName() const override;

	// SDL has functions to query the state of touch presses, but unfortunately
	// they are updated on a different thread in some backends, which causes
	// issues especially if the user is iterating through the current touches
	// when they're updated. So we only update our touch press state in
	// love::event::sdl::Event::convert.
	void onEvent(Uint32 eventtype, const TouchInfo &info);

private:

	// All current touches.
	std::vector<TouchInfo> touches;

}; // Touch

} // sdl
} // touch
} // love

#endif // LOVE_TOUCH_SDL_TOUCH_H
