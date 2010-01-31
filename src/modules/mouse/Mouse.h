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

#ifndef LOVE_MOUSE_MOUSE_H
#define LOVE_MOUSE_MOUSE_H

// LOVE
#include <common/Module.h>
#include <common/StringMap.h>

namespace love
{
namespace mouse
{
	class Mouse : public Module
	{
	public:
		
		enum Button
		{
			BUTTON_INVALID,
			BUTTON_LEFT,
			BUTTON_MIDDLE,
			BUTTON_RIGHT,
			BUTTON_WHEELUP,
			BUTTON_WHEELDOWN,
			BUTTON_X1,
			BUTTON_X2,
			BUTTON_MAX_ENUM
		};

		virtual ~Mouse(){};

		static bool getConstant(const char * in, Button & out);
		static bool getConstant(Button in, const char *& out);

	private:

		static StringMap<Button, BUTTON_MAX_ENUM>::Entry buttonEntries[];
		static StringMap<Button, BUTTON_MAX_ENUM> buttons;

	}; // Mouse

} // mouse
} // love

#endif // LOVE_MOUSE_MOUSE_H