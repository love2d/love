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

#pragma once

namespace love
{

// Currently only meant for simple and small types.
template <typename T>
struct Optional
{
	T value;
	bool hasValue;

	Optional()
		: value(T())
		, hasValue(false)
	{}

	Optional(T val)
		: value(val)
		, hasValue(true)
	{}

	void set(T val)
	{
		value = val;
		hasValue = true;
	}
};

typedef Optional<bool> OptionalBool;
typedef Optional<float> OptionalFloat;
typedef Optional<double> OptionalDouble;
typedef Optional<int> OptionalInt;

} // love
