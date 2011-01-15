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

#ifndef LOVE_THREAD_SDL_WRAP_THREAD_H
#define LOVE_THREAD_SDL_WRAP_THREAD_H

// LOVE
#include <common/config.h>
#include "Thread.h"

namespace love
{
	extern StringMap<Type, TYPE_MAX_ENUM> types;
namespace thread
{
namespace sdl
{
	Thread *luax_checkthread(lua_State *L, int idx);
	int w_Thread_start(lua_State *L);
	int w_Thread_kill(lua_State *L);
	int w_Thread_wait(lua_State *L);
	int w_Thread_getName(lua_State *L);
	int w_Thread_receive(lua_State *L);
	int w_Thread_demand(lua_State *L);
	int w_Thread_peek(lua_State *L);
	int w_Thread_send(lua_State *L);

	int luaopen_thread(lua_State *L);

	int w_newThread(lua_State *L);
	int w_getThreads(lua_State *L);
	int w_getThread(lua_State *L);

	extern "C" LOVE_EXPORT int luaopen_love_thread(lua_State * L);
} // sdl
} // thread
} // love

#endif // LOVE_THREAD_SDL_WRAP_THREAD_H
