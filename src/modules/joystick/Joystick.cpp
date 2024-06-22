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

STRINGMAP_CLASS_BEGIN(Joystick, Joystick::Hat, Joystick::HAT_MAX_ENUM, hat)
{
	{ "c",  Joystick::HAT_CENTERED  },
	{ "u",  Joystick::HAT_UP        },
	{ "r",  Joystick::HAT_RIGHT     },
	{ "d",  Joystick::HAT_DOWN      },
	{ "l",  Joystick::HAT_LEFT      },
	{ "ru", Joystick::HAT_RIGHTUP   },
	{ "rd", Joystick::HAT_RIGHTDOWN },
	{ "lu", Joystick::HAT_LEFTUP    },
	{ "ld", Joystick::HAT_LEFTDOWN  },
}
STRINGMAP_CLASS_END(Joystick, Joystick::Hat, Joystick::HAT_MAX_ENUM, hat)

STRINGMAP_CLASS_BEGIN(Joystick, Joystick::JoystickType, Joystick::JOYSTICK_TYPE_MAX_ENUM, joystickType)
{
	{ "unknown",     Joystick::JOYSTICK_TYPE_UNKNOWN      },
	{ "gamepad",     Joystick::JOYSTICK_TYPE_GAMEPAD      },
	{ "wheel",       Joystick::JOYSTICK_TYPE_WHEEL        },
	{ "arcadestick", Joystick::JOYSTICK_TYPE_ARCADE_STICK },
	{ "flightstick", Joystick::JOYSTICK_TYPE_FLIGHT_STICK },
	{ "dancepad",    Joystick::JOYSTICK_TYPE_DANCE_PAD    },
	{ "guitar",      Joystick::JOYSTICK_TYPE_GUITAR       },
	{ "drumkit",     Joystick::JOYSTICK_TYPE_DRUM_KIT     },
	{ "arcadepad",   Joystick::JOYSTICK_TYPE_ARCADE_PAD   },
	{ "throttle",    Joystick::JOYSTICK_TYPE_THROTTLE     },
}
STRINGMAP_CLASS_END(Joystick, Joystick::JoystickType, Joystick::JOYSTICK_TYPE_MAX_ENUM, joystickType)

STRINGMAP_CLASS_BEGIN(Joystick, Joystick::GamepadType, Joystick::GAMEPAD_TYPE_MAX_ENUM, gamepadType)
{
	{ "unknown",      Joystick::GAMEPAD_TYPE_UNKNOWN             },
	{ "xbox360",      Joystick::GAMEPAD_TYPE_XBOX360             },
	{ "xboxone",      Joystick::GAMEPAD_TYPE_XBOXONE             },
	{ "ps3",          Joystick::GAMEPAD_TYPE_PS3                 },
	{ "ps4",          Joystick::GAMEPAD_TYPE_PS4                 },
	{ "ps5",          Joystick::GAMEPAD_TYPE_PS5                 },
	{ "switchpro",    Joystick::GAMEPAD_TYPE_NINTENDO_SWITCH_PRO },
	{ "amazonluna",   Joystick::GAMEPAD_TYPE_AMAZON_LUNA         },
	{ "stadia",       Joystick::GAMEPAD_TYPE_STADIA              },
	{ "virtual",      Joystick::GAMEPAD_TYPE_VIRTUAL             },
	{ "shield",       Joystick::GAMEPAD_TYPE_NVIDIA_SHIELD       },
	{ "joyconleft",   Joystick::GAMEPAD_TYPE_JOYCON_LEFT         },
	{ "joyconright",  Joystick::GAMEPAD_TYPE_JOYCON_RIGHT        },
	{ "joyconpair",   Joystick::GAMEPAD_TYPE_JOYCON_PAIR         },
}
STRINGMAP_CLASS_END(Joystick, Joystick::GamepadType, Joystick::GAMEPAD_TYPE_MAX_ENUM, gamepadType)

STRINGMAP_CLASS_BEGIN(Joystick, Joystick::GamepadAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM, gpAxis)
{
	{ "leftx",        Joystick::GAMEPAD_AXIS_LEFTX        },
	{ "lefty",        Joystick::GAMEPAD_AXIS_LEFTY        },
	{ "rightx",       Joystick::GAMEPAD_AXIS_RIGHTX       },
	{ "righty",       Joystick::GAMEPAD_AXIS_RIGHTY       },
	{ "triggerleft",  Joystick::GAMEPAD_AXIS_TRIGGERLEFT  },
	{ "triggerright", Joystick::GAMEPAD_AXIS_TRIGGERRIGHT },
}
STRINGMAP_CLASS_END(Joystick, Joystick::GamepadAxis, Joystick::GAMEPAD_AXIS_MAX_ENUM, gpAxis)

STRINGMAP_CLASS_BEGIN(Joystick, Joystick::GamepadButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM, gpButton)
{
	{ "a",             Joystick::GAMEPAD_BUTTON_A             },
	{ "b",             Joystick::GAMEPAD_BUTTON_B             },
	{ "x",             Joystick::GAMEPAD_BUTTON_X             },
	{ "y",             Joystick::GAMEPAD_BUTTON_Y             },
	{ "back",          Joystick::GAMEPAD_BUTTON_BACK          },
	{ "guide",         Joystick::GAMEPAD_BUTTON_GUIDE         },
	{ "start",         Joystick::GAMEPAD_BUTTON_START         },
	{ "leftstick",     Joystick::GAMEPAD_BUTTON_LEFTSTICK     },
	{ "rightstick",    Joystick::GAMEPAD_BUTTON_RIGHTSTICK    },
	{ "leftshoulder",  Joystick::GAMEPAD_BUTTON_LEFTSHOULDER  },
	{ "rightshoulder", Joystick::GAMEPAD_BUTTON_RIGHTSHOULDER },
	{ "dpup",          Joystick::GAMEPAD_BUTTON_DPAD_UP       },
	{ "dpdown",        Joystick::GAMEPAD_BUTTON_DPAD_DOWN     },
	{ "dpleft",        Joystick::GAMEPAD_BUTTON_DPAD_LEFT     },
	{ "dpright",       Joystick::GAMEPAD_BUTTON_DPAD_RIGHT    },
	{ "misc1",         Joystick::GAMEPAD_BUTTON_MISC1         },
	{ "paddle1",       Joystick::GAMEPAD_BUTTON_PADDLE1       },
	{ "paddle2",       Joystick::GAMEPAD_BUTTON_PADDLE2       },
	{ "paddle3",       Joystick::GAMEPAD_BUTTON_PADDLE3       },
	{ "paddle4",       Joystick::GAMEPAD_BUTTON_PADDLE4       },
	{ "touchpad",      Joystick::GAMEPAD_BUTTON_TOUCHPAD      },
}
STRINGMAP_CLASS_END(Joystick, Joystick::GamepadButton, Joystick::GAMEPAD_BUTTON_MAX_ENUM, gpButton)

STRINGMAP_CLASS_BEGIN(Joystick, Joystick::InputType, Joystick::INPUT_TYPE_MAX_ENUM, inputType)
{
	{ "axis",   Joystick::INPUT_TYPE_AXIS   },
	{ "button", Joystick::INPUT_TYPE_BUTTON },
	{ "hat",    Joystick::INPUT_TYPE_HAT    },
}
STRINGMAP_CLASS_END(Joystick, Joystick::InputType, Joystick::INPUT_TYPE_MAX_ENUM, inputType)

} // joystick
} // love
