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

#include <cassert>
#include <algorithm>

#include "PhysfsIo.h"

namespace love
{
namespace filesystem
{
namespace physfs
{

bool StripSuffixIo::determineStrippedLength()
{
	if (!file) {
		return false;
	}
	const int64_t fullSize = fullLength();
	int64_t chunkSize = std::min(fullSize, (int64_t)8192);
	std::string buffer;
	buffer.reserve(chunkSize);
	int64_t i = fullSize - chunkSize;
	// I don't think we really need to go through the whole file. The main known use
	// case for this functionality is to skip windows codesign signatures, which are
	// from what I have seen ~12KB or so, but trying is better than just failing.
	while (i >= 0)
	{
		buffer.resize(chunkSize);
		if (seek(i) == 0)
			return false;
		const auto n = read(&buffer[0], chunkSize);
		if (n <= 0)
			return false;
		buffer.resize(n);
		// We are looking for the magic bytes that indicate the start
		// of the "End of cental directory record (EOCD)".
		// As this is most likely not a multi-disk zip, we could include 4 bytes of 0x00,
		// but I'd rather make fewer assumptions.
		const auto endOfCentralDirectory = buffer.rfind("\x50\x4B\x05\x06");
		if (endOfCentralDirectory != std::string::npos)
		{
			i = i + endOfCentralDirectory;
			break;
		}
		if (i == 0)
			break;
		i = std::max((int64_t)0, i - chunkSize);
	}

	if (i > 0)
	{
		// The EOCD record is at least 22 bytes but may include a comment
		if (i + 22 > fullSize)
			return false; // Incomplete central directory
		// The comment length (u16) is located 20 bytes from the start of the EOCD record
		if (seek(i + 20) == 0)
			return false;
		uint8_t buffer[2];
		const auto n = read(buffer, 2);
		if (n <= 0)
			return false;
		const auto commentSize = (buffer[1] << 8) | buffer[0];
		if (i + 22 + commentSize > fullSize) // Comment incomplete
			return false;
		// We pretend the file ends just after the comment
		// (which should be the end of the embedded zip file)
		strippedLength_ = i + 22 + commentSize;
	}
	else
	{
		strippedLength_ = fullSize;
	}

	if (seek(0) == 0)
		return false;
	return true;
}

int64_t StripSuffixIo::read(void* buf, uint64_t len)
{
	if (!file)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	const auto ret = std::fread(buf, 1, len, file);
	if (ret == 0)
	{
		if (std::feof(file))
		{
			PHYSFS_setErrorCode(PHYSFS_ERR_OK);
			return 0;
		}
		else
		{
			PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
			return -1;
		}
	}
	else if (ret < len && std::ferror(file))
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	PHYSFS_setErrorCode(PHYSFS_ERR_OK);
	return ret;
}

int64_t StripSuffixIo::write(const void* /*buf*/, uint64_t /*len*/)
{
	PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
	return -1;
}

int64_t StripSuffixIo::seek(uint64_t offset)
{
	if (!file)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return 0;
	}
	const auto ret = std::fseek(file, offset, SEEK_SET);
	PHYSFS_setErrorCode(ret != 0 ? PHYSFS_ERR_OS_ERROR : PHYSFS_ERR_OK);
	return ret == 0 ? 1 : 0;
}

int64_t StripSuffixIo::tell()
{
	if (!file)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	return std::ftell(file);
}

int64_t StripSuffixIo::length()
{
	return strippedLength_;
}

int64_t StripSuffixIo::flush()
{
	if (!file)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return 0;
	}
	return std::fflush(file) == 0 ? 1 : 0;
}

int64_t StripSuffixIo::fullLength()
{
	assert(file);
	const auto cur = std::ftell(file);
	if (cur == -1)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	if (std::fseek(file, 0, SEEK_END) != 0)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	const auto len = std::ftell(file);
	if (len == -1)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	if (std::fseek(file, cur, SEEK_SET) != 0)
	{
		// We do have the length now, but something is wrong, so we return an error anyways
		PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
		return -1;
	}
	return len;
}

} // physfs
} // filesystem
} // love
