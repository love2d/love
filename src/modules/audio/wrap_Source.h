/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#ifndef LOVE_AUDIO_WRAP_SOURCE_H
#define LOVE_AUDIO_WRAP_SOURCE_H

#include <common/runtime.h>
#include "Source.h"

namespace love
{
namespace audio
{
	Source * luax_checksource(lua_State * L, int idx);
	int w_Source_play(lua_State * L);
	int w_Source_stop(lua_State * L);
	int w_Source_pause(lua_State * L);
	int w_Source_resume(lua_State * L);
	int w_Source_rewind(lua_State * L);
	int w_Source_setPitch(lua_State * L);
	int w_Source_getPitch(lua_State * L);
	int w_Source_setVolume(lua_State * L);
	int w_Source_getVolume(lua_State * L);
	int w_Source_seek(lua_State * L);
	int w_Source_tell(lua_State * L);
	int w_Source_setPosition(lua_State * L);
	int w_Source_getPosition(lua_State * L);
	int w_Source_setVelocity(lua_State * L);
	int w_Source_getVelocity(lua_State * L);
	int w_Source_setDirection(lua_State * L);
	int w_Source_getDirection(lua_State * L);
	int w_Source_setLooping(lua_State * L);
	int w_Source_isLooping(lua_State * L);
	int w_Source_isStopped(lua_State * L);
	int w_Source_isPaused(lua_State * L);
	int w_Source_isStatic(lua_State * L);
	int luaopen_source(lua_State * L);

} // audio
} // love

#endif // LOVE_AUDIO_WRAP_SOURCE_H
