/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "File.h"

// STD
#include <cstring>

// LOVE
#include "Filesystem.h"
#include <filesystem/FileData.h>

namespace love
{
namespace filesystem
{
namespace physfs
{
	extern bool hack_setupWriteDirectory();

	File::File(std::string filename) 
		: filename(filename), file(0), mode(filesystem::File::CLOSED)
	{
	}

	File::~File()
	{
	}
	
	bool File::open(Mode mode)
	{
		if(mode == CLOSED)
			return true;

		// File must exist if read mode.
		if((mode == READ) && !PHYSFS_exists(filename.c_str()))
			throw love::Exception("Could not open file %s. Does not exist.", filename.c_str());

		// Check whether the write directory is set.
		if((mode == APPEND || mode == WRITE) && (PHYSFS_getWriteDir() == 0) && !hack_setupWriteDirectory())
			throw love::Exception("Could not set write directory.");

		// File already open?
		if(file != 0)
			return false;

		this->mode = mode;

		switch(mode)
		{
		case READ:
			file = PHYSFS_openRead(filename.c_str());
			break;
		case APPEND:
			file = PHYSFS_openAppend(filename.c_str());
			break;
		case WRITE:
			file = PHYSFS_openWrite(filename.c_str());
			break;
		default:
			break;
		}

		return (file != 0);
	}
	
	bool File::close()
	{
		if(!PHYSFS_close(file))
			return false;
		mode = CLOSED;
		file = 0;
		return true;
	}

	unsigned int File::getSize()
	{
		// If the file is closed, open it to
		// check the size.
		if(file == 0)
		{
			open(READ);
			unsigned int size = (unsigned int)PHYSFS_fileLength(file);
			close();
			return size;
		}

		return (unsigned int)PHYSFS_fileLength(file);
	}


	Data * File::read(int size)
	{
		bool isOpen = (file != 0);

		if(!isOpen && !open(READ))
			throw love::Exception("Could not read file %s.", filename.c_str());

		int max = (int)PHYSFS_fileLength(file);
		size = (size == ALL) ? max : size;
		size = (size > max) ? max : size;

		FileData * fileData = new FileData(size, getFilename());

		read(fileData->getData(), size);

		if(!isOpen)
			close();

		return fileData;
	}

	int File::read(void * dst, int size)
	{
		bool isOpen = (file != 0);

		if(!isOpen)
			open(READ);

		int max = (int)PHYSFS_fileLength(file);
		size = (size == ALL) ? max : size;
		size = (size > max) ? max : size;

		int read = (int)PHYSFS_read(file, dst, 1, size);

		if(!isOpen)
			close();

		return read;
	}

	bool File::write(const void * data, int size)
	{
		if(file == 0)
			throw love::Exception("Could not write to file. File not open.");

		// Try to write.
		int written = static_cast<int>(PHYSFS_write(file, data, 1, size));

		// Check that correct amount of data was written.
		if(written != size)
			return false;

		return true;
	}

	bool File::write(const Data * data, int size)
	{
		return write(data->getData(), (size == ALL) ? data->getSize() : size);
	}

	bool File::eof()
	{
		if(file == 0 || PHYSFS_eof(file))
			return true;
		return false;
	}

	int File::tell()
	{
		if(file == 0)
			return -1;

		return (int)PHYSFS_tell(file);
	}

	bool File::seek(int pos)
	{
		if(file == 0)
			return false;
		
		if(!PHYSFS_seek(file, (PHYSFS_uint64)pos))
			return false;
		return true;
	}

	std::string File::getFilename() const
	{
		return filename;
	}

	std::string File::getExtension() const
	{
		std::string::size_type idx = filename.rfind('.');

		if(idx != std::string::npos)
			return filename.substr(idx+1);
		else
			return std::string();
	}

	filesystem::File::Mode File::getMode()
	{
		return mode;
	}

} // physfs
} // filesystem
} // love
