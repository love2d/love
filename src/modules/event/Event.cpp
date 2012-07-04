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

#include "Event.h"

using love::thread::Mutex;
using love::thread::Lock;

namespace love
{
namespace event
{

Message::Message(std::string name, Variant *a, Variant *b, Variant *c, Variant *d)
	: name(name)
	, nargs(0)
{
	args[0] = a;
	args[1] = b;
	args[2] = c;
	args[3] = d;
	for (int i = 0; i < 4; i++)
	{
		if (!args[i])
			continue;
		args[i]->retain();
		nargs++;
	}
}

Message::~Message()
{
	for (int i = 0; i < nargs; i++)
		args[i]->release();
}

int Message::toLua(lua_State *L)
{
	luax_pushstring(L, name);
	for (int i = 0; i < nargs; i++)
		args[i]->toLua(L);
	return nargs+1;
}

Message *Message::fromLua(lua_State *L, int n)
{
	std::string name = luax_checkstring(L, n);
	n++;
	Message *m = new Message(name);
	for (int i = 0; i < 4; i++)
	{
		if (lua_isnoneornil(L, n+i))
			break;
		m->args[i] = Variant::fromLua(L, n+i);
		if (!m->args[i])
		{
			delete m;
			luaL_error(L, "Argument %d can't be stored safely\nExpected boolean, number, string or userdata.", n+i);
			return NULL;
		}
		m->nargs++;
	}
	return m;
}

Event::~Event()
{
}

void Event::push(Message *msg)
{
	Lock lock(mutex);
	msg->retain();
	queue.push(msg);
}

bool Event::poll(Message *&msg)
{
	Lock lock(mutex);
	if (queue.empty())
		return false;
	msg = queue.front();
	queue.pop();
	return true;
}

void Event::clear()
{
	Lock lock(mutex);
	while (!queue.empty())
	{
		queue.back()->release();
		queue.pop();
	}
}

bool Event::getConstant(const char *in, love::mouse::Mouse::Button &out)
{
	return buttons.find(in, out);
}

bool Event::getConstant(love::mouse::Mouse::Button in, const char  *&out)
{
	return buttons.find(in, out);
}

bool Event::getConstant(const char *in, love::keyboard::Keyboard::Key &out)
{
	return keys.find(in, out);
}

bool Event::getConstant(love::keyboard::Keyboard::Key in, const char  *&out)
{
	return keys.find(in, out);
}

StringMap<love::mouse::Mouse::Button, love::mouse::Mouse::BUTTON_MAX_ENUM>::Entry Event::buttonEntries[] =
{
	{"l", love::mouse::Mouse::BUTTON_LEFT},
	{"m", love::mouse::Mouse::BUTTON_MIDDLE},
	{"r", love::mouse::Mouse::BUTTON_RIGHT},
	{"wu", love::mouse::Mouse::BUTTON_WHEELUP},
	{"wd", love::mouse::Mouse::BUTTON_WHEELDOWN},
	{"x1", love::mouse::Mouse::BUTTON_X1},
	{"x2", love::mouse::Mouse::BUTTON_X2},
};

StringMap<love::mouse::Mouse::Button, love::mouse::Mouse::BUTTON_MAX_ENUM> Event::buttons(Event::buttonEntries, sizeof(Event::buttonEntries));

StringMap<love::keyboard::Keyboard::Key, love::keyboard::Keyboard::KEY_MAX_ENUM>::Entry Event::keyEntries[] =
{
	{"backspace", love::keyboard::Keyboard::KEY_BACKSPACE},
	{"tab", love::keyboard::Keyboard::KEY_TAB},
	{"clear", love::keyboard::Keyboard::KEY_CLEAR},
	{"return", love::keyboard::Keyboard::KEY_RETURN},
	{"pause", love::keyboard::Keyboard::KEY_PAUSE},
	{"escape", love::keyboard::Keyboard::KEY_ESCAPE},
	{" ", love::keyboard::Keyboard::KEY_SPACE},
	{"!", love::keyboard::Keyboard::KEY_EXCLAIM},
	{"\"", love::keyboard::Keyboard::KEY_QUOTEDBL},
	{"#", love::keyboard::Keyboard::KEY_HASH},
	{"$", love::keyboard::Keyboard::KEY_DOLLAR},
	{"&", love::keyboard::Keyboard::KEY_AMPERSAND},
	{"'", love::keyboard::Keyboard::KEY_QUOTE},
	{"(", love::keyboard::Keyboard::KEY_LEFTPAREN},
	{")", love::keyboard::Keyboard::KEY_RIGHTPAREN},
	{"*", love::keyboard::Keyboard::KEY_ASTERISK},
	{"+", love::keyboard::Keyboard::KEY_PLUS},
	{",", love::keyboard::Keyboard::KEY_COMMA},
	{"-", love::keyboard::Keyboard::KEY_MINUS},
	{".", love::keyboard::Keyboard::KEY_PERIOD},
	{"/", love::keyboard::Keyboard::KEY_SLASH},
	{"0", love::keyboard::Keyboard::KEY_0},
	{"1", love::keyboard::Keyboard::KEY_1},
	{"2", love::keyboard::Keyboard::KEY_2},
	{"3", love::keyboard::Keyboard::KEY_3},
	{"4", love::keyboard::Keyboard::KEY_4},
	{"5", love::keyboard::Keyboard::KEY_5},
	{"6", love::keyboard::Keyboard::KEY_6},
	{"7", love::keyboard::Keyboard::KEY_7},
	{"8", love::keyboard::Keyboard::KEY_8},
	{"9", love::keyboard::Keyboard::KEY_9},
	{":", love::keyboard::Keyboard::KEY_COLON},
	{";", love::keyboard::Keyboard::KEY_SEMICOLON},
	{"<", love::keyboard::Keyboard::KEY_LESS},
	{"=", love::keyboard::Keyboard::KEY_EQUALS},
	{">", love::keyboard::Keyboard::KEY_GREATER},
	{"?", love::keyboard::Keyboard::KEY_QUESTION},
	{"@", love::keyboard::Keyboard::KEY_AT},

	{"[", love::keyboard::Keyboard::KEY_LEFTBRACKET},
	{"\\", love::keyboard::Keyboard::KEY_BACKSLASH},
	{"]", love::keyboard::Keyboard::KEY_RIGHTBRACKET},
	{"^", love::keyboard::Keyboard::KEY_CARET},
	{"_", love::keyboard::Keyboard::KEY_UNDERSCORE},
	{"`", love::keyboard::Keyboard::KEY_BACKQUOTE},
	{"a", love::keyboard::Keyboard::KEY_A},
	{"b", love::keyboard::Keyboard::KEY_B},
	{"c", love::keyboard::Keyboard::KEY_C},
	{"d", love::keyboard::Keyboard::KEY_D},
	{"e", love::keyboard::Keyboard::KEY_E},
	{"f", love::keyboard::Keyboard::KEY_F},
	{"g", love::keyboard::Keyboard::KEY_G},
	{"h", love::keyboard::Keyboard::KEY_H},
	{"i", love::keyboard::Keyboard::KEY_I},
	{"j", love::keyboard::Keyboard::KEY_J},
	{"k", love::keyboard::Keyboard::KEY_K},
	{"l", love::keyboard::Keyboard::KEY_L},
	{"m", love::keyboard::Keyboard::KEY_M},
	{"n", love::keyboard::Keyboard::KEY_N},
	{"o", love::keyboard::Keyboard::KEY_O},
	{"p", love::keyboard::Keyboard::KEY_P},
	{"q", love::keyboard::Keyboard::KEY_Q},
	{"r", love::keyboard::Keyboard::KEY_R},
	{"s", love::keyboard::Keyboard::KEY_S},
	{"t", love::keyboard::Keyboard::KEY_T},
	{"u", love::keyboard::Keyboard::KEY_U},
	{"v", love::keyboard::Keyboard::KEY_V},
	{"w", love::keyboard::Keyboard::KEY_W},
	{"x", love::keyboard::Keyboard::KEY_X},
	{"y", love::keyboard::Keyboard::KEY_Y},
	{"z", love::keyboard::Keyboard::KEY_Z},
	{"delete", love::keyboard::Keyboard::KEY_DELETE},

	{"kp0", love::keyboard::Keyboard::KEY_KP0},
	{"kp1", love::keyboard::Keyboard::KEY_KP1},
	{"kp2", love::keyboard::Keyboard::KEY_KP2},
	{"kp3", love::keyboard::Keyboard::KEY_KP3},
	{"kp4", love::keyboard::Keyboard::KEY_KP4},
	{"kp5", love::keyboard::Keyboard::KEY_KP5},
	{"kp6", love::keyboard::Keyboard::KEY_KP6},
	{"kp7", love::keyboard::Keyboard::KEY_KP7},
	{"kp8", love::keyboard::Keyboard::KEY_KP8},
	{"kp9", love::keyboard::Keyboard::KEY_KP9},
	{"kp.", love::keyboard::Keyboard::KEY_KP_PERIOD},
	{"kp/", love::keyboard::Keyboard::KEY_KP_DIVIDE},
	{"kp*", love::keyboard::Keyboard::KEY_KP_MULTIPLY},
	{"kp-", love::keyboard::Keyboard::KEY_KP_MINUS},
	{"kp+", love::keyboard::Keyboard::KEY_KP_PLUS},
	{"kpenter", love::keyboard::Keyboard::KEY_KP_ENTER},
	{"kp=", love::keyboard::Keyboard::KEY_KP_EQUALS},

	{"up", love::keyboard::Keyboard::KEY_UP},
	{"down", love::keyboard::Keyboard::KEY_DOWN},
	{"right", love::keyboard::Keyboard::KEY_RIGHT},
	{"left", love::keyboard::Keyboard::KEY_LEFT},
	{"insert", love::keyboard::Keyboard::KEY_INSERT},
	{"home", love::keyboard::Keyboard::KEY_HOME},
	{"end", love::keyboard::Keyboard::KEY_END},
	{"pageup", love::keyboard::Keyboard::KEY_PAGEUP},
	{"pagedown", love::keyboard::Keyboard::KEY_PAGEDOWN},

	{"f1", love::keyboard::Keyboard::KEY_F1},
	{"f2", love::keyboard::Keyboard::KEY_F2},
	{"f3", love::keyboard::Keyboard::KEY_F3},
	{"f4", love::keyboard::Keyboard::KEY_F4},
	{"f5", love::keyboard::Keyboard::KEY_F5},
	{"f6", love::keyboard::Keyboard::KEY_F6},
	{"f7", love::keyboard::Keyboard::KEY_F7},
	{"f8", love::keyboard::Keyboard::KEY_F8},
	{"f9", love::keyboard::Keyboard::KEY_F9},
	{"f10", love::keyboard::Keyboard::KEY_F10},
	{"f11", love::keyboard::Keyboard::KEY_F11},
	{"f12", love::keyboard::Keyboard::KEY_F12},
	{"f13", love::keyboard::Keyboard::KEY_F13},
	{"f14", love::keyboard::Keyboard::KEY_F14},
	{"f15", love::keyboard::Keyboard::KEY_F15},

	{"numlock", love::keyboard::Keyboard::KEY_NUMLOCK},
	{"capslock", love::keyboard::Keyboard::KEY_CAPSLOCK},
	{"scrollock", love::keyboard::Keyboard::KEY_SCROLLOCK},
	{"rshift", love::keyboard::Keyboard::KEY_RSHIFT},
	{"lshift", love::keyboard::Keyboard::KEY_LSHIFT},
	{"rctrl", love::keyboard::Keyboard::KEY_RCTRL},
	{"lctrl", love::keyboard::Keyboard::KEY_LCTRL},
	{"ralt", love::keyboard::Keyboard::KEY_RALT},
	{"lalt", love::keyboard::Keyboard::KEY_LALT},
	{"rmeta", love::keyboard::Keyboard::KEY_RMETA},
	{"lmeta", love::keyboard::Keyboard::KEY_LMETA},
	{"lsuper", love::keyboard::Keyboard::KEY_LSUPER},
	{"rsuper", love::keyboard::Keyboard::KEY_RSUPER},
	{"mode", love::keyboard::Keyboard::KEY_MODE},
	{"compose", love::keyboard::Keyboard::KEY_COMPOSE},

	{"help", love::keyboard::Keyboard::KEY_HELP},
	{"print", love::keyboard::Keyboard::KEY_PRINT},
	{"sysreq", love::keyboard::Keyboard::KEY_SYSREQ},
	{"break", love::keyboard::Keyboard::KEY_BREAK},
	{"menu", love::keyboard::Keyboard::KEY_MENU},
	{"power", love::keyboard::Keyboard::KEY_POWER},
	{"euro", love::keyboard::Keyboard::KEY_EURO},
	{"undo", love::keyboard::Keyboard::KEY_UNDO},

	{"unknown", love::keyboard::Keyboard::KEY_UNKNOWN},
};

StringMap<love::keyboard::Keyboard::Key, love::keyboard::Keyboard::KEY_MAX_ENUM> Event::keys(Event::keyEntries, sizeof(Event::keyEntries));

} // event
} // love
