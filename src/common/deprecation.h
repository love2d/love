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

#pragma once

#include "int.h"

#include <string>
#include <vector>

namespace love
{

enum APIType
{
	API_FUNCTION,
	API_METHOD,
	API_FIELD,
	API_CONSTANT,
};

enum DeprecationType
{
	DEPRECATED_NO_REPLACEMENT,
	DEPRECATED_REPLACED,
	DEPRECATED_RENAMED,
};

struct DeprecationInfo
{
	DeprecationType type;
	APIType apiType;
	int64 uses;
	std::string name;
	std::string replacement;
	std::string where;
};

void initDeprecation();
void deinitDeprecation();

void setDeprecationOutputEnabled(bool enable);
bool isDeprecationOutputEnabled();

std::string getDeprecationNotice(const DeprecationInfo &info, bool usewhere);

struct GetDeprecated
{
	GetDeprecated();
	~GetDeprecated();

	const std::vector<const DeprecationInfo *> &all;
};

struct MarkDeprecated
{
	MarkDeprecated(const char *name, APIType api);
	MarkDeprecated(const char *name, APIType api, DeprecationType type, const char *replacement);
	~MarkDeprecated();

	DeprecationInfo *info;
};

} // love
