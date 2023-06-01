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

#ifndef LOVE_FILESYSTEM_PHYSFS_PHYSFSIO_H
#define LOVE_FILESYSTEM_PHYSFS_PHYSFSIO_H

#include <cstdint>
#include <cstdio>
#include <string>

#include "libraries/physfs/physfs.h"

namespace love
{
namespace filesystem
{
namespace physfs
{

template <typename Derived>
struct PhysfsIo : PHYSFS_Io
{
protected:

	PhysfsIo()
	: PHYSFS_Io()
	{
		// Direct initialization of PHYSFS_Io members in the initializer list
		// doesn't work in VS2013.
		this->version = Derived::version;
		this->opaque = this;
		this->read = staticRead; // May be null.
		this->write = staticWrite; // May be null.
		this->seek = staticSeek;
		this->tell = staticTell;
		this->length = staticLength;
		this->duplicate = staticDuplicate;
		this->flush = staticFlush; // May be null.
		this->destroy = staticDestroy;
	}

	virtual ~PhysfsIo() {}

private:

	// Returns: number of bytes read, 0 on EOF, -1 on failure
	static PHYSFS_sint64 staticRead(struct PHYSFS_Io* io, void* buf, PHYSFS_uint64 len)
	{
		return derived(io)->read(buf, len);
	}

	// Returns: number of bytes written, -1 on failure
	static PHYSFS_sint64 staticWrite(struct PHYSFS_Io* io, const void* buf, PHYSFS_uint64 len)
	{
		return derived(io)->write(buf, len);
	}

	// Returns: non-zero on success, zero on error
	static int staticSeek(struct PHYSFS_Io* io, PHYSFS_uint64 offset)
	{
		return derived(io)->seek(offset);
	}

	// Returns: current offset from start, -1 on error
	static PHYSFS_sint64 staticTell(struct PHYSFS_Io* io)
	{
		return derived(io)->tell();
	}

	// Returns: total size in bytes, -1 on error
	static PHYSFS_sint64 staticLength(struct PHYSFS_Io* io)
	{
		return derived(io)->length();
	}

	static struct PHYSFS_Io* staticDuplicate(struct PHYSFS_Io* io)
	{
		// Just use copy constructor
		return new Derived(*derived(io));
	}

	// Returns: non-zero on success, zero on error
	static int staticFlush(struct PHYSFS_Io* io)
	{
		return derived(io)->flush();
	}

	static void staticDestroy(struct PHYSFS_Io* io)
	{
		// Just use destructor
		delete derived(io);
	}

	static Derived* derived(PHYSFS_Io* io)
	{
		return static_cast<Derived*>(reinterpret_cast<PhysfsIo*>(io->opaque));
	}
};

struct StripSuffixIo : public PhysfsIo<StripSuffixIo>
{
	static const uint32_t version = 0;

	std::string filename;
	FILE* file = nullptr;

	// The constructor is private in favor of this function to prevent stack allocation
	// because Physfs will take ownership of this object and call destroy on it later.
	static StripSuffixIo* create(std::string f) { return new StripSuffixIo(f); }

	virtual ~StripSuffixIo()
	{
		if (file)
		{
			std::fclose(file);
		}
	}

	StripSuffixIo(const StripSuffixIo& other)
		: StripSuffixIo(other.filename)
	{
	}

	bool determineStrippedLength();

	int64_t read(void* buf, uint64_t len);
	int64_t write(const void* buf, uint64_t len);
	int64_t seek(uint64_t offset);
	int64_t tell();
	int64_t length();
	int64_t flush();

private:

	StripSuffixIo(std::string f)
		: filename(std::move(f))
		, file(std::fopen(filename.c_str(), "rb"))
	{
	}

	int64_t fullLength();

	int64_t strippedLength_ = -1;
};

} // physfs
} // filesystem
} // love

#endif
