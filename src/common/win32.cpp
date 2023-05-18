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

#include "win32.h"

#ifdef LOVE_WINDOWS

#include <windows.h>

namespace love
{
namespace windows
{

const std::tuple<void*, size_t> getGameInResource()
{
	static void *archiveData = nullptr;
	static size_t archiveSize = 0;
	static bool tested = false;

	if (!tested)
	{
		HRSRC resource = FindResourceA(nullptr, "GAME", RT_RCDATA);
		tested = true;

		if (resource != nullptr)
		{
			HGLOBAL resourceData = LoadResource(nullptr, resource);

			if (resourceData != nullptr)
			{
				archiveSize = SizeofResource(nullptr, resource);
				archiveData = LockResource(resourceData);
			}
		}
	}

	return std::make_tuple(archiveData, archiveSize);
}

} // windows
} // love

#endif // LOVE_WINDOWS
