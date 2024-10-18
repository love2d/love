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

#ifndef LOVE_KEYBOARD_KEYBOARD_H
#define LOVE_KEYBOARD_KEYBOARD_H

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"

// C++
#include <vector>
#include <map>

namespace love
{
namespace keyboard
{

class Keyboard : public Module
{
public:

	/**
	 * Scancodes represent physical keys independent of the current layout.
	 * Their names may not match the names of the keys printed on the keyboard.
	 * Some of them are very esoteric...
	 **/
	enum Scancode
	{
		SCANCODE_UNKNOWN,

		SCANCODE_A,
		SCANCODE_B,
		SCANCODE_C,
		SCANCODE_D,
		SCANCODE_E,
		SCANCODE_F,
		SCANCODE_G,
		SCANCODE_H,
		SCANCODE_I,
		SCANCODE_J,
		SCANCODE_K,
		SCANCODE_L,
		SCANCODE_M,
		SCANCODE_N,
		SCANCODE_O,
		SCANCODE_P,
		SCANCODE_Q,
		SCANCODE_R,
		SCANCODE_S,
		SCANCODE_T,
		SCANCODE_U,
		SCANCODE_V,
		SCANCODE_W,
		SCANCODE_X,
		SCANCODE_Y,
		SCANCODE_Z,

		SCANCODE_1,
		SCANCODE_2,
		SCANCODE_3,
		SCANCODE_4,
		SCANCODE_5,
		SCANCODE_6,
		SCANCODE_7,
		SCANCODE_8,
		SCANCODE_9,
		SCANCODE_0,

		SCANCODE_RETURN,
		SCANCODE_ESCAPE,
		SCANCODE_BACKSPACE,
		SCANCODE_TAB,
		SCANCODE_SPACE,

		SCANCODE_MINUS,
		SCANCODE_EQUALS,
		SCANCODE_LEFTBRACKET,
		SCANCODE_RIGHTBRACKET,
		SCANCODE_BACKSLASH,
		SCANCODE_NONUSHASH,
		SCANCODE_SEMICOLON,
		SCANCODE_APOSTROPHE,
		SCANCODE_GRAVE,
		SCANCODE_COMMA,
		SCANCODE_PERIOD,
		SCANCODE_SLASH,

		SCANCODE_CAPSLOCK,

		SCANCODE_F1,
		SCANCODE_F2,
		SCANCODE_F3,
		SCANCODE_F4,
		SCANCODE_F5,
		SCANCODE_F6,
		SCANCODE_F7,
		SCANCODE_F8,
		SCANCODE_F9,
		SCANCODE_F10,
		SCANCODE_F11,
		SCANCODE_F12,

		SCANCODE_PRINTSCREEN,
		SCANCODE_SCROLLLOCK,
		SCANCODE_PAUSE,
		SCANCODE_INSERT,
		SCANCODE_HOME,
		SCANCODE_PAGEUP,
		SCANCODE_DELETE,
		SCANCODE_END,
		SCANCODE_PAGEDOWN,
		SCANCODE_RIGHT,
		SCANCODE_LEFT,
		SCANCODE_DOWN,
		SCANCODE_UP,

		SCANCODE_NUMLOCKCLEAR,
		SCANCODE_KP_DIVIDE,
		SCANCODE_KP_MULTIPLY,
		SCANCODE_KP_MINUS,
		SCANCODE_KP_PLUS,
		SCANCODE_KP_ENTER,
		SCANCODE_KP_1,
		SCANCODE_KP_2,
		SCANCODE_KP_3,
		SCANCODE_KP_4,
		SCANCODE_KP_5,
		SCANCODE_KP_6,
		SCANCODE_KP_7,
		SCANCODE_KP_8,
		SCANCODE_KP_9,
		SCANCODE_KP_0,
		SCANCODE_KP_PERIOD,

