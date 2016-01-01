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

#include "macosx.h"

#ifdef LOVE_MACOSX

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include <SDL2/SDL.h>

namespace love
{
namespace macosx
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
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_DROPFILE, SDL_DROPFILE) > 0)
	{
		if (event.type == SDL_DROPFILE)
		{
			dropstr = std::string(event.drop.file);
			SDL_free(event.drop.file);
		}
	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	return dropstr;
}

std::string getExecutablePath()
{
	@autoreleasepool
	{
		return std::string([NSBundle mainBundle].executablePath.UTF8String);
	}
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

} // osx
} // love

#endif // LOVE_MACOSX
