/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#include "Event.h"

using love::thread::Mutex;
using love::thread::Lock;

namespace love
{
namespace event
{

Message::Message(const std::string &name, const std::vector<Variant> &vargs)
	: name(name)
	, args(vargs)
{
}

Message::~Message()
{
}

Event::Event(const char *name)
	: Module(M_EVENT, name)
{
}

Event::~Event()
{
}

void Event::push(Message *msg)
{
	Lock lock(mutex);
	msg->retain();
	queue.push(msg);
}

bool Event::poll(Message *&msg)
{
	Lock lock(mutex);
	if (queue.empty())
		return false;
	msg = queue.front();
	queue.pop();
	return true;
}

void Event::clear()
{
	Lock lock(mutex);
	while (!queue.empty())
	{
		// std::queue::pop will remove the first (front) element.
		queue.front()->release();
		queue.pop();
	}
}

} // event
} // love
