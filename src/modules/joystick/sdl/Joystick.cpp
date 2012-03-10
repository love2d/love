/**
* Copyright (c) 2006-2012 LOVE Development Team
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

// STD
#include <cmath>

namespace love
{
namespace joystick
{
namespace sdl
{
	Joystick::Joystick()
		: joysticks(0)
	{
		// Init the SDL joystick system.
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
			throw Exception(SDL_GetError());

		// Start joystick event watching.
		SDL_JoystickEventState(SDL_ENABLE);

		// Open all connected joysticks.
		int numjoysticks = this->getNumJoysticks();
		this->joysticks = (SDL_Joystick **)calloc(numjoysticks, sizeof(SDL_Joystick*));

		for (int i = 0;i<numjoysticks;i++)
			this->open(i);
	}

	Joystick::~Joystick()
	{
		// Closes any open joysticks.
		for (int i = 0; i != getNumJoysticks(); i++)
		{
			if (isOpen(i))
				close(i);
		}

		free(joysticks);

		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}

	const char * Joystick::getName() const
	{
		return "love.joystick.sdl";
	}

	bool Joystick::checkIndex(int index)
	{
		return index >= 0 && index < getNumJoysticks();
	}

	int Joystick::getNumJoysticks()
	{
		int num = SDL_NumJoysticks();
		return num < 0 ? 0 : num;
	}

	const char * Joystick::getName(int index)
	{
		return SDL_JoystickName(index);
	}

	bool Joystick::open(int index)
	{
		if (isOpen(index))
			return true;

		if (!checkIndex(index))
			return false;

		if (!(joysticks[index] = SDL_JoystickOpen(index)) )
			return false;

		return true;
	}

	bool Joystick::isOpen(int index)
	{
		if (!checkIndex(index))
			return false;

		return joysticks[index] != 0 ? true : false;
	}

	bool Joystick::verifyJoystick(int index)
	{
		if (!checkIndex(index))
			return false;

		if (!isOpen(index))
			return false;

		return true;
	}

	int Joystick::getNumAxes(int index)
	{
		return verifyJoystick(index) ? SDL_JoystickNumAxes(joysticks[index]) : 0;
	}

	int Joystick::getNumBalls(int index)
	{
		return verifyJoystick(index) ? SDL_JoystickNumBalls(joysticks[index]) : 0;
	}

	int Joystick::getNumButtons(int index)
	{
		return verifyJoystick(index) ? SDL_JoystickNumButtons(joysticks[index]) : 0;
	}

	int Joystick::getNumHats(int index)
	{
		return verifyJoystick(index) ? SDL_JoystickNumHats(joysticks[index]) : 0;
	}

	float Joystick::clampval(float x)
	{
		if (fabs((double)x) < 0.01) return 0.0f;
		if (x < -0.99f) return -1.0f;
		if (x > 0.99f) return 1.0f;
		return x;
	}

	float Joystick::getAxis(int index, int axis)
	{
		if (!verifyJoystick(index))
			return 0;

		if (axis >= getNumAxes(index))
			return 0;

		return clampval(((float)SDL_JoystickGetAxis(joysticks[index], axis))/32768.0f);
	}

	int Joystick::getAxes(lua_State * L)
	{
		love::luax_assert_argc(L, 1, 1);
		int index = (int)lua_tointeger(L, 1) - 1;

		if (!verifyJoystick(index))
			return 0;

		int num = getNumAxes(index);

		for (int i = 0; i<num; i++)
			lua_pushnumber(L, clampval(((float)SDL_JoystickGetAxis(joysticks[index], i))/32768.0f));
		return num;
	}

	int Joystick::getBall(lua_State * L)
	{
		love::luax_assert_argc(L, 2, 2);
		int index = (int)lua_tointeger(L, 1) - 1;
		int ball = (int)lua_tointeger(L, 2) - 1;

		if (!verifyJoystick(index))
			return 0;

		if (ball >= getNumBalls(index))
			return 0;

		int dx, dy;
		SDL_JoystickGetBall(joysticks[index], ball, &dx, &dy);

		lua_pushnumber(L, dx);
		lua_pushnumber(L, dy);
		return 2;
	}

	bool Joystick::isDown(int index, int * buttonlist)
	{
		if (!verifyJoystick(index))
			return false;

		int num = getNumButtons(index);

		for (int button = *buttonlist; button != -1; button = *(++buttonlist))
		{
			if (button >= 0 && button < num && SDL_JoystickGetButton(joysticks[index], button) == 1)
				return true;
		}

		return false;
	}

	Joystick::Hat Joystick::getHat(int index, int hat)
	{
		Hat h = HAT_INVALID;

		if (!verifyJoystick(index))
			return h;

		if (hat >= getNumHats(index))
			return h;

		hats.find(SDL_JoystickGetHat(joysticks[index], hat), h);

		return h;
	}

	void Joystick::close(int index)
	{
		if (!checkIndex(index))
			return;

		if (joysticks[index]!=0)
		{
			SDL_JoystickClose(joysticks[index]);
			joysticks[index] = 0;
		}
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

} // sdl
} // joystick
} // love
