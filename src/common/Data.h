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

#ifndef LOVE_DATA_H
#define LOVE_DATA_H

// LOVE
#include "config.h"
#include "Object.h"

// C
#include <stddef.h>

namespace love
{

/**
 * This class is a simple abstraction over all objects which contain data.
 **/
class Data : public Object
{
public:

	static love::Type type;

	/**
	 * Destructor.
	 **/
	virtual ~Data() {}

	/**
	 * Creates a duplicate of Data derived class instance.
	 **/
	virtual Data *clone() const = 0;
	/**
	 * Gets a pointer to the data. This pointer will obviously not
	 * be valid if the Data object is destroyed.
	 **/
	virtual void *getData() const = 0;

	/**
	 * Gets the size of the Data in bytes.
	 **/
	virtual size_t getSize() const = 0;

}; // Data

} // love

#endif // LOVE_DATA_H
