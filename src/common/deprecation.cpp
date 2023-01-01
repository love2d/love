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
#include "deprecation.h"
#include "thread/threads.h"

#include <atomic>
#include <map>

namespace love
{

static std::map<std::string, DeprecationInfo> *deprecated = nullptr;
static std::vector<const DeprecationInfo *> *deprecatedList = nullptr;

static std::atomic<int> initCount;

static thread::Mutex *mutex = nullptr;
static bool outputEnabled = false;

void initDeprecation()
{
	if (initCount.fetch_add(1) == 0)
	{
		mutex = thread::newMutex();

		// These are heap-allocated because we want to clear them on deinit,
		// and deinit may be called when the program is shutting down in the
		// middle of static variable cleanup (eg in the Math module destructor).
		// Calling std::map::clear() in that case was causing segfaults.
		deprecated = new std::map<std::string, DeprecationInfo>();
		deprecatedList = new std::vector<const DeprecationInfo *>();
	}
}

void deinitDeprecation()
{
	if (initCount.fetch_sub(1) == 1)
	{
		delete deprecated;
		delete deprecatedList;
		delete mutex;

		deprecated = nullptr;
		deprecatedList = nullptr;
		mutex = nullptr;
	}
}

static void printDeprecationNotice(const DeprecationInfo &info)
{
	std::string notice = getDeprecationNotice(info, true);
	printf("LOVE - Warning: %s\n", notice.c_str());
}

void setDeprecationOutputEnabled(bool enable)
{
	if (enable == outputEnabled)
		return;

	outputEnabled = enable;

	if (enable)
	{
		GetDeprecated deprecated;

		for (const DeprecationInfo *info : deprecated.all)
		{
			if (info->uses == 1)
				printDeprecationNotice(*info);
		}
	}
}

bool isDeprecationOutputEnabled()
{
	return outputEnabled;
}

std::string getDeprecationNotice(const DeprecationInfo &info, bool usewhere)
{
	std::string notice;

	if (usewhere)
		notice += info.where;

	notice += "Using deprecated ";

	if (info.apiType == API_FUNCTION)
		notice += "function ";
	else if (info.apiType == API_METHOD)
		notice += "method ";
	else if (info.apiType == API_FIELD)
		notice += "field ";
	else if (info.apiType == API_CONSTANT)
		notice += "constant ";
	else
		notice += "API ";

	notice += info.name;

	if (info.type == DEPRECATED_REPLACED && !info.replacement.empty())
		notice += " (replaced by " + info.replacement + ")";
	else if (info.type == DEPRECATED_RENAMED && !info.replacement.empty())
		notice += " (renamed to " + info.replacement + ")";

	return notice;
}

GetDeprecated::GetDeprecated()
	: all(*deprecatedList)
{
	if (mutex != nullptr)
		mutex->lock();
}

GetDeprecated::~GetDeprecated()
{
	if (mutex != nullptr)
		mutex->unlock();
}

MarkDeprecated::MarkDeprecated(const char *name, APIType api)
	: MarkDeprecated(name, api, DEPRECATED_NO_REPLACEMENT, nullptr)
{
}

MarkDeprecated::MarkDeprecated(const char *name, APIType api, DeprecationType type, const char *replacement)
	: info(nullptr)
{
	if (mutex != nullptr)
		mutex->lock();

	auto it = deprecated->find(name);

	if (it != deprecated->end())
	{
		it->second.uses++;
		info = &it->second;
	}
	else
	{
		DeprecationInfo newinfo = {};

		newinfo.type = type;
		newinfo.apiType = api;
		newinfo.uses = 1;
		newinfo.name = name;

		if (replacement != nullptr)
			newinfo.replacement = replacement;

		auto inserted = deprecated->insert(std::make_pair(newinfo.name, newinfo));

		info = &inserted.first->second;
		deprecatedList->push_back(info);
	}
}

MarkDeprecated::~MarkDeprecated()
{
	if (outputEnabled && info != nullptr && info->uses == 1)
		printDeprecationNotice(*info);

	if (mutex != nullptr)
		mutex->unlock();
}

} // love
