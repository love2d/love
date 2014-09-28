/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

Keyboard::Keyboard()
	: key_repeat(false)
{
}

const char *Keyboard::getName() const
{
	return "love.keyboard.sdl";
}

void Keyboard::setKeyRepeat(bool enable)
{
	key_repeat = enable;
}

bool Keyboard::hasKeyRepeat() const
{
	return key_repeat;
}

bool Keyboard::isDown(Key *keylist) const
{
	const Uint8 *keystate = SDL_GetKeyboardState(nullptr);

	for (Key key = *keylist; key != KEY_MAX_ENUM; key = *(++keylist))
	{
		SDL_Scancode scancode = SDL_GetScancodeFromKey(keymap[key]);
		if (keystate[scancode])
			return true;
	}

	return false;
}

void Keyboard::setTextInput(bool enable)
{
	if (enable)
		SDL_StartTextInput();
	else
		SDL_StopTextInput();
}

bool Keyboard::hasTextInput() const
{
	return SDL_IsTextInputActive();
}

const SDL_Keycode *Keyboard::createKeyMap()
{
	// Array must be static so its lifetime continues once the function returns.
	static SDL_Keycode k[Keyboard::KEY_MAX_ENUM] = {SDLK_UNKNOWN};

	k[Keyboard::KEY_UNKNOWN] = SDLK_UNKNOWN;

	k[Keyboard::KEY_RETURN] = SDLK_RETURN;
	k[Keyboard::KEY_ESCAPE] = SDLK_ESCAPE;
	k[Keyboard::KEY_BACKSPACE] = SDLK_BACKSPACE;
	k[Keyboard::KEY_TAB] = SDLK_TAB;
	k[Keyboard::KEY_SPACE] = SDLK_SPACE;
	k[Keyboard::KEY_EXCLAIM] = SDLK_EXCLAIM;
	k[Keyboard::KEY_QUOTEDBL] = SDLK_QUOTEDBL;
	k[Keyboard::KEY_HASH] = SDLK_HASH;
	k[Keyboard::KEY_DOLLAR] = SDLK_DOLLAR;
	k[Keyboard::KEY_AMPERSAND] = SDLK_AMPERSAND;
	k[Keyboard::KEY_QUOTE] = SDLK_QUOTE;
	k[Keyboard::KEY_LEFTPAREN] = SDLK_LEFTPAREN;
	k[Keyboard::KEY_RIGHTPAREN] = SDLK_RIGHTPAREN;
	k[Keyboard::KEY_ASTERISK] = SDLK_ASTERISK;
	k[Keyboard::KEY_PLUS] = SDLK_PLUS;
	k[Keyboard::KEY_COMMA] = SDLK_COMMA;
	k[Keyboard::KEY_MINUS] = SDLK_MINUS;
	k[Keyboard::KEY_PERIOD] = SDLK_PERIOD;
	k[Keyboard::KEY_SLASH] = SDLK_SLASH;
	k[Keyboard::KEY_0] = SDLK_0;
	k[Keyboard::KEY_1] = SDLK_1;
	k[Keyboard::KEY_2] = SDLK_2;
	k[Keyboard::KEY_3] = SDLK_3;
	k[Keyboard::KEY_4] = SDLK_4;
	k[Keyboard::KEY_5] = SDLK_5;
	k[Keyboard::KEY_6] = SDLK_6;
	k[Keyboard::KEY_7] = SDLK_7;
	k[Keyboard::KEY_8] = SDLK_8;
	k[Keyboard::KEY_9] = SDLK_9;
	k[Keyboard::KEY_COLON] = SDLK_COLON;
	k[Keyboard::KEY_SEMICOLON] = SDLK_SEMICOLON;
	k[Keyboard::KEY_LESS] = SDLK_LESS;
	k[Keyboard::KEY_EQUALS] = SDLK_EQUALS;
	k[Keyboard::KEY_GREATER] = SDLK_GREATER;
	k[Keyboard::KEY_QUESTION] = SDLK_QUESTION;
	k[Keyboard::KEY_AT] = SDLK_AT;

	k[Keyboard::KEY_LEFTBRACKET] = SDLK_LEFTBRACKET;
	k[Keyboard::KEY_BACKSLASH] = SDLK_BACKSLASH;
	k[Keyboard::KEY_RIGHTBRACKET] = SDLK_RIGHTBRACKET;
	k[Keyboard::KEY_CARET] = SDLK_CARET;
	k[Keyboard::KEY_UNDERSCORE] = SDLK_UNDERSCORE;
	k[Keyboard::KEY_BACKQUOTE] = SDLK_BACKQUOTE;
	k[Keyboard::KEY_A] = SDLK_a;
	k[Keyboard::KEY_B] = SDLK_b;
	k[Keyboard::KEY_C] = SDLK_c;
	k[Keyboard::KEY_D] = SDLK_d;
	k[Keyboard::KEY_E] = SDLK_e;
	k[Keyboard::KEY_F] = SDLK_f;
	k[Keyboard::KEY_G] = SDLK_g;
	k[Keyboard::KEY_H] = SDLK_h;
	k[Keyboard::KEY_I] = SDLK_i;
	k[Keyboard::KEY_J] = SDLK_j;
	k[Keyboard::KEY_K] = SDLK_k;
	k[Keyboard::KEY_L] = SDLK_l;
	k[Keyboard::KEY_M] = SDLK_m;
	k[Keyboard::KEY_N] = SDLK_n;
	k[Keyboard::KEY_O] = SDLK_o;
	k[Keyboard::KEY_P] = SDLK_p;
	k[Keyboard::KEY_Q] = SDLK_q;
	k[Keyboard::KEY_R] = SDLK_r;
	k[Keyboard::KEY_S] = SDLK_s;
	k[Keyboard::KEY_T] = SDLK_t;
	k[Keyboard::KEY_U] = SDLK_u;
	k[Keyboard::KEY_V] = SDLK_v;
	k[Keyboard::KEY_W] = SDLK_w;
	k[Keyboard::KEY_X] = SDLK_x;
	k[Keyboard::KEY_Y] = SDLK_y;
	k[Keyboard::KEY_Z] = SDLK_z;

	k[Keyboard::KEY_CAPSLOCK] = SDLK_CAPSLOCK;

	k[Keyboard::KEY_F1] = SDLK_F1;
	k[Keyboard::KEY_F2] = SDLK_F2;
	k[Keyboard::KEY_F3] = SDLK_F3;
	k[Keyboard::KEY_F4] = SDLK_F4;
	k[Keyboard::KEY_F5] = SDLK_F5;
	k[Keyboard::KEY_F6] = SDLK_F6;
	k[Keyboard::KEY_F7] = SDLK_F7;
	k[Keyboard::KEY_F8] = SDLK_F8;
	k[Keyboard::KEY_F9] = SDLK_F9;
	k[Keyboard::KEY_F10] = SDLK_F10;
	k[Keyboard::KEY_F11] = SDLK_F11;
	k[Keyboard::KEY_F12] = SDLK_F12;

	k[Keyboard::KEY_PRINTSCREEN] = SDLK_PRINTSCREEN;
	k[Keyboard::KEY_SCROLLLOCK] = SDLK_SCROLLLOCK;
	k[Keyboard::KEY_PAUSE] = SDLK_PAUSE;
	k[Keyboard::KEY_INSERT] = SDLK_INSERT;
	k[Keyboard::KEY_HOME] = SDLK_HOME;
	k[Keyboard::KEY_PAGEUP] = SDLK_PAGEUP;
	k[Keyboard::KEY_DELETE] = SDLK_DELETE;
	k[Keyboard::KEY_END] = SDLK_END;
	k[Keyboard::KEY_PAGEDOWN] = SDLK_PAGEDOWN;
	k[Keyboard::KEY_RIGHT] = SDLK_RIGHT;
	k[Keyboard::KEY_LEFT] = SDLK_LEFT;
	k[Keyboard::KEY_DOWN] = SDLK_DOWN;
	k[Keyboard::KEY_UP] = SDLK_UP;

	k[Keyboard::KEY_NUMLOCKCLEAR] = SDLK_NUMLOCKCLEAR;
	k[Keyboard::KEY_KP_DIVIDE] = SDLK_KP_DIVIDE;
	k[Keyboard::KEY_KP_MULTIPLY] = SDLK_KP_MULTIPLY;
	k[Keyboard::KEY_KP_MINUS] = SDLK_KP_MINUS;
	k[Keyboard::KEY_KP_PLUS] = SDLK_KP_PLUS;
	k[Keyboard::KEY_KP_ENTER] = SDLK_KP_ENTER;
	k[Keyboard::KEY_KP_0] = SDLK_KP_0;
	k[Keyboard::KEY_KP_1] = SDLK_KP_1;
	k[Keyboard::KEY_KP_2] = SDLK_KP_2;
	k[Keyboard::KEY_KP_3] = SDLK_KP_3;
	k[Keyboard::KEY_KP_4] = SDLK_KP_4;
	k[Keyboard::KEY_KP_5] = SDLK_KP_5;
	k[Keyboard::KEY_KP_6] = SDLK_KP_6;
	k[Keyboard::KEY_KP_7] = SDLK_KP_7;
	k[Keyboard::KEY_KP_8] = SDLK_KP_8;
	k[Keyboard::KEY_KP_9] = SDLK_KP_9;
	k[Keyboard::KEY_KP_PERIOD] = SDLK_KP_PERIOD;
	k[Keyboard::KEY_KP_COMMA] = SDLK_KP_COMMA;
	k[Keyboard::KEY_KP_EQUALS] = SDLK_KP_EQUALS;

	k[Keyboard::KEY_APPLICATION] = SDLK_APPLICATION;
	k[Keyboard::KEY_POWER] = SDLK_POWER;
	k[Keyboard::KEY_F13] = SDLK_F13;
	k[Keyboard::KEY_F14] = SDLK_F14;
	k[Keyboard::KEY_F15] = SDLK_F15;
	k[Keyboard::KEY_F16] = SDLK_F16;
	k[Keyboard::KEY_F17] = SDLK_F17;
	k[Keyboard::KEY_F18] = SDLK_F18;
	k[Keyboard::KEY_F19] = SDLK_F19;
	k[Keyboard::KEY_F20] = SDLK_F20;
	k[Keyboard::KEY_F21] = SDLK_F21;
	k[Keyboard::KEY_F22] = SDLK_F22;
	k[Keyboard::KEY_F23] = SDLK_F23;
	k[Keyboard::KEY_F24] = SDLK_F24;
	k[Keyboard::KEY_EXECUTE] = SDLK_EXECUTE;
	k[Keyboard::KEY_HELP] = SDLK_HELP;
	k[Keyboard::KEY_MENU] = SDLK_MENU;
	k[Keyboard::KEY_SELECT] = SDLK_SELECT;
	k[Keyboard::KEY_STOP] = SDLK_STOP;
	k[Keyboard::KEY_AGAIN] = SDLK_AGAIN;
	k[Keyboard::KEY_UNDO] = SDLK_UNDO;
	k[Keyboard::KEY_CUT] = SDLK_CUT;
	k[Keyboard::KEY_COPY] = SDLK_COPY;
	k[Keyboard::KEY_PASTE] = SDLK_PASTE;
	k[Keyboard::KEY_FIND] = SDLK_FIND;
	k[Keyboard::KEY_MUTE] = SDLK_MUTE;
	k[Keyboard::KEY_VOLUMEUP] = SDLK_VOLUMEUP;
	k[Keyboard::KEY_VOLUMEDOWN] = SDLK_VOLUMEDOWN;

	k[Keyboard::KEY_ALTERASE] = SDLK_ALTERASE;
	k[Keyboard::KEY_SYSREQ] = SDLK_SYSREQ;
	k[Keyboard::KEY_CANCEL] = SDLK_CANCEL;
	k[Keyboard::KEY_CLEAR] = SDLK_CLEAR;
	k[Keyboard::KEY_PRIOR] = SDLK_PRIOR;
	k[Keyboard::KEY_RETURN2] = SDLK_RETURN2;
	k[Keyboard::KEY_SEPARATOR] = SDLK_SEPARATOR;
	k[Keyboard::KEY_OUT] = SDLK_OUT;
	k[Keyboard::KEY_OPER] = SDLK_OPER;
	k[Keyboard::KEY_CLEARAGAIN] = SDLK_CLEARAGAIN;

	k[Keyboard::KEY_THOUSANDSSEPARATOR] = SDLK_THOUSANDSSEPARATOR;
	k[Keyboard::KEY_DECIMALSEPARATOR] = SDLK_DECIMALSEPARATOR;
	k[Keyboard::KEY_CURRENCYUNIT] = SDLK_CURRENCYUNIT;
	k[Keyboard::KEY_CURRENCYSUBUNIT] = SDLK_CURRENCYSUBUNIT;

	k[Keyboard::KEY_LCTRL] = SDLK_LCTRL;
	k[Keyboard::KEY_LSHIFT] = SDLK_LSHIFT;
	k[Keyboard::KEY_LALT] = SDLK_LALT;
	k[Keyboard::KEY_LGUI] = SDLK_LGUI;
	k[Keyboard::KEY_RCTRL] = SDLK_RCTRL;
	k[Keyboard::KEY_RSHIFT] = SDLK_RSHIFT;
	k[Keyboard::KEY_RALT] = SDLK_RALT;
	k[Keyboard::KEY_RGUI] = SDLK_RGUI;

	k[Keyboard::KEY_MODE] = SDLK_MODE;

	k[Keyboard::KEY_AUDIONEXT] = SDLK_AUDIONEXT;
	k[Keyboard::KEY_AUDIOPREV] = SDLK_AUDIOPREV;
	k[Keyboard::KEY_AUDIOSTOP] = SDLK_AUDIOSTOP;
	k[Keyboard::KEY_AUDIOPLAY] = SDLK_AUDIOPLAY;
	k[Keyboard::KEY_AUDIOMUTE] = SDLK_AUDIOMUTE;
	k[Keyboard::KEY_MEDIASELECT] = SDLK_MEDIASELECT;
	k[Keyboard::KEY_WWW] = SDLK_WWW;
	k[Keyboard::KEY_MAIL] = SDLK_MAIL;
	k[Keyboard::KEY_CALCULATOR] = SDLK_CALCULATOR;
	k[Keyboard::KEY_COMPUTER] = SDLK_COMPUTER;
	k[Keyboard::KEY_APP_SEARCH] = SDLK_AC_SEARCH;
	k[Keyboard::KEY_APP_HOME] = SDLK_AC_HOME;
	k[Keyboard::KEY_APP_BACK] = SDLK_AC_BACK;
	k[Keyboard::KEY_APP_FORWARD] = SDLK_AC_FORWARD;
	k[Keyboard::KEY_APP_STOP] = SDLK_AC_STOP;
	k[Keyboard::KEY_APP_REFRESH] = SDLK_AC_REFRESH;
	k[Keyboard::KEY_APP_BOOKMARKS] = SDLK_AC_BOOKMARKS;

	k[Keyboard::KEY_BRIGHTNESSDOWN] = SDLK_BRIGHTNESSDOWN;
	k[Keyboard::KEY_BRIGHTNESSUP] = SDLK_BRIGHTNESSUP;
	k[Keyboard::KEY_DISPLAYSWITCH] = SDLK_DISPLAYSWITCH;
	k[Keyboard::KEY_KBDILLUMTOGGLE] = SDLK_KBDILLUMTOGGLE;
	k[Keyboard::KEY_KBDILLUMDOWN] = SDLK_KBDILLUMDOWN;
	k[Keyboard::KEY_KBDILLUMUP] = SDLK_KBDILLUMUP;
	k[Keyboard::KEY_EJECT] = SDLK_EJECT;
	k[Keyboard::KEY_SLEEP] = SDLK_SLEEP;

	return k;
}

const SDL_Keycode *Keyboard::keymap = Keyboard::createKeyMap();

} // sdl
} // keyboard
} // love
