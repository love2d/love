/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_WRAP_ANIMATION_H
#define LOVE_GRAPHICS_OPENGL_WRAP_ANIMATION_H

// LOVE
#include <common/runtime.h>
#include "Animation.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Animation * luax_checkanimation(lua_State * L, int idx);
	int _wrap_Animation_addFrame(lua_State * L);
	int _wrap_Animation_play(lua_State * L);
	int _wrap_Animation_stop(lua_State * L);
	int _wrap_Animation_reset(lua_State * L);
	int _wrap_Animation_seek(lua_State * L);
	int _wrap_Animation_getCurrentFrame(lua_State * L);
	int _wrap_Animation_getSize(lua_State * L);
	int _wrap_Animation_setDelay(lua_State * L);
	int _wrap_Animation_setSpeed(lua_State * L);
	int _wrap_Animation_getSpeed(lua_State * L);
	int _wrap_Animation_update(lua_State * L);
	int _wrap_Animation_getWidth(lua_State * L);
	int _wrap_Animation_getHeight(lua_State * L);
	int wrap_Animation_open(lua_State * L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_ANIMATION_H
