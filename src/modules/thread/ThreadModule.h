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

#ifndef LOVE_THREAD_THREADMODULE_H
#define LOVE_THREAD_THREADMODULE_H

// STL
#include <string>
#include <map>

// LOVE
#include "common/Data.h"
#include "common/Module.h"
#include "common/StringMap.h"

#include "Thread.h"
#include "Channel.h"
#include "LuaThread.h"
#include "threads.h"

namespace love
{
namespace thread
{
class ThreadModule : public love::Module
{
public:

	virtual ~ThreadModule() {}
	virtual LuaThread *newThread(const std::string &name, love::Data *data);
	virtual Channel *newChannel();
	virtual Channel *getChannel(const std::string &name);

	// Implements Module.
	virtual const char *getName() const;
	virtual ModuleType getModuleType() const { return M_THREAD; }

private:

	std::map<std::string, StrongRef<Channel>> namedChannels;
	MutexRef namedChannelMutex;

}; // ThreadModule

} // thread
} // love

#endif // LOVE_THREAD_THREADMODULE_H
