/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#ifndef LOVE_STREAM_H
#define LOVE_STREAM_H

// LOVE
#include <stddef.h>
#include "Object.h"

namespace love
{

class Stream : public Object
{
public:
	virtual ~Stream() {}

	// getData and getSize are assumed to talk about
	// the buffer

	/**
	 * A callback, gets called when some Stream consumer exhausts the data
	 **/
	virtual void fillBackBuffer() {}

	/**
	 * Get the front buffer, Streams are supposed to be (at least) double-buffered
	 **/
	virtual const void *getFrontBuffer() const = 0;

	/**
	 * Get the size of any (and in particular the front) buffer
	 **/
	virtual size_t getSize() const = 0;

	/**
	 * Swap buffers. Returns true if there is new data in the front buffer,
     * false otherwise.
	 * NOTE: If there is no back buffer ready, this call must be ignored
	 **/
	virtual bool swapBuffers() = 0;
}; // Stream

} // love

#endif // LOVE_STREAM_H
