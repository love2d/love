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

#include "File.h"

namespace love
{
namespace filesystem
{

File::~File()
{
}

bool File::getConstant(const char *in, Mode &out)
{
	return modes.find(in, out);
}

bool File::getConstant(Mode in, const char  *&out)
{
	return modes.find(in, out);
}

StringMap<File::Mode, File::MODE_MAX_ENUM>::Entry File::modeEntries[] =
{
	{"c", File::CLOSED},
	{"r", File::READ},
	{"w", File::WRITE},
	{"a", File::APPEND},
};

StringMap<File::Mode, File::MODE_MAX_ENUM> File::modes(File::modeEntries, sizeof(File::modeEntries));

} // filesystem
} // love
