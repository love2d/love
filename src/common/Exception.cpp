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

#include "common/config.h"
#include "Exception.h"

#include <iostream>

namespace love
{

Exception::Exception(const char *fmt, ...)
{
	va_list args;
	int size_buffer = 256, size_out;
	char *buffer;
	while (true)
	{
		buffer = new char[size_buffer];
		memset(buffer, 0, size_buffer);

		va_start(args, fmt);
		size_out = vsnprintf(buffer, size_buffer, fmt, args);
		va_end(args);

		// see http://perfec.to/vsnprintf/pasprintf.c
		// if size_out ...
		//      == -1             --> output was truncated
		//      == size_buffer    --> output was truncated
		//      == size_buffer-1  --> ambiguous, /may/ have been truncated
		//       > size_buffer    --> output was truncated, and size_out
		//                            bytes would have been written
		if (size_out == size_buffer || size_out == -1 || size_out == size_buffer-1)
			size_buffer *= 2;
		else if (size_out > size_buffer)
			size_buffer = size_out + 2; // to avoid the ambiguous case
		else
			break;

		delete[] buffer;
	}
	message = std::string(buffer);
	delete[] buffer;
}

Exception::~Exception() throw()
{
}

}
