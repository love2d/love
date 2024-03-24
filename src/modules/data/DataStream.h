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

#pragma once

#include "common/Stream.h"

namespace love
{
namespace data
{

class DataStream : public love::Stream
{
public:

	static love::Type type;

	DataStream(Data *data);
	virtual ~DataStream();

	// Implements Stream.
	DataStream *clone() override;

	bool isReadable() const override;
	bool isWritable() const override;
	bool isSeekable() const override;

	int64 read(void* data, int64 size) override;
	bool write(const void* data, int64 size) override;

	bool flush() override;

	int64 getSize() override;

	bool seek(int64 pos, SeekOrigin origin = SEEKORIGIN_BEGIN) override;
	int64 tell() override;

private:

	DataStream(const DataStream &other);

	StrongRef<Data> data;
	const uint8 *memory;
	uint8 *writableMemory;
	size_t offset;
	size_t size;

}; // DataStream

} // data
} // love
