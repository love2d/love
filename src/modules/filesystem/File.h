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

#ifndef LOVE_FILESYSTEM_FILE_H
#define LOVE_FILESYSTEM_FILE_H

// STD
#include <string>

// LOVE
#include "common/Data.h"
#include "common/Object.h"
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
class File : public Object
{
public:

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
	 * Used to indicate ALL data in a file.
	 **/
	static const int64 ALL = -1;

	/**
	 * Destructor.
	 **/
	virtual ~File();

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
	 * Gets the size of the file.
	 *
	 * @return The size of the file.
	 **/
	virtual int64 getSize() = 0;

	/**
	 * Reads data from the file and allocates a Data object.
	 *
	 * @param size The number of bytes to attempt reading, or -1 for EOF.
	 * @return A newly allocated Data object.
	 **/
	virtual FileData *read(int64 size = ALL);

	/**
	 * Reads data into the destination buffer.
	 *
	 * @param dst The destination buffer.
	 * @param size The number of bytes to attempt reading.
	 * @return The number of bytes actually read.
	 **/
	virtual int64 read(void *dst, int64 size) = 0;

	/**
	 * Writes data into the File.
	 *
	 * @param data The source buffer.
	 * @param size The size of the buffer.
	 * @return True of success, false otherwise.
	 **/
	virtual bool write(const void *data, int64 size) = 0;

	/**
	 * Writes a Data object into the File.
	 *
	 * @param data The data object to write into the file.
	 * @param size The number of bytes to attempt writing, or -1 for everything.
	 * @return True of success, false otherwise.
	 **/
	virtual bool write(const Data *data, int64 size = ALL);

	/**
	 * Flushes the currently buffered file data to disk. Only applicable in
	 * write mode.
	 **/
	virtual bool flush() = 0;

	/**
	 * Checks whether we are currently at end-of-file.
	 *
	 * @return True if EOF, false otherwise.
	 **/
	virtual bool isEOF() = 0;

	/**
	 * Gets the current position in the File.
	 *
	 * @return The current byte position in the File.
	 **/
	virtual int64 tell() = 0;

	/**
	 * Seeks to a certain position in the File.
	 *
	 * @param pos The byte position in the file.
	 * @return True on success, false otherwise.
	 **/
	virtual bool seek(uint64 pos) = 0;

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

	static bool getConstant(const char *in, Mode &out);
	static bool getConstant(Mode in, const char *&out);

	static bool getConstant(const char *in, BufferMode &out);
	static bool getConstant(BufferMode in, const char *&out);

private:

	static StringMap<Mode, MODE_MAX_ENUM>::Entry modeEntries[];
	static StringMap<Mode, MODE_MAX_ENUM> modes;

	static StringMap<BufferMode, BUFFER_MAX_ENUM>::Entry bufferModeEntries[];
	static StringMap<BufferMode, BUFFER_MAX_ENUM> bufferModes;

}; // File

} // filesystem
} // love

#endif // LOVE_FILESYSTEM_FILE_H
