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

		KEY_RETURN,
		KEY_ESCAPE,
		KEY_BACKSPACE,
		KEY_TAB,
		KEY_SPACE,
		KEY_EXCLAIM,
		KEY_QUOTEDBL,
		KEY_HASH,
		KEY_PERCENT,
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

		KEY_CAPSLOCK,

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

		KEY_PRINTSCREEN,
		KEY_SCROLLLOCK,
		KEY_PAUSE,
		KEY_INSERT,
		KEY_HOME,
		KEY_PAGEUP,
		KEY_DELETE,
		KEY_END,
		KEY_PAGEDOWN,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,

		KEY_NUMLOCKCLEAR,
		KEY_KP_DIVIDE,
		KEY_KP_MULTIPLY,
		KEY_KP_MINUS,
		KEY_KP_PLUS,
		KEY_KP_ENTER,
		KEY_KP_1,
		KEY_KP_2,
		KEY_KP_3,
		KEY_KP_4,
		KEY_KP_5,
		KEY_KP_6,
		KEY_KP_7,
		KEY_KP_8,
		KEY_KP_9,
		KEY_KP_0,
		KEY_KP_PERIOD,
		KEY_KP_COMMA,
		KEY_KP_EQUALS,

		KEY_APPLICATION,
		KEY_POWER,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,
		KEY_F23,
		KEY_F24,
		KEY_EXECUTE,
		KEY_HELP,
		KEY_MENU,
		KEY_SELECT,
		KEY_STOP,
		KEY_AGAIN,
		KEY_UNDO,
		KEY_CUT,
		KEY_COPY,
		KEY_PASTE,
		KEY_FIND,
		KEY_MUTE,
		KEY_VOLUMEUP,
		KEY_VOLUMEDOWN,

		KEY_ALTERASE,
		KEY_SYSREQ,
		KEY_CANCEL,
		KEY_CLEAR,
		KEY_PRIOR,
		KEY_RETURN2,
		KEY_SEPARATOR,
		KEY_OUT,
		KEY_OPER,
		KEY_CLEARAGAIN,

		KEY_THOUSANDSSEPARATOR,
		KEY_DECIMALSEPARATOR,
		KEY_CURRENCYUNIT,
		KEY_CURRENCYSUBUNIT,

		KEY_LCTRL,
		KEY_LSHIFT,
		KEY_LALT,
		KEY_LGUI,
		KEY_RCTRL,
		KEY_RSHIFT,
		KEY_RALT,
		KEY_RGUI,

		KEY_MODE,

		KEY_AUDIONEXT,
		KEY_AUDIOPREV,
		KEY_AUDIOSTOP,
		KEY_AUDIOPLAY,
		KEY_AUDIOMUTE,
		KEY_MEDIASELECT,

		KEY_BRIGHTNESSDOWN,
		KEY_BRIGHTNESSUP,
		KEY_DISPLAYSWITCH,
		KEY_KBDILLUMTOGGLE,
		KEY_KBDILLUMDOWN,
		KEY_KBDILLUMUP,
		KEY_EJECT,
		KEY_SLEEP,

		KEY_MAX_ENUM = 512
	};

	virtual ~Keyboard() {}

	/**
	 * Sets whether repeat keypress events should be sent if a key is held down.
	 * Does not affect text input events.
	 * @param enable Whether to send repeat key press events.
	 **/
	virtual void setKeyRepeat(bool enable) = 0;

	/**
	 * Gets whether repeat keypress events will be sent if a key is held down.
	 **/
	virtual bool hasKeyRepeat() const = 0;

	/**
	 * Checks whether certain keys are down or not.
	 * @param keylist An array of key identifiers, terminated by KEY_MAX_ENUM.
	 * @return boolean
	 **/
	virtual bool isDown(Key *keylist) const = 0;

	/**
	 * Sets whether text input events should be sent
	 * @param enable Whether to send text input events.
	 **/
	virtual void setTextInput(bool enable) = 0;

	/**
	 * Gets whether text input events are enabled.
	 **/
	virtual bool hasTextInput() const = 0;

	static bool getConstant(const char *in, Key &out);
	static bool getConstant(Key in, const char  *&out);

private:

	static StringMap<Key, KEY_MAX_ENUM>::Entry keyEntries[];
	static StringMap<Key, KEY_MAX_ENUM> keys;

}; // Keyboard

} // keyboard
} // love

#endif // LOVE_KEYBOARD_KEYBOARD_H
