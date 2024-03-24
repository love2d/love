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
#include <SDL_version.h>

// C++
#include <algorithm>
#include <limits>

#if !SDL_VERSION_ATLEAST(3, 0, 0)

#ifndef SDL_TICKS_PASSED
#define SDL_TICKS_PASSED(A, B)  ((Sint32)((B) - (A)) <= 0)
#endif

namespace love
{
namespace joystick
{
namespace sdl
{

Joystick::Joystick(int id)
	: joyhandle(nullptr)
	, controller(nullptr)
	, haptic(nullptr)
	, joystickType(JOYSTICK_TYPE_UNKNOWN)
	, instanceid(-1)
	, id(id)
	, vibration()
{
}

Joystick::Joystick(int id, int joyindex)
	: joyhandle(nullptr)
	, controller(nullptr)
	, haptic(nullptr)
	, instanceid(-1)
	, id(id)
	, vibration()
{
	open(joyindex);
}

Joystick::~Joystick()
{
	close();
}

bool Joystick::open(int64 deviceid)
{
	int deviceindex = (int) deviceid;

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

		switch (SDL_JoystickGetType(joyhandle))
		{
		case SDL_JOYSTICK_TYPE_GAMECONTROLLER:
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
	}

	return isConnected();
}

void Joystick::close()
{
	if (haptic)
		SDL_HapticClose(haptic);

	if (controller)
		SDL_GameControllerClose(controller);

	if (joyhandle)
		SDL_JoystickClose(joyhandle);

	joyhandle = nullptr;
	controller = nullptr;
	haptic = nullptr;
	instanceid = -1;
	vibration = Vibration();
}

bool Joystick::isConnected() const
{
	return joyhandle != nullptr && SDL_JoystickGetAttached(joyhandle);
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

	int numbuttons = getButtonCount();

	for (int button : buttonlist)
	{
		if (button < 0 || button >= numbuttons)
			continue;

		if (SDL_JoystickGetButton(joyhandle, button) == 1)
			return true;
	}

	return false;
}

void Joystick::setPlayerIndex(int index)
{
	if (!isConnected())
		return;

#if SDL_VERSION_ATLEAST(2, 0, 12)
	SDL_JoystickSetPlayerIndex(joyhandle, index);
#else
	LOVE_UNUSED(index);
#endif
}

int Joystick::getPlayerIndex() const
{
	if (!isConnected())
		return -1;

#if SDL_VERSION_ATLEAST(2, 0, 12)
	return SDL_JoystickGetPlayerIndex(joyhandle);
#else
	return -1;
#endif
}

bool Joystick::openGamepad(int64 deviceid)
{
	int deviceindex = (int) deviceid;

	if (!SDL_IsGameController(deviceindex))
		return false;

	if (isGamepad())
	{
		SDL_GameControllerClose(controller);
		controller = nullptr;
	}

	controller = SDL_GameControllerOpen(deviceindex);
	return isGamepad();
}

bool Joystick::isGamepad() const
{
	return controller != nullptr;
}

Joystick::GamepadType Joystick::getGamepadType() const
{
	if (controller == nullptr)
		return GAMEPAD_TYPE_UNKNOWN;

#if SDL_VERSION_ATLEAST(2, 0, 12)
	switch (SDL_GameControllerGetType(controller))
	{
		case SDL_CONTROLLER_TYPE_UNKNOWN: return GAMEPAD_TYPE_UNKNOWN;
		case SDL_CONTROLLER_TYPE_XBOX360: return GAMEPAD_TYPE_XBOX360;
		case SDL_CONTROLLER_TYPE_XBOXONE: return GAMEPAD_TYPE_XBOXONE;
		case SDL_CONTROLLER_TYPE_PS3: return GAMEPAD_TYPE_PS3;
		case SDL_CONTROLLER_TYPE_PS4: return GAMEPAD_TYPE_PS4;
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO: return GAMEPAD_TYPE_NINTENDO_SWITCH_PRO;
#if SDL_VERSION_ATLEAST(2, 0, 14)
		case SDL_CONTROLLER_TYPE_VIRTUAL: return GAMEPAD_TYPE_VIRTUAL;
		case SDL_CONTROLLER_TYPE_PS5: return GAMEPAD_TYPE_PS5;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 16)
		case SDL_CONTROLLER_TYPE_AMAZON_LUNA: return GAMEPAD_TYPE_AMAZON_LUNA;
		case SDL_CONTROLLER_TYPE_GOOGLE_STADIA: return GAMEPAD_TYPE_STADIA;
#endif
#if SDL_VERSION_ATLEAST(2, 24, 0)
		case SDL_CONTROLLER_TYPE_NVIDIA_SHIELD: return GAMEPAD_TYPE_NVIDIA_SHIELD;
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT: return GAMEPAD_TYPE_JOYCON_LEFT;
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT: return GAMEPAD_TYPE_JOYCON_RIGHT;
		case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR: return GAMEPAD_TYPE_JOYCON_PAIR;
#endif
	}
#endif

