/**
* Copyright (c) 2006-2009 LOVE Development Team
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
#include <common/Data.h>
#include <common/Object.h>

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
			CLOSED,
			READ,
			WRITE,
			APPEND,
		};

		/**
		* Used to indicate ALL data in a file.
		**/
		static const int ALL = -1;

		/**
		* Destructor.
		**/
		virtual ~File(){};

		/**
		* Opens the file in a certain mode.
		* 
		* @param mode READ, WRITE, APPEND.
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
		* Gets the size of the file.
		* 
		* @return The size of the file.
		**/
		virtual unsigned int getSize() = 0;

		/**
		* Reads data from the file and allocates a Data object.
		* 
		* @param size The number of bytes to attempt reading, or -1 for EOF.
		* @return A newly allocated Data object.
		**/
		virtual Data * read(int size = ALL) = 0;

		/**
		* Reads data into the destination buffer.
		* 
		* @param dst The destination buffer.
		* @param size The number of bytes to attempt reading.
		* @return The number of bytes actually read.
		**/
		virtual int read(void * dst, int size) = 0;

		/**
		* Writes data into the File.
		* 
		* @param data The source buffer.
		* @param size The size of the buffer.
		* @return True of success, false otherwise.
		**/
		virtual bool write(const void * data, int size) = 0;

		/**
		* Writes a Data object into the File.
		* 
		* @param data The data object to write into the file.
		* @param size The number of bytes to attempt writing, or -1 for everything.
		* @return True of success, false otherwise.
		**/
		virtual bool write(const Data * data, int size = ALL) = 0;

		/**
		* Checks whether we are currently at end-of-file.
		* 
		* @return True if EOF, false otherwise.
		**/
		virtual bool eof() = 0;

		/**
		* Gets the current position in the File.
		* 
		* @return The current byte position in the File.
		**/
		virtual int tell() = 0;

		/**
		* Seeks to a certain position in the File.
		* 
		* @param pos The byte position in the file.
		* @return True on success, false otherwise.
		**/
		virtual bool seek(int pos) = 0;

		/**
		* Gets the current mode of the File. 
		* @return The current mode of the File; CLOSED, READ, WRITE or APPEND.
		**/
		virtual Mode getMode() = 0;

		/**
		* Gets the filename for this File, or empty string if none.
		* @return The filename for this File.
		**/
		virtual std::string getFilename() const = 0;

		/**
		* Gets the file extension for this File, or empty string if none.
		* @return The file extension for this File (without the dot).
		**/
		virtual std::string getExtension() const = 0;

	}; // File

} // filesystem
} // love

#endif // LOVE_FILESYSTEM_FILE_H
