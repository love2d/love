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

// LOVE
#include "Joystick.h"

// STL
#include <cmath>

namespace love
{
namespace joystick
{

love::Type Joystick::type("Joystick", &Object::type);

float Joystick::clampval(float x)
{
	if (fabsf(x) < 0.01)
		return 0.0f;

	if (x < -0.99f) return -1.0f;
	if (x > 0.99f) return 1.0f;

	return x;
}

bool Joystick::getConstant(const char *in, Joystick::Hat &out)
{
	return hats.find(in, out);
}

bool Joystick::getConstant(Joystick::Hat in, const char *&out)
{
	return hats.find(in, out);
}

bool Joystick::getConstant(const char *in, Joystick::GamepadAxis &out)
{
	return gpAxes.find(in, out);
}

bool Joystick::getConstant(Joystick::GamepadAxis in, const char *&out)
{
	return gpAxes.find(in, out);
}

bool Joystick::getConstant(const char *in, Joystick::GamepadButton &out)
{
	return gpButtons.find(in, out);
}

bool Joystick::getConstant(Joystick::GamepadButton in, const char *&out)
{
	return gpButtons.find(in, out);
}

bool Joystick::getConstant(const char *in, Joystick::InputType &out)
{
	return inputTypes.find(in, out);
}

bool Joystick::getConstant(Joystick::InputType in, const char *&out)
{
	return inputTypes.find(in, out);
}

StringMap<Joystick::Hat, Joystick::HAT_MAX_ENUM>::Entry Joystick::hatEntries[] =
{
	{"c", Joystick::HAT_CENTERED},
	{"u", Joystick::HAT_UP},
	{"r", Joystick::HAT_RIGHT},
	{"d", Joystick::HAT_DOWN},
	{"l", Joystick::HAT_LEFT},
	{"ru", Joystick::HAT_RIGHTUP},
	{"rd", Joystick::HAT_RIGHTDOWN},
	{"lu", Joystick::HAT_LEFTUP},
	{"ld", Joystick::HAT_LEFTDOWN},
};

StringMap<Joystick::Hat, Joystick::HAT_MAX_ENUM> Joystick::hats(Joystick::hatEntries, sizeof(Joystick::hatEntries));

StringMap<Joystick::GamepadAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM>::Entry Joystick::gpAxisEntries[] =
{
	{"leftx", GAMEPAD_AXIS_LEFTX},
	{"lefty", GAMEPAD_AXIS_LEFTY},
	{"rightx", GAMEPAD_AXIS_RIGHTX},
	{"righty", GAMEPAD_AXIS_RIGHTY},
	{"triggerleft", GAMEPAD_AXIS_TRIGGERLEFT},
	{"triggerright", GAMEPAD_AXIS_TRIGGERRIGHT},
};

StringMap<Joystick::GamepadAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM> Joystick::gpAxes(Joystick::gpAxisEntries, sizeof(Joystick::gpAxisEntries));

StringMap<Joystick::GamepadButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM>::Entry Joystick::gpButtonEntries[] =
{
	{"a", GAMEPAD_BUTTON_A},
	{"b", GAMEPAD_BUTTON_B},
	{"x", GAMEPAD_BUTTON_X},
	{"y", GAMEPAD_BUTTON_Y},
	{"back", GAMEPAD_BUTTON_BACK},
	{"guide", GAMEPAD_BUTTON_GUIDE},
	{"start", GAMEPAD_BUTTON_START},
	{"leftstick", GAMEPAD_BUTTON_LEFTSTICK},
	{"rightstick", GAMEPAD_BUTTON_RIGHTSTICK},
	{"leftshoulder", GAMEPAD_BUTTON_LEFTSHOULDER},
	{"rightshoulder", GAMEPAD_BUTTON_RIGHTSHOULDER},
	{"dpup", GAMEPAD_BUTTON_DPAD_UP},
	{"dpdown", GAMEPAD_BUTTON_DPAD_DOWN},
	{"dpleft", GAMEPAD_BUTTON_DPAD_LEFT},
	{"dpright", GAMEPAD_BUTTON_DPAD_RIGHT},
};

StringMap<Joystick::GamepadButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM> Joystick::gpButtons(Joystick::gpButtonEntries, sizeof(Joystick::gpButtonEntries));

StringMap<Joystick::InputType, Joystick::INPUT_TYPE_MAX_ENUM>::Entry Joystick::inputTypeEntries[] =
{
	{"axis", Joystick::INPUT_TYPE_AXIS},
	{"button", Joystick::INPUT_TYPE_BUTTON},
	{"hat", Joystick::INPUT_TYPE_HAT},
};

StringMap<Joystick::InputType, Joystick::INPUT_TYPE_MAX_ENUM> Joystick::inputTypes(Joystick::inputTypeEntries, sizeof(Joystick::inputTypeEntries));

} // joystick
} // love
