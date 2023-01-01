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

#include "config.h"

#ifdef LOVE_WINDOWS

#include <string>
#include <windows.h>

namespace love
{

/**
 * Convert the wide string to a UTF-8 encoded string.
 * @param wstr The wide-char string.
 * @return A UTF-8 string.
 **/
std::string to_utf8(LPCWSTR wstr);

/**
 * Convert a UTF-8 encoded string to a wide string.
 * @param str The UTF-8 string.
 * @return A wide string.
**/
std::wstring to_widestr(const std::string &str);

/**
 * Replace all occurences of 'find' with 'replace' in a string.
 * @param str The string to modify.
 * @param find The character to match.
 * @param replace The character to replace matches.
 **/
void replace_char(std::string &str, char find, char replace);

} // love

#endif // LOVE_WINDOWS
