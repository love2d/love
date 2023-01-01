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

// LOVE
#include "Window.h"

namespace love
{
namespace window
{

Window::~Window()
{
}

void Window::swapBuffers()
{
}

bool Window::getConstant(const char *in, FullscreenType &out)
{
	return fullscreenTypes.find(in, out);
}

bool Window::getConstant(FullscreenType in, const char *&out)
{
	return fullscreenTypes.find(in, out);
}

std::vector<std::string> Window::getConstants(FullscreenType)
{
	return fullscreenTypes.getNames();
}

bool Window::getConstant(const char *in, Setting &out)
{
	return settings.find(in, out);
}

bool Window::getConstant(Setting in, const char *&out)
{
	return settings.find(in, out);
}

bool Window::getConstant(const char *in, MessageBoxType &out)
{
	return messageBoxTypes.find(in, out);
}

bool Window::getConstant(MessageBoxType in, const char *&out)
{
	return messageBoxTypes.find(in, out);
}

std::vector<std::string> Window::getConstants(MessageBoxType)
{
	return messageBoxTypes.getNames();
}

bool Window::getConstant(const char *in, DisplayOrientation &out)
{
	return orientations.find(in, out);
}

bool Window::getConstant(DisplayOrientation in, const char *&out)
{
	return orientations.find(in, out);
}

std::vector<std::string> Window::getConstants(DisplayOrientation)
{
	return orientations.getNames();
}

StringMap<Window::Setting, Window::SETTING_MAX_ENUM>::Entry Window::settingEntries[] =
{
	{"fullscreen", SETTING_FULLSCREEN},
	{"fullscreentype", SETTING_FULLSCREEN_TYPE},
	{"vsync", SETTING_VSYNC},
	{"msaa", SETTING_MSAA},
	{"stencil", SETTING_STENCIL},
	{"depth", SETTING_DEPTH},
	{"resizable", SETTING_RESIZABLE},
	{"minwidth", SETTING_MIN_WIDTH},
	{"minheight", SETTING_MIN_HEIGHT},
	{"borderless", SETTING_BORDERLESS},
	{"centered", SETTING_CENTERED},
	{"display", SETTING_DISPLAY},
	{"highdpi", SETTING_HIGHDPI},
	{"usedpiscale", SETTING_USE_DPISCALE},
	{"refreshrate", SETTING_REFRESHRATE},
	{"x", SETTING_X},
	{"y", SETTING_Y},
};

StringMap<Window::Setting, Window::SETTING_MAX_ENUM> Window::settings(Window::settingEntries, sizeof(Window::settingEntries));

StringMap<Window::FullscreenType, Window::FULLSCREEN_MAX_ENUM>::Entry Window::fullscreenTypeEntries[] =
{
	{"exclusive", FULLSCREEN_EXCLUSIVE},
	{"desktop", FULLSCREEN_DESKTOP},
};

StringMap<Window::FullscreenType, Window::FULLSCREEN_MAX_ENUM> Window::fullscreenTypes(Window::fullscreenTypeEntries, sizeof(Window::fullscreenTypeEntries));

StringMap<Window::MessageBoxType, Window::MESSAGEBOX_MAX_ENUM>::Entry Window::messageBoxTypeEntries[] =
{
	{"error", MESSAGEBOX_ERROR},
	{"warning", MESSAGEBOX_WARNING},
	{"info", MESSAGEBOX_INFO},
};

StringMap<Window::MessageBoxType, Window::MESSAGEBOX_MAX_ENUM> Window::messageBoxTypes(Window::messageBoxTypeEntries, sizeof(Window::messageBoxTypeEntries));

StringMap<Window::DisplayOrientation, Window::ORIENTATION_MAX_ENUM>::Entry Window::orientationEntries[] =
{
	{"unknown", ORIENTATION_UNKNOWN},
	{"landscape", ORIENTATION_LANDSCAPE},
	{"landscapeflipped", ORIENTATION_LANDSCAPE_FLIPPED},
	{"portrait", ORIENTATION_PORTRAIT},
	{"portraitflipped", ORIENTATION_PORTRAIT_FLIPPED},
};

StringMap<Window::DisplayOrientation, Window::ORIENTATION_MAX_ENUM> Window::orientations(Window::orientationEntries, sizeof(Window::orientationEntries));

} // window
} // love
