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

#pragma once

#include "config.h"

#ifdef LOVE_MACOS

#include <string>

typedef struct SDL_Window SDL_Window;

namespace love
{
namespace macos
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
 * Bounce the dock icon, if the app isn't in the foreground.
 **/
void requestAttention(bool continuous);

/**
 * Sets whether vsync is enabled for the given CAMetalLayer
 **/
void setMetalLayerVSync(void *metallayer, bool vsync);
bool getMetalLayerVSync(void *metallayer);

/**
 * Explicitly sets the window's color space to be sRGB - which stops the OS
 * from interpreting the backbuffer output as P3 on P3-capable displays.
 **/
void setWindowSRGBColorSpace(SDL_Window *window);

} // macos
} // love

#endif // LOVE_MACOS
