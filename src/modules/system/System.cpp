/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "System.h"

namespace love
{
namespace system
{

std::string System::getOS() const
{
#ifdef LOVE_MACOSX
	return "OS X";
#elif LOVE_WINDOWS
	return "Windows";
#elif LOVE_LINUX
	return "Linux";
#else
	return "Unknown";
#endif
}

bool System::getConstant(const char *in, System::PowerState &out)
{
	return powerStates.find(in, out);
}

bool System::getConstant(System::PowerState in, const char *&out)
{
	return powerStates.find(in, out);
}

StringMap<System::PowerState, System::POWER_MAX_ENUM>::Entry System::powerEntries[] =
{
	{"unknown", System::POWER_UNKNOWN},
	{"battery", System::POWER_BATTERY},
	{"nobattery", System::POWER_NO_BATTERY},
	{"charging", System::POWER_CHARGING},
	{"charged", System::POWER_CHARGED},
};

StringMap<System::PowerState, System::POWER_MAX_ENUM> System::powerStates(System::powerEntries, sizeof(System::powerEntries));

} // system
} // love
