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

#ifndef LOVE_EVENT_EVENT_H
#define LOVE_EVENT_EVENT_H

// LOVE
#include <common/Module.h>
#include <common/StringMap.h>
#include <common/Variant.h>
#include <keyboard/Keyboard.h>
#include <mouse/Mouse.h>
#include <thread/threads.h>

// STL
#include <queue>

namespace love
{
namespace event
{
	class Message : public Object
	{
	private:
		std::string name;
		Variant *args[4];
		int nargs;

	public:
		Message(std::string name, Variant *a = NULL, Variant *b = NULL, Variant *c = NULL, Variant *d = NULL);
		~Message();

		int toLua(lua_State *L);
		static Message *fromLua(lua_State *L, int n);
	};

	class Event : public Module
	{
	public:
		virtual ~Event();

		void push(Message *msg);
		bool poll(Message *&msg);
		virtual void clear();

		virtual void pump() = 0;

		static bool getConstant(const char * in, love::mouse::Mouse::Button & out);
		static bool getConstant(love::mouse::Mouse::Button in, const char *& out);
		static bool getConstant(const char * in, love::keyboard::Keyboard::Key & out);
		static bool getConstant(love::keyboard::Keyboard::Key in, const char *& out);

	protected:
		thread::Mutex mutex;
		std::queue<Message*> queue;
		static StringMap<love::mouse::Mouse::Button, love::mouse::Mouse::BUTTON_MAX_ENUM>::Entry buttonEntries[];
		static StringMap<love::mouse::Mouse::Button, love::mouse::Mouse::BUTTON_MAX_ENUM> buttons;
		static StringMap<love::keyboard::Keyboard::Key, love::keyboard::Keyboard::KEY_MAX_ENUM>::Entry keyEntries[];
		static StringMap<love::keyboard::Keyboard::Key, love::keyboard::Keyboard::KEY_MAX_ENUM> keys;

	}; // Event

} // event
} // love

#endif // LOVE_EVENT_EVENT_H
