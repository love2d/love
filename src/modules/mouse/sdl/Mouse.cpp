/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
#include "Mouse.h"
#include "window/sdl/Window.h"

// SDL
#include <SDL_mouse.h>

namespace love
{
namespace mouse
{
namespace sdl
{

// SDL reports mouse coordinates in the window coordinate system in OS X, but
// we want them in pixel coordinates (may be different with high-DPI enabled.)
static void windowToPixelCoords(int *x, int *y)
{
	double scale = 1.0;

	love::window::Window *window = love::window::sdl::Window::getSingleton();
	if (window != nullptr)
		scale = window->getPixelScale();

	if (x != nullptr)
		*x = int(double(*x) * scale);

	if (y != nullptr)
		*y = int(double(*x) * scale);
}

// And vice versa for setting mouse coordinates.
static void pixelToWindowCoords(int *x, int *y)
{
	double scale = 1.0;

	love::window::Window *window = love::window::sdl::Window::getSingleton();
	if (window != nullptr)
		scale = window->getPixelScale();

	if (x != nullptr)
		*x = int(double(*x) / scale);

	if (y != nullptr)
		*y = int(double(*x) / scale);
}

const char *Mouse::getName() const
{
	return "love.mouse.sdl";
}

Mouse::Mouse()
	: curCursor(0)
{
}

Mouse::~Mouse()
{
	if (curCursor)
		setCursor();

	for (auto it = systemCursors.begin(); it != systemCursors.end(); ++it)
		it->second->release();
}

love::mouse::Cursor *Mouse::newCursor(love::image::ImageData *data, int hotx, int hoty)
{
	return new Cursor(data, hotx, hoty);
}

love::mouse::Cursor *Mouse::getSystemCursor(Cursor::SystemCursor cursortype)
{
	Cursor *cursor = NULL;
	auto it = systemCursors.find(cursortype);

	if (it != systemCursors.end())
		cursor = it->second;
	else
	{
		cursor = new Cursor(cursortype);
		systemCursors[cursortype] = cursor;
	}

	return cursor;
}

void Mouse::setCursor(love::mouse::Cursor *cursor)
{
	Object::AutoRelease cursorrelease(curCursor);

	curCursor = cursor;
	curCursor->retain();

	SDL_SetCursor((SDL_Cursor *) cursor->getHandle());
}

void Mouse::setCursor()
{
	Object::AutoRelease cursorrelease(curCursor);
	curCursor = NULL;

	SDL_SetCursor(SDL_GetDefaultCursor());
}

love::mouse::Cursor *Mouse::getCursor() const
{
	return curCursor;
}

int Mouse::getX() const
{
	int x;
	SDL_GetMouseState(&x, nullptr);
	windowToPixelCoords(&x, nullptr);

	return x;
}

int Mouse::getY() const
{
	int y;
	SDL_GetMouseState(nullptr, &y);
	windowToPixelCoords(nullptr, &y);

	return y;
}

void Mouse::getPosition(int &x, int &y) const
{
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	windowToPixelCoords(&mx, &my);

	x = mx;
	y = my;
}

void Mouse::setPosition(int x, int y)
{
	love::window::Window *window = love::window::sdl::Window::getSingleton();

	SDL_Window *handle = nullptr;
	if (window)
		handle = (SDL_Window *) window->getHandle();

	pixelToWindowCoords(&x, &y);
	SDL_WarpMouseInWindow(handle, x, y);
}

void Mouse::setX(int x)
{
	int y = getY();
	setPosition(x, y);
}

void Mouse::setY(int y)
{
	int x = getX();
	setPosition(x, y);
}

void Mouse::setVisible(bool visible)
{
	SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

bool Mouse::isDown(Button *buttonlist) const
{
	Uint32 buttonstate = SDL_GetMouseState(0, 0);

	for (Button button = *buttonlist; button != BUTTON_MAX_ENUM; button = *(++buttonlist))
	{
		if (buttonstate & SDL_BUTTON(button))
			return true;
	}

	return false;
}

bool Mouse::isVisible() const
{
	return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
}

void Mouse::setGrabbed(bool grab)
{
	love::window::Window *window = love::window::sdl::Window::getSingleton();
	if (window)
		window->setMouseGrab(grab);
}

bool Mouse::isGrabbed() const
{
	love::window::Window *window = love::window::sdl::Window::getSingleton();
	if (window)
		return window->isMouseGrabbed();
	else
		return false;
}

} // sdl
} // mouse
} // love
