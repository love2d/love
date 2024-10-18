/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include <SDL3/SDL.h>

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
	: love::joystick::JoystickModule("love.joystick.sdl")
{
	if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		throw love::Exception("Could not initialize SDL joystick subsystem (%s)", SDL_GetError());

	// Initialize any joysticks which are already connected.
	int count = 0;
	SDL_JoystickID *sticks = SDL_GetJoysticks(&count);
	for (int i = 0; i < count; i++)
		addJoystick((int64) sticks[i]);
	SDL_free(sticks);

	// Start joystick event watching. Joysticks are automatically added and
	// removed via love.event.
	SDL_SetJoystickEventsEnabled(true);
	SDL_SetGamepadEventsEnabled(true);
}

JoystickModule::~JoystickModule()
{
	// Close any open Joysticks.
	for (auto stick : joysticks)
	{
		stick->close();
		stick->release();
	}

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD);
}

love::joystick::Joystick *JoystickModule::getJoystick(int joyindex)
{
	if (joyindex < 0 || (size_t) joyindex >= activeSticks.size())
		return nullptr;

	return activeSticks[joyindex];
}

int JoystickModule::getIndex(const love::joystick::Joystick *joystick)
{
	for (int i = 0; i < (int) activeSticks.size(); i++)
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
	for (auto stick : activeSticks)
	{
		if (stick->getInstanceID() == instanceid)
			return stick;
	}

	return nullptr;
}

love::joystick::Joystick *JoystickModule::addJoystick(int64 deviceid)
{
	if (deviceid == 0)
		return nullptr;

	std::string guidstr = getDeviceGUID(deviceid);
	joystick::Joystick *joystick = 0;
	bool reused = false;

	for (auto stick : joysticks)
	{
		// Try to re-use a disconnected Joystick with the same GUID.
		if (!stick->isConnected() && stick->getGUID() == guidstr)
		{
			joystick = stick;
			reused = true;
			break;
		}
	}

	if (!joystick)
	{
		joystick = new Joystick((int) joysticks.size());
		joysticks.push_back(joystick);
	}

	// Make sure the Joystick object isn't in the active list already.
	removeJoystick(joystick);

	if (!joystick->open(deviceid))
		return nullptr;

	// Make sure multiple instances of the same physical joystick aren't added
	// to the active list.
	for (auto activestick : activeSticks)
	{
		if (joystick->getHandle() == activestick->getHandle())
		{
			joystick->close();

			// If we just created the stick, remove it since it's a duplicate.
			if (!reused)
			{
				joysticks.remove(joystick);
				joystick->release();
			}

			return activestick;
		}
	}

	if (joystick->isGamepad())
		recentGamepadGUIDs[joystick->getGUID()] = true;

	activeSticks.push_back(joystick);
	return joystick;
}

