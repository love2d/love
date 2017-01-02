/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

#ifndef LOVE_AUDIO_FILTERS_H
#define LOVE_AUDIO_FILTERS_H

#include "common/Object.h"
#include "common/StringMap.h"
#include <map>

namespace love
{
namespace audio
{

class Filter
{
public:
	enum Type
	{
		TYPE_LOWPASS,
		TYPE_HIGHPASS,
		TYPE_BANDPASS,
		TYPE_MAX_ENUM
	};

	Filter();
	virtual ~Filter();
	Type getType() const;

	static bool getConstant(const char *in, Type &out);
	static bool getConstant(Type in, const char *&out);
	static int getParameterCount(Type in);
	static int getParameterCount();

protected:
	Type type;

private:
	static StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<Type, TYPE_MAX_ENUM> types;
	static std::map<Type, int> parameterCount;
};

} //audio
} //love

#endif //LOVE_AUDIO_FILTERS_H
