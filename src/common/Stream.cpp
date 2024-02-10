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

// LOVE
#include "Stream.h"
#include "Data.h"
#include "data/ByteData.h"
#include "Exception.h"

namespace love
{

love::Type Stream::type("Stream", &Object::type);

Data *Stream::read(int64 size)
{
	int64 max = LOVE_INT64_MAX;
	int64 cur = 0;

	if (isSeekable())
	{
		max = getSize();
		cur = tell();
	}

	if (cur < 0)
		cur = 0;
	else if (cur > max)
		cur = max;

	if (cur + size > max)
		size = max - cur;

	StrongRef<data::ByteData> dst(new data::ByteData(size, false), Acquire::NORETAIN);

	int64 bytesRead = read(dst->getData(), size);

	if (bytesRead < 0 || (bytesRead == 0 && bytesRead != size))
		throw love::Exception("Could not read from stream.");

	if (bytesRead < size)
		dst.set(new data::ByteData(dst->getData(), (size_t) bytesRead), Acquire::NORETAIN);

	dst->retain();
	return dst;
}

bool Stream::write(Data *src)
{
	return write(src, 0, src->getSize());
}

bool Stream::write(Data *src, int64 offset, int64 size)
{
	if (offset < 0 || size < 0 || offset + size > (int64) src->getSize())
		throw love::Exception("Offset and size parameters do not fit within the given Data's size.");

	return write((const uint8 *) src->getData() + offset, size);
}

} // love
