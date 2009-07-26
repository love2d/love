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

#ifndef LOVE_EVENT_EVENT_H
#define LOVE_EVENT_EVENT_H

// LOVE
#include <common/Module.h>

namespace love
{
namespace event
{
	class Event : public Module
	{
	protected:
		virtual ~Event(){};
	public:

		enum
		{
		   EVENT_NOEVENT = 0,
		   EVENT_ACTIVEEVENT,
		   EVENT_KEYDOWN,
		   EVENT_KEYUP,
		   EVENT_MOUSEMOTION,	
		   EVENT_MOUSEBUTTONDOWN,
		   EVENT_MOUSEBUTTONUP,
		   EVENT_JOYAXISMOTION,
		   EVENT_JOYBALLMOTION,
		   EVENT_JOYHATMOTION,
		   EVENT_JOYBUTTONDOWN,
		   EVENT_JOYBUTTONUP,
		   EVENT_QUIT,
		   EVENT_SYSWMEVENT,
		   EVENT_RESERVEDA,
		   EVENT_RESERVEDB,
		   EVENT_VIDEORESIZE,
		   EVENT_VIDEOEXPOSE,	
		   EVENT_RESERVED2,
		   EVENT_RESERVED3,	
		   EVENT_RESERVED4,
		   EVENT_RESERVED5,	
		   EVENT_RESERVED6,
		   EVENT_RESERVED7,	
		   EVENT_USEREVENT = 24,
		   EVENT_NUMEVENTS = 32
		};

	}; // Event

} // event
} // love

#endif // LOVE_EVENT_EVENT_H
