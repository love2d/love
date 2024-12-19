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

#ifndef LOVE_FILESYSTEM_FILE_H
#define LOVE_FILESYSTEM_FILE_H

// STD
#include <string>

// LOVE
#include "common/Data.h"
#include "common/Object.h"
#include "common/Stream.h"
#include "common/StringMap.h"
#include "common/int.h"
#include "FileData.h"

namespace love
{
namespace filesystem
{

/**
 * A File interface, providing generic means of reading from and
 * writing to files.
 **/
class File : public Stream
{
public:

	static love::Type type;

	static const int64 SIZE_ALL = -1;

	/**
	 * File open mode.
	 **/
	enum Mode
	{
		MODE_CLOSED,
		MODE_READ,
		MODE_WRITE,
		MODE_APPEND,
		MODE_MAX_ENUM
	};

	enum BufferMode
	{
		BUFFER_NONE,
		BUFFER_LINE,
		BUFFER_FULL,
		BUFFER_MAX_ENUM
	};

	/**
	 * Destructor.
	 **/
	virtual ~File();

	// Implements Stream.
	bool isReadable() const override { return getMode() == MODE_READ; }
	bool isWritable() const override { return getMode() == MODE_WRITE || getMode() == MODE_APPEND; }
	bool isSeekable() const override { return isOpen(); }

	using Stream::read;
	using Stream::write;

	/**
	 * Opens the file in a certain mode.
	 *
	 * @param mode MODE_READ, MODE_WRITE, MODE_APPEND.
	 * @return True if successful, false otherwise.
	 **/
	virtual bool open(Mode mode) = 0;

	/**
	 * Closes the file.
	 *
	 * @return True if successful, false otherwise.
	 **/
	virtual bool close() = 0;

	/**
	 * Gets whether the file is open.
	 **/
	virtual bool isOpen() const = 0;

	/**
	 * Reads data from the file and allocates a Data object.
	 *
	 * @param size The number of bytes to attempt reading.
	 * @return A newly allocated Data object.
	 **/
	FileData *read(int64 size) override;
	FileData *read();

	/**
	 * Checks whether we are currently at end-of-file.
	 *
	 * @return True if EOF, false otherwise.
	 **/
	virtual bool isEOF() = 0;

	/**
	 * Sets the buffering mode for the file. When buffering is enabled, the file
	 * will not write to disk (or will pre-load data if in read mode) until the
	 * buffer's capacity is reached.
	 * In the BUFFER_LINE mode, the file will also write to disk if a newline is
	 * written.
	 *
	 * @param bufmode The buffer mode.
	 * @param size The size in bytes of the buffer.
	 **/
	virtual bool setBuffer(BufferMode bufmode, int64 size) = 0;

	/**
	 * @param[out] size The size in bytes of the buffer.
	 * @return The current buffer mode.
	 **/
	virtual BufferMode getBuffer(int64 &size) const = 0;

	/**
	 * Gets the current mode of the File.
	 * @return The current mode of the File; CLOSED, READ, WRITE or APPEND.
	 **/
	virtual Mode getMode() const = 0;

	/**
	 * Gets the filename for this File, or empty string if none.
	 * @return The filename for this File.
	 **/
	virtual const std::string &getFilename() const = 0;

	/**
	 * Gets the file extension for this File, or empty string if none.
	 * @return The file extension for this File (without the dot).
	 **/
	virtual std::string getExtension() const;

	STRINGMAP_CLASS_DECLARE(Mode);
	STRINGMAP_CLASS_DECLARE(BufferMode);

}; // File

} // filesystem
} // love

#endif // LOVE_FILESYSTEM_FILE_H
