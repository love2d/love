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

#ifndef LOVE_KEYBOARD_KEYBOARD_H
#define LOVE_KEYBOARD_KEYBOARD_H

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"

namespace love
{
namespace keyboard
{

class Keyboard : public Module
{
public:

	enum Key
	{
		KEY_UNKNOWN,
		KEY_BACKSPACE,
		KEY_TAB,
		KEY_CLEAR,
		KEY_RETURN,
		KEY_PAUSE,
		KEY_ESCAPE,
		KEY_SPACE,
		KEY_EXCLAIM,
		KEY_QUOTEDBL,
		KEY_HASH,
		KEY_DOLLAR,
		KEY_AMPERSAND,
		KEY_QUOTE,
		KEY_LEFTPAREN,
		KEY_RIGHTPAREN,
		KEY_ASTERISK,
		KEY_PLUS,
		KEY_COMMA,
		KEY_MINUS,
		KEY_PERIOD,
		KEY_SLASH,
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_COLON,
		KEY_SEMICOLON,
		KEY_LESS,
		KEY_EQUALS,
		KEY_GREATER,
		KEY_QUESTION,
		KEY_AT,

		KEY_LEFTBRACKET,
		KEY_BACKSLASH,
		KEY_RIGHTBRACKET,
		KEY_CARET,
		KEY_UNDERSCORE,
		KEY_BACKQUOTE,
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_DELETE,

		KEY_KP0,
		KEY_KP1,
		KEY_KP2,
		KEY_KP3,
		KEY_KP4,
		KEY_KP5,
		KEY_KP6,
		KEY_KP7,
		KEY_KP8,
		KEY_KP9,
		KEY_KP_PERIOD,
		KEY_KP_DIVIDE,
		KEY_KP_MULTIPLY,
		KEY_KP_MINUS,
		KEY_KP_PLUS,
		KEY_KP_ENTER,
		KEY_KP_EQUALS,

		KEY_UP,
		KEY_DOWN,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_INSERT,
		KEY_HOME,
		KEY_END,
		KEY_PAGEUP,
		KEY_PAGEDOWN,

		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,

		KEY_NUMLOCK,
		KEY_CAPSLOCK,
		KEY_SCROLLOCK,
		KEY_RSHIFT,
		KEY_LSHIFT,
		KEY_RCTRL,
		KEY_LCTRL,
		KEY_RALT,
		KEY_LALT,
		KEY_RMETA,
		KEY_LMETA,
		KEY_LSUPER,
		KEY_RSUPER,
		KEY_MODE,
		KEY_COMPOSE,

		KEY_HELP,
		KEY_PRINT,
		KEY_SYSREQ,
		KEY_BREAK,
		KEY_MENU,
		KEY_POWER,
		KEY_EURO,
		KEY_UNDO,
		KEY_MAX_ENUM = 512
	};

	static const int DEFAULT = -1;

	virtual ~Keyboard() {}

	/**
	 * Checks whether a certain key is down or not.
	 * @param key A key identifier.
	 * @return boolean
	 **/
	virtual bool isDown(Key *keylist) const = 0;

	/**
	 * Enables key repeating.
	 * @param delay The amount of delay before repeating the key (in milliseconds)
	 * @param interval Specifies the amount of time between repeats (in milliseconds)
	 **/
	virtual void setKeyRepeat(int delay, int interval) const = 0;

	/**
	 * Gets the specified delay for the key repeat.
	 * @return int
	 **/
	virtual int getKeyRepeatDelay() const = 0;

	/**
	 * Gets the specified interval for the key repeat.
	 * @return int
	 **/
	virtual int getKeyRepeatInterval() const = 0;

	static bool getConstant(const char *in, Key &out);
	static bool getConstant(Key in, const char  *&out);

private:

	static StringMap<Key, KEY_MAX_ENUM>::Entry keyEntries[];
	static StringMap<Key, KEY_MAX_ENUM> keys;

}; // Keyboard

} // keyboard
} // love

#endif // LOVE_KEYBOARD_KEYBOARD_H
