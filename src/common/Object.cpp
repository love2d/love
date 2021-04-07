/**
 * Copyright (c) 2006-2021 LOVE Development Team
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

// LOVE
#include "Object.h"

namespace love
{

love::Type Object::type("Object", nullptr);

Object::Object()
	: count(1)
{
}

Object::Object(const Object & /*other*/)
	: count(1) // Always start with a reference count of 1.
{
}

Object::~Object()
{
}

int Object::getReferenceCount() const
{
	return count;
}

void Object::retain()
{
	count.fetch_add(1, std::memory_order_relaxed);
}

void Object::release()
{
	// http://www.boost.org/doc/libs/1_56_0/doc/html/atomic/usage_examples.html
	if (count.fetch_sub(1, std::memory_order_release) == 1)
	{
		std::atomic_thread_fence(std::memory_order_acquire);
		delete this;
	}
}

} // love
