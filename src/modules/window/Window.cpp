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

// LOVE
#include "Window.h"

namespace love
{
namespace window
{

Window *Window::singleton = NULL;

Window::~Window()
{
	if (singleton == this)
		singleton = NULL;
}

void Window::swapBuffers()
{
}

WindowFlags::WindowFlags()
	: fullscreen(false)
	, fstype(Window::FULLSCREEN_TYPE_NORMAL)
	, vsync(true)
	, fsaa(0)
	, resizable(false)
	, minwidth(0)
	, minheight(0)
	, borderless(false)
	, centered(true)
	, display(0)
{
}

bool Window::getConstant(const char *in, Window::FullscreenType &out)
{
	return fullscreenTypes.find(in, out);
}

bool Window::getConstant(Window::FullscreenType in, const char *&out)
{
	return fullscreenTypes.find(in, out);
}

StringMap<Window::FullscreenType, Window::FULLSCREEN_TYPE_MAX_ENUM>::Entry Window::fullscreenTypeEntries[] =
{
	{"normal", Window::FULLSCREEN_TYPE_NORMAL},
	{"desktop", Window::FULLSCREEN_TYPE_DESKTOP},
};

StringMap<Window::FullscreenType, Window::FULLSCREEN_TYPE_MAX_ENUM> Window::fullscreenTypes(Window::fullscreenTypeEntries, sizeof(Window::fullscreenTypeEntries));

} // window
} // love