		SCANCODE_NONUSBACKSLASH,
		SCANCODE_APPLICATION,
		SCANCODE_POWER,
		SCANCODE_KP_EQUALS,
		SCANCODE_F13,
		SCANCODE_F14,
		SCANCODE_F15,
		SCANCODE_F16,
		SCANCODE_F17,
		SCANCODE_F18,
		SCANCODE_F19,
		SCANCODE_F20,
		SCANCODE_F21,
		SCANCODE_F22,
		SCANCODE_F23,
		SCANCODE_F24,
		SCANCODE_EXECUTE,
		SCANCODE_HELP,
		SCANCODE_MENU,
		SCANCODE_SELECT,
		SCANCODE_STOP,
		SCANCODE_AGAIN,
		SCANCODE_UNDO,
		SCANCODE_CUT,
		SCANCODE_COPY,
		SCANCODE_PASTE,
		SCANCODE_FIND,
		SCANCODE_MUTE,
		SCANCODE_VOLUMEUP,
		SCANCODE_VOLUMEDOWN,
		SCANCODE_KP_COMMA,
		SCANCODE_KP_EQUALSAS400,

		SCANCODE_INTERNATIONAL1,
		SCANCODE_INTERNATIONAL2,
		SCANCODE_INTERNATIONAL3,
		SCANCODE_INTERNATIONAL4,
		SCANCODE_INTERNATIONAL5,
		SCANCODE_INTERNATIONAL6,
		SCANCODE_INTERNATIONAL7,
		SCANCODE_INTERNATIONAL8,
		SCANCODE_INTERNATIONAL9,
		SCANCODE_LANG1,
		SCANCODE_LANG2,
		SCANCODE_LANG3,
		SCANCODE_LANG4,
		SCANCODE_LANG5,
		SCANCODE_LANG6,
		SCANCODE_LANG7,
		SCANCODE_LANG8,
		SCANCODE_LANG9,

		SCANCODE_ALTERASE,
		SCANCODE_SYSREQ,
		SCANCODE_CANCEL,
		SCANCODE_CLEAR,
		SCANCODE_PRIOR,
		SCANCODE_RETURN2,
		SCANCODE_SEPARATOR,
		SCANCODE_OUT,
		SCANCODE_OPER,
		SCANCODE_CLEARAGAIN,
		SCANCODE_CRSEL,
		SCANCODE_EXSEL,

		SCANCODE_KP_00,
		SCANCODE_KP_000,
		SCANCODE_THOUSANDSSEPARATOR,
		SCANCODE_DECIMALSEPARATOR,
		SCANCODE_CURRENCYUNIT,
		SCANCODE_CURRENCYSUBUNIT,
		SCANCODE_KP_LEFTPAREN,
		SCANCODE_KP_RIGHTPAREN,
		SCANCODE_KP_LEFTBRACE,
		SCANCODE_KP_RIGHTBRACE,
		SCANCODE_KP_TAB,
		SCANCODE_KP_BACKSPACE,
		SCANCODE_KP_A,
		SCANCODE_KP_B,
		SCANCODE_KP_C,
		SCANCODE_KP_D,
		SCANCODE_KP_E,
		SCANCODE_KP_F,
		SCANCODE_KP_XOR,
		SCANCODE_KP_POWER,
		SCANCODE_KP_PERCENT,
		SCANCODE_KP_LESS,
		SCANCODE_KP_GREATER,
		SCANCODE_KP_AMPERSAND,
		SCANCODE_KP_DBLAMPERSAND,
		SCANCODE_KP_VERTICALBAR,
		SCANCODE_KP_DBLVERTICALBAR,
		SCANCODE_KP_COLON,
		SCANCODE_KP_HASH,
		SCANCODE_KP_SPACE,
		SCANCODE_KP_AT,
		SCANCODE_KP_EXCLAM,
		SCANCODE_KP_MEMSTORE,
		SCANCODE_KP_MEMRECALL,
		SCANCODE_KP_MEMCLEAR,
		SCANCODE_KP_MEMADD,
		SCANCODE_KP_MEMSUBTRACT,
		SCANCODE_KP_MEMMULTIPLY,
		SCANCODE_KP_MEMDIVIDE,
		SCANCODE_KP_PLUSMINUS,
		SCANCODE_KP_CLEAR,
		SCANCODE_KP_CLEARENTRY,
		SCANCODE_KP_BINARY,
		SCANCODE_KP_OCTAL,
		SCANCODE_KP_DECIMAL,
		SCANCODE_KP_HEXADECIMAL,

