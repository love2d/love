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

#ifndef LOVE_THREAD_CHANNEL_H
#define LOVE_THREAD_CHANNEL_H

// STL
#include <queue>

// LOVE
#include "common/Variant.h"
#include "common/int.h"
#include "threads.h"

namespace love
{
namespace thread
{

class Channel : public love::Object
{
// FOR WRAPPER USE ONLY
friend int w_Channel_performAtomic(lua_State *);

public:

	static love::Type type;

	Channel();
	~Channel();

	uint64 push(const Variant &var);
	bool supply(const Variant &var); // blocking push
	bool supply(const Variant &var, double timeout);
	bool pop(Variant *var);
	bool demand(Variant *var); // blocking pop
	bool demand(Variant *var, double timeout); // blocking pop
	bool peek(Variant *var);
	int getCount() const;
	bool hasRead(uint64 id) const;
	void clear();

private:

	void lockMutex();
	void unlockMutex();

	MutexRef mutex;
	ConditionalRef cond;
	std::queue<Variant> queue;

	uint64 sent;
	uint64 received;

}; // Channel

} // thread
} // love

#endif // LOVE_THREAD_CHANNEL_H
