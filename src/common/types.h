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

#ifndef LOVE_TYPES_H
#define LOVE_TYPES_H

#include "int.h"

// STD
#include <bitset>
#include <vector>

namespace love
{

class Type
{
public:
	static const uint32 MAX_TYPES = 128;

	Type(const char *name, Type *parent);
	Type(const Type&) = delete;

	static Type *byName(const char *name);

	void init();
	uint32 getId();
	const char *getName() const;

	bool isa(const uint32 &other)
	{
		if (!inited)
			init();
		return bits[other];
	}

	bool isa(const Type &other)
	{
		if (!inited)
			init();
		// Note that if this type implements the other
		// calling init above will also have inited
		// the other.
		return bits[other.id];
	}

private:
	const char * const name;
	Type * const parent;
	uint32 id;
	bool inited;
	std::bitset<MAX_TYPES> bits;
};

} // love

#endif // LOVE_TYPES_H
