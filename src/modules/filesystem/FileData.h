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

#ifndef LOVE_FILESYSTEM_FILE_DATA_H
#define LOVE_FILESYSTEM_FILE_DATA_H

// LOVE
#include "common/Data.h"
#include "common/int.h"
#include "common/Exception.h"

#include <string>

namespace love
{
namespace filesystem
{

class FileData : public Data
{
public:

	static love::Type type;

	FileData(uint64 size, const std::string &filename);
	FileData(const FileData &c);

	virtual ~FileData();

	// Implements Data.
	FileData *clone() const;
	void *getData() const;
	size_t getSize() const;

	const std::string &getFilename() const;
	const std::string &getExtension() const;
	const std::string &getName() const;

private:

	// The actual data.
	char *data;

	// Size of the data.
	uint64 size;

	// The filename used for error purposes.
	std::string filename;

	// The extension (without dot). Used to identify file type.
	std::string extension;

	// The file name without the extension (and without the dot).
	std::string name;

}; // FileData

} // filesystem
} // love

#endif // LOVE_FILESYSTEM_FILE_DATA_H
