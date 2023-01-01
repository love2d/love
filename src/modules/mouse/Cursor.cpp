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

#include "Cursor.h"

namespace love
{
namespace mouse
{

love::Type Cursor::type("Cursor", &Object::type);

Cursor::~Cursor()
{
}

bool Cursor::getConstant(const char *in, SystemCursor &out)
{
	return systemCursors.find(in, out);
}

bool Cursor::getConstant(SystemCursor in, const char *&out)
{
	return systemCursors.find(in, out);
}

bool Cursor::getConstant(const char *in, CursorType &out)
{
	return types.find(in, out);
}

bool Cursor::getConstant(CursorType in, const char *&out)
{
	return types.find(in, out);
}

StringMap<Cursor::SystemCursor, Cursor::CURSOR_MAX_ENUM>::Entry Cursor::systemCursorEntries[] =
{
	{"arrow", Cursor::CURSOR_ARROW},
	{"ibeam", Cursor::CURSOR_IBEAM},
	{"wait", Cursor::CURSOR_WAIT},
	{"crosshair", Cursor::CURSOR_CROSSHAIR},
	{"waitarrow", Cursor::CURSOR_WAITARROW},
	{"sizenwse", Cursor::CURSOR_SIZENWSE},
	{"sizenesw", Cursor::CURSOR_SIZENESW},
	{"sizewe", Cursor::CURSOR_SIZEWE},
	{"sizens", Cursor::CURSOR_SIZENS},
	{"sizeall", Cursor::CURSOR_SIZEALL},
	{"no", Cursor::CURSOR_NO},
	{"hand", Cursor::CURSOR_HAND},
};

StringMap<Cursor::SystemCursor, Cursor::CURSOR_MAX_ENUM> Cursor::systemCursors(Cursor::systemCursorEntries, sizeof(Cursor::systemCursorEntries));

StringMap<Cursor::CursorType, Cursor::CURSORTYPE_MAX_ENUM>::Entry Cursor::typeEntries[] =
{
	{"system", Cursor::CURSORTYPE_SYSTEM},
	{"image", Cursor::CURSORTYPE_IMAGE},
};

StringMap<Cursor::CursorType, Cursor::CURSORTYPE_MAX_ENUM> Cursor::types(Cursor::typeEntries, sizeof(Cursor::typeEntries));

} // mouse
} // love
