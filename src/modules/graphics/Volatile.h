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

#ifndef LOVE_GRAPHICS_VOLATILE_H
#define LOVE_GRAPHICS_VOLATILE_H

// STL
#include <list>

namespace love
{
namespace graphics
{

/**
 * This class is the superclass of all objects which must completely or
 * partially reload when the user changes the display resolution. All
 * volatile objects will be notified when the display mode changes.
 *
 * @author Anders Ruud
 **/
class Volatile
{
private:

	// A list of all Volatile object currently alive.
	static std::list<Volatile *> all;

public:

	/**
	 * Constructor. Automatically adds \c this into the list
	 * of volatile objects.
	 **/
	Volatile();

	/**
	 * Destructor. Removes \c this from the list of volatile
	 * objects.
	 **/
	virtual ~Volatile();

	/**
	 * Loads the part(s) of the object which is destroyed when
	 * the display mode is changed.
	 *
	 * @return True if successful, false on errors.
	 **/
	virtual bool loadVolatile() = 0;

	/**
	 * Unloads the part(s) of the objects which would be destroyed
	 * anyway when the display mode is changed.
	 **/
	virtual void unloadVolatile() = 0;

	// Static:

	/**
	 * Calls \c loadVolatile() on each element in the list of volatiles.
	 *
	 * @return True if all elements succeeded, false if one or more failed.
	 **/
	static bool loadAll();

	/**
	 * Calls \c unloadVolatile() on each element in the list of volatiles.
	 **/
	static void unloadAll();

}; // Volatile

} // graphics
} // love

#endif // LOVE_GRAPHICS_VOLATILE_H