		SCANCODE_LCTRL,
		SCANCODE_LSHIFT,
		SCANCODE_LALT,
		SCANCODE_LGUI,
		SCANCODE_RCTRL,
		SCANCODE_RSHIFT,
		SCANCODE_RALT,
		SCANCODE_RGUI,

		SCANCODE_MODE,

		SCANCODE_AUDIONEXT,
		SCANCODE_AUDIOPREV,
		SCANCODE_AUDIOSTOP,
		SCANCODE_AUDIOPLAY,
		SCANCODE_AUDIOMUTE,
		SCANCODE_MEDIASELECT,
		SCANCODE_AC_SEARCH,
		SCANCODE_AC_HOME,
		SCANCODE_AC_BACK,
		SCANCODE_AC_FORWARD,
		SCANCODE_AC_STOP,
		SCANCODE_AC_REFRESH,
		SCANCODE_AC_BOOKMARKS,

		SCANCODE_EJECT,
		SCANCODE_SLEEP,

		SCANCODE_MAX_ENUM
	};

	/**
	 * Modifier keys. These are special keys that temporarily modify the normal function of a button when active.
	 **/
	enum ModifierKey
	{
		MODKEY_NUMLOCK,
		MODKEY_CAPSLOCK,
		MODKEY_SCROLLLOCK,
		MODKEY_MODE,
		MODKEY_MAX_ENUM
	};

	static constexpr uint32 KEY_SCANCODE_MASK = 1u << 30;

	#define LOVE_KEY(scancode) (scancode | KEY_SCANCODE_MASK)

	/**
	 * Keyboard keys. They are dependent on the current layout of the keyboard.
	 * This is not a full list of all keys - any unicode character can be a key,
	 * which is why the ASCII keys in this list use their ASCII value directly.
	 **/
	enum Key
	{
		KEY_UNKNOWN = 0,

		KEY_RETURN = '\r',
		KEY_ESCAPE = '\x1B',
		KEY_BACKSPACE = '\b',
		KEY_TAB = '\t',
		KEY_SPACE = ' ',
		KEY_EXCLAIM = '!',
		KEY_QUOTEDBL = '"',
		KEY_HASH = '#',
		KEY_PERCENT = '%',
		KEY_DOLLAR = '$',
		KEY_AMPERSAND = '&',
		KEY_QUOTE = '\'',
		KEY_LEFTPAREN = '(',
		KEY_RIGHTPAREN = ')',
		KEY_ASTERISK = '*',
		KEY_PLUS = '+',
		KEY_COMMA = ',',
		KEY_MINUS = '-',
		KEY_PERIOD = '.',
		KEY_SLASH = '/',
		KEY_0 = '0',
		KEY_1 = '1',
		KEY_2 = '2',
		KEY_3 = '3',
		KEY_4 = '4',
		KEY_5 = '5',
		KEY_6 = '6',
		KEY_7 = '7',
		KEY_8 = '8',
		KEY_9 = '9',
		KEY_COLON = ':',
		KEY_SEMICOLON = ';',
		KEY_LESS = '<',
		KEY_EQUALS = '=',
		KEY_GREATER = '>',
		KEY_QUESTION = '?',
		KEY_AT = '@',

		KEY_LEFTBRACKET = '[',
		KEY_BACKSLASH = '\\',
		KEY_RIGHTBRACKET = ']',
		KEY_CARET = '^',
		KEY_UNDERSCORE = '_',
		KEY_BACKQUOTE = '`',
		KEY_A = 'a',
		KEY_B = 'b',
		KEY_C = 'c',
		KEY_D = 'd',
		KEY_E = 'e',
		KEY_F = 'f',
		KEY_G = 'g',
		KEY_H = 'h',
		KEY_I = 'i',
		KEY_J = 'j',
		KEY_K = 'k',
		KEY_L = 'l',
		KEY_M = 'm',
		KEY_N = 'n',
		KEY_O = 'o',
		KEY_P = 'p',
		KEY_Q = 'q',
		KEY_R = 'r',
		KEY_S = 's',
		KEY_T = 't',
		KEY_U = 'u',
		KEY_V = 'v',
		KEY_W = 'w',
		KEY_X = 'x',
		KEY_Y = 'y',
		KEY_Z = 'z',

