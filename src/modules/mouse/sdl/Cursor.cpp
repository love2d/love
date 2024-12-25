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
#include "Cursor.h"
#include "common/config.h"

namespace love
{
namespace mouse
{
namespace sdl
{

Cursor::Cursor(const std::vector<image::ImageData *> &imageData, int hotx, int hoty)
	: cursor(nullptr)
	, type(CURSORTYPE_IMAGE)
	, systemType(CURSOR_MAX_ENUM)
{
	if (imageData.empty())
		throw love::Exception("At least one ImageData must be provided for a custom cursor.");

	std::vector<SDL_Surface *> surfaces;

	for (image::ImageData *data : imageData)
	{
		int w = data->getWidth();
		int h = data->getHeight();
		int pitch = w * 4;

		if (getLinearPixelFormat(data->getFormat()) != PIXELFORMAT_RGBA8_UNORM)
		{
			for (SDL_Surface *surface : surfaces)
				SDL_DestroySurface(surface);
			throw love::Exception("Cannot create cursor: ImageData pixel format must be rgba8.");
		}

		surfaces.push_back(SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ABGR8888, data->getData(), pitch));

		if (surfaces.back() == nullptr)
		{
			for (SDL_Surface *surface : surfaces)
				SDL_DestroySurface(surface);
			throw love::Exception("Cannot create cursor: out of memory.");
		}
	}

	// Add alternate representations for the OS to use in different DPI scales.
	for (size_t i = 1; i < surfaces.size(); i++)
		SDL_AddSurfaceAlternateImage(surfaces[0], surfaces[i]);

	cursor = SDL_CreateColorCursor(surfaces[0], hotx, hoty);

	for (SDL_Surface *surface : surfaces)
		SDL_DestroySurface(surface);

	if (cursor == nullptr)
		throw love::Exception("Cannot create cursor: %s", SDL_GetError());
}

Cursor::Cursor(mouse::Cursor::SystemCursor cursortype)
	: cursor(nullptr)
	, type(CURSORTYPE_SYSTEM)
	, systemType(cursortype)
{
	SDL_SystemCursor sdlcursortype;

	if (systemCursors.find(cursortype, sdlcursortype))
		cursor = SDL_CreateSystemCursor(sdlcursortype);
	else
		throw love::Exception("Cannot create system cursor: invalid type.");

	if (cursor == nullptr)
		throw love::Exception("Cannot create system cursor: %s", SDL_GetError());
}

Cursor::~Cursor()
{
	if (cursor)
		SDL_DestroyCursor(cursor);
}

void *Cursor::getHandle() const
{
	return cursor;
}

Cursor::CursorType Cursor::getType() const
{
	return type;
}

Cursor::SystemCursor Cursor::getSystemType() const
{
	return systemType;
}

EnumMap<Cursor::SystemCursor, SDL_SystemCursor, Cursor::CURSOR_MAX_ENUM>::Entry Cursor::systemCursorEntries[] =
{
	{Cursor::CURSOR_ARROW, SDL_SYSTEM_CURSOR_DEFAULT},
	{Cursor::CURSOR_IBEAM, SDL_SYSTEM_CURSOR_TEXT},
	{Cursor::CURSOR_WAIT, SDL_SYSTEM_CURSOR_WAIT},
	{Cursor::CURSOR_CROSSHAIR, SDL_SYSTEM_CURSOR_CROSSHAIR},
	{Cursor::CURSOR_WAITARROW, SDL_SYSTEM_CURSOR_PROGRESS},
	{Cursor::CURSOR_SIZENWSE, SDL_SYSTEM_CURSOR_NWSE_RESIZE},
	{Cursor::CURSOR_SIZENESW, SDL_SYSTEM_CURSOR_NESW_RESIZE},
	{Cursor::CURSOR_SIZEWE, SDL_SYSTEM_CURSOR_EW_RESIZE},
	{Cursor::CURSOR_SIZENS, SDL_SYSTEM_CURSOR_NS_RESIZE},
	{Cursor::CURSOR_SIZEALL, SDL_SYSTEM_CURSOR_MOVE},
	{Cursor::CURSOR_NO, SDL_SYSTEM_CURSOR_NOT_ALLOWED},
	{Cursor::CURSOR_HAND, SDL_SYSTEM_CURSOR_POINTER},
};

EnumMap<Cursor::SystemCursor, SDL_SystemCursor, Cursor::CURSOR_MAX_ENUM> Cursor::systemCursors(Cursor::systemCursorEntries, sizeof(Cursor::systemCursorEntries));

} // sdl
} // mouse
} // love
