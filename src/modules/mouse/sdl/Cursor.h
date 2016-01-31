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

#ifndef LOVE_MOUSE_SDL_CURSOR_H
#define LOVE_MOUSE_SDL_CURSOR_H

// LOVE
#include "mouse/Cursor.h"
#include "common/EnumMap.h"

// SDL
#include <SDL_mouse.h>

namespace love
{
namespace mouse
{
namespace sdl
{

class Cursor : public love::mouse::Cursor
{
public:

	Cursor(image::ImageData *imageData, int hotx, int hoty);
	Cursor(SystemCursor cursortype);
	~Cursor();

	void *getHandle() const;
	CursorType getType() const;
	SystemCursor getSystemType() const;

private:

	SDL_Cursor *cursor;
	CursorType type;
	SystemCursor systemType;

	static EnumMap<SystemCursor, SDL_SystemCursor, CURSOR_MAX_ENUM>::Entry systemCursorEntries[];
	static EnumMap<SystemCursor, SDL_SystemCursor, CURSOR_MAX_ENUM> systemCursors;
};

} // sdl
} // mouse
} // love

#endif // LOVE_MOUSE_SDL_CURSOR_H