		KEY_DELETE = '\x7F',

		KEY_CAPSLOCK = LOVE_KEY(SCANCODE_CAPSLOCK),

		KEY_F1 = LOVE_KEY(SCANCODE_F1),
		KEY_F2 = LOVE_KEY(SCANCODE_F2),
		KEY_F3 = LOVE_KEY(SCANCODE_F3),
		KEY_F4 = LOVE_KEY(SCANCODE_F4),
		KEY_F5 = LOVE_KEY(SCANCODE_F5),
		KEY_F6 = LOVE_KEY(SCANCODE_F6),
		KEY_F7 = LOVE_KEY(SCANCODE_F7),
		KEY_F8 = LOVE_KEY(SCANCODE_F8),
		KEY_F9 = LOVE_KEY(SCANCODE_F9),
		KEY_F10 = LOVE_KEY(SCANCODE_F10),
		KEY_F11 = LOVE_KEY(SCANCODE_F11),
		KEY_F12 = LOVE_KEY(SCANCODE_F12),

		KEY_PRINTSCREEN = LOVE_KEY(SCANCODE_PRINTSCREEN),
		KEY_SCROLLLOCK = LOVE_KEY(SCANCODE_SCROLLLOCK),
		KEY_PAUSE = LOVE_KEY(SCANCODE_PAUSE),
		KEY_INSERT = LOVE_KEY(SCANCODE_INSERT),
		KEY_HOME = LOVE_KEY(SCANCODE_HOME),
		KEY_PAGEUP = LOVE_KEY(SCANCODE_PAGEUP),
		KEY_END = LOVE_KEY(SCANCODE_END),
		KEY_PAGEDOWN = LOVE_KEY(SCANCODE_PAGEDOWN),
		KEY_RIGHT = LOVE_KEY(SCANCODE_RIGHT),
		KEY_LEFT = LOVE_KEY(SCANCODE_LEFT),
		KEY_DOWN = LOVE_KEY(SCANCODE_DOWN),
		KEY_UP = LOVE_KEY(SCANCODE_UP),

		KEY_NUMLOCKCLEAR = LOVE_KEY(SCANCODE_NUMLOCKCLEAR),
		KEY_KP_DIVIDE = LOVE_KEY(SCANCODE_KP_DIVIDE),
		KEY_KP_MULTIPLY = LOVE_KEY(SCANCODE_KP_MULTIPLY),
		KEY_KP_MINUS = LOVE_KEY(SCANCODE_KP_MINUS),
		KEY_KP_PLUS = LOVE_KEY(SCANCODE_KP_PLUS),
		KEY_KP_ENTER = LOVE_KEY(SCANCODE_KP_ENTER),
		KEY_KP_1 = LOVE_KEY(SCANCODE_KP_1),
		KEY_KP_2 = LOVE_KEY(SCANCODE_KP_2),
		KEY_KP_3 = LOVE_KEY(SCANCODE_KP_3),
		KEY_KP_4 = LOVE_KEY(SCANCODE_KP_4),
		KEY_KP_5 = LOVE_KEY(SCANCODE_KP_5),
		KEY_KP_6 = LOVE_KEY(SCANCODE_KP_6),
		KEY_KP_7 = LOVE_KEY(SCANCODE_KP_7),
		KEY_KP_8 = LOVE_KEY(SCANCODE_KP_8),
		KEY_KP_9 = LOVE_KEY(SCANCODE_KP_9),
		KEY_KP_0 = LOVE_KEY(SCANCODE_KP_0),
		KEY_KP_PERIOD = LOVE_KEY(SCANCODE_KP_PERIOD),
		KEY_KP_COMMA = LOVE_KEY(SCANCODE_KP_COMMA),
		KEY_KP_EQUALS = LOVE_KEY(SCANCODE_KP_EQUALS),

