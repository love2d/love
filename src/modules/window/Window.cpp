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
#include "Window.h"

namespace love
{
namespace window
{

static bool highDPIAllowed = false;

// TODO: find a cleaner way to do this...
// The window backend (e.g. love.window.sdl) is expected to implement this.
void setHighDPIAllowedImplementation(bool enable);

void setHighDPIAllowed(bool enable)
{
	setHighDPIAllowedImplementation(enable);
	highDPIAllowed = enable;
}

bool isHighDPIAllowed()
{
	return highDPIAllowed;
}

Window::Window(const char *name)
	: Module(M_WINDOW, name)
{
}

Window::~Window()
{
}

void Window::swapBuffers()
{
}

STRINGMAP_CLASS_BEGIN(Window, Window::Setting, Window::SETTING_MAX_ENUM, setting)
{
	{"fullscreen", Window::SETTING_FULLSCREEN},
	{"fullscreentype", Window::SETTING_FULLSCREEN_TYPE},
	{"vsync", Window::SETTING_VSYNC},
	{"msaa", Window::SETTING_MSAA},
	{"stencil", Window::SETTING_STENCIL},
	{"depth", Window::SETTING_DEPTH},
	{"resizable", Window::SETTING_RESIZABLE},
	{"minwidth", Window::SETTING_MIN_WIDTH},
	{"minheight", Window::SETTING_MIN_HEIGHT},
	{"borderless", Window::SETTING_BORDERLESS},
	{"centered", Window::SETTING_CENTERED},
	{"displayindex", Window::SETTING_DISPLAYINDEX},
	{"display", Window::SETTING_DISPLAY},
	{"highdpi", Window::SETTING_HIGHDPI},
	{"usedpiscale", Window::SETTING_USE_DPISCALE},
	{"refreshrate", Window::SETTING_REFRESHRATE},
	{"x", Window::SETTING_X},
	{"y", Window::SETTING_Y},
}
STRINGMAP_CLASS_END(Window, Window::Setting, Window::SETTING_MAX_ENUM, setting)

STRINGMAP_CLASS_BEGIN(Window, Window::FullscreenType, Window::FULLSCREEN_MAX_ENUM, fullscreenType)
{
	{"exclusive", Window::FULLSCREEN_EXCLUSIVE},
	{"desktop", Window::FULLSCREEN_DESKTOP},
}
STRINGMAP_CLASS_END(Window, Window::FullscreenType, Window::FULLSCREEN_MAX_ENUM, fullscreenType)

STRINGMAP_CLASS_BEGIN(Window, Window::MessageBoxType, Window::MESSAGEBOX_MAX_ENUM, messageBoxType)
{
	{"error", Window::MESSAGEBOX_ERROR},
	{"warning", Window::MESSAGEBOX_WARNING},
	{"info", Window::MESSAGEBOX_INFO},
}
STRINGMAP_CLASS_END(Window, Window::MessageBoxType, Window::MESSAGEBOX_MAX_ENUM, messageBoxType)

STRINGMAP_CLASS_BEGIN(Window, Window::FileDialogType, Window::FILEDIALOG_MAX_ENUM, fileDialogType)
{
	{ "openfile", Window::FILEDIALOG_OPENFILE },
	{ "openfolder", Window::FILEDIALOG_OPENFOLDER },
	{ "savefile", Window::FILEDIALOG_SAVEFILE },
}
STRINGMAP_CLASS_END(Window, Window::FileDialogType, Window::FILEDIALOG_MAX_ENUM, fileDialogType)

STRINGMAP_CLASS_BEGIN(Window, Window::DisplayOrientation, Window::ORIENTATION_MAX_ENUM, orientation)
{
	{"unknown", Window::ORIENTATION_UNKNOWN},
	{"landscape", Window::ORIENTATION_LANDSCAPE},
	{"landscapeflipped", Window::ORIENTATION_LANDSCAPE_FLIPPED},
	{"portrait", Window::ORIENTATION_PORTRAIT},
	{"portraitflipped", Window::ORIENTATION_PORTRAIT_FLIPPED},
}
STRINGMAP_CLASS_END(Window, Window::DisplayOrientation, Window::ORIENTATION_MAX_ENUM, orientation)

} // window
} // love