	return GAMEPAD_TYPE_UNKNOWN;
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

	for (GamepadButton button : blist)
	{
		if (!getConstant(button, sdlbutton))
			continue;

		if (SDL_GameControllerGetButton(controller, sdlbutton) == 1)
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

	SDL_GameControllerButtonBind sdlbind = {};
	sdlbind.bindType = SDL_CONTROLLER_BINDTYPE_NONE;

	SDL_GameControllerButton sdlbutton;
	SDL_GameControllerAxis sdlaxis;

	switch (input.type)
	{
	case INPUT_TYPE_BUTTON:
		if (getConstant(input.button, sdlbutton))
			sdlbind = SDL_GameControllerGetBindForButton(controller, sdlbutton);
		break;
	case INPUT_TYPE_AXIS:
		if (getConstant(input.axis, sdlaxis))
			sdlbind = SDL_GameControllerGetBindForAxis(controller, sdlaxis);
		break;
	default:
		break;
	}

	switch (sdlbind.bindType)
	{
	case SDL_CONTROLLER_BINDTYPE_BUTTON:
		jinput.type = INPUT_TYPE_BUTTON;
		jinput.button = sdlbind.value.button;
		break;
	case SDL_CONTROLLER_BINDTYPE_AXIS:
		jinput.type = INPUT_TYPE_AXIS;
		jinput.axis = sdlbind.value.axis;
		break;
	case SDL_CONTROLLER_BINDTYPE_HAT:
		if (getConstant(sdlbind.value.hat.hat_mask, jinput.hat.value))
		{
			jinput.type = INPUT_TYPE_HAT;
			jinput.hat.index = sdlbind.value.hat.hat;
		}
		break;
	case SDL_CONTROLLER_BINDTYPE_NONE:
	default:
		break;
	}

	return jinput;
}

std::string Joystick::getGamepadMappingString() const
{
	char *sdlmapping = nullptr;

	if (controller != nullptr)
		sdlmapping = SDL_GameControllerMapping(controller);

	if (sdlmapping == nullptr)
	{
		SDL_JoystickGUID sdlguid = SDL_JoystickGetGUIDFromString(pguid.c_str());
		sdlmapping = SDL_GameControllerMappingForGUID(sdlguid);
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
		vendorID = SDL_JoystickGetVendor(joyhandle);
		productID = SDL_JoystickGetProduct(joyhandle);
		productVersion = SDL_JoystickGetProductVersion(joyhandle);
	}
	else
	{
		vendorID = 0;
		productID = 0;
		productVersion = 0;
	}
}

bool Joystick::checkCreateHaptic()
{
	if (!isConnected())
		return false;

	if (!SDL_WasInit(SDL_INIT_HAPTIC) && SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0)
		return false;

	if (haptic && SDL_HapticIndex(haptic) != -1)
		return true;

	if (haptic)
	{
		SDL_HapticClose(haptic);
		haptic = nullptr;
	}

	haptic = SDL_HapticOpenFromJoystick(joyhandle);
	vibration = Vibration();

	return haptic != nullptr;
}

bool Joystick::isVibrationSupported()
{
#if SDL_VERSION_ATLEAST(2, 0, 18)
	if (isConnected() && SDL_JoystickHasRumble(joyhandle) == SDL_TRUE)
		return true;
#endif

	if (!checkCreateHaptic())
		return false;

	unsigned int features = SDL_HapticQuery(haptic);

	if ((features & SDL_HAPTIC_LEFTRIGHT) != 0)
		return true;

	// Some gamepad drivers only support left/right motors via a custom effect.
	if (isGamepad() && (features & SDL_HAPTIC_CUSTOM) != 0)
		return true;

	// Test for simple sine wave support as a last resort.
	if ((features & SDL_HAPTIC_SINE) != 0)
		return true;

	return false;
}

bool Joystick::runVibrationEffect()
{
	if (vibration.id != -1)
	{
		if (SDL_HapticUpdateEffect(haptic, vibration.id, &vibration.effect) == 0)
		{
			if (SDL_HapticRunEffect(haptic, vibration.id, 1) == 0)
				return true;
		}

		// If the effect fails to update, we should destroy and re-create it.
		SDL_HapticDestroyEffect(haptic, vibration.id);
		vibration.id = -1;
	}

	vibration.id = SDL_HapticNewEffect(haptic, &vibration.effect);

	if (vibration.id != -1 && SDL_HapticRunEffect(haptic, vibration.id, 1) == 0)
		return true;

	return false;
}

bool Joystick::setVibration(float left, float right, float duration)
{
	left = std::min(std::max(left, 0.0f), 1.0f);
	right = std::min(std::max(right, 0.0f), 1.0f);

	if (left == 0.0f && right == 0.0f)
		return setVibration();

	if (!isConnected())
	{
		vibration.left = vibration.right = 0.0f;
		vibration.endtime = SDL_HAPTIC_INFINITY;
		return false;
	}

	Uint32 length = SDL_HAPTIC_INFINITY;
	if (duration >= 0.0f)
	{
		float maxduration = (float) (std::numeric_limits<Uint32>::max() / 1000.0);
		length = Uint32(std::min(duration, maxduration) * 1000);
	}

	bool success = false;

	if (SDL_JoystickRumble(joyhandle, (Uint16)(left * LOVE_UINT16_MAX), (Uint16)(right * LOVE_UINT16_MAX), length) == 0)
		success = true;

	if (!success && !checkCreateHaptic())
		return false;

	unsigned int features = SDL_HapticQuery(haptic);
	int axes = SDL_HapticNumAxes(haptic);

	if (!success && (features & SDL_HAPTIC_LEFTRIGHT) != 0)
	{
		memset(&vibration.effect, 0, sizeof(SDL_HapticEffect));
		vibration.effect.type = SDL_HAPTIC_LEFTRIGHT;

		vibration.effect.leftright.length = length;
		vibration.effect.leftright.large_magnitude = Uint16(left * LOVE_UINT16_MAX);
		vibration.effect.leftright.small_magnitude = Uint16(right * LOVE_UINT16_MAX);

		success = runVibrationEffect();
	}

	// Some gamepad drivers only give support for controlling individual motors
	// through a custom FF effect.
	if (!success && isGamepad() && (features & SDL_HAPTIC_CUSTOM) && axes == 2)
	{
		// NOTE: this may cause issues with drivers which support custom effects
		// but aren't similar to https://github.com/d235j/360Controller .

		// Custom effect data is clamped to 0x7FFF in SDL.
		vibration.data[0] = vibration.data[2] = Uint16(left * 0x7FFF);
		vibration.data[1] = vibration.data[3] = Uint16(right * 0x7FFF);

		memset(&vibration.effect, 0, sizeof(SDL_HapticEffect));
		vibration.effect.type = SDL_HAPTIC_CUSTOM;

		vibration.effect.custom.length = length;
		vibration.effect.custom.channels = 2;
		vibration.effect.custom.period = 10;
		vibration.effect.custom.samples = 2;
		vibration.effect.custom.data = vibration.data;

		success = runVibrationEffect();
	}

	// Fall back to a simple sine wave if all else fails. This only supports a
	// single strength value.
	if (!success && (features & SDL_HAPTIC_SINE) != 0)
	{
		memset(&vibration.effect, 0, sizeof(SDL_HapticEffect));
		vibration.effect.type = SDL_HAPTIC_SINE;

		vibration.effect.periodic.length = length;
		vibration.effect.periodic.period = 10;

		float strength = std::max(left, right);
		vibration.effect.periodic.magnitude = Sint16(strength * 0x7FFF);

		success = runVibrationEffect();
	}

	if (success)
	{
		vibration.left = left;
		vibration.right = right;

		if (length == SDL_HAPTIC_INFINITY)
			vibration.endtime = SDL_HAPTIC_INFINITY;
		else
			vibration.endtime = SDL_GetTicks() + length;
	}
	else
	{
		vibration.left = vibration.right = 0.0f;
		vibration.endtime = SDL_HAPTIC_INFINITY;
	}

	return success;
}

bool Joystick::setVibration()
{
	bool success = false;

	if (!success)
		success = isConnected() && SDL_JoystickRumble(joyhandle, 0, 0, 0) == 0;

	if (!success && SDL_WasInit(SDL_INIT_HAPTIC) && haptic && SDL_HapticIndex(haptic) != -1)
		success = (SDL_HapticStopEffect(haptic, vibration.id) == 0);

	if (success)
		vibration.left = vibration.right = 0.0f;

	return success;
}

void Joystick::getVibration(float &left, float &right)
{
	if (vibration.endtime != SDL_HAPTIC_INFINITY)
	{
		// With some drivers, the effect physically stops at the right time, but
		// SDL_HapticGetEffectStatus still thinks it's playing. So we explicitly
		// stop it once it's done, just to be sure.
		if (SDL_TICKS_PASSED(SDL_GetTicks(), vibration.endtime))
		{
			setVibration();
			vibration.endtime = SDL_HAPTIC_INFINITY;
		}
	}

	// Check if the haptic effect has stopped playing.
	int id = vibration.id;
	if (!haptic || id == -1 || SDL_HapticGetEffectStatus(haptic, id) != 1)
		vibration.left = vibration.right = 0.0f;

	left = vibration.left;
	right = vibration.right;
}

bool Joystick::hasSensor(Sensor::SensorType type) const
{
#if SDL_VERSION_ATLEAST(2, 0, 14) && defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		return false;

	return SDL_GameControllerHasSensor(controller, SDLSensor::convert(type)) == SDL_TRUE;
#else
	return false;
#endif
}

bool Joystick::isSensorEnabled(Sensor::SensorType type) const
{
#if SDL_VERSION_ATLEAST(2, 0, 14) && defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		return false;

	return SDL_GameControllerIsSensorEnabled(controller, SDLSensor::convert(type)) == SDL_TRUE;
#else
	return false;
#endif
}

void Joystick::setSensorEnabled(Sensor::SensorType type, bool enabled)
{
#if SDL_VERSION_ATLEAST(2, 0, 14) && defined(LOVE_ENABLE_SENSOR)
	using SDLSensor = love::sensor::sdl::Sensor;

	if (!isGamepad())
		throw love::Exception("Sensor is only supported on gamepad");

	if (SDL_GameControllerSetSensorEnabled(controller, SDLSensor::convert(type), enabled ? SDL_TRUE : SDL_FALSE) != 0)
	{
		const char *name = nullptr;
		SDLSensor::getConstant(type, name);

		throw love::Exception("Could not open \"%s\" SDL gamepad sensor (%s)", name, SDL_GetError());
	}
#else
	throw love::Exception("Compiled version of SDL or LOVE does not support gamepad sensor");
#endif
}

std::vector<float> Joystick::getSensorData(Sensor::SensorType type) const
{
#if SDL_VERSION_ATLEAST(2, 0, 14) && defined(LOVE_ENABLE_SENSOR)
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

	if (SDL_GameControllerGetSensorData(controller, SDLSensor::convert(type), data.data(), (int) data.size()) != 0)
	{
		const char *name = nullptr;
		SDLSensor::getConstant(type, name);

		throw love::Exception("Could not get \"%s\" SDL gamepad sensor data (%s)", name, SDL_GetError());
	}

	return data;
#else
	throw love::Exception("Compiled version of SDL or LOVE does not support gamepad sensor");
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
#if SDL_VERSION_ATLEAST(2, 0, 14)
	{Joystick::GAMEPAD_BUTTON_MISC1, SDL_CONTROLLER_BUTTON_MISC1},
	{Joystick::GAMEPAD_BUTTON_PADDLE1, SDL_CONTROLLER_BUTTON_PADDLE1},
	{Joystick::GAMEPAD_BUTTON_PADDLE2, SDL_CONTROLLER_BUTTON_PADDLE2},
	{Joystick::GAMEPAD_BUTTON_PADDLE3, SDL_CONTROLLER_BUTTON_PADDLE3},
	{Joystick::GAMEPAD_BUTTON_PADDLE4, SDL_CONTROLLER_BUTTON_PADDLE4},
	{Joystick::GAMEPAD_BUTTON_TOUCHPAD, SDL_CONTROLLER_BUTTON_TOUCHPAD},
#endif
};

EnumMap<Joystick::GamepadButton, SDL_GameControllerButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM> Joystick::gpButtons(Joystick::gpButtonEntries, sizeof(Joystick::gpButtonEntries));

} // sdl
} // joystick
} // love

#endif // !SDL_VERSION_ATLEAST(3, 0, 0)
