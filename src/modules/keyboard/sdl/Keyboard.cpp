/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "Keyboard.h"

namespace love
{
namespace keyboard
{
namespace sdl
{
	const char * Keyboard::getName() const
	{
		return "love.keyboard.sdl";
	}

	bool Keyboard::isDown(Key * keylist) const
	{
		SDLKey k;
		Uint8 * keystate = SDL_GetKeyState(0);
		
		for (Key key = *keylist; key != KEY_MAX_ENUM; key = *(++keylist))
		{
			if (keys.find(key, k) && keystate[(unsigned)k] == 1)
				return true;
		}

		return false;
	}

	void Keyboard::setKeyRepeat(int delay, int interval) const
	{
		delay = (delay == DEFAULT) ? SDL_DEFAULT_REPEAT_DELAY : delay;
		interval = (interval == DEFAULT) ? SDL_DEFAULT_REPEAT_INTERVAL : interval;

		SDL_EnableKeyRepeat(delay, interval);
	}

	int Keyboard::getKeyRepeatDelay() const
	{
		int delay, interval = 0;
		SDL_GetKeyRepeat(&delay, &interval);
		return delay;
	}

	int Keyboard::getKeyRepeatInterval() const
	{
		int delay, interval = 0;
		SDL_GetKeyRepeat(&delay, &interval);
		return interval;
	}

