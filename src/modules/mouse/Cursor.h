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

#ifndef LOVE_MOUSE_CURSOR_H
#define LOVE_MOUSE_CURSOR_H

// LOVE
#include "image/ImageData.h"
#include "common/Object.h"
#include "common/StringMap.h"

namespace love
{
namespace mouse
{

class Cursor : public Object
{
public:

	// Types of system cursors.
	enum SystemCursor
	{
		CURSOR_ARROW,
		CURSOR_IBEAM,
		CURSOR_WAIT,
		CURSOR_CROSSHAIR,
		CURSOR_WAITARROW,
		CURSOR_SIZENWSE,
		CURSOR_SIZENESW,
		CURSOR_SIZEWE,
		CURSOR_SIZENS,
		CURSOR_SIZEALL,
		CURSOR_NO,
		CURSOR_HAND,
		CURSOR_MAX_ENUM
	};

	enum CursorType
	{
		CURSORTYPE_SYSTEM,
		CURSORTYPE_IMAGE,
		CURSORTYPE_MAX_ENUM
	};

	virtual ~Cursor();

	/**
	 * Returns a pointer to the implementation-dependent handle of this Cursor.
	 **/
	virtual void *getHandle() const = 0;

	/**
	 * Returns whether this Cursor is system-defined or a custom image.
	 **/
	virtual CursorType getType() const = 0;

	/**
	 * Returns the type type of system cursor used, if this Cursor is using a
	 * system-defined image.
	 **/
	virtual SystemCursor getSystemType() const = 0;

	static bool getConstant(const char *in, SystemCursor &out);
	static bool getConstant(SystemCursor in, const char *&out);

	static bool getConstant(const char *in, CursorType &out);
	static bool getConstant(CursorType in, const char *&out);

private:

	static StringMap<SystemCursor, CURSOR_MAX_ENUM>::Entry systemCursorEntries[];
	static StringMap<SystemCursor, CURSOR_MAX_ENUM> systemCursors;

	static StringMap<CursorType, CURSORTYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<CursorType, CURSORTYPE_MAX_ENUM> types;

};

} // mouse
} // love

#endif // LOVE_MOUSE_CURSOR_H
