/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

Message::Message(const std::string &name, Variant *a, Variant *b, Variant *c, Variant *d)
	: name(name)
	, nargs(0)
{
	args[0] = a;
	args[1] = b;
	args[2] = c;
	args[3] = d;
	for (int i = 0; i < 4; i++)
	{
		if (!args[i])
			break;
		args[i]->retain();
		nargs++;
	}
}

Message::~Message()
{
	for (int i = 0; i < nargs; i++)
		args[i]->release();
}

int Message::toLua(lua_State *L)
{
	luax_pushstring(L, name);
	for (int i = 0; i < nargs; i++)
		args[i]->toLua(L);
	return nargs+1;
}

Message *Message::fromLua(lua_State *L, int n)
{
	std::string name = luax_checkstring(L, n);
	n++;
	Message *m = new Message(name);
	for (int i = 0; i < 4; i++)
	{
		if (lua_isnoneornil(L, n+i))
			break;
		m->args[i] = Variant::fromLua(L, n+i);
		if (!m->args[i])
		{
			delete m;
			luaL_error(L, "Argument %d can't be stored safely\nExpected boolean, number, string or userdata.", n+i);
			return NULL;
		}
		m->nargs++;
	}
	return m;
}

Event::Event()
{
	mutex = thread::newMutex();
}

Event::~Event()
{
	delete mutex;
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
