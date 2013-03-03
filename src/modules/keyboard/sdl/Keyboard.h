/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#ifndef LOVE_KEYBOARD_SDL_KEYBOARD_H
#define LOVE_KEYBOARD_SDL_KEYBOARD_H

// LOVE
#include "keyboard/Keyboard.h"
#include "common/EnumMap.h"

// SDL
#include <SDL.h>

namespace love
{
namespace keyboard
{
namespace sdl
{

class Keyboard : public love::keyboard::Keyboard
{
public:

	// Implements Module.
	const char *getName() const;
	bool isDown(Key *keylist) const;
	void setKeyRepeat(int delay, int interval) const;
	int getKeyRepeatDelay() const;
	int getKeyRepeatInterval() const;

private:

	static EnumMap<Key, SDLKey, Keyboard::KEY_MAX_ENUM>::Entry keyEntries[];
	static EnumMap<Key, SDLKey, Keyboard::KEY_MAX_ENUM> keys;

}; // Keyboard

} // sdl
} // keyboard
} // love

#endif // LOVE_KEYBOARD_SDL_KEYBOARD_H
