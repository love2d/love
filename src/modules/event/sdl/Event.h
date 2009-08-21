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

#ifndef LOVE_EVENT_SDL_EVENT_H
#define LOVE_EVENT_SDL_EVENT_H

// LOVE
#include <event/Event.h>
#include <common/runtime.h>

// SDL
#include <SDL.h>

namespace love
{
namespace event
{
namespace sdl
{
	class Event : public event::Event
	{
	public:
		
		// Implements Module.
		const char * getName() const;

		/**
		* Pumps the event queue. This function gathers all the pending input information 
		* from devices and places it on the event queue. Normally not needed if you poll
		* for events.
		**/ 
		void pump();

		/**
		* Returns an iterator function for iterating over pending events.
		**/
		int poll(lua_State * L);
		
		/**
		* Waits for the next event (indefinitely). Useful for creating games where
		* the screen and game state only needs updating when the user interacts with
		* the window.
		**/
		int wait(lua_State * L);

		/**
		* Push a quit event. Calling this does not mean the application
		* will exit immediately, it just means an quit event will be issued. 
		* How to respond to the quit event is up the application. 
		**/
		void quit();

		/**
		* Pushes an event into the queue.
		**/
		int push(lua_State * L);

		/**
		* The iterator function.
		**/
		static int poll_i(lua_State * L);
		static int wait_i(lua_State * L);

	private:

		static int pushEvent(lua_State * L, SDL_Event & e);
		static int getEvent(lua_State * L, SDL_Event & e);

	}; // System

} // sdl
} // event
} // love

#endif // LOVE_EVENT_SDL_EVENT_H