		KEY_APPLICATION = LOVE_KEY(SCANCODE_APPLICATION),
		KEY_POWER = LOVE_KEY(SCANCODE_POWER),
		KEY_F13 = LOVE_KEY(SCANCODE_F13),
		KEY_F14 = LOVE_KEY(SCANCODE_F14),
		KEY_F15 = LOVE_KEY(SCANCODE_F15),
		KEY_F16 = LOVE_KEY(SCANCODE_F16),
		KEY_F17 = LOVE_KEY(SCANCODE_F17),
		KEY_F18 = LOVE_KEY(SCANCODE_F18),
		KEY_F19 = LOVE_KEY(SCANCODE_F19),
		KEY_F20 = LOVE_KEY(SCANCODE_F20),
		KEY_F21 = LOVE_KEY(SCANCODE_F21),
		KEY_F22 = LOVE_KEY(SCANCODE_F22),
		KEY_F23 = LOVE_KEY(SCANCODE_F23),
		KEY_F24 = LOVE_KEY(SCANCODE_F24),
		KEY_EXECUTE = LOVE_KEY(SCANCODE_EXECUTE),
		KEY_HELP = LOVE_KEY(SCANCODE_HELP),
		KEY_MENU = LOVE_KEY(SCANCODE_MENU),
		KEY_SELECT = LOVE_KEY(SCANCODE_SELECT),
		KEY_STOP = LOVE_KEY(SCANCODE_STOP),
		KEY_AGAIN = LOVE_KEY(SCANCODE_AGAIN),
		KEY_UNDO = LOVE_KEY(SCANCODE_UNDO),
		KEY_CUT = LOVE_KEY(SCANCODE_CUT),
		KEY_COPY = LOVE_KEY(SCANCODE_COPY),
		KEY_PASTE = LOVE_KEY(SCANCODE_PASTE),
		KEY_FIND = LOVE_KEY(SCANCODE_FIND),
		KEY_MUTE = LOVE_KEY(SCANCODE_MUTE),
		KEY_VOLUMEUP = LOVE_KEY(SCANCODE_VOLUMEUP),
		KEY_VOLUMEDOWN = LOVE_KEY(SCANCODE_VOLUMEDOWN),

		KEY_ALTERASE = LOVE_KEY(SCANCODE_ALTERASE),
		KEY_SYSREQ = LOVE_KEY(SCANCODE_SYSREQ),
		KEY_CANCEL = LOVE_KEY(SCANCODE_CANCEL),
		KEY_CLEAR = LOVE_KEY(SCANCODE_CLEAR),
		KEY_PRIOR = LOVE_KEY(SCANCODE_PRIOR),
		KEY_RETURN2 = LOVE_KEY(SCANCODE_RETURN2),
		KEY_SEPARATOR = LOVE_KEY(SCANCODE_SEPARATOR),
		KEY_OUT = LOVE_KEY(SCANCODE_OUT),
		KEY_OPER = LOVE_KEY(SCANCODE_OPER),
		KEY_CLEARAGAIN = LOVE_KEY(SCANCODE_CLEARAGAIN),

		KEY_THOUSANDSSEPARATOR = LOVE_KEY(SCANCODE_THOUSANDSSEPARATOR),
		KEY_DECIMALSEPARATOR = LOVE_KEY(SCANCODE_DECIMALSEPARATOR),
		KEY_CURRENCYUNIT = LOVE_KEY(SCANCODE_CURRENCYUNIT),
		KEY_CURRENCYSUBUNIT = LOVE_KEY(SCANCODE_CURRENCYSUBUNIT),

		KEY_LCTRL = LOVE_KEY(SCANCODE_LCTRL),
		KEY_LSHIFT = LOVE_KEY(SCANCODE_LSHIFT),
		KEY_LALT = LOVE_KEY(SCANCODE_LALT),
		KEY_LGUI = LOVE_KEY(SCANCODE_LGUI),
		KEY_RCTRL = LOVE_KEY(SCANCODE_RCTRL),
		KEY_RSHIFT = LOVE_KEY(SCANCODE_RSHIFT),
		KEY_RALT = LOVE_KEY(SCANCODE_RALT),
		KEY_RGUI = LOVE_KEY(SCANCODE_RGUI),

		KEY_MODE = LOVE_KEY(SCANCODE_MODE),

