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

#ifndef LOVE_SYSTEM_SDL_SYSTEM_H
#define LOVE_SYSTEM_SDL_SYSTEM_H

// LOVE
#include "system/System.h"
#include "common/EnumMap.h"

// SDL
#include <SDL_power.h>

namespace love
{
namespace system
{
namespace sdl
{

class System : public love::system::System
{
public:

	System();
	virtual ~System() {}

	// Implements Module.
	const char *getName() const;

	int getProcessorCount() const;

	void setClipboardText(const std::string &text) const;
	std::string getClipboardText() const;

	PowerState getPowerInfo(int &seconds, int &percent) const;

private:

	static EnumMap<PowerState, SDL_PowerState, POWER_MAX_ENUM>::Entry powerEntries[];
	static EnumMap<PowerState, SDL_PowerState, POWER_MAX_ENUM> powerStates;

}; // System

} // sdl
} // system
} // love

#endif // LOVE_SYSTEM_SDL_SYSTEM_H
