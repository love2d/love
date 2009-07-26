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

#include "Keyboard.h"

// SDL
#include <SDL.h>

namespace love
{
namespace keyboard
{
namespace sdl
{
	const char * Keyboard::getName() const
	{
		return "love.keyboard.sdl";
	}

	bool Keyboard::isDown(int key) const
	{
		Uint8 * keystate = SDL_GetKeyState(0);
		return keystate[key] == 1;		
	}

	void Keyboard::setKeyRepeat(int delay, int interval) const
	{
		if(delay == KEY_REPEAT_DELAY)
			delay = SDL_DEFAULT_REPEAT_DELAY;

		if(interval == KEY_REPEAT_INTERVAL)
			interval = SDL_DEFAULT_REPEAT_INTERVAL;
		
		if(SDL_EnableKeyRepeat(delay, interval) == -1)
			throw new Exception("[Keyboard::enableKeyRepeat] Unable to enable key repeat");
	}

	int Keyboard::getKeyRepeatDelay() const
	{
		int delay, interval = 0;
		SDL_GetKeyRepeat(&delay, &interval);
		return delay;
	}
	
	int Keyboard::getKeyRepeatInterval() const
	{
		int delay, interval = 0;
		SDL_GetKeyRepeat(&delay, &interval);
		return interval;
	}

} // sdl
} // keyboard
} // love
