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

#ifndef LOVE_JOYSTICK_JOYSTICK_MODULE_H
#define LOVE_JOYSTICK_JOYSTICK_MODULE_H

// LOVE
#include "common/Module.h"
#include "Joystick.h"

namespace love
{
namespace joystick
{

class JoystickModule : public Module
{
public:

	virtual ~JoystickModule() {}

	// Implements Module.
	ModuleType getModuleType() const override { return M_JOYSTICK; }

	/**
	 * Adds a connected Joystick device and opens it for use.
	 * Returns NULL if the Joystick could not be added.
	 **/
	virtual Joystick *addJoystick(int deviceindex) = 0;

	/**
	 * Removes a disconnected Joystick device.
	 **/
	virtual void removeJoystick(Joystick *joystick) = 0;

	/**
	 * Gets a connected Joystick from its unique Instance ID.
	 * Returns NULL if the instance ID does not correspond to a connected stick.
	 **/
	virtual Joystick *getJoystickFromID(int instanceid) = 0;

	/**
	 * Gets a connected Joystick.
	 * Returns NULL if joyindex is not in the range of [0, getJoystickCount()).
	 **/
	virtual Joystick *getJoystick(int joyindex) = 0;

	/**
	 * Gets the index of a connected joystick.
	 * Returns -1 if the joystick is not connected.
	 **/
	virtual int getIndex(const Joystick *joystick) = 0;

	/**
	 * Gets the number of currently connected Joysticks.
	 **/
	virtual int getJoystickCount() const = 0;

	/**
	 * Sets the virtual Gamepad mapping for a joystick input value for all
	 * joystick devices with the specified joystick product GUID.
	 * If any joysticks with the specified GUID are connected, they will be
	 * added as Gamepads if they aren't already, otherwise their Gamepad mapping
	 * will be updated.
	 **/
	virtual bool setGamepadMapping(const std::string &pguid, Joystick::GamepadInput gpinput, Joystick::JoystickInput joyinput) = 0;

	/**
	 * Loads a newline-separated list of virtual Gamepad mapping strings for
	 * multiple joysticks at a time. The mapping strings must have been
	 * generated with saveGamepadMappings, via Steam, or some other tool which
	 * generates SDL GameController mappings.
	 **/
	virtual void loadGamepadMappings(const std::string &mappings) = 0;

	/**
	 * Gets a newline-separated list of virtual Gamepad mapping strings for
	 * all used or modified Joysticks which are identified as Gamepads.
	 **/
	virtual std::string saveGamepadMappings() = 0;

	/**
	 * Gets the gamepad mapping string for the given GUID.
	 **/
	virtual std::string getGamepadMappingString(const std::string &guid) const = 0;

}; // JoystickModule

} // joystick
} // love

#endif // LOVE_JOYSTICK_JOYSTICK_MODULE_H
