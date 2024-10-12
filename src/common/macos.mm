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

#include "macos.h"

#ifdef LOVE_MACOS

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#if __has_include(<SDL3/SDL.h>)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#include <SDL_syswm.h>
#endif

namespace love
{
namespace macos
{

std::string getLoveInResources()
{
	std::string path;

	@autoreleasepool
	{
		// Check to see if there are any .love files in Resources.
		NSString *lovepath = [[NSBundle mainBundle] pathForResource:nil ofType:@"love"];

		if (lovepath != nil)
			path = lovepath.UTF8String;
	}

	return path;
}

std::string checkDropEvents()
{
	std::string dropstr;
	SDL_Event event;

	SDL_InitSubSystem(SDL_INIT_VIDEO);

	SDL_PumpEvents();
#if SDL_VERSION_ATLEAST(3, 0, 0)
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_DROP_FILE, SDL_EVENT_DROP_FILE) > 0)
	{
		if (event.type == SDL_EVENT_DROP_FILE)
			dropstr = std::string(event.drop.data);
	}
#else
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_DROPFILE, SDL_DROPFILE) > 0)
	{
		if (event.type == SDL_DROPFILE)
		{
			dropstr = std::string(event.drop.file);
			SDL_free(event.drop.file);
		}
	}
#endif

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	return dropstr;
}

void requestAttention(bool continuous)
{
	@autoreleasepool
	{
		if (continuous)
			[NSApp requestUserAttention:NSCriticalRequest];
		else
			[NSApp requestUserAttention:NSInformationalRequest];
	}
}

void setMetalLayerVSync(void *metallayer, bool vsync)
{
	@autoreleasepool
	{
		if (@available(macOS 10.13, *))
		{
			CAMetalLayer *layer = (__bridge CAMetalLayer *) metallayer;
			layer.displaySyncEnabled = vsync;
		}
	}
}

bool getMetalLayerVSync(void *metallayer)
{
	@autoreleasepool
	{
		if (@available(macOS 10.13, *))
		{
			CAMetalLayer *layer = (__bridge CAMetalLayer *) metallayer;
			return layer.displaySyncEnabled;
		}
	}

	return true;
}

void setWindowSRGBColorSpace(SDL_Window *window)
{
	@autoreleasepool
	{
		// This works on earlier macOS versions, but performance may be worse
		// (at least, it was back when I tested in December 2016).
		if (@available(macOS 11.0, *))
		{
#if SDL_VERSION_ATLEAST(3, 0, 0)
			SDL_PropertiesID props = SDL_GetWindowProperties(window);
			NSWindow *window = (__bridge NSWindow *) SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
			window.colorSpace = [NSColorSpace sRGBColorSpace];
#else
			SDL_SysWMinfo info = {};
			if (SDL_GetWindowWMInfo(window, &info))
				info.info.cocoa.window.colorSpace = [NSColorSpace sRGBColorSpace];
#endif
		}
	}
}

} // macos
} // love

#endif // LOVE_MACOS
