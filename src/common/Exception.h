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

#ifndef LOVE_EXCEPTION_H
#define LOVE_EXCEPTION_H

#include <exception>
#include <cstdarg> // vararg
#include <cstdio> // vsnprintf
#include <cstring> // strncpy
#include <string>

namespace love
{

/**
 * A convenient vararg-enabled exception class.
 **/
class Exception : public std::exception
{
public:

	/**
	 * Creates a new Exception according to printf-rules.
	 *
	 * See: http://www.cplusplus.com/reference/clibrary/cstdio/printf/
	 *
	 * @param fmt The format string (see printf).
	 **/
	Exception(const char *fmt, ...);
	virtual ~Exception() throw();

	/**
	 * Returns a string containing reason for the exception.
	 * @return A description of the exception.
	 **/
	inline virtual const char *what() const throw()
	{
		return message.c_str();
	}

private:

	std::string message;

}; // Exception

} // love

#endif // LOVE_EXCEPTION_H
