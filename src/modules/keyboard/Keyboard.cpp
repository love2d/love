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

#include "common/config.h"

#include "Keyboard.h"

namespace love
{
namespace keyboard
{

bool Keyboard::getConstant(const char *in, Keyboard::Key &out)
{
	return keys.find(in, out);
}

bool Keyboard::getConstant(Keyboard::Key in, const char  *&out)
{
	return keys.find(in, out);
}

StringMap<Keyboard::Key, Keyboard::KEY_MAX_ENUM>::Entry Keyboard::keyEntries[] =
{
	{"return", Keyboard::KEY_RETURN},
	{"escape", Keyboard::KEY_ESCAPE},
	{"backspace", Keyboard::KEY_BACKSPACE},
	{"tab", Keyboard::KEY_TAB},
	{" ", Keyboard::KEY_SPACE},
	{"!", Keyboard::KEY_EXCLAIM},
	{"\"", Keyboard::KEY_QUOTEDBL},
	{"#", Keyboard::KEY_HASH},
	{"$", Keyboard::KEY_DOLLAR},
	{"&", Keyboard::KEY_AMPERSAND},
	{"'", Keyboard::KEY_QUOTE},
	{"(", Keyboard::KEY_LEFTPAREN},
	{")", Keyboard::KEY_RIGHTPAREN},
	{"*", Keyboard::KEY_ASTERISK},
	{"+", Keyboard::KEY_PLUS},
	{",", Keyboard::KEY_COMMA},
	{"-", Keyboard::KEY_MINUS},
	{".", Keyboard::KEY_PERIOD},
	{"/", Keyboard::KEY_SLASH},
	{"0", Keyboard::KEY_0},
	{"1", Keyboard::KEY_1},
	{"2", Keyboard::KEY_2},
	{"3", Keyboard::KEY_3},
	{"4", Keyboard::KEY_4},
	{"5", Keyboard::KEY_5},
	{"6", Keyboard::KEY_6},
	{"7", Keyboard::KEY_7},
	{"8", Keyboard::KEY_8},
	{"9", Keyboard::KEY_9},
	{":", Keyboard::KEY_COLON},
	{";", Keyboard::KEY_SEMICOLON},
	{"<", Keyboard::KEY_LESS},
	{"=", Keyboard::KEY_EQUALS},
	{">", Keyboard::KEY_GREATER},
	{"?", Keyboard::KEY_QUESTION},
	{"@", Keyboard::KEY_AT},

	{"[", Keyboard::KEY_LEFTBRACKET},
	{"\\", Keyboard::KEY_BACKSLASH},
	{"]", Keyboard::KEY_RIGHTBRACKET},
	{"^", Keyboard::KEY_CARET},
	{"_", Keyboard::KEY_UNDERSCORE},
	{"`", Keyboard::KEY_BACKQUOTE},
	{"a", Keyboard::KEY_A},
	{"b", Keyboard::KEY_B},
	{"c", Keyboard::KEY_C},
	{"d", Keyboard::KEY_D},
	{"e", Keyboard::KEY_E},
	{"f", Keyboard::KEY_F},
	{"g", Keyboard::KEY_G},
	{"h", Keyboard::KEY_H},
	{"i", Keyboard::KEY_I},
	{"j", Keyboard::KEY_J},
	{"k", Keyboard::KEY_K},
	{"l", Keyboard::KEY_L},
	{"m", Keyboard::KEY_M},
	{"n", Keyboard::KEY_N},
	{"o", Keyboard::KEY_O},
	{"p", Keyboard::KEY_P},
	{"q", Keyboard::KEY_Q},
	{"r", Keyboard::KEY_R},
	{"s", Keyboard::KEY_S},
	{"t", Keyboard::KEY_T},
	{"u", Keyboard::KEY_U},
	{"v", Keyboard::KEY_V},
	{"w", Keyboard::KEY_W},
	{"x", Keyboard::KEY_X},
	{"y", Keyboard::KEY_Y},
	{"z", Keyboard::KEY_Z},

	{"capslock", Keyboard::KEY_CAPSLOCK},

	{"f1", Keyboard::KEY_F1},
	{"f2", Keyboard::KEY_F2},
	{"f3", Keyboard::KEY_F3},
	{"f4", Keyboard::KEY_F4},
	{"f5", Keyboard::KEY_F5},
	{"f6", Keyboard::KEY_F6},
	{"f7", Keyboard::KEY_F7},
	{"f8", Keyboard::KEY_F8},
	{"f9", Keyboard::KEY_F9},
	{"f10", Keyboard::KEY_F10},
	{"f11", Keyboard::KEY_F11},
	{"f12", Keyboard::KEY_F12},

	{"printscreen", Keyboard::KEY_PRINTSCREEN},
	{"scrolllock", Keyboard::KEY_SCROLLLOCK},
	{"pause", Keyboard::KEY_PAUSE},
	{"insert", Keyboard::KEY_INSERT},
	{"home", Keyboard::KEY_HOME},
	{"pageup", Keyboard::KEY_PAGEUP},
	{"delete", Keyboard::KEY_DELETE},
	{"end", Keyboard::KEY_END},
	{"pagedown", Keyboard::KEY_PAGEDOWN},
	{"right", Keyboard::KEY_RIGHT},
	{"left", Keyboard::KEY_LEFT},
	{"down", Keyboard::KEY_DOWN},
	{"up", Keyboard::KEY_UP},

	{"numlock", Keyboard::KEY_NUMLOCKCLEAR},
	{"kp/", Keyboard::KEY_KP_DIVIDE},
	{"kp*", Keyboard::KEY_KP_MULTIPLY},
	{"kp-", Keyboard::KEY_KP_MINUS},
	{"kp+", Keyboard::KEY_KP_PLUS},
	{"kpenter", Keyboard::KEY_KP_ENTER},
	{"kp0", Keyboard::KEY_KP_0},
	{"kp1", Keyboard::KEY_KP_1},
	{"kp2", Keyboard::KEY_KP_2},
	{"kp3", Keyboard::KEY_KP_3},
	{"kp4", Keyboard::KEY_KP_4},
	{"kp5", Keyboard::KEY_KP_5},
	{"kp6", Keyboard::KEY_KP_6},
	{"kp7", Keyboard::KEY_KP_7},
	{"kp8", Keyboard::KEY_KP_8},
	{"kp9", Keyboard::KEY_KP_9},
	{"kp.", Keyboard::KEY_KP_PERIOD},
	{"kp,", Keyboard::KEY_KP_COMMA},
	{"kp=", Keyboard::KEY_KP_EQUALS},

	{"application", Keyboard::KEY_APPLICATION},
	{"power", Keyboard::KEY_POWER},
	{"f13", Keyboard::KEY_F13},
	{"f14", Keyboard::KEY_F14},
	{"f15", Keyboard::KEY_F15},
	{"f16", Keyboard::KEY_F16},
	{"f17", Keyboard::KEY_F17},
	{"f18", Keyboard::KEY_F18},
	{"f19", Keyboard::KEY_F19},
	{"f20", Keyboard::KEY_F20},
	{"f21", Keyboard::KEY_F21},
	{"f22", Keyboard::KEY_F22},
	{"f23", Keyboard::KEY_F23},
	{"f24", Keyboard::KEY_F24},
	{"execute", Keyboard::KEY_EXECUTE},
	{"help", Keyboard::KEY_HELP},
	{"menu", Keyboard::KEY_MENU},
	{"select", Keyboard::KEY_SELECT},
	{"stop", Keyboard::KEY_STOP},
	{"again", Keyboard::KEY_AGAIN},
	{"undo", Keyboard::KEY_UNDO},
	{"cut", Keyboard::KEY_CUT},
	{"copy", Keyboard::KEY_COPY},
	{"paste", Keyboard::KEY_PASTE},
	{"find", Keyboard::KEY_FIND},
	{"mute", Keyboard::KEY_MUTE},
	{"volumeup", Keyboard::KEY_VOLUMEUP},
	{"volumedown", Keyboard::KEY_VOLUMEDOWN},

	{"alterase", Keyboard::KEY_ALTERASE},
	{"sysreq", Keyboard::KEY_SYSREQ},
	{"cancel", Keyboard::KEY_CANCEL},
	{"clear", Keyboard::KEY_CLEAR},
	{"prior", Keyboard::KEY_PRIOR},
	{"return2", Keyboard::KEY_RETURN2},
	{"separator", Keyboard::KEY_SEPARATOR},
	{"out", Keyboard::KEY_OUT},
	{"oper", Keyboard::KEY_OPER},
	{"clearagain", Keyboard::KEY_CLEARAGAIN},

	{"thsousandsseparator", Keyboard::KEY_THOUSANDSSEPARATOR},
	{"decimalseparator", Keyboard::KEY_DECIMALSEPARATOR},
	{"currencyunit", Keyboard::KEY_CURRENCYUNIT},
	{"currencysubunit", Keyboard::KEY_CURRENCYSUBUNIT},

	{"lctrl", Keyboard::KEY_LCTRL},
	{"lshift", Keyboard::KEY_LSHIFT},
	{"lalt", Keyboard::KEY_LALT},
	{"lgui", Keyboard::KEY_LGUI},
	{"rctrl", Keyboard::KEY_RCTRL},
	{"rshift", Keyboard::KEY_RSHIFT},
	{"ralt", Keyboard::KEY_RALT},
	{"rgui", Keyboard::KEY_RGUI},

	{"mode", Keyboard::KEY_MODE},

	{"audionext", Keyboard::KEY_AUDIONEXT},
	{"audioprev", Keyboard::KEY_AUDIOPREV},
	{"audiostop", Keyboard::KEY_AUDIOSTOP},
	{"audioplay", Keyboard::KEY_AUDIOPLAY},
	{"audiomute", Keyboard::KEY_AUDIOMUTE},
	{"mediaselect", Keyboard::KEY_MEDIASELECT},
	{"www", Keyboard::KEY_WWW},
	{"mail", Keyboard::KEY_MAIL},
	{"calculator", Keyboard::KEY_CALCULATOR},
	{"computer", Keyboard::KEY_COMPUTER},
	{"appsearch", Keyboard::KEY_APP_SEARCH},
	{"apphome", Keyboard::KEY_APP_HOME},
	{"appback", Keyboard::KEY_APP_BACK},
	{"appforward", Keyboard::KEY_APP_FORWARD},
	{"appstop", Keyboard::KEY_APP_STOP},
	{"apprefresh", Keyboard::KEY_APP_REFRESH},
	{"appbookmarks", Keyboard::KEY_APP_BOOKMARKS},

	{"brightnessdown", Keyboard::KEY_BRIGHTNESSDOWN},
	{"brightnessup", Keyboard::KEY_BRIGHTNESSUP},
	{"displayswitch", Keyboard::KEY_DISPLAYSWITCH},
	{"kbdillumtoggle", Keyboard::KEY_KBDILLUMTOGGLE},
	{"kbdillumdown", Keyboard::KEY_KBDILLUMDOWN},
	{"kbdillumup", Keyboard::KEY_KBDILLUMUP},
	{"eject", Keyboard::KEY_EJECT},
	{"sleep", Keyboard::KEY_SLEEP},
};

StringMap<Keyboard::Key, Keyboard::KEY_MAX_ENUM> Keyboard::keys(Keyboard::keyEntries, sizeof(Keyboard::keyEntries));


} // keyboard
} // love
