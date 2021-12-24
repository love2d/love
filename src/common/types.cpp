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

// STL
#include <unordered_map>

#include "types.h"

namespace love
{

static std::unordered_map<std::string, Type*> types;

Type::Type(const char *name, Type *parent)
	: name(name)
	, parent(parent)
	, id(0)
	, inited(false)
{
}

void Type::init()
{
	static uint32 nextId = 1;

	// Make sure we don't init twice, that would be bad
	if (inited)
		return;

	// Note: we add it here, not in the constructor, because some Types can get initialized before the map!
	types[name] = this;
	id = nextId++;
	bits[id] = true;
	inited = true;

	if (!parent)
		return;
	if (!parent->inited)
		parent->init();
	bits |= parent->bits;
}

uint32 Type::getId()
{
	if (!inited)
		init();
	return id;
}

const char *Type::getName() const
{
	return name;
}

Type *Type::byName(const char *name)
{
	auto pos = types.find(name);
	if (pos == types.end())
		return nullptr;
	return pos->second;
}

} // love
