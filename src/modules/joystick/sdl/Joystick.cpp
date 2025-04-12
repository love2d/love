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

// LOVE
#include "common/config.h"
#include "Joystick.h"
#include "common/int.h"
#include "sensor/sdl/Sensor.h"

// SDL
#include <SDL3/SDL_guid.h>

// C++
#include <algorithm>
#include <limits>

namespace love
{
namespace joystick
{
namespace sdl
{

Joystick::Joystick(int id)
	: joyhandle(nullptr)
	, controller(nullptr)
	, joystickType(JOYSTICK_TYPE_UNKNOWN)
	, gamepadType(GAMEPAD_TYPE_UNKNOWN)
	, instanceid(-1)
	, id(id)
{
}

Joystick::~Joystick()
{
	close();
}

bool Joystick::open(int64 deviceid)
{
	close();

	joyhandle = SDL_OpenJoystick((SDL_JoystickID) deviceid);

	if (joyhandle)
	{
		instanceid = SDL_GetJoystickID(joyhandle);

		// SDL_JoystickGetGUIDString uses 32 bytes plus the null terminator.
		char cstr[33];

		SDL_GUID sdlguid = SDL_GetJoystickGUID(joyhandle);
		SDL_GUIDToString(sdlguid, cstr, (int) sizeof(cstr));

		pguid = std::string(cstr);

		// See if SDL thinks this is a Game Controller.
		openGamepad(deviceid);

		// Prefer the Joystick name for consistency.
		const char *joyname = SDL_GetJoystickName(joyhandle);
		if (!joyname && controller)
			joyname = SDL_GetGamepadName(controller);

		if (joyname)
			name = joyname;

		switch (SDL_GetJoystickType(joyhandle))
		{
		case SDL_JOYSTICK_TYPE_GAMEPAD:
			joystickType = JOYSTICK_TYPE_GAMEPAD;
			break;
		case SDL_JOYSTICK_TYPE_WHEEL:
			joystickType = JOYSTICK_TYPE_WHEEL;
			break;
		case SDL_JOYSTICK_TYPE_ARCADE_STICK:
			joystickType = JOYSTICK_TYPE_ARCADE_STICK;
			break;
		case SDL_JOYSTICK_TYPE_FLIGHT_STICK:
			joystickType = JOYSTICK_TYPE_FLIGHT_STICK;
			break;
		case SDL_JOYSTICK_TYPE_DANCE_PAD:
			joystickType = JOYSTICK_TYPE_DANCE_PAD;
			break;
		case SDL_JOYSTICK_TYPE_GUITAR:
			joystickType = JOYSTICK_TYPE_GUITAR;
			break;
		case SDL_JOYSTICK_TYPE_DRUM_KIT:
			joystickType = JOYSTICK_TYPE_DRUM_KIT;
			break;
		case SDL_JOYSTICK_TYPE_ARCADE_PAD:
			joystickType = JOYSTICK_TYPE_ARCADE_PAD;
			break;
		case SDL_JOYSTICK_TYPE_THROTTLE:
			joystickType = JOYSTICK_TYPE_THROTTLE;
			break;
		default:
			joystickType = JOYSTICK_TYPE_UNKNOWN;
			break;
		}

		if (controller != nullptr)
		{
			switch (SDL_GetGamepadType(controller))
			{
			case SDL_GAMEPAD_TYPE_UNKNOWN:
				gamepadType = GAMEPAD_TYPE_UNKNOWN;
				break;
			case SDL_GAMEPAD_TYPE_XBOX360:
				gamepadType = GAMEPAD_TYPE_XBOX360;
				break;
			case SDL_GAMEPAD_TYPE_XBOXONE:
				gamepadType = GAMEPAD_TYPE_XBOXONE;
				break;
			case SDL_GAMEPAD_TYPE_PS3:
				gamepadType = GAMEPAD_TYPE_PS3;
				break;
			case SDL_GAMEPAD_TYPE_PS4:
				gamepadType = GAMEPAD_TYPE_PS4;
				break;
			case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
				gamepadType = GAMEPAD_TYPE_NINTENDO_SWITCH_PRO;
				break;
			case SDL_GAMEPAD_TYPE_PS5:
				gamepadType = GAMEPAD_TYPE_PS5;
				break;
			case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
				gamepadType = GAMEPAD_TYPE_JOYCON_LEFT;
				break;
			case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
				gamepadType = GAMEPAD_TYPE_JOYCON_RIGHT;
				break;
			case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
				gamepadType = GAMEPAD_TYPE_JOYCON_PAIR;
				break;
			default:
				gamepadType = GAMEPAD_TYPE_UNKNOWN;
				break;
			}
		}
		else
			gamepadType = GAMEPAD_TYPE_UNKNOWN;
	}

	return isConnected();
}

void Joystick::close()
{
	if (controller)
		SDL_CloseGamepad(controller);

	if (joyhandle)
		SDL_CloseJoystick(joyhandle);

	joyhandle = nullptr;
	controller = nullptr;
	instanceid = -1;
}

bool Joystick::isConnected() const
{
	return joyhandle != nullptr && SDL_JoystickConnected(joyhandle);
}

const char *Joystick::getName() const
{
	return name.c_str();
}

Joystick::JoystickType Joystick::getJoystickType() const
{
	return joystickType;
}

int Joystick::getAxisCount() const
{
	return isConnected() ? SDL_GetNumJoystickAxes(joyhandle) : 0;
}

int Joystick::getButtonCount() const
{
	return isConnected() ? SDL_GetNumJoystickButtons(joyhandle) : 0;
}

int Joystick::getHatCount() const
{
	return isConnected() ? SDL_GetNumJoystickHats(joyhandle) : 0;
}

float Joystick::getAxis(int axisindex) const
{
	if (!isConnected() || axisindex < 0 || axisindex >= getAxisCount())
		return 0;

	return clampval(((float) SDL_GetJoystickAxis(joyhandle, axisindex))/32768.0f);
}

std::vector<float> Joystick::getAxes() const
{
	std::vector<float> axes;
	int count = getAxisCount();

	if (!isConnected() || count <= 0)
		return axes;

	axes.reserve(count);

	for (int i = 0; i < count; i++)
		axes.push_back(clampval(((float) SDL_GetJoystickAxis(joyhandle, i))/32768.0f));

	return axes;
}

Joystick::Hat Joystick::getHat(int hatindex) const
{
	Hat h = HAT_INVALID;

	if (!isConnected() || hatindex < 0 || hatindex >= getHatCount())
		return h;

	getConstant(SDL_GetJoystickHat(joyhandle, hatindex), h);

	return h;
}

bool Joystick::isDown(const std::vector<int> &buttonlist) const
{
	if (!isConnected())
		return false;

	int numbuttons = getButtonCount();

	for (int button : buttonlist)
	{
		if (button < 0 || button >= numbuttons)
			continue;

		if (SDL_GetJoystickButton(joyhandle, button))
			return true;
	}

	return false;
}

void Joystick::setPlayerIndex(int index)
{
	if (!isConnected())
		return;

	SDL_SetJoystickPlayerIndex(joyhandle, index);
}

int Joystick::getPlayerIndex() const
{
	if (!isConnected())
		return -1;

	return SDL_GetJoystickPlayerIndex(joyhandle);
}

bool Joystick::openGamepad(int64 deviceid)
{
	if (!SDL_IsGamepad((SDL_JoystickID)deviceid))
		return false;

	if (isGamepad())
	{
		SDL_CloseGamepad(controller);
		controller = nullptr;
	}

	controller = SDL_OpenGamepad((SDL_JoystickID)deviceid);
	return isGamepad();
}

bool Joystick::isGamepad() const
{
	return controller != nullptr;
}

Joystick::GamepadType Joystick::getGamepadType() const
{
	return gamepadType;
}

float Joystick::getGamepadAxis(love::joystick::Joystick::GamepadAxis axis) const
{
	if (!isConnected() || !isGamepad())
		return 0.f;

	SDL_GamepadAxis sdlaxis;
	if (!getConstant(axis, sdlaxis))
		return 0.f;

	Sint16 value = SDL_GetGamepadAxis(controller, sdlaxis);

	return clampval((float) value / 32768.0f);
}

bool Joystick::isGamepadDown(const std::vector<GamepadButton> &blist) const
{
	if (!isConnected() || !isGamepad())
		return false;

	SDL_GamepadButton sdlbutton;

	for (GamepadButton button : blist)
	{
		if (!getConstant(button, sdlbutton))
			continue;

		if (SDL_GetGamepadButton(controller, sdlbutton))
			return true;
	}

	return false;
}

Joystick::JoystickInput Joystick::getGamepadMapping(const GamepadInput &input) const
{
	Joystick::JoystickInput jinput;
	jinput.type = INPUT_TYPE_MAX_ENUM;

	if (!isGamepad())
		return jinput;

	SDL_GamepadButton sdlbutton = SDL_GAMEPAD_BUTTON_INVALID;
	SDL_GamepadAxis sdlaxis = SDL_GAMEPAD_AXIS_INVALID;

	switch (input.type)
	{
	case INPUT_TYPE_BUTTON:
		getConstant(input.button, sdlbutton);
		break;
	case INPUT_TYPE_AXIS:
		getConstant(input.axis, sdlaxis);
		break;
	default:
		break;
	}

	int bindcount = 0;
	SDL_GamepadBinding **sdlbindings = SDL_GetGamepadBindings(controller, &bindcount);
	for (int i = 0; i < bindcount; i++)
	{
		const SDL_GamepadBinding *b = sdlbindings[i];
		if ((input.type == INPUT_TYPE_BUTTON && b->output_type == SDL_GAMEPAD_BINDTYPE_BUTTON && b->output.button == sdlbutton)
			|| (input.type == INPUT_TYPE_AXIS && b->output_type == SDL_GAMEPAD_BINDTYPE_AXIS && b->output.axis.axis == sdlaxis))
		{
			switch (b->input_type)
			{
			case SDL_GAMEPAD_BINDTYPE_BUTTON:
				jinput.type = INPUT_TYPE_BUTTON;
				jinput.button = b->input.button;
				break;
			case SDL_GAMEPAD_BINDTYPE_AXIS:
				jinput.type = INPUT_TYPE_AXIS;
				jinput.axis = b->input.axis.axis;
				break;
			case SDL_GAMEPAD_BINDTYPE_HAT:
				if (getConstant(b->input.hat.hat_mask, jinput.hat.value))
				{
					jinput.type = INPUT_TYPE_HAT;
					jinput.hat.index = b->input.hat.hat;
				}
				break;
			case SDL_GAMEPAD_BINDTYPE_NONE:
			default:
				break;
			}

			break;
		}
	}

	SDL_free(sdlbindings);

	return jinput;
}

std::string Joystick::getGamepadMappingString() const
{
	char *sdlmapping = nullptr;

	if (controller != nullptr)
		sdlmapping = SDL_GetGamepadMapping(controller);

	if (sdlmapping == nullptr)
	{
		SDL_GUID sdlguid = SDL_StringToGUID(pguid.c_str());
		sdlmapping = SDL_GetGamepadMappingForGUID(sdlguid);
	}

	if (sdlmapping == nullptr)
		return "";

	std::string mappingstr(sdlmapping);
	SDL_free(sdlmapping);

	// Matches SDL_GameControllerAddMappingsFromRW.
	if (mappingstr.find_last_of(',') != mappingstr.length() - 1)
		mappingstr += ",";

	if (mappingstr.find("platform:") == std::string::npos)
		mappingstr += "platform:" + std::string(SDL_GetPlatform());

	return mappingstr;
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

void Joystick::getDeviceInfo(int &vendorID, int &productID, int &productVersion) const
{
	if (joyhandle != nullptr)
	{
		vendorID = SDL_GetJoystickVendor(joyhandle);
		productID = SDL_GetJoystickProduct(joyhandle);
		productVersion = SDL_GetJoystickProductVersion(joyhandle);
	}
	else
	{
		vendorID = 0;
		productID = 0;
		productVersion = 0;
	}
}

bool Joystick::isVibrationSupported()
{
	if (!isConnected())
		return false;

	SDL_PropertiesID props = SDL_GetJoystickProperties(joyhandle);
	return SDL_GetBooleanProperty(props, SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN, false);
}

bool Joystick::setVibration(float left, float right, float duration)
{
	left = std::min(std::max(left, 0.0f), 1.0f);
	right = std::min(std::max(right, 0.0f), 1.0f);

	if (left == 0.0f && right == 0.0f)
		return setVibration();

	if (!isConnected())
		return false;

	Uint32 length = LOVE_UINT32_MAX;
	if (duration >= 0.0f)
	{
		float maxduration = (float) (std::numeric_limits<Uint32>::max() / 1000.0);
		length = Uint32(std::min(duration, maxduration) * 1000);
	}

	return SDL_RumbleJoystick(joyhandle, (Uint16)(left * LOVE_UINT16_MAX), (Uint16)(right * LOVE_UINT16_MAX), length);
}

bool Joystick::setVibration()
{
	return isConnected() && SDL_RumbleJoystick(joyhandle, 0, 0, 0);
}

void Joystick::getVibration(float &left, float &right)
{
	// Deprecated.
	left = 0.0f;
	right = 0.0f;
}

bool Joystick::hasSensor(Sensor::SensorType type) const
{
#if defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		return false;

	return SDL_GamepadHasSensor(controller, SDLSensor::convert(type));
#else
	return false;
#endif
}

bool Joystick::isSensorEnabled(Sensor::SensorType type) const
{
#if defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		return false;

	return SDL_GamepadSensorEnabled(controller, SDLSensor::convert(type));
#else
	return false;
#endif
}

void Joystick::setSensorEnabled(Sensor::SensorType type, bool enabled)
{
#if defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		throw love::Exception("Sensor is only supported on gamepad");

	if (!SDL_SetGamepadSensorEnabled(controller, SDLSensor::convert(type), enabled))
	{
		const char *name = nullptr;
		SDLSensor::getConstant(type, name);

		throw love::Exception("Could not open \"%s\" SDL gamepad sensor (%s)", name, SDL_GetError());
	}
#else
	throw love::Exception("Compiled version of LOVE does not support gamepad sensor");
#endif
}

std::vector<float> Joystick::getSensorData(Sensor::SensorType type) const
{
#if defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		throw love::Exception("Sensor is only supported on gamepad");

	std::vector<float> data(3);

	if (!isSensorEnabled(type))
	{
		const char *name = nullptr;
		SDLSensor::getConstant(type, name);

		throw love::Exception("\"%s\" gamepad sensor is not enabled", name);
	}

	if (!SDL_GetGamepadSensorData(controller, SDLSensor::convert(type), data.data(), (int) data.size()))
	{
		const char *name = nullptr;
		SDLSensor::getConstant(type, name);

		throw love::Exception("Could not get \"%s\" SDL gamepad sensor data (%s)", name, SDL_GetError());
	}

	return data;
#else
	throw love::Exception("Compiled version of LOVE does not support gamepad sensor");
#endif
}

bool Joystick::getConstant(Uint8 in, Joystick::Hat &out)
{
	return hats.find(in, out);
}

bool Joystick::getConstant(Joystick::Hat in, Uint8 &out)
{
	return hats.find(in, out);
}

bool Joystick::getConstant(SDL_GamepadAxis in, Joystick::GamepadAxis &out)
{
	return gpAxes.find(in, out);
}

bool Joystick::getConstant(Joystick::GamepadAxis in, SDL_GamepadAxis &out)
{
	return gpAxes.find(in, out);
}

bool Joystick::getConstant(SDL_GamepadButton in, Joystick::GamepadButton &out)
{
	return gpButtons.find(in, out);
}

bool Joystick::getConstant(Joystick::GamepadButton in, SDL_GamepadButton &out)
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

EnumMap<Joystick::GamepadAxis, SDL_GamepadAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM>::Entry Joystick::gpAxisEntries[] =
{
	{Joystick::GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTX},
	{Joystick::GAMEPAD_AXIS_LEFTY, SDL_GAMEPAD_AXIS_LEFTY},
	{Joystick::GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTX},
	{Joystick::GAMEPAD_AXIS_RIGHTY, SDL_GAMEPAD_AXIS_RIGHTY},
	{Joystick::GAMEPAD_AXIS_TRIGGERLEFT, SDL_GAMEPAD_AXIS_LEFT_TRIGGER},
	{Joystick::GAMEPAD_AXIS_TRIGGERRIGHT, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER},
};

EnumMap<Joystick::GamepadAxis, SDL_GamepadAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM> Joystick::gpAxes(Joystick::gpAxisEntries, sizeof(Joystick::gpAxisEntries));

EnumMap<Joystick::GamepadButton, SDL_GamepadButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM>::Entry Joystick::gpButtonEntries[] =
{
	{Joystick::GAMEPAD_BUTTON_A, SDL_GAMEPAD_BUTTON_SOUTH},
	{Joystick::GAMEPAD_BUTTON_B, SDL_GAMEPAD_BUTTON_EAST},
	{Joystick::GAMEPAD_BUTTON_X, SDL_GAMEPAD_BUTTON_WEST},
	{Joystick::GAMEPAD_BUTTON_Y, SDL_GAMEPAD_BUTTON_NORTH},
	{Joystick::GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_BACK},
	{Joystick::GAMEPAD_BUTTON_GUIDE, SDL_GAMEPAD_BUTTON_GUIDE},
	{Joystick::GAMEPAD_BUTTON_START, SDL_GAMEPAD_BUTTON_START},
	{Joystick::GAMEPAD_BUTTON_LEFTSTICK, SDL_GAMEPAD_BUTTON_LEFT_STICK},
	{Joystick::GAMEPAD_BUTTON_RIGHTSTICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK},
	{Joystick::GAMEPAD_BUTTON_LEFTSHOULDER, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER},
	{Joystick::GAMEPAD_BUTTON_RIGHTSHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER},
	{Joystick::GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_UP},
	{Joystick::GAMEPAD_BUTTON_DPAD_DOWN, SDL_GAMEPAD_BUTTON_DPAD_DOWN},
	{Joystick::GAMEPAD_BUTTON_DPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_LEFT},
	{Joystick::GAMEPAD_BUTTON_DPAD_RIGHT, SDL_GAMEPAD_BUTTON_DPAD_RIGHT},
	{Joystick::GAMEPAD_BUTTON_MISC1, SDL_GAMEPAD_BUTTON_MISC1},
	{Joystick::GAMEPAD_BUTTON_PADDLE1, SDL_GAMEPAD_BUTTON_LEFT_PADDLE1},
	{Joystick::GAMEPAD_BUTTON_PADDLE2, SDL_GAMEPAD_BUTTON_LEFT_PADDLE2},
	{Joystick::GAMEPAD_BUTTON_PADDLE3, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1},
	{Joystick::GAMEPAD_BUTTON_PADDLE4, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2},
	{Joystick::GAMEPAD_BUTTON_TOUCHPAD, SDL_GAMEPAD_BUTTON_TOUCHPAD},
};

EnumMap<Joystick::GamepadButton, SDL_GamepadButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM> Joystick::gpButtons(Joystick::gpButtonEntries, sizeof(Joystick::gpButtonEntries));

} // sdl
} // joystick
} // love
