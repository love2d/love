/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "types.h"
#include "StringMap.h"

namespace love
{

static StringMap<uint32, love::Type::MAX_TYPES> types(nullptr, 0);

void addTypeName(uint32 type, const char *name)
{
	const char *n;
	if (!types.find(type, n))
		types.add(name, type);
}

bool getTypeName(const char *in, uint32 &out)
{
	return types.find(in, out);
}

bool getTypeName(uint32 in, const char *&out)
{
	return types.find(in, out);
}

uint32 love::Type::nextId = 1;

Type::Type(Type *parent)
	: inited(false)
	, parent(parent)
{
}

void love::Type::init()
{
	id = nextId++;
	bits[id] = true;
	inited = true;

	if (!parent)
		return;
	if (!parent->inited)
		parent->init();
	bits |= parent->bits;
}

uint32 love::Type::getId()
{
	if (!inited)
		init();
	return id;
}

bool love::Type::isa(const uint32 &other)
{
	if (!inited)
		init();
	return bits[other];
}

bool love::Type::isa(love::Type &other)
{
	if (!other.inited)
		other.init();
	return isa(other.id);
}

} // love

// FIXME: Move this to the relevant files
#include "Data.h"
#include "Stream.h"
#include "graphics/Drawable.h"
#include "filesystem/Filesystem.h"
#include "image/Image.h"

love::Type love::Data::type(&Object::type);
love::Type love::Stream::type(&Object::type);
love::Type love::graphics::Drawable::type(&Object::type);
love::Type love::filesystem::Filesystem::type(&Module::type);
love::Type love::image::Image::type(&Module::type);
