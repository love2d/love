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

#ifndef LOVE_SIGNAL_POSIX_SIGNAL_H
#define LOVE_SIGNAL_POSIX_SIGNAL_H

// signal
#include <csignal>

// LOVE
#include <common/Module.h>
#include <common/Reference.h>

namespace love
{
namespace signal
{
namespace posix
{
	class Signal : public Module
	{
	private:
		int signals;

	public:
		Signal();
		~Signal();
		bool hook(int sgn);
		void setCallback(lua_State *L);
		const char * getName() const;
		bool raise(int sgn);
	}; // Signal
	
	void handler(int signal);
	static Reference *cb;
} // posix
} // signal
} // love

#endif // LOVE_SIGNAL_POSIX_SIGNAL_H
