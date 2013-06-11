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

#include "Event.h"

#include "keyboard/Keyboard.h"
#include "mouse/Mouse.h"

#include <cmath>

namespace love
{
namespace event
{
namespace sdl
{

const char *Event::getName() const
{
	return "love.event.sdl";
}

Event::Event()
{
	SDL_EnableUNICODE(1);
}

void Event::pump()
{
	SDL_PumpEvents();

	static SDL_Event e;
	SDL_EnableUNICODE(1);

	Message *msg;

	while (SDL_PollEvent(&e))
	{
		msg = convert(e);
		if (msg)
		{
			push(msg);
			msg->release();
		}
	}
}

Message *Event::wait()
{
	static SDL_Event e;
	bool ok = (SDL_WaitEvent(&e) == 1);
	if (!ok)
		return NULL;
	return convert(e);
}

void Event::clear()
{
	static SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		// Do nothing with 'e' ...
	}

	love::event::Event::clear();
}

Message *Event::convert(SDL_Event &e)
{
	Message *msg = NULL;
	love::keyboard::Keyboard::Key key;
	love::mouse::Mouse::Button button;
	love::joystick::Joystick::Hat hat;
	Variant *arg1, *arg2, *arg3, *arg4;
	const char *txt;
	switch (e.type)
	{
	case SDL_KEYDOWN:
		if (!keys.find(e.key.keysym.sym, key))
			key = love::keyboard::Keyboard::KEY_UNKNOWN;
		if (!love::keyboard::Keyboard::getConstant(key, txt))
			txt = "unknown";
		arg1 = new Variant(txt, strlen(txt));
		arg2 = new Variant((double) e.key.keysym.unicode);
		msg = new Message("keypressed", arg1, arg2);
		arg1->release();
		arg2->release();
		break;
	case SDL_KEYUP:
		if (!keys.find(e.key.keysym.sym, key))
			key = love::keyboard::Keyboard::KEY_UNKNOWN;
		if (!love::keyboard::Keyboard::getConstant(key, txt))
			txt = "unknown";
		arg1 = new Variant(txt, strlen(txt));
		msg = new Message("keyreleased", arg1);
		arg1->release();
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if (buttons.find(e.button.button, button) && love::mouse::Mouse::getConstant(button, txt))
		{
			arg1 = new Variant((double) e.button.x);
			arg2 = new Variant((double) e.button.y);
			arg3 = new Variant(txt, strlen(txt));
			msg = new Message((e.type == SDL_MOUSEBUTTONDOWN) ?
							  "mousepressed" : "mousereleased",
							  arg1, arg2, arg3);
			arg1->release();
			arg2->release();
			arg3->release();
		}
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		arg1 = new Variant((double)(e.jbutton.which+1));
		arg2 = new Variant((double)(e.jbutton.button+1));
		msg = new Message((e.type == SDL_JOYBUTTONDOWN) ?
						  "joystickpressed" : "joystickreleased",
						  arg1, arg2);
		arg1->release();
		arg2->release();
		break;
	case SDL_JOYAXISMOTION:
		{
			arg1 = new Variant((double)(e.jaxis.which+1));
			arg2 = new Variant((double)(e.jaxis.axis+1));
			float value = e.jaxis.value / 32768.0f;
			if (fabsf(value) < 0.001f) value = 0.0f;
			if (value < -0.99f) value = -1.0f;
			if (value > 0.99f) value = 1.0f;
			arg3 = new Variant((double) value);
			msg = new Message("joystickaxis", arg1, arg2, arg3);
			arg1->release();
			arg2->release();
			arg3->release();
		}
		break;
	case SDL_JOYBALLMOTION:
		arg1 = new Variant((double)(e.jball.which+1));
		arg2 = new Variant((double)(e.jball.ball+1));
		arg3 = new Variant((double)e.jball.xrel);
		arg4 = new Variant((double)e.jball.yrel);
		msg = new Message("joystickball", arg1, arg2, arg3, arg4);
		arg1->release();
		arg2->release();
		arg3->release();
		arg4->release();
		break;
	case SDL_JOYHATMOTION:
		if (hats.find(e.jhat.value, hat) && love::joystick::Joystick::getConstant(hat, txt))
		{
			arg1 = new Variant((double)(e.jhat.which+1));
			arg2 = new Variant((double)(e.jhat.hat+1));
			arg3 = new Variant(txt, strlen(txt));
			msg = new Message("joystickhat", arg1, arg2, arg3);
			arg1->release();
			arg2->release();
			arg3->release();
		}
		break;
	case SDL_ACTIVEEVENT:
		arg1 = new Variant(e.active.gain != 0);
		if (e.active.state & SDL_APPINPUTFOCUS)
			msg = new Message("focus", arg1);
		else if (e.active.state & SDL_APPMOUSEFOCUS)
			msg = new Message("mousefocus", arg1);
		else if (e.active.state & SDL_APPACTIVE)
			msg = new Message("visible", arg1);
		arg1->release();
		break;
	case SDL_QUIT:
		msg = new Message("quit");
		break;
	case SDL_VIDEORESIZE:
		arg1 = new Variant((double) e.resize.w);
		arg2 = new Variant((double) e.resize.h);
		msg = new Message("resize", arg1, arg2);
		arg1->release();
		arg2->release();
		break;
	}

	return msg;
}

EnumMap<love::keyboard::Keyboard::Key, SDLKey, love::keyboard::Keyboard::KEY_MAX_ENUM>::Entry Event::keyEntries[] =
{
	{ love::keyboard::Keyboard::KEY_BACKSPACE, SDLK_BACKSPACE},
	{ love::keyboard::Keyboard::KEY_TAB, SDLK_TAB},
	{ love::keyboard::Keyboard::KEY_CLEAR, SDLK_CLEAR},
	{ love::keyboard::Keyboard::KEY_RETURN, SDLK_RETURN},
	{ love::keyboard::Keyboard::KEY_PAUSE, SDLK_PAUSE},
	{ love::keyboard::Keyboard::KEY_ESCAPE, SDLK_ESCAPE },
	{ love::keyboard::Keyboard::KEY_SPACE, SDLK_SPACE },
	{ love::keyboard::Keyboard::KEY_EXCLAIM, SDLK_EXCLAIM },
	{ love::keyboard::Keyboard::KEY_QUOTEDBL, SDLK_QUOTEDBL },
	{ love::keyboard::Keyboard::KEY_HASH, SDLK_HASH },
	{ love::keyboard::Keyboard::KEY_DOLLAR, SDLK_DOLLAR },
	{ love::keyboard::Keyboard::KEY_AMPERSAND, SDLK_AMPERSAND },
	{ love::keyboard::Keyboard::KEY_QUOTE, SDLK_QUOTE },
	{ love::keyboard::Keyboard::KEY_LEFTPAREN, SDLK_LEFTPAREN },
	{ love::keyboard::Keyboard::KEY_RIGHTPAREN, SDLK_RIGHTPAREN },
	{ love::keyboard::Keyboard::KEY_ASTERISK, SDLK_ASTERISK },
	{ love::keyboard::Keyboard::KEY_PLUS, SDLK_PLUS },
	{ love::keyboard::Keyboard::KEY_COMMA, SDLK_COMMA },
	{ love::keyboard::Keyboard::KEY_MINUS, SDLK_MINUS },
	{ love::keyboard::Keyboard::KEY_PERIOD, SDLK_PERIOD },
	{ love::keyboard::Keyboard::KEY_SLASH, SDLK_SLASH },
	{ love::keyboard::Keyboard::KEY_0, SDLK_0 },
	{ love::keyboard::Keyboard::KEY_1, SDLK_1 },
	{ love::keyboard::Keyboard::KEY_2, SDLK_2 },
	{ love::keyboard::Keyboard::KEY_3, SDLK_3 },
	{ love::keyboard::Keyboard::KEY_4, SDLK_4 },
	{ love::keyboard::Keyboard::KEY_5, SDLK_5 },
	{ love::keyboard::Keyboard::KEY_6, SDLK_6 },
	{ love::keyboard::Keyboard::KEY_7, SDLK_7 },
	{ love::keyboard::Keyboard::KEY_8, SDLK_8 },
	{ love::keyboard::Keyboard::KEY_9, SDLK_9 },
	{ love::keyboard::Keyboard::KEY_COLON, SDLK_COLON },
	{ love::keyboard::Keyboard::KEY_SEMICOLON, SDLK_SEMICOLON },
	{ love::keyboard::Keyboard::KEY_LESS, SDLK_LESS },
	{ love::keyboard::Keyboard::KEY_EQUALS, SDLK_EQUALS },
	{ love::keyboard::Keyboard::KEY_GREATER, SDLK_GREATER },
	{ love::keyboard::Keyboard::KEY_QUESTION, SDLK_QUESTION },
	{ love::keyboard::Keyboard::KEY_AT, SDLK_AT },

	{ love::keyboard::Keyboard::KEY_LEFTBRACKET, SDLK_LEFTBRACKET },
	{ love::keyboard::Keyboard::KEY_BACKSLASH, SDLK_BACKSLASH },
	{ love::keyboard::Keyboard::KEY_RIGHTBRACKET, SDLK_RIGHTBRACKET },
	{ love::keyboard::Keyboard::KEY_CARET, SDLK_CARET },
	{ love::keyboard::Keyboard::KEY_UNDERSCORE, SDLK_UNDERSCORE },
	{ love::keyboard::Keyboard::KEY_BACKQUOTE, SDLK_BACKQUOTE },
	{ love::keyboard::Keyboard::KEY_A, SDLK_a },
	{ love::keyboard::Keyboard::KEY_B, SDLK_b },
	{ love::keyboard::Keyboard::KEY_C, SDLK_c },
	{ love::keyboard::Keyboard::KEY_D, SDLK_d },
	{ love::keyboard::Keyboard::KEY_E, SDLK_e },
	{ love::keyboard::Keyboard::KEY_F, SDLK_f },
	{ love::keyboard::Keyboard::KEY_G, SDLK_g },
	{ love::keyboard::Keyboard::KEY_H, SDLK_h },
	{ love::keyboard::Keyboard::KEY_I, SDLK_i },
	{ love::keyboard::Keyboard::KEY_J, SDLK_j },
	{ love::keyboard::Keyboard::KEY_K, SDLK_k },
	{ love::keyboard::Keyboard::KEY_L, SDLK_l },
	{ love::keyboard::Keyboard::KEY_M, SDLK_m },
	{ love::keyboard::Keyboard::KEY_N, SDLK_n },
	{ love::keyboard::Keyboard::KEY_O, SDLK_o },
	{ love::keyboard::Keyboard::KEY_P, SDLK_p },
	{ love::keyboard::Keyboard::KEY_Q, SDLK_q },
	{ love::keyboard::Keyboard::KEY_R, SDLK_r },
	{ love::keyboard::Keyboard::KEY_S, SDLK_s },
	{ love::keyboard::Keyboard::KEY_T, SDLK_t },
	{ love::keyboard::Keyboard::KEY_U, SDLK_u },
	{ love::keyboard::Keyboard::KEY_V, SDLK_v },
	{ love::keyboard::Keyboard::KEY_W, SDLK_w },
	{ love::keyboard::Keyboard::KEY_X, SDLK_x },
	{ love::keyboard::Keyboard::KEY_Y, SDLK_y },
	{ love::keyboard::Keyboard::KEY_Z, SDLK_z },
	{ love::keyboard::Keyboard::KEY_DELETE, SDLK_DELETE },

	{ love::keyboard::Keyboard::KEY_KP0, SDLK_KP0 },
	{ love::keyboard::Keyboard::KEY_KP1, SDLK_KP1 },
	{ love::keyboard::Keyboard::KEY_KP2, SDLK_KP2 },
	{ love::keyboard::Keyboard::KEY_KP3, SDLK_KP3 },
	{ love::keyboard::Keyboard::KEY_KP4, SDLK_KP4 },
	{ love::keyboard::Keyboard::KEY_KP5, SDLK_KP5 },
	{ love::keyboard::Keyboard::KEY_KP6, SDLK_KP6 },
	{ love::keyboard::Keyboard::KEY_KP7, SDLK_KP7 },
	{ love::keyboard::Keyboard::KEY_KP8, SDLK_KP8 },
	{ love::keyboard::Keyboard::KEY_KP9, SDLK_KP9 },
	{ love::keyboard::Keyboard::KEY_KP_PERIOD, SDLK_KP_PERIOD },
	{ love::keyboard::Keyboard::KEY_KP_DIVIDE, SDLK_KP_DIVIDE },
	{ love::keyboard::Keyboard::KEY_KP_MULTIPLY, SDLK_KP_MULTIPLY},
	{ love::keyboard::Keyboard::KEY_KP_MINUS, SDLK_KP_MINUS },
	{ love::keyboard::Keyboard::KEY_KP_PLUS, SDLK_KP_PLUS },
	{ love::keyboard::Keyboard::KEY_KP_ENTER, SDLK_KP_ENTER },
	{ love::keyboard::Keyboard::KEY_KP_EQUALS, SDLK_KP_EQUALS },

	{ love::keyboard::Keyboard::KEY_UP, SDLK_UP },
	{ love::keyboard::Keyboard::KEY_DOWN, SDLK_DOWN },
	{ love::keyboard::Keyboard::KEY_RIGHT, SDLK_RIGHT },
	{ love::keyboard::Keyboard::KEY_LEFT, SDLK_LEFT },
	{ love::keyboard::Keyboard::KEY_INSERT, SDLK_INSERT },
	{ love::keyboard::Keyboard::KEY_HOME, SDLK_HOME },
	{ love::keyboard::Keyboard::KEY_END, SDLK_END },
	{ love::keyboard::Keyboard::KEY_PAGEUP, SDLK_PAGEUP },
	{ love::keyboard::Keyboard::KEY_PAGEDOWN, SDLK_PAGEDOWN },

	{ love::keyboard::Keyboard::KEY_F1, SDLK_F1 },
	{ love::keyboard::Keyboard::KEY_F2, SDLK_F2 },
	{ love::keyboard::Keyboard::KEY_F3, SDLK_F3 },
	{ love::keyboard::Keyboard::KEY_F4, SDLK_F4 },
	{ love::keyboard::Keyboard::KEY_F5, SDLK_F5 },
	{ love::keyboard::Keyboard::KEY_F6, SDLK_F6 },
	{ love::keyboard::Keyboard::KEY_F7, SDLK_F7 },
	{ love::keyboard::Keyboard::KEY_F8, SDLK_F8 },
	{ love::keyboard::Keyboard::KEY_F9, SDLK_F9 },
	{ love::keyboard::Keyboard::KEY_F10, SDLK_F10 },
	{ love::keyboard::Keyboard::KEY_F11, SDLK_F11 },
	{ love::keyboard::Keyboard::KEY_F12, SDLK_F12 },
	{ love::keyboard::Keyboard::KEY_F13, SDLK_F13 },
	{ love::keyboard::Keyboard::KEY_F14, SDLK_F14 },
	{ love::keyboard::Keyboard::KEY_F15, SDLK_F15 },

	{ love::keyboard::Keyboard::KEY_NUMLOCK, SDLK_NUMLOCK },
	{ love::keyboard::Keyboard::KEY_CAPSLOCK, SDLK_CAPSLOCK },
	{ love::keyboard::Keyboard::KEY_SCROLLOCK, SDLK_SCROLLOCK },
	{ love::keyboard::Keyboard::KEY_RSHIFT, SDLK_RSHIFT },
	{ love::keyboard::Keyboard::KEY_LSHIFT, SDLK_LSHIFT },
	{ love::keyboard::Keyboard::KEY_RCTRL, SDLK_RCTRL },
	{ love::keyboard::Keyboard::KEY_LCTRL, SDLK_LCTRL },
	{ love::keyboard::Keyboard::KEY_RALT, SDLK_RALT },
	{ love::keyboard::Keyboard::KEY_LALT, SDLK_LALT },
	{ love::keyboard::Keyboard::KEY_RMETA, SDLK_RMETA },
	{ love::keyboard::Keyboard::KEY_LMETA, SDLK_LMETA },
	{ love::keyboard::Keyboard::KEY_LSUPER, SDLK_LSUPER },
	{ love::keyboard::Keyboard::KEY_RSUPER, SDLK_RSUPER },
	{ love::keyboard::Keyboard::KEY_MODE, SDLK_MODE },
	{ love::keyboard::Keyboard::KEY_COMPOSE, SDLK_COMPOSE },

	{ love::keyboard::Keyboard::KEY_HELP, SDLK_HELP },
	{ love::keyboard::Keyboard::KEY_PRINT, SDLK_PRINT },
	{ love::keyboard::Keyboard::KEY_SYSREQ, SDLK_SYSREQ },
	{ love::keyboard::Keyboard::KEY_BREAK, SDLK_BREAK },
	{ love::keyboard::Keyboard::KEY_MENU, SDLK_MENU },
	{ love::keyboard::Keyboard::KEY_POWER, SDLK_POWER },
	{ love::keyboard::Keyboard::KEY_EURO, SDLK_EURO },
	{ love::keyboard::Keyboard::KEY_UNDO, SDLK_UNDO },

	{ love::keyboard::Keyboard::KEY_UNKNOWN, SDLK_UNKNOWN },
};

EnumMap<love::keyboard::Keyboard::Key, SDLKey, love::keyboard::Keyboard::KEY_MAX_ENUM> Event::keys(Event::keyEntries, sizeof(Event::keyEntries));

EnumMap<love::mouse::Mouse::Button, Uint8, love::mouse::Mouse::BUTTON_MAX_ENUM>::Entry Event::buttonEntries[] =
{
	{ love::mouse::Mouse::BUTTON_LEFT, SDL_BUTTON_LEFT},
	{ love::mouse::Mouse::BUTTON_MIDDLE, SDL_BUTTON_MIDDLE},
	{ love::mouse::Mouse::BUTTON_RIGHT, SDL_BUTTON_RIGHT},
	{ love::mouse::Mouse::BUTTON_WHEELUP, SDL_BUTTON_WHEELUP},
	{ love::mouse::Mouse::BUTTON_WHEELDOWN, SDL_BUTTON_WHEELDOWN},
	{ love::mouse::Mouse::BUTTON_X1, SDL_BUTTON_X1},
	{ love::mouse::Mouse::BUTTON_X2, SDL_BUTTON_X2},
};

EnumMap<love::mouse::Mouse::Button, Uint8, love::mouse::Mouse::BUTTON_MAX_ENUM> Event::buttons(Event::buttonEntries, sizeof(Event::buttonEntries));

EnumMap<love::joystick::Joystick::Hat, Uint8, love::joystick::Joystick::HAT_MAX_ENUM>::Entry Event::hatEntries[] =
{
	{love::joystick::Joystick::HAT_CENTERED, SDL_HAT_CENTERED},
	{love::joystick::Joystick::HAT_UP, SDL_HAT_UP},
	{love::joystick::Joystick::HAT_RIGHT, SDL_HAT_RIGHT},
	{love::joystick::Joystick::HAT_DOWN, SDL_HAT_DOWN},
	{love::joystick::Joystick::HAT_LEFT, SDL_HAT_LEFT},
	{love::joystick::Joystick::HAT_RIGHTUP, SDL_HAT_RIGHTUP},
	{love::joystick::Joystick::HAT_RIGHTDOWN, SDL_HAT_RIGHTDOWN},
	{love::joystick::Joystick::HAT_LEFTUP, SDL_HAT_LEFTUP},
	{love::joystick::Joystick::HAT_LEFTDOWN, SDL_HAT_LEFTDOWN},
};

EnumMap<love::joystick::Joystick::Hat, Uint8, love::joystick::Joystick::HAT_MAX_ENUM> Event::hats(Event::hatEntries, sizeof(Event::hatEntries));

} // sdl
} // event
} // love
