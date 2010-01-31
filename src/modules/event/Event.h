/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#ifndef LOVE_EVENT_EVENT_H
#define LOVE_EVENT_EVENT_H

// LOVE
#include <common/Module.h>
#include <common/StringMap.h>
#include <keyboard/Keyboard.h>
#include <mouse/Mouse.h>

namespace love
{
namespace event
{
	class Event : public Module
	{
	public:

		enum Type
		{
		   TYPE_INVALID,
		   TYPE_KEY_PRESSED,
		   TYPE_KEY_RELEASED,
		   TYPE_MOUSE_PRESSED,
		   TYPE_MOUSE_RELEASED,
		   TYPE_JOYSTICK_RELEASED,
		   TYPE_JOYSTICK_PRESSED,
		   TYPE_QUIT,
		   TYPE_MAX_ENUM = 32
		};

		union Message
		{
			Type type;

			struct 
			{
				Type type;
				love::mouse::Mouse::Button b;
				unsigned x;
				unsigned y;
			} mouse;

			struct
			{
				Type type;
				unsigned index;
				unsigned button;
			} joystick;

			struct
			{
				Type type;
				love::keyboard::Keyboard::Key k;
				unsigned short u;
			} keyboard;
		};

		virtual ~Event();

		static bool getConstant(const char * in, Type & out);
		static bool getConstant(Type in, const char *& out);
		static bool getConstant(const char * in, love::mouse::Mouse::Button & out);
		static bool getConstant(love::mouse::Mouse::Button in, const char *& out);
		static bool getConstant(const char * in, love::keyboard::Keyboard::Key & out);
		static bool getConstant(love::keyboard::Keyboard::Key in, const char *& out);

	private:

		static StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[];
		static StringMap<Type, TYPE_MAX_ENUM> types;
		static StringMap<love::mouse::Mouse::Button, love::mouse::Mouse::BUTTON_MAX_ENUM>::Entry buttonEntries[];
		static StringMap<love::mouse::Mouse::Button, love::mouse::Mouse::BUTTON_MAX_ENUM> buttons;
		static StringMap<love::keyboard::Keyboard::Key, love::keyboard::Keyboard::KEY_MAX_ENUM>::Entry keyEntries[];
		static StringMap<love::keyboard::Keyboard::Key, love::keyboard::Keyboard::KEY_MAX_ENUM> keys;

	}; // Event

} // event
} // love

#endif // LOVE_EVENT_EVENT_H
