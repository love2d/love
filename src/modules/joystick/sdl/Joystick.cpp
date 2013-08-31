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

#include "Joystick.h"

namespace love
{
namespace joystick
{
namespace sdl
{

Joystick::Joystick(int id)
	: joyhandle(0)
	, controller(0)
	, instanceid(-1)
	, id(id)
{
}

Joystick::Joystick(int id, int joyindex)
	: joyhandle(0)
	, controller(0)
	, instanceid(-1)
	, id(id)
{
	open(joyindex);
}

Joystick::~Joystick()
{
	close();
}

bool Joystick::open(int deviceindex)
{
	close();

	joyhandle = SDL_JoystickOpen(deviceindex);

	if (joyhandle)
	{
		instanceid = SDL_JoystickInstanceID(joyhandle);

		// SDL_JoystickGetGUIDString uses 32 bytes plus the null terminator.
		char cstr[33];

		SDL_JoystickGUID sdlguid = SDL_JoystickGetGUID(joyhandle);
		SDL_JoystickGetGUIDString(sdlguid, cstr, (int) sizeof(cstr));

		pguid = std::string(cstr);

		// See if SDL thinks this is a Game Controller.
		openGamepad(deviceindex);

		// Prefer the Joystick name for consistency.
		const char *joyname = SDL_JoystickName(joyhandle);
		if (!joyname && controller)
			joyname = SDL_GameControllerName(controller);

		if (joyname)
			name = joyname;
	}

	return isConnected();
}

void Joystick::close()
{
	if (controller)
		SDL_GameControllerClose(controller);

	if (joyhandle)
		SDL_JoystickClose(joyhandle);

	joyhandle = 0;
	controller = 0;
	instanceid = -1;
}

bool Joystick::isConnected() const
{
	return joyhandle != 0 && SDL_JoystickGetAttached(joyhandle);
}

const char *Joystick::getName() const
{
	// Use the saved name if this Joystick isn't connected anymore.
	if (!isConnected())
		return name.c_str();

	// Prefer SDL's GameController name, if possible.
	if (isGamepad())
		return SDL_GameControllerName(controller);

	return SDL_JoystickName(joyhandle);
}

int Joystick::getAxisCount() const
{
	return isConnected() ? SDL_JoystickNumAxes(joyhandle) : 0;
}

int Joystick::getButtonCount() const
{
	return isConnected() ? SDL_JoystickNumButtons(joyhandle) : 0;
}

int Joystick::getHatCount() const
{
	return isConnected() ? SDL_JoystickNumHats(joyhandle) : 0;
}

float Joystick::getAxis(int axisindex) const
{
	if (!isConnected() || axisindex < 0 || axisindex >= getAxisCount())
		return 0;

	return clampval(((float) SDL_JoystickGetAxis(joyhandle, axisindex))/32768.0f);
}

std::vector<float> Joystick::getAxes() const
{
	std::vector<float> axes;
	int count = getAxisCount();

	if (!isConnected() || count <= 0)
		return axes;

	axes.reserve(count);

	for (int i = 0; i < count; i++)
		axes.push_back(clampval(((float) SDL_JoystickGetAxis(joyhandle, i))/32768.0f));

	return axes;
}

Joystick::Hat Joystick::getHat(int hatindex) const
{
	Hat h = HAT_INVALID;

	if (!isConnected() || hatindex < 0 || hatindex >= getHatCount())
		return h;

	getConstant(SDL_JoystickGetHat(joyhandle, hatindex), h);

	return h;
}

bool Joystick::isDown(const std::vector<int> &buttonlist) const
{
	if (!isConnected())
		return false;

	int num = getButtonCount();

	for (size_t i = 0; i < buttonlist.size(); i++)
	{
		int button = buttonlist[i];
		if (button >= 0 && button < num && SDL_JoystickGetButton(joyhandle, button) == 1)
			return true;
	}

	return false;
}

bool Joystick::openGamepad(int deviceindex)
{
	if (!SDL_IsGameController(deviceindex))
		return false;

	if (isGamepad())
	{
		SDL_GameControllerClose(controller);
		controller = 0;
	}

	controller = SDL_GameControllerOpen(deviceindex);
	return isGamepad();
}

bool Joystick::isGamepad() const
{
	return controller != 0;
}

float Joystick::getGamepadAxis(love::joystick::Joystick::GamepadAxis axis) const
{
	if (!isConnected() || !isGamepad())
		return 0.f;

	SDL_GameControllerAxis sdlaxis;
	if (!getConstant(axis, sdlaxis))
		return 0.f;

	Sint16 value = SDL_GameControllerGetAxis(controller, sdlaxis);

	return clampval((float) value / 32768.0f);
}

bool Joystick::isGamepadDown(const std::vector<GamepadButton> &blist) const
{
	if (!isConnected() || !isGamepad())
		return false;

	SDL_GameControllerButton sdlbutton;

	for (size_t i = 0; i < blist.size(); i++)
	{
		if (!getConstant(blist[i], sdlbutton))
			continue;

		if (SDL_GameControllerGetButton(controller, sdlbutton) == 1)
			return true;
	}

	return false;
}

void *Joystick::getHandle() const
{
	return joyhandle;
}

std::string Joystick::getGUID() const
{
	// SDL2's GUIDs identify *classes* of devices, instead of unique devices.
	return pguid;
}

int Joystick::getInstanceID() const
{
	return instanceid;
}

int Joystick::getID() const
{
	return id;
}

bool Joystick::getConstant(Uint8 in, Joystick::Hat &out)
{
	return hats.find(in, out);
}

bool Joystick::getConstant(Joystick::Hat in, Uint8 &out)
{
	return hats.find(in, out);
}

bool Joystick::getConstant(SDL_GameControllerAxis in, Joystick::GamepadAxis &out)
{
	return gpAxes.find(in, out);
}

bool Joystick::getConstant(Joystick::GamepadAxis in, SDL_GameControllerAxis &out)
{
	return gpAxes.find(in, out);
}

bool Joystick::getConstant(SDL_GameControllerButton in, Joystick::GamepadButton &out)
{
	return gpButtons.find(in, out);
}

bool Joystick::getConstant(Joystick::GamepadButton in, SDL_GameControllerButton &out)
{
	return gpButtons.find(in, out);
}

EnumMap<Joystick::Hat, Uint8, Joystick::HAT_MAX_ENUM>::Entry Joystick::hatEntries[] =
{
	{Joystick::HAT_CENTERED, SDL_HAT_CENTERED},
	{Joystick::HAT_UP, SDL_HAT_UP},
	{Joystick::HAT_RIGHT, SDL_HAT_RIGHT},
	{Joystick::HAT_DOWN, SDL_HAT_DOWN},
	{Joystick::HAT_LEFT, SDL_HAT_LEFT},
	{Joystick::HAT_RIGHTUP, SDL_HAT_RIGHTUP},
	{Joystick::HAT_RIGHTDOWN, SDL_HAT_RIGHTDOWN},
	{Joystick::HAT_LEFTUP, SDL_HAT_LEFTUP},
	{Joystick::HAT_LEFTDOWN, SDL_HAT_LEFTDOWN},
};

EnumMap<Joystick::Hat, Uint8, Joystick::HAT_MAX_ENUM> Joystick::hats(Joystick::hatEntries, sizeof(Joystick::hatEntries));

EnumMap<Joystick::GamepadAxis, SDL_GameControllerAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM>::Entry Joystick::gpAxisEntries[] =
{
	{Joystick::GAMEPAD_AXIS_LEFTX, SDL_CONTROLLER_AXIS_LEFTX},
	{Joystick::GAMEPAD_AXIS_LEFTY, SDL_CONTROLLER_AXIS_LEFTY},
	{Joystick::GAMEPAD_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTX},
	{Joystick::GAMEPAD_AXIS_RIGHTY, SDL_CONTROLLER_AXIS_RIGHTY},
	{Joystick::GAMEPAD_AXIS_TRIGGERLEFT, SDL_CONTROLLER_AXIS_TRIGGERLEFT},
	{Joystick::GAMEPAD_AXIS_TRIGGERRIGHT, SDL_CONTROLLER_AXIS_TRIGGERRIGHT},
};

EnumMap<Joystick::GamepadAxis, SDL_GameControllerAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM> Joystick::gpAxes(Joystick::gpAxisEntries, sizeof(Joystick::gpAxisEntries));

EnumMap<Joystick::GamepadButton, SDL_GameControllerButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM>::Entry Joystick::gpButtonEntries[] =
{
	{Joystick::GAMEPAD_BUTTON_A, SDL_CONTROLLER_BUTTON_A},
	{Joystick::GAMEPAD_BUTTON_B, SDL_CONTROLLER_BUTTON_B},
	{Joystick::GAMEPAD_BUTTON_X, SDL_CONTROLLER_BUTTON_X},
	{Joystick::GAMEPAD_BUTTON_Y, SDL_CONTROLLER_BUTTON_Y},
	{Joystick::GAMEPAD_BUTTON_BACK, SDL_CONTROLLER_BUTTON_BACK},
	{Joystick::GAMEPAD_BUTTON_GUIDE, SDL_CONTROLLER_BUTTON_GUIDE},
	{Joystick::GAMEPAD_BUTTON_START, SDL_CONTROLLER_BUTTON_START},
	{Joystick::GAMEPAD_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_LEFTSTICK},
	{Joystick::GAMEPAD_BUTTON_RIGHTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK},
	{Joystick::GAMEPAD_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
	{Joystick::GAMEPAD_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
	{Joystick::GAMEPAD_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_UP},
	{Joystick::GAMEPAD_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN},
	{Joystick::GAMEPAD_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT},
	{Joystick::GAMEPAD_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
};

EnumMap<Joystick::GamepadButton, SDL_GameControllerButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM> Joystick::gpButtons(Joystick::gpButtonEntries, sizeof(Joystick::gpButtonEntries));

} // sdl
} // joystick
} // love
