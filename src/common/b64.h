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

#include "config.h"

#ifndef LOVE_B64_H
#define LOVE_B64_H

namespace love
{

/**
 * Decode base64 encoded data.
 *
 * @param src The string containing the base64 data.
 * @param slen The length of the string.
 * @param size The size of the binary data is stored here.
 * @return A chunk of memory containing the binary data (allocated with new[]).
 */
char *b64_decode(const char *src, int slen, int &size);

} // love

#endif // LOVE_B64_H
