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

#ifndef LOVE_OSX_H
#define LOVE_OSX_H

#include "config.h"

#ifdef LOVE_MACOSX

#include <string>

typedef struct SDL_Window SDL_Window;

namespace love
{
namespace macosx
{

/**
 * Returns the filepath of the first detected love file in the Resources folder
 * in the main bundle (love.app.)
 * Returns an empty string if no love file is found.
 **/
LOVE_EXPORT std::string getLoveInResources();

/**
 * Checks for drop-file events. Returns the filepath if an event occurred, or
 * an empty string otherwise.
 **/
LOVE_EXPORT std::string checkDropEvents();

/**
 * Returns the full path to the executable.
 **/
std::string getExecutablePath();

/**
 * Bounce the dock icon, if the app isn't in the foreground.
 **/
void requestAttention(bool continuous);

/**
 * Explicitly sets the window's color space to be sRGB - which stops the OS
 * from interpreting the backbuffer output as P3 on P3-capable displays.
 **/
void setWindowSRGBColorSpace(SDL_Window *window);

} // macosx
} // love

#endif // LOVE_MACOSX

#endif // LOVE_OSX_H
