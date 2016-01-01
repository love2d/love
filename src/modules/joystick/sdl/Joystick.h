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

#ifndef LOVE_JOYSTICK_SDL_JOYSTICK_H
#define LOVE_JOYSTICK_SDL_JOYSTICK_H

// LOVE
#include "joystick/Joystick.h"
#include "common/EnumMap.h"

// SDL
#include <SDL.h>

namespace love
{
namespace joystick
{
namespace sdl
{

class Joystick : public love::joystick::Joystick
{
public:

	Joystick(int id);
	Joystick(int id, int joyindex);

	virtual ~Joystick();

	bool open(int deviceindex);
	void close();

	bool isConnected() const;

	const char *getName() const;

	int getAxisCount() const;
	int getButtonCount() const;
	int getHatCount() const;

	float getAxis(int axisindex) const;
	std::vector<float> getAxes() const;
	Hat getHat(int hatindex) const;

	bool isDown(const std::vector<int> &buttonlist) const;

	bool openGamepad(int deviceindex);
	bool isGamepad() const;

	float getGamepadAxis(GamepadAxis axis) const;
	bool isGamepadDown(const std::vector<GamepadButton> &blist) const;

	void *getHandle() const;

	std::string getGUID() const;
	int getInstanceID() const;
	int getID() const;

	bool isVibrationSupported();
	bool setVibration(float left, float right, float duration = -1.0f);
	bool setVibration();
	void getVibration(float &left, float &right);

	static bool getConstant(Hat in, Uint8 &out);
	static bool getConstant(Uint8 in, Hat &out);

	static bool getConstant(SDL_GameControllerAxis in, GamepadAxis &out);
	static bool getConstant(GamepadAxis in, SDL_GameControllerAxis &out);

	static bool getConstant(SDL_GameControllerButton in, GamepadButton &out);
	static bool getConstant(GamepadButton in, SDL_GameControllerButton &out);

private:

	Joystick() {}

	bool checkCreateHaptic();
	bool runVibrationEffect();

	SDL_Joystick *joyhandle;
	SDL_GameController *controller;
	SDL_Haptic *haptic;

	SDL_JoystickID instanceid;
	std::string pguid;
	int id;

	std::string name;

	struct Vibration
	{
		float left  = 0.0f;
		float right = 0.0f;
		SDL_HapticEffect effect = {};
		Uint16 data[4];
		int id = -1;
		Uint32 endtime = SDL_HAPTIC_INFINITY;

		Vibration()
			: data() // VS2013 can't initialize the array above...
		{}

	} vibration;

	static EnumMap<Hat, Uint8, Joystick::HAT_MAX_ENUM>::Entry hatEntries[];
	static EnumMap<Hat, Uint8, Joystick::HAT_MAX_ENUM> hats;

	static EnumMap<GamepadAxis, SDL_GameControllerAxis, GAMEPAD_AXIS_MAX_ENUM>::Entry gpAxisEntries[];
	static EnumMap<GamepadAxis, SDL_GameControllerAxis, GAMEPAD_AXIS_MAX_ENUM> gpAxes;

	static EnumMap<GamepadButton, SDL_GameControllerButton, GAMEPAD_BUTTON_MAX_ENUM>::Entry gpButtonEntries[];
	static EnumMap<GamepadButton, SDL_GameControllerButton, GAMEPAD_BUTTON_MAX_ENUM> gpButtons;

};

} // sdl
} // joystick
} // love

#endif // LOVE_JOYSTICK_SDL_JOYSTICK_H
