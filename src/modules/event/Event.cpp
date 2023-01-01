/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

int Message::toLua(lua_State *L)
{
	luax_pushstring(L, name);

	for (const Variant &v : args)
		v.toLua(L);

	return (int) args.size() + 1;
}

Message *Message::fromLua(lua_State *L, int n)
{
	std::string name = luax_checkstring(L, n);
	std::vector<Variant> vargs;

	int count = lua_gettop(L) - n;
	n++;

	Variant varg;

	for (int i = 0; i < count; i++)
	{
		if (lua_isnoneornil(L, n+i))
			break;

		luax_catchexcept(L, [&]() {
			vargs.push_back(Variant::fromLua(L, n+i));
		});

		if (vargs.back().getType() == Variant::UNKNOWN)
		{
			vargs.clear();
			luaL_error(L, "Argument %d can't be stored safely\nExpected boolean, number, string or userdata.", n+i);
			return nullptr;
		}
	}

	return new Message(name, vargs);
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
