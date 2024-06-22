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

#include <memory>

#include "Variant.h"
#include "common/StringMap.h"

namespace love
{

Variant::Variant(Type vtype)
	: type(vtype)
{}

Variant::Variant()
	: type(NIL)
{}

Variant::Variant(bool boolean)
	: type(BOOLEAN)
{
	data.boolean = boolean;
}

Variant::Variant(double number)
	: type(NUMBER)
{
	data.number = number;
}

Variant::Variant(const char *str, size_t len)
{
	if (len <= MAX_SMALL_STRING_LENGTH)
	{
		type = SMALLSTRING;
		memcpy(data.smallstring.str, str, len);
		data.smallstring.len = (uint8) len;
	}
	else
	{
		type = STRING;
		data.string = new SharedString(str, len);
	}
}

Variant::Variant(const std::string &str)
	: Variant(str.c_str(), str.length())
{
}

Variant::Variant(void *lightuserdata)
	: type(LUSERDATA)
{
	data.userdata = lightuserdata;
}

Variant::Variant(love::Type *lovetype, love::Object *object)
	: type(LOVEOBJECT)
{
	data.objectproxy.type = lovetype;
	data.objectproxy.object = object;

	if (data.objectproxy.object != nullptr)
		data.objectproxy.object->retain();
}

// Variant gets ownership of the vector.
Variant::Variant(SharedTable *table)
	: type(TABLE)
{
	data.table = table;
}

Variant::Variant(const Variant &v)
	: type(v.type)
	, data(v.data)
{
	if (type == STRING)
		data.string->retain();
	else if (type == LOVEOBJECT && data.objectproxy.object != nullptr)
		data.objectproxy.object->retain();
	else if (type == TABLE)
		data.table->retain();
}

Variant::Variant(Variant &&v)
	: type(std::move(v.type))
	, data(std::move(v.data))
{
	v.type = NIL;
}

Variant::~Variant()
{
	if (type == STRING)
		data.string->release();
	else if (type == LOVEOBJECT && data.objectproxy.object != nullptr)
		data.objectproxy.object->release();
	else if (type == TABLE)
		data.table->release();
}

Variant &Variant::operator = (const Variant &v)
{
	if (v.type == STRING)
		v.data.string->retain();
	else if (v.type == LOVEOBJECT && v.data.objectproxy.object != nullptr)
		v.data.objectproxy.object->retain();
	else if (v.type == TABLE)
		v.data.table->retain();

	if (type == STRING)
		data.string->release();
	else if (type == LOVEOBJECT && data.objectproxy.object != nullptr)
		data.objectproxy.object->release();
	else if (type == TABLE)
		data.table->release();

	type = v.type;
	data = v.data;

	return *this;
}

} // love
