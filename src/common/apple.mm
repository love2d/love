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

#include "apple.h"

#if defined(LOVE_IOS) || defined(LOVE_MACOS)

#import <Foundation/Foundation.h>

namespace love
{
namespace apple
{

std::string getUserDirectory(UserDirectory dir)
{
	std::string path;
	NSSearchPathDirectory nsdir = NSTrashDirectory;

	@autoreleasepool
	{
		switch (dir)
		{
		case USER_DIRECTORY_HOME:
			return NSHomeDirectory().UTF8String;
		case USER_DIRECTORY_APPSUPPORT:
			nsdir = NSApplicationSupportDirectory;
			break;
		case USER_DIRECTORY_DOCUMENTS:
			nsdir = NSDocumentDirectory;
			break;
		case USER_DIRECTORY_DESKTOP:
			nsdir = NSDesktopDirectory;
			break;
		case USER_DIRECTORY_CACHES:
			nsdir = NSCachesDirectory;
			break;
		case USER_DIRECTORY_TEMP:
			nsdir = NSItemReplacementDirectory;
			break;
		}

		NSArray<NSURL *> *dirs = [[NSFileManager defaultManager] URLsForDirectory:nsdir inDomains:NSUserDomainMask];
		if (dirs.count > 0)
			path = [dirs[0].path UTF8String];
	}

	return path;
}

std::string getExecutablePath()
{
	@autoreleasepool
	{
		return std::string([NSBundle mainBundle].executablePath.UTF8String);
	}
}

} // apple
} // love

#endif // defined(LOVE_IOS) || defined(LOVE_MACOS)