void JoystickModule::removeJoystick(love::joystick::Joystick *joystick)
{
	if (!joystick)
		return;

	// Close the Joystick and remove it from the active joystick list.
	auto it = std::find(activeSticks.begin(), activeSticks.end(), joystick);
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

	SDL_GUID sdlguid = SDL_StringToGUID(guid.c_str());
	std::string mapstr;

	char *sdlmapstr = SDL_GetGamepadMappingForGUID(sdlguid);
	if (sdlmapstr)
	{
		mapstr = sdlmapstr;
		SDL_free(sdlmapstr);
	}
	else
	{
		std::string name = "Controller";

		for (love::joystick::Joystick *stick : joysticks)
		{
			if (stick->getGUID() == guid)
			{
				name = stick->getName();
				break;
			}
		}

		mapstr = guid + "," + name + ",";
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
		if (joyinput.hat.index >= 0 && Joystick::getConstant(joyinput.hat.value, sdlhat))
			joyinputstream << "h" << joyinput.hat.index << "." << int(sdlhat);
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
	size_t findpos = mapstr.find("," + gpinputname + ":");
	if (findpos != std::string::npos)
	{
		// The bind string ends at the next comma, or the end of the string.
		size_t endpos = mapstr.find_first_of(',', findpos + 1);
		if (endpos == std::string::npos)
			endpos = mapstr.length() - 1;

		mapstr.replace(findpos + 1, endpos - findpos, insertstr);
	}
	else
	{
		// Just append to the end (or before the platform section if that exists),
		// if we don't need to replace anything.
		size_t platformpos = mapstr.find("platform:");
		if (platformpos != std::string::npos)
			mapstr.insert(platformpos, insertstr);
		else
			mapstr += insertstr;
	}

	// 1 == added, 0 == updated, -1 == error.
	int status = SDL_AddGamepadMapping(mapstr.c_str());

	if (status != -1)
		recentGamepadGUIDs[guid] = true;

	// FIXME: massive hack until missing APIs are added to SDL 2:
	// https://bugzilla.libsdl.org/show_bug.cgi?id=1975
	if (status == 1)
		checkGamepads(guid);

	return status >= 0;
}

std::string JoystickModule::stringFromGamepadInput(Joystick::GamepadInput gpinput) const
{
	SDL_GamepadAxis sdlaxis;
	SDL_GamepadButton sdlbutton;

	const char *gpinputname = nullptr;

	switch (gpinput.type)
	{
	case Joystick::INPUT_TYPE_AXIS:
		if (Joystick::getConstant(gpinput.axis, sdlaxis))
			gpinputname = SDL_GetGamepadStringForAxis(sdlaxis);
		break;
	case Joystick::INPUT_TYPE_BUTTON:
		if (Joystick::getConstant(gpinput.button, sdlbutton))
			gpinputname = SDL_GetGamepadStringForButton(sdlbutton);
		break;
	default:
		break;
	}

	if (!gpinputname)
		throw love::Exception("Invalid gamepad axis/button.");

	return std::string(gpinputname);
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

	// Find the start of the entire bind by looking for the separator between
	// the end of one section of the map string and the start of this section.
	size_t bindstart = mapstr.rfind(',', joybindpos);
	if (bindstart != std::string::npos && bindstart < mapstr.length() - 1)
	{
		// The start of the bind is directly after the separator.
		bindstart++;

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
	int count = 0;
	SDL_JoystickID *sdlsticks = SDL_GetJoysticks(&count);
	for (int d_index = 0; d_index < count; d_index++)
	{
		if (!SDL_IsGamepad(sdlsticks[d_index]))
			continue;

		auto sdlid = sdlsticks[d_index];

		if (guid.compare(getDeviceGUID(sdlid)) != 0)
			continue;

		for (auto stick : activeSticks)
		{
			if (guid.compare(stick->getGUID()) != 0)
				continue;

			// Big hack time: open the index as a game controller and compare
			// the underlying joystick handle to the active stick's.
			SDL_Gamepad *controller = SDL_OpenGamepad(sdlid);
			if (controller == nullptr)
				continue;

			// GameController objects are reference-counted in SDL, so we don't want to
			// have a joystick open when trying to re-initialize it
			SDL_Joystick *sdlstick = SDL_GetGamepadJoystick(controller);
			bool open_gamepad = (sdlstick == (SDL_Joystick *) stick->getHandle());
			SDL_CloseGamepad(controller);

			// open as gamepad if necessary
			if (open_gamepad)
				stick->openGamepad(sdlid);
		}
	}
}

std::string JoystickModule::getDeviceGUID(int64 deviceid) const
{
	// SDL_JoystickGetGUIDString uses 32 bytes plus the null terminator.
	char guidstr[33] = {'\0'};

	if (deviceid <= 0)
		return std::string("");

	// SDL's GUIDs identify *classes* of devices, instead of unique devices.
	SDL_GUID guid = SDL_GetJoystickGUIDForID((SDL_JoystickID)deviceid);
	SDL_GUIDToString(guid, guidstr, sizeof(guidstr));

	return std::string(guidstr);
}

void JoystickModule::loadGamepadMappings(const std::string &mappings)
{
	// TODO: We should use SDL_GameControllerAddMappingsFromRW. We're
	// duplicating its functionality for now because it was added after
	// SDL 2.0.0's release, and we want runtime compat with 2.0.0 on Linux...

	std::stringstream ss(mappings);
	std::string mapping;
	bool success = false;

	// The mappings string contains newline-separated mappings.
	while (std::getline(ss, mapping))
	{
		if (mapping.empty())
			continue;

		// Lines starting with "#" are comments.
		if (mapping[0] == '#')
			continue;

		// Strip out and compare any "platform:XYZ," in the mapping.
		size_t pstartpos = mapping.find("platform:");
		if (pstartpos != std::string::npos)
		{
			pstartpos += strlen("platform:");

			size_t pendpos = mapping.find_first_of(',', pstartpos);
			std::string platform = mapping.substr(pstartpos, pendpos - pstartpos);

			if (platform.compare(SDL_GetPlatform()) != 0)
			{
				// Ignore the mapping but still acknowledge that it is one.
				success = true;
				continue;
			}

			pstartpos -= strlen("platform:");
			mapping.erase(pstartpos, pendpos - pstartpos + 1);
		}

		if (SDL_AddGamepadMapping(mapping.c_str()) != -1)
		{
			success = true;
			std::string guid = mapping.substr(0, mapping.find_first_of(','));
			recentGamepadGUIDs[guid] = true;

			// FIXME: massive hack until missing APIs are added to SDL 2:
			// https://bugzilla.libsdl.org/show_bug.cgi?id=1975
			checkGamepads(guid);
		}
	}

	// Don't error when an empty string is given, since saveGamepadMappings can
	// produce an empty string if there are no recently seen gamepads to save.
	if (!success && !mappings.empty())
		throw love::Exception("Invalid gamepad mappings.");
}

std::string JoystickModule::getGamepadMappingString(const std::string &guid) const
{
	SDL_GUID sdlguid = SDL_StringToGUID(guid.c_str());
	char *sdlmapping = SDL_GetGamepadMappingForGUID(sdlguid);
	if (sdlmapping == nullptr)
		return "";

	std::string mapping(sdlmapping);
	SDL_free(sdlmapping);

	// Matches SDL_GameControllerAddMappingsFromRW.
	if (mapping.find_last_of(',') != mapping.length() - 1)
		mapping += ",";

	if (mapping.find("platform:") == std::string::npos)
		mapping += "platform:" + std::string(SDL_GetPlatform());

	return mapping;
}

std::string JoystickModule::saveGamepadMappings()
{
	std::string mappings;

	for (const auto &g : recentGamepadGUIDs)
	{
		SDL_GUID sdlguid = SDL_StringToGUID(g.first.c_str());
		char *sdlmapping = SDL_GetGamepadMappingForGUID(sdlguid);
		if (sdlmapping == nullptr)
			continue;

		std::string mapping = sdlmapping;
		SDL_free(sdlmapping);

		if (mapping.find_last_of(',') != mapping.length() - 1)
			mapping += ",";

		// Matches SDL_GameControllerAddMappingsFromRW.
		if (mapping.find("platform:") == std::string::npos)
			mapping += "platform:" + std::string(SDL_GetPlatform()) + ",";

		mappings += mapping + "\n";
	}

	return mappings;
}

} // sdl
} // joystick
} // love
