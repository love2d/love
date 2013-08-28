/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "JoystickModule.h"
#include "Joystick.h"

// SDL
#include <SDL.h>

// C++
#include <sstream>
#include <algorithm>

// C
#include <cstdlib>

namespace love
{
namespace joystick
{
namespace sdl
{

JoystickModule::JoystickModule()
{
	// Init the SDL Joystick and Game Controller systems.
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
		throw love::Exception("%s", SDL_GetError());

	// Start joystick event watching. Joysticks are automatically added and
	// removed via love.event.
	SDL_JoystickEventState(SDL_ENABLE);
	SDL_GameControllerEventState(SDL_ENABLE);
}

JoystickModule::~JoystickModule()
{
	// Close any open Joysticks.
	for (size_t i = 0; i < joysticks.size(); i++)
	{
		joysticks[i]->close();
		joysticks[i]->release();
	}

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
}

const char *JoystickModule::getName() const
{
	return "love.joystick.sdl";
}

love::joystick::Joystick *JoystickModule::getJoystick(int joyindex)
{
	if (joyindex < 0 || (size_t) joyindex >= activeSticks.size())
		return 0;

	return activeSticks[joyindex];
}

int JoystickModule::getIndex(const love::joystick::Joystick *joystick)
{
	for (size_t i = 0; i < activeSticks.size(); i++)
	{
		if (activeSticks[i] == joystick)
			return i;
	}

	// Joystick is not connected.
	return -1;
}

int JoystickModule::getJoystickCount() const
{
	return (int) activeSticks.size();
}

love::joystick::Joystick *JoystickModule::getJoystickFromID(int instanceid)
{
	for (size_t i = 0; i < activeSticks.size(); i++)
	{
		if (instanceid == activeSticks[i]->getInstanceID())
			return activeSticks[i];
	}

	return 0;
}

love::joystick::Joystick *JoystickModule::addJoystick(int deviceindex)
{
	if (deviceindex < 0 || deviceindex >= SDL_NumJoysticks())
		return 0;

	std::string guidstr = getDeviceProductGUID(deviceindex);

	joystick::Joystick *joystick = 0;

	for (size_t i = 0; i < joysticks.size(); i++)
	{
		// Try to re-use a disconnected Joystick with the same GUID.
		if (!joysticks[i]->isConnected() && joysticks[i]->getProductGUID() == guidstr)
		{
			joystick = joysticks[i];
			joystick->open(deviceindex);
			break;
		}
	}

	if (!joystick)
	{
		// Make a new Joystick and add it to the persistent list, if we can't
		// re-use one.
		joystick = new Joystick(joysticks.size(), deviceindex);
		joysticks.push_back(joystick);
	}

	// Add the Joystick to the connected list, if it's not there already.
	if (std::find(activeSticks.begin(), activeSticks.end(), joystick) == activeSticks.end())
		activeSticks.push_back(joystick);

	return joystick;
}

void JoystickModule::removeJoystick(love::joystick::Joystick *joystick)
{
	if (!joystick)
		return;

	// Close the Joystick and remove it from the active joystick list.
	std::vector<joystick::Joystick *>::iterator it = std::find(activeSticks.begin(), activeSticks.end(), joystick);
	if (it != activeSticks.end())
	{
		(*it)->close();
		activeSticks.erase(it);
	}
}

bool JoystickModule::setGamepadMapping(const std::string &guid, Joystick::GamepadInput gpinput, Joystick::JoystickInput joyinput)
{
	// All SDL joystick GUID strings are 32 characters.
	if (guid.length() != 32)
		throw love::Exception("Invalid joystick GUID: %s", guid.c_str());

	SDL_JoystickGUID sdlguid = SDL_JoystickGetGUIDFromString(guid.c_str());
	std::string mapstr;

	char *sdlmapstr = SDL_GameControllerMappingForGUID(sdlguid);
	if (sdlmapstr)
	{
		mapstr = sdlmapstr;
		SDL_free(sdlmapstr);
	}
	else
	{
		// Use a generic name if we have to create a new mapping string.
		mapstr = guid + ",Controller,";
	}

	std::stringstream joyinputstream;
	Uint8 sdlhat;

	// We can't have negative int values in the bind string.
	switch (joyinput.type)
	{
	case Joystick::INPUT_TYPE_AXIS:
		if (joyinput.axis >= 0)
			joyinputstream << "a" << joyinput.axis;
		break;
	case Joystick::INPUT_TYPE_BUTTON:
		if (joyinput.button >= 0)
			joyinputstream << "b" << joyinput.button;
		break;
	case Joystick::INPUT_TYPE_HAT:
		if (joyinput.hat.value >= 0 && Joystick::getConstant(joyinput.hat.value, sdlhat))
			joyinputstream << "h" << joyinput.hat.value << "." << int(sdlhat);
		break;
	default:
		break;
	}

	std::string joyinputstr = joyinputstream.str();

	if (joyinputstr.length() == 0)
		throw love::Exception("Invalid joystick input value.");

	// SDL's name for the gamepad input value, e.g. "guide".
	std::string gpinputname = stringFromGamepadInput(gpinput);

	// We should remove any existing joystick bind for this gamepad buttton/axis
	// so SDL's parser doesn't get mixed up.
	removeBindFromMapString(mapstr, joyinputstr);

	// The string we'll be adding to the mapping string, e.g. "guide:b10,"
	std::string insertstr = gpinputname + ":" + joyinputstr + ",";

	// We should replace any existing gamepad bind.
	size_t findpos = mapstr.find(gpinputname + ":");
	if (findpos != std::string::npos)
	{
		// The bind string ends at the next comma, or the end of the string.
		size_t endpos = mapstr.find_first_of(',', findpos);
		if (endpos == std::string::npos)
			endpos = mapstr.length() - 1;

		mapstr.replace(findpos, endpos - findpos + 1, insertstr);
	}
	else
	{
		// Just append to the end if we don't need to replace anything.
		mapstr += insertstr;
	}

	// 1 == added, 0 == updated, -1 == error.
	int status = SDL_GameControllerAddMapping(mapstr.c_str());

	// FIXME: massive hack until missing APIs are added to SDL 2:
	// https://bugzilla.libsdl.org/show_bug.cgi?id=1975
	if (status == 1)
		checkGamepads(guid);

	return  status >= 0;
}

Joystick::JoystickInput JoystickModule::getGamepadMapping(const std::string &guid, Joystick::GamepadInput gpinput)
{
	// All SDL joystick GUID strings are 32 characters.
	if (guid.length() != 32)
		throw love::Exception("Invalid joystick GUID: %s", guid.c_str());

	Joystick::JoystickInput jinput;
	jinput.type = Joystick::INPUT_TYPE_MAX_ENUM;

	SDL_JoystickGUID sdlguid = SDL_JoystickGetGUIDFromString(guid.c_str());

	std::string mapstr;

	char *sdlmapstr = SDL_GameControllerMappingForGUID(sdlguid);
	if (!sdlmapstr)
		return jinput;

	mapstr = sdlmapstr;
	SDL_free(sdlmapstr);

	std::string gpbindname = stringFromGamepadInput(gpinput);

	size_t findpos = mapstr.find(std::string(",") + gpbindname + ":");
	if (findpos == std::string::npos)
		return jinput;

	size_t endpos = mapstr.find_first_of(',', findpos);
	if (endpos == std::string::npos)
	{
		// Assume end-of-string if we can't find the next comma.
		endpos = mapstr.length() - 1;
	}

	if (endpos >= mapstr.length())
		return jinput; // Something went wrong.

	// Strip out the trailing comma from our search position, if it exists.
	if (mapstr[endpos] == ',')
		endpos--;

	// New start position: comma + gamepadinputlength + ":".
	findpos += 1 + gpbindname.length() + 1;
	std::string jbindstr = mapstr.substr(findpos, endpos - findpos + 1);

	jinput = JoystickInputFromString(jbindstr);
	return jinput;
}

std::string JoystickModule::stringFromGamepadInput(Joystick::GamepadInput gpinput) const
{
	SDL_GameControllerAxis sdlaxis;
	SDL_GameControllerButton sdlbutton;

	const char *gpinputname = 0;

	switch (gpinput.type)
	{
	case Joystick::INPUT_TYPE_AXIS:
		if (Joystick::getConstant(gpinput.axis, sdlaxis))
			gpinputname = SDL_GameControllerGetStringForAxis(sdlaxis);
		break;
	case Joystick::INPUT_TYPE_BUTTON:
		if (Joystick::getConstant(gpinput.button, sdlbutton))
			gpinputname = SDL_GameControllerGetStringForButton(sdlbutton);
		break;
	default:
		break;
	}

	if (!gpinputname)
		throw love::Exception("Invalid gamepad axis/button.");

	return std::string(gpinputname);
}

Joystick::JoystickInput JoystickModule::JoystickInputFromString(const std::string &str) const
{
	Joystick::JoystickInput jinput;
	jinput.type = Joystick::INPUT_TYPE_MAX_ENUM;

	// Return an invalid value rather than throwing an exception.
	if (str.length() < 2)
		return jinput;

	// The input type will always be the first character in the string.
	char inputtype = str[0];
	std::string bindvalues = str.substr(1);

	Uint8 sdlhat;
	switch (inputtype)
	{
	case 'a':
		jinput.type = Joystick::INPUT_TYPE_AXIS;
		jinput.axis = atoi(bindvalues.c_str());
		break;
	case 'b':
		jinput.type = Joystick::INPUT_TYPE_BUTTON;
		jinput.button = atoi(bindvalues.c_str());
		break;
	case 'h':
		// Hat string syntax is "index.value".
		if (bindvalues.length() < 3)
			break;
		jinput.type = Joystick::INPUT_TYPE_HAT;
		jinput.hat.index = atoi(bindvalues.substr(0, 1).c_str());
		sdlhat = (Uint8) atoi(bindvalues.substr(2, 1).c_str());
		if (!Joystick::getConstant(sdlhat, jinput.hat.value))
		{
			// Return an invalid value if we can't find the hat constant.
			jinput.type = Joystick::INPUT_TYPE_MAX_ENUM;
			return jinput;
		}
		break;
	default:
		break;
	}

	return jinput;
}

void JoystickModule::removeBindFromMapString(std::string &mapstr, const std::string &joybindstr) const
{
	// Find the joystick part of the bind in the string.
	size_t joybindpos = mapstr.find(joybindstr + ",");
	if (joybindpos == std::string::npos)
	{
		joybindpos = mapstr.rfind(joybindstr);
		if (joybindpos != mapstr.length() - joybindstr.length())
			return;
	}

	if (joybindpos == std::string::npos)
		return;

	// Find the start of the entire bind.
	size_t bindstart = mapstr.rfind(',', joybindpos);
	if (bindstart != std::string::npos && bindstart < mapstr.length() - 1)
	{
		size_t bindend = mapstr.find(',', bindstart + 1);
		if (bindend == std::string::npos)
			bindend = mapstr.length() - 1;

		// Replace it with an empty string (remove it.)
		mapstr.replace(bindstart, bindend - bindstart + 1, "");
	}
}

void JoystickModule::checkGamepads(const std::string &guid) const
{
	// FIXME: massive hack until missing APIs are added to SDL 2:
	// https://bugzilla.libsdl.org/show_bug.cgi?id=1975

	// Make sure all connected joysticks of a certain guid that are
	// gamepad-capable are opened as such.
	for (int d_index = 0; d_index < SDL_NumJoysticks(); d_index++)
	{
		if (!SDL_IsGameController(d_index))
			continue;

		if (guid.compare(getDeviceProductGUID(d_index)) != 0)
			continue;

		std::vector<love::joystick::Joystick *>::const_iterator it;
		for (it = activeSticks.begin(); it != activeSticks.end(); ++it)
		{
			if ((*it)->isGamepad() || guid.compare((*it)->getProductGUID()) != 0)
				continue;

			// Big hack time: open the index as a game controller and compare
			// the underlying joystick handle to the active stick's.
			SDL_GameController *ctrl = SDL_GameControllerOpen(d_index);
			if (ctrl == NULL)
				continue;

			SDL_Joystick *stick = SDL_GameControllerGetJoystick(ctrl);
			if (stick == (SDL_Joystick *) (*it)->getHandle())
				(*it)->openGamepad(d_index);

			SDL_GameControllerClose(ctrl);
		}
	}
}

std::string JoystickModule::getDeviceProductGUID(int deviceindex) const
{
	if (deviceindex < 0 || deviceindex >= SDL_NumJoysticks())
		return std::string("");

	// SDL_JoystickGetGUIDString uses 32 bytes plus the null terminator.
	char guidstr[33] = {'\0'};

	// SDL2's GUIDs identify *classes* of devices, instead of unique devices.
	SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(deviceindex);
	SDL_JoystickGetGUIDString(guid, guidstr, sizeof(guidstr));

	return std::string(guidstr);
}

} // sdl
} // joystick
} // love
