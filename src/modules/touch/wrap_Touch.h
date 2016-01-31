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

#ifndef LOVE_TOUCH_WRAP_TOUCH_H
#define LOVE_TOUCH_WRAP_TOUCH_H

// LOVE
#include "Touch.h"
#include "common/runtime.h"
#include "common/int.h"

namespace love
{
namespace touch
{

int64 luax_checktouchid(lua_State *L, int idx);
extern "C" LOVE_EXPORT int luaopen_love_touch(lua_State *L);

} // touch
} // love

#endif // LOVE_TOUCH_WRAP_TOUCH_H