	EnumMap<Keyboard::Key, SDLKey, Keyboard::KEY_MAX_ENUM>::Entry Keyboard::keyEntries[] =
	{
		{ Keyboard::KEY_BACKSPACE, SDLK_BACKSPACE},
		{ Keyboard::KEY_TAB, SDLK_TAB},
		{ Keyboard::KEY_CLEAR, SDLK_CLEAR},
		{ Keyboard::KEY_RETURN, SDLK_RETURN},
		{ Keyboard::KEY_PAUSE, SDLK_PAUSE},
		{ Keyboard::KEY_ESCAPE, SDLK_ESCAPE },
		{ Keyboard::KEY_SPACE, SDLK_SPACE },
		{ Keyboard::KEY_EXCLAIM, SDLK_EXCLAIM },
		{ Keyboard::KEY_QUOTEDBL, SDLK_QUOTEDBL },
		{ Keyboard::KEY_HASH, SDLK_HASH },
		{ Keyboard::KEY_DOLLAR, SDLK_DOLLAR },
		{ Keyboard::KEY_AMPERSAND, SDLK_AMPERSAND },
		{ Keyboard::KEY_QUOTE, SDLK_QUOTE },
		{ Keyboard::KEY_LEFTPAREN, SDLK_LEFTPAREN },
		{ Keyboard::KEY_RIGHTPAREN, SDLK_RIGHTPAREN },
		{ Keyboard::KEY_ASTERISK, SDLK_ASTERISK },
		{ Keyboard::KEY_PLUS, SDLK_PLUS },
		{ Keyboard::KEY_COMMA, SDLK_COMMA },
		{ Keyboard::KEY_MINUS, SDLK_MINUS },
		{ Keyboard::KEY_PERIOD, SDLK_PERIOD },
		{ Keyboard::KEY_SLASH, SDLK_SLASH },
		{ Keyboard::KEY_0, SDLK_0 },
		{ Keyboard::KEY_1, SDLK_1 },
		{ Keyboard::KEY_2, SDLK_2 },
		{ Keyboard::KEY_3, SDLK_3 },
		{ Keyboard::KEY_4, SDLK_4 },
		{ Keyboard::KEY_5, SDLK_5 },
		{ Keyboard::KEY_6, SDLK_6 },
		{ Keyboard::KEY_7, SDLK_7 },
		{ Keyboard::KEY_8, SDLK_8 },
		{ Keyboard::KEY_9, SDLK_9 },
		{ Keyboard::KEY_COLON, SDLK_COLON },
		{ Keyboard::KEY_SEMICOLON, SDLK_SEMICOLON },
		{ Keyboard::KEY_LESS, SDLK_LESS },
		{ Keyboard::KEY_EQUALS, SDLK_EQUALS },
		{ Keyboard::KEY_GREATER, SDLK_GREATER },
		{ Keyboard::KEY_QUESTION, SDLK_QUESTION },
		{ Keyboard::KEY_AT, SDLK_AT },

		{ Keyboard::KEY_LEFTBRACKET, SDLK_LEFTBRACKET },
		{ Keyboard::KEY_BACKSLASH, SDLK_BACKSLASH },
		{ Keyboard::KEY_RIGHTBRACKET, SDLK_RIGHTBRACKET },
		{ Keyboard::KEY_CARET, SDLK_CARET },
		{ Keyboard::KEY_UNDERSCORE, SDLK_UNDERSCORE },
		{ Keyboard::KEY_BACKQUOTE, SDLK_BACKQUOTE },
		{ Keyboard::KEY_A, SDLK_a },
		{ Keyboard::KEY_B, SDLK_b },
		{ Keyboard::KEY_C, SDLK_c },
		{ Keyboard::KEY_D, SDLK_d },
		{ Keyboard::KEY_E, SDLK_e },
		{ Keyboard::KEY_F, SDLK_f },
		{ Keyboard::KEY_G, SDLK_g },
		{ Keyboard::KEY_H, SDLK_h },
		{ Keyboard::KEY_I, SDLK_i },
		{ Keyboard::KEY_J, SDLK_j },
		{ Keyboard::KEY_K, SDLK_k },
		{ Keyboard::KEY_L, SDLK_l },
		{ Keyboard::KEY_M, SDLK_m },
		{ Keyboard::KEY_N, SDLK_n },
		{ Keyboard::KEY_O, SDLK_o },
		{ Keyboard::KEY_P, SDLK_p },
		{ Keyboard::KEY_Q, SDLK_q },
		{ Keyboard::KEY_R, SDLK_r },
		{ Keyboard::KEY_S, SDLK_s },
		{ Keyboard::KEY_T, SDLK_t },
		{ Keyboard::KEY_U, SDLK_u },
		{ Keyboard::KEY_V, SDLK_v },
		{ Keyboard::KEY_W, SDLK_w },
		{ Keyboard::KEY_X, SDLK_x },
		{ Keyboard::KEY_Y, SDLK_y },
		{ Keyboard::KEY_Z, SDLK_z },
		{ Keyboard::KEY_DELETE, SDLK_DELETE },

		{ Keyboard::KEY_KP0, SDLK_KP0 },
		{ Keyboard::KEY_KP1, SDLK_KP1 },
		{ Keyboard::KEY_KP2, SDLK_KP2 },
		{ Keyboard::KEY_KP3, SDLK_KP3 },
		{ Keyboard::KEY_KP4, SDLK_KP4 },
		{ Keyboard::KEY_KP5, SDLK_KP5 },
		{ Keyboard::KEY_KP6, SDLK_KP6 },
		{ Keyboard::KEY_KP7, SDLK_KP7 },
		{ Keyboard::KEY_KP8, SDLK_KP8 },
		{ Keyboard::KEY_KP9, SDLK_KP9 },
		{ Keyboard::KEY_KP_PERIOD, SDLK_KP_PERIOD },
		{ Keyboard::KEY_KP_DIVIDE, SDLK_KP_DIVIDE },
		{ Keyboard::KEY_KP_MULTIPLY, SDLK_KP_MULTIPLY},
		{ Keyboard::KEY_KP_MINUS, SDLK_KP_MINUS },
		{ Keyboard::KEY_KP_PLUS, SDLK_KP_PLUS },
		{ Keyboard::KEY_KP_ENTER, SDLK_KP_ENTER },
		{ Keyboard::KEY_KP_EQUALS, SDLK_KP_EQUALS },

		{ Keyboard::KEY_UP, SDLK_UP },
		{ Keyboard::KEY_DOWN, SDLK_DOWN },
		{ Keyboard::KEY_RIGHT, SDLK_RIGHT },
		{ Keyboard::KEY_LEFT, SDLK_LEFT },
		{ Keyboard::KEY_INSERT, SDLK_INSERT },
		{ Keyboard::KEY_HOME, SDLK_HOME },
		{ Keyboard::KEY_END, SDLK_END },
		{ Keyboard::KEY_PAGEUP, SDLK_PAGEUP },
		{ Keyboard::KEY_PAGEDOWN, SDLK_PAGEDOWN },

		{ Keyboard::KEY_F1, SDLK_F1 },
		{ Keyboard::KEY_F2, SDLK_F2 },
		{ Keyboard::KEY_F3, SDLK_F3 },
		{ Keyboard::KEY_F4, SDLK_F4 },
		{ Keyboard::KEY_F5, SDLK_F5 },
		{ Keyboard::KEY_F6, SDLK_F6 },
		{ Keyboard::KEY_F7, SDLK_F7 },
		{ Keyboard::KEY_F8, SDLK_F8 },
		{ Keyboard::KEY_F9, SDLK_F9 },
		{ Keyboard::KEY_F10, SDLK_F10 },
		{ Keyboard::KEY_F11, SDLK_F11 },
		{ Keyboard::KEY_F12, SDLK_F12 },
		{ Keyboard::KEY_F13, SDLK_F13 },
		{ Keyboard::KEY_F14, SDLK_F14 },
		{ Keyboard::KEY_F15, SDLK_F15 },

		{ Keyboard::KEY_NUMLOCK, SDLK_NUMLOCK },
		{ Keyboard::KEY_CAPSLOCK, SDLK_CAPSLOCK },
		{ Keyboard::KEY_SCROLLOCK, SDLK_SCROLLOCK },
		{ Keyboard::KEY_RSHIFT, SDLK_RSHIFT },
		{ Keyboard::KEY_LSHIFT, SDLK_LSHIFT },
		{ Keyboard::KEY_RCTRL, SDLK_RCTRL },
		{ Keyboard::KEY_LCTRL, SDLK_LCTRL },
		{ Keyboard::KEY_RALT, SDLK_RALT },
		{ Keyboard::KEY_LALT, SDLK_LALT },
		{ Keyboard::KEY_RMETA, SDLK_RMETA },
		{ Keyboard::KEY_LMETA, SDLK_LMETA },
		{ Keyboard::KEY_LSUPER, SDLK_LSUPER },
		{ Keyboard::KEY_RSUPER, SDLK_RSUPER },
		{ Keyboard::KEY_MODE, SDLK_MODE },
		{ Keyboard::KEY_COMPOSE, SDLK_COMPOSE },

		{ Keyboard::KEY_HELP, SDLK_HELP },
		{ Keyboard::KEY_PRINT, SDLK_PRINT },
		{ Keyboard::KEY_SYSREQ, SDLK_SYSREQ },
		{ Keyboard::KEY_BREAK, SDLK_BREAK },
		{ Keyboard::KEY_MENU, SDLK_MENU },
		{ Keyboard::KEY_POWER, SDLK_POWER },
		{ Keyboard::KEY_EURO, SDLK_EURO },
		{ Keyboard::KEY_UNDO, SDLK_UNDO },
	};

	EnumMap<Keyboard::Key, SDLKey, Keyboard::KEY_MAX_ENUM> Keyboard::keys(Keyboard::keyEntries, sizeof(Keyboard::keyEntries));


} // sdl
} // keyboard
} // love
