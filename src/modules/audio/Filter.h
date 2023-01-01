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

#ifndef LOVE_AUDIO_FILTERS_H
#define LOVE_AUDIO_FILTERS_H

#include "common/Object.h"
#include "common/StringMap.h"
#include <map>

template<typename T>
class LazierAndSlowerButEasilyArrayableStringMap2
{
public:
	struct Entry
	{
		const char *key;
		T value;
	};
	LazierAndSlowerButEasilyArrayableStringMap2()
	{
	}

	LazierAndSlowerButEasilyArrayableStringMap2(const std::vector<Entry> &entries)
	{
		for (auto entry : entries)
		{
			forward[entry.key] = entry.value;
			reverse[entry.value] = entry.key;
		}
	}

	bool find(const char *key, T &t)
	{
		if (forward.find(key) == forward.end())
			return false;
		t = forward[key];
		return true;
	}

	bool find(T key, const char *&str)
	{
		if (reverse.find(key) == reverse.end())
			return false;
		str = reverse[key];
		return true;
	}

private:
	std::map<std::string, T> forward;
	std::map<T, const char*> reverse;
};

namespace love
{
namespace audio
{

class Filter
{
public:
	enum Type
	{
		TYPE_BASIC,
		TYPE_LOWPASS,
		TYPE_HIGHPASS,
		TYPE_BANDPASS,
		TYPE_MAX_ENUM
	};

	enum Parameter
	{
		FILTER_TYPE,
		FILTER_VOLUME,

		FILTER_LOWGAIN,
		FILTER_HIGHGAIN,

		FILTER_MAX_ENUM
	};

	enum ParameterType
	{
		PARAM_TYPE,
		PARAM_FLOAT,
		PARAM_MAX_ENUM
	};

	Filter();
	virtual ~Filter();
	Type getType() const;

	static bool getConstant(const char *in, Type &out);
	static bool getConstant(Type in, const char *&out);
	static std::vector<std::string> getConstants(Type);
	static bool getConstant(const char *in, Parameter &out, Type t);
	static bool getConstant(Parameter in, const char *&out, Type t);
	static ParameterType getParameterType(Parameter in);

protected:
	Type type;

private:
	static StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<Type, TYPE_MAX_ENUM> types;
#define StringMap LazierAndSlowerButEasilyArrayableStringMap2
	static std::vector<StringMap<Filter::Parameter>::Entry> basicParameters;
	static std::vector<StringMap<Filter::Parameter>::Entry> lowpassParameters;
	static std::vector<StringMap<Filter::Parameter>::Entry> highpassParameters;
	static std::vector<StringMap<Filter::Parameter>::Entry> bandpassParameters;
	static std::map<Type, StringMap<Parameter>> parameterNames;
#undef StringMap
	static std::map<Parameter, ParameterType> parameterTypes;
};

} //audio
} //love

#endif //LOVE_AUDIO_FILTERS_H
