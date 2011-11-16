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

#include "Mouse.h"

// SDL
#include <SDL.h>

namespace love
{
namespace mouse
{
namespace sdl
{
	const char * Mouse::getName() const
	{
		return "love.mouse.sdl";
	}

	int Mouse::getX() const
	{
		int x;
		SDL_GetMouseState(&x, 0);
		return x;
	}

	int Mouse::getY() const
	{
		int y;
		SDL_GetMouseState(0, &y);
		return y;
	}

	void Mouse::getPosition(int & x, int & y) const
	{
		SDL_GetMouseState(&x, &y);
	}

	void Mouse::setPosition(int x, int y)
	{
		SDL_WarpMouse(x, y);
	}

	void Mouse::setVisible(bool visible)
	{
		SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
	}

	bool Mouse::isDown(Button * buttonlist) const
	{
		Uint8 buttonstate = SDL_GetMouseState(0, 0);

		for (Button button = *buttonlist; button != BUTTON_MAX_ENUM; button = *(++buttonlist))
		{
			if (buttonstate & SDL_BUTTON(button))
				return true;
		}

		return false;
	}

	bool Mouse::isVisible() const
	{
		return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) ? true : false;
	}

	void Mouse::setGrab(bool grab)
	{
		SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
	}

	bool Mouse::isGrabbed() const
	{
		return (SDL_WM_GrabInput(SDL_GRAB_QUERY) ==  SDL_GRAB_ON ? true : false);
	}

} // sdl
} // mouse
} // love
