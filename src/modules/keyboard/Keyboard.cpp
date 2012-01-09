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

#include <common/config.h>

#include "Keyboard.h"

namespace love
{
namespace keyboard
{
	bool Keyboard::getConstant(const char * in, Keyboard::Key & out)
	{
		return keys.find(in, out);
	}

	bool Keyboard::getConstant(Keyboard::Key in, const char *& out)
	{
		return keys.find(in, out);
	}

	StringMap<Keyboard::Key, Keyboard::KEY_MAX_ENUM>::Entry Keyboard::keyEntries[] =
	{
		{"backspace", Keyboard::KEY_BACKSPACE},
		{"tab", Keyboard::KEY_TAB},
		{"clear", Keyboard::KEY_CLEAR},
		{"return", Keyboard::KEY_RETURN},
		{"pause", Keyboard::KEY_PAUSE},
		{"escape", Keyboard::KEY_ESCAPE},
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
		{"delete", Keyboard::KEY_DELETE},

		{"kp0", Keyboard::KEY_KP0},
		{"kp1", Keyboard::KEY_KP1},
		{"kp2", Keyboard::KEY_KP2},
		{"kp3", Keyboard::KEY_KP3},
		{"kp4", Keyboard::KEY_KP4},
		{"kp5", Keyboard::KEY_KP5},
		{"kp6", Keyboard::KEY_KP6},
		{"kp7", Keyboard::KEY_KP7},
		{"kp8", Keyboard::KEY_KP8},
		{"kp9", Keyboard::KEY_KP9},
		{"kp.", Keyboard::KEY_KP_PERIOD},
		{"kp/", Keyboard::KEY_KP_DIVIDE},
		{"kp*", Keyboard::KEY_KP_MULTIPLY},
		{"kp-", Keyboard::KEY_KP_MINUS},
		{"kp+", Keyboard::KEY_KP_PLUS},
		{"kpenter", Keyboard::KEY_KP_ENTER},
		{"kp=", Keyboard::KEY_KP_EQUALS},

		{"up", Keyboard::KEY_UP},
		{"down", Keyboard::KEY_DOWN},
		{"right", Keyboard::KEY_RIGHT},
		{"left", Keyboard::KEY_LEFT},
		{"insert", Keyboard::KEY_INSERT},
		{"home", Keyboard::KEY_HOME},
		{"end", Keyboard::KEY_END},
		{"pageup", Keyboard::KEY_PAGEUP},
		{"pagedown", Keyboard::KEY_PAGEDOWN},

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
		{"f13", Keyboard::KEY_F13},
		{"f14", Keyboard::KEY_F14},
		{"f15", Keyboard::KEY_F15},

		{"numlock", Keyboard::KEY_NUMLOCK},
		{"capslock", Keyboard::KEY_CAPSLOCK},
		{"scrollock", Keyboard::KEY_SCROLLOCK},
		{"rshift", Keyboard::KEY_RSHIFT},
		{"lshift", Keyboard::KEY_LSHIFT},
		{"rctrl", Keyboard::KEY_RCTRL},
		{"lctrl", Keyboard::KEY_LCTRL},
		{"ralt", Keyboard::KEY_RALT},
		{"lalt", Keyboard::KEY_LALT},
		{"rmeta", Keyboard::KEY_RMETA},
		{"lmeta", Keyboard::KEY_LMETA},
		{"lsuper", Keyboard::KEY_LSUPER},
		{"rsuper", Keyboard::KEY_RSUPER},
		{"mode", Keyboard::KEY_MODE},
		{"compose", Keyboard::KEY_COMPOSE},

		{"help", Keyboard::KEY_HELP},
		{"print", Keyboard::KEY_PRINT},
		{"sysreq", Keyboard::KEY_SYSREQ},
		{"break", Keyboard::KEY_BREAK},
		{"menu", Keyboard::KEY_MENU},
		{"power", Keyboard::KEY_POWER},
		{"euro", Keyboard::KEY_EURO},
		{"undo", Keyboard::KEY_UNDO},
	};

	StringMap<Keyboard::Key, Keyboard::KEY_MAX_ENUM> Keyboard::keys(Keyboard::keyEntries, sizeof(Keyboard::keyEntries));


} // keyboard
} // love
