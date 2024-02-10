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

#ifndef LOVE_STREAM_H
#define LOVE_STREAM_H

// LOVE
#include <stddef.h>
#include "Object.h"
#include "int.h"

namespace love
{

class Data;

class Stream : public Object
{
public:

	enum SeekOrigin
	{
		SEEKORIGIN_BEGIN,
		SEEKORIGIN_CURRENT,
		SEEKORIGIN_END,
		SEEKORIGIN_MAX_ENUM
	};

	static love::Type type;

	virtual ~Stream() {}

	/**
	 * Creates a new copy of the Stream, with the same settings as the original.
	 * The seek position will be reset in the copy.
	 **/
	virtual Stream *clone() = 0;

	/**
	 * Gets whether read() is supported for this Stream.
	 **/
	virtual bool isReadable() const = 0;

	/**
	 * Gets whether write() is supported for this Stream.
	 **/
	virtual bool isWritable() const = 0;

	/**
	 * Gets whether seek(), tell(), and getSize() are supported for this Stream.
	 **/
	virtual bool isSeekable() const = 0;

	/**
	 * Reads data into the destination buffer, and returns the number of bytes
	 * actually read.
	 **/
	virtual int64 read(void *dst, int64 size) = 0;

	/**
	 * Reads data into a new Data object.
	 **/
	virtual Data *read(int64 size);

	/**
	 * Writes data from the source buffer into the Stream.
	 **/
	virtual bool write(const void *src, int64 size) = 0;

	/**
	 * Writes data from the source Data object into the Stream.
	 **/
	virtual bool write(Data *src, int64 offset, int64 size);
	bool write(Data *src);

	/**
	 * Flushes all data written to the Stream.
	 **/
	virtual bool flush() = 0;

	/**
	 * Gets the total size of the Stream, if supported.
	 **/
	virtual int64 getSize() = 0;

	/**
	 * Sets the current position in the Stream, if supported.
	 **/
	virtual bool seek(int64 pos, SeekOrigin origin = SEEKORIGIN_BEGIN) = 0;

	/**
	 * Gets the current position in the Stream, if supported.
	 **/
	virtual int64 tell() = 0;

}; // Stream

} // love

#endif // LOVE_STREAM_H
