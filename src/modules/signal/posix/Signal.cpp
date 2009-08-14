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

#include "Signal.h"

namespace love
{
namespace signal
{
namespace posix
{

	Signal::Signal()
		: signals(0)
	{
		cb = 0;
	}
	
	Signal::~Signal()
	{
		::signal(signals, SIG_DFL);
	}
	
	bool Signal::registerSignal(int sgn)
	{
		signals |= sgn;
		::signal(sgn, (void (*)(int)) &handler);
	}
	
	void Signal::setCallback(lua_State *L)
	{
		luax_assert_argc(L, 1, 1);
		luax_assert_function(L, -1);

		if(cb != 0)
		{
			delete cb;
			cb = 0;
		}

		cb = new Reference(L);
	}
	
	void handler(int signal)
	{
		if (cb == 0)
			return;
		lua_State *L = cb->getL();
		cb->push();
		lua_pushnumber(L, signal);
		lua_call(L, 1, 0);
	}
	
	const char * Signal::getName() const
	{
		return "love.signal.posix";
	}

} // posix
} // signal
} // love
