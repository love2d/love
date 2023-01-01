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

#ifndef LOVE_JOYSTICK_SDL_JOYSTICK_MODULE_H
#define LOVE_JOYSTICK_SDL_JOYSTICK_MODULE_H

// LOVE
#include "joystick/JoystickModule.h"

// C++
#include <string>
#include <vector>
#include <list>
#include <map>

namespace love
{
namespace joystick
{
namespace sdl
{

class JoystickModule : public love::joystick::JoystickModule
{
public:

	JoystickModule();
	virtual ~JoystickModule();

	// Implements Module.
	const char *getName() const override;

	// Implements JoystickModule.
	love::joystick::Joystick *addJoystick(int deviceindex) override;
	void removeJoystick(love::joystick::Joystick *joystick) override;
	love::joystick::Joystick *getJoystickFromID(int instanceid) override;
	love::joystick::Joystick *getJoystick(int joyindex) override;
	int getIndex(const love::joystick::Joystick *joystick) override;
	int getJoystickCount() const override;

	bool setGamepadMapping(const std::string &guid, Joystick::GamepadInput gpinput, Joystick::JoystickInput joyinput) override;
	void loadGamepadMappings(const std::string &mappings) override;

	std::string getGamepadMappingString(const std::string &guid) const override;
	std::string saveGamepadMappings() override;

private:

	std::string stringFromGamepadInput(Joystick::GamepadInput gpinput) const;
	void removeBindFromMapString(std::string &mapstr, const std::string &joybindstr) const;

	void checkGamepads(const std::string &guid) const;

	// SDL2's GUIDs identify *classes* of devices, instead of unique devices.
	std::string getDeviceGUID(int deviceindex) const;

	// Lists of currently connected Joysticks.
	std::vector<Joystick *> activeSticks;

	// Persistent list of all Joysticks which have been connected at some point.
	std::list<Joystick *> joysticks;

	// Persistent map indicating GUIDs for Gamepads which have been connected or
	// modified at some point.
	std::map<std::string, bool> recentGamepadGUIDs;

}; // JoystickModule

} // sdl
} // joystick
} // love

#endif // LOVE_JOYSTICK_SDL_JOYSTICK_MODULE_H