		KEY_AUDIONEXT = LOVE_KEY(SCANCODE_AUDIONEXT),
		KEY_AUDIOPREV = LOVE_KEY(SCANCODE_AUDIOPREV),
		KEY_AUDIOSTOP = LOVE_KEY(SCANCODE_AUDIOSTOP),
		KEY_AUDIOPLAY = LOVE_KEY(SCANCODE_AUDIOPLAY),
		KEY_AUDIOMUTE = LOVE_KEY(SCANCODE_AUDIOMUTE),
		KEY_MEDIASELECT = LOVE_KEY(SCANCODE_MEDIASELECT),
		KEY_APP_SEARCH = LOVE_KEY(SCANCODE_AC_SEARCH),
		KEY_APP_HOME = LOVE_KEY(SCANCODE_AC_HOME),
		KEY_APP_BACK = LOVE_KEY(SCANCODE_AC_BACK),
		KEY_APP_FORWARD = LOVE_KEY(SCANCODE_AC_FORWARD),
		KEY_APP_STOP = LOVE_KEY(SCANCODE_AC_STOP),
		KEY_APP_REFRESH = LOVE_KEY(SCANCODE_AC_REFRESH),
		KEY_APP_BOOKMARKS = LOVE_KEY(SCANCODE_AC_BOOKMARKS),

		KEY_EJECT = LOVE_KEY(SCANCODE_EJECT),
		KEY_SLEEP = LOVE_KEY(SCANCODE_SLEEP),
	};

	#undef LOVE_KEY

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
	 * Checks whether certain keys are pressed or not.
	 * @param keylist A list of key identifiers.
	 * @return Whether any of the specified keys are pressed.
	 **/
	virtual bool isDown(const std::vector<Key> &keylist) const = 0;

	/**
	 * Checks whether certain scancodes are pressed or not.
	 * @param scancodelist A list of scancodes.
	 * @return Whether any of the specified scancodes are pressed.
	 **/
	virtual bool isScancodeDown(const std::vector<Scancode> &scancodelist) const = 0;

	/**
	 * Checks whether specific modifier key is active or not.
	 * @param key Modifier key to check.
	 * @return Whether the specified modifier key is active.
	 **/
	virtual bool isModifierActive(ModifierKey key) const = 0;

	/**
	 * Gets the key corresponding to the specified scancode according to the
	 * current keyboard layout.
	 **/
	virtual Key getKeyFromScancode(Scancode scancode) const = 0;

	/**
	 * Gets the scancode corresponding to the specified key according to the
	 * current keyboard layout.
	 **/
	virtual Scancode getScancodeFromKey(Key key) const = 0;

	/**
	 * Sets whether text input events should be received.
	 * @param enable Whether to receive text input events.
	 **/
	virtual void setTextInput(bool enable) = 0;

	/**
	 * Sets whether text input events should be received, and specifies where
	 * on the screen the text will appear. This is used as a hint so on-screen
	 * keyboards don't cover the text area.
	 **/
	virtual void setTextInput(bool enable, double x, double y, double w, double h) = 0;

	/**
	 * Gets whether text input events are enabled.
	 **/
	virtual bool hasTextInput() const = 0;

	/**
	 * Gets whether the system will display an on-screen keyboard when text input
	 * events are enabled.
	 **/
	virtual bool hasScreenKeyboard() const = 0;

	static bool getConstant(const char *in, Key &out);
	static bool getConstant(Key in, const char *&out);

	static bool getConstant(const char *in, Scancode &out);
	static bool getConstant(Scancode in, const char *&out);

	static bool getConstant(const char *in, ModifierKey &out);
	static bool getConstant(ModifierKey in, const char *&out);

protected:

	Keyboard(const char *name);

private:

	static std::map<uint32, std::string> keyToString;
	static std::map<std::string, uint32> stringToKey;

	static StringMap<Scancode, SCANCODE_MAX_ENUM>::Entry scancodeEntries[];
	static StringMap<Scancode, SCANCODE_MAX_ENUM> scancodes;

	static StringMap<ModifierKey, MODKEY_MAX_ENUM>::Entry modifierEntries[];
	static StringMap<ModifierKey, MODKEY_MAX_ENUM> modifiers;

}; // Keyboard

} // keyboard
} // love

#endif // LOVE_KEYBOARD_KEYBOARD_H
