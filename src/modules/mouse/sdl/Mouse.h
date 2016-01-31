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

#ifndef LOVE_MOUSE_SDL_MOUSE_H
#define LOVE_MOUSE_SDL_MOUSE_H

// LOVE
#include "mouse/Mouse.h"
#include "Cursor.h"

// C++
#include <map>

namespace love
{
namespace mouse
{
namespace sdl
{

class Mouse : public love::mouse::Mouse
{
public:

	// Implements Module.
	const char *getName() const override;

	Mouse();
	virtual ~Mouse();

	love::mouse::Cursor *newCursor(love::image::ImageData *data, int hotx, int hoty) override;
	love::mouse::Cursor *getSystemCursor(Cursor::SystemCursor cursortype) override;

	void setCursor(love::mouse::Cursor *cursor) override;
	void setCursor() override;

	love::mouse::Cursor *getCursor() const override;

	bool hasCursor() const override;

	double getX() const override;
	double getY() const override;
	void getPosition(double &x, double &y) const override;
	void setX(double x) override;
	void setY(double y) override;
	void setPosition(double x, double y) override;
	void setVisible(bool visible) override;
	bool isDown(const std::vector<int> &buttons) const override;
	bool isVisible() const override;
	void setGrabbed(bool grab) override;
	bool isGrabbed() const override;
	bool setRelativeMode(bool relative) override;
	bool getRelativeMode() const override;

private:

	StrongRef<love::mouse::Cursor> curCursor;

	std::map<Cursor::SystemCursor, Cursor *> systemCursors;

}; // Mouse

} // sdl
} // mouse
} // love

#endif // LOVE_MOUSE_SDL_MOUSE_H
