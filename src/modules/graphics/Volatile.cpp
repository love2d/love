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

#include "Volatile.h"

namespace love
{
namespace graphics
{

// Static members.
std::list<Volatile *> Volatile::all;

Volatile::Volatile()
{
	// Insert this object into "all".
	all.push_back(this);
}

Volatile::~Volatile()
{
	// Remove the pointer to this object.
	all.remove(this);
}

bool Volatile::loadAll()
{
	bool success = true;

	for (Volatile *v : all)
		success = success && v->loadVolatile();

	return success;
}

void Volatile::unloadAll()
{
	for (Volatile *v : all)
		v->unloadVolatile();
}

} // graphics
} // love
