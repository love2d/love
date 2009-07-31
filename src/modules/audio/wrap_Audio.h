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

#ifndef LOVE_AUDIO_WRAP_AUDIO_H
#define LOVE_AUDIO_WRAP_AUDIO_H

// LOVE
#include "Audio.h"
#include "wrap_Sound.h"
#include "wrap_Music.h"
#include "wrap_Source.h"

namespace love
{
namespace audio
{
	int _wrap_getNumSources(lua_State * L);
	int _wrap_newSound(lua_State * L);
	int _wrap_newMusic(lua_State * L);
	int _wrap_newSource(lua_State * L);
	int _wrap_play(lua_State * L);
	int _wrap_stop(lua_State * L);
	int _wrap_pause(lua_State * L);
	int _wrap_resume(lua_State * L);
	int _wrap_rewind(lua_State * L);
	int _wrap_setVolume(lua_State * L);
	int _wrap_getVolume(lua_State * L);
	int wrap_Audio_open(lua_State * L);

} // audio
} // love


#endif // LOVE_AUDIO_WRAP_AUDIO_H
