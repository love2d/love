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

#ifndef LOVE_MOUSE_MOUSE_H
#define LOVE_MOUSE_MOUSE_H

// LOVE
#include "Cursor.h"
#include "common/Module.h"
#include "image/ImageData.h"

// C++
#include <vector>

namespace love
{
namespace mouse
{

class Mouse : public Module
{
public:

	virtual ~Mouse() {}

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_MOUSE; }

	virtual Cursor *newCursor(love::image::ImageData *data, int hotx, int hoty) = 0;
	virtual Cursor *getSystemCursor(Cursor::SystemCursor cursortype) = 0;

	virtual void setCursor(Cursor *cursor) = 0;
	virtual void setCursor() = 0;

	virtual Cursor *getCursor() const = 0;

	virtual bool hasCursor() const = 0;

	virtual double getX() const = 0;
	virtual double getY() const = 0;
	virtual void getPosition(double &x, double &y) const = 0;
	virtual void setX(double x) = 0;
	virtual void setY(double y) = 0;
	virtual void setPosition(double x, double y) = 0;
	virtual void setVisible(bool visible) = 0;
	virtual bool isDown(const std::vector<int> &buttons) const = 0;
	virtual bool isVisible() const = 0;
	virtual void setGrabbed(bool grab) = 0;
	virtual bool isGrabbed() const = 0;
	virtual bool setRelativeMode(bool relative) = 0;
	virtual bool getRelativeMode() const = 0;

}; // Mouse

} // mouse
} // love

#endif // LOVE_MOUSE_MOUSE_H
