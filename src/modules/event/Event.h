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

#ifndef LOVE_EVENT_EVENT_H
#define LOVE_EVENT_EVENT_H

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"
#include "common/Variant.h"
#include "keyboard/Keyboard.h"
#include "mouse/Mouse.h"
#include "joystick/Joystick.h"
#include "thread/threads.h"

// C++
#include <queue>
#include <vector>

namespace love
{
namespace event
{

class Message : public Object
{
public:

	Message(const std::string &name, const std::vector<Variant> &vargs = {});
	~Message();

	int toLua(lua_State *L);
	static Message *fromLua(lua_State *L, int n);

	const std::string name;
	const std::vector<Variant> args;

}; // Message

class Event : public Module
{
public:
	virtual ~Event();

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_EVENT; }

	void push(Message *msg);
	bool poll(Message *&msg);
	virtual void clear();

	virtual void pump() = 0;
	virtual Message *wait() = 0;

protected:
	love::thread::MutexRef mutex;
	std::queue<Message *> queue;

}; // Event

} // event
} // love

#endif // LOVE_EVENT_EVENT_H
