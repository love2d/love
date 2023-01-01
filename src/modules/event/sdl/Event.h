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

#ifndef LOVE_EVENT_SDL_EVENT_H
#define LOVE_EVENT_SDL_EVENT_H

// LOVE
#include "event/Event.h"
#include "audio/Source.h"

// SDL
#include <SDL_events.h>

// STL
#include <map>

namespace love
{
namespace event
{
namespace sdl
{

class Event : public love::event::Event
{
public:

	// Implements Module.
	const char *getName() const;

	Event();
	virtual ~Event();

	/**
	 * Pumps the event queue. This function gathers all the pending input information
	 * from devices and places it on the event queue. Normally not needed if you poll
	 * for events.
	 **/
	void pump();

	/**
	 * Waits for the next event (indefinitely). Useful for creating games where
	 * the screen and game state only needs updating when the user interacts with
	 * the window.
	 **/
	Message *wait();

	/**
	 * Clears the event queue.
	 */
	void clear();

private:

	void exceptionIfInRenderPass(const char *name);

	Message *convert(const SDL_Event &e);
	Message *convertJoystickEvent(const SDL_Event &e) const;
	Message *convertWindowEvent(const SDL_Event &e);

	static std::map<SDL_Keycode, love::keyboard::Keyboard::Key> createKeyMap();
	static std::map<SDL_Keycode, love::keyboard::Keyboard::Key> keys;

}; // Event

} // sdl
} // event
} // love

#endif // LOVE_EVENT_SDL_EVENT_H
