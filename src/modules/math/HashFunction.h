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

#ifndef LOVE_MATH_HASH_FUNCTION_H
#define LOVE_MATH_HASH_FUNCTION_H

// LOVE
#include "common/Data.h"
#include "common/StringMap.h"

// STL
#include <cstdint>

namespace love
{
namespace math
{

class HashFunction
{
public:
	enum Function
	{
		FUNCTION_MD5,
		FUNCTION_SHA1,
		FUNCTION_SHA224,
		FUNCTION_SHA256,
		FUNCTION_SHA384,
		FUNCTION_SHA512,
		FUNCTION_MAX_ENUM
	};

	/**
	 * Get a HashFunction instance for the given function.
	 *
	 * @param[in] function The selected hash function.
	 * @return An instance of HashFunction for the given function, or NULL if
	 *         not available.
	 **/
	static HashFunction *getHashFunction(Function function);

	virtual ~HashFunction() {}

	/**
	 * Hash the input, producing an set of bytes as output.
	 *
	 * @param[in] function The selected hash function.
	 * @param[in] input The input data to hash.
	 * @param[in] length The length of the input data.
	 * @return A string of bytes, representing the result of the hash function.
	 **/
	virtual std::string hash(Function function, const char *input, uint64_t length) const = 0;

	/**
	 * @param[in] function The requested hash function.
	 * @return Whether this HashFunction instance implements the given function.
	 **/
	virtual bool isSupported(Function function) const = 0;

	static bool getConstant(const char *in, Function &out);
	static bool getConstant(const Function &in, const char *&out);

protected:
	HashFunction() {}

private:
	static StringMap<Function, FUNCTION_MAX_ENUM>::Entry functionEntries[];
	static StringMap<Function, FUNCTION_MAX_ENUM> functionNames;
}; // HashFunction

} // math
} // love

#endif // LOVE_MATH_HASH_FUNCTION_H
