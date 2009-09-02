/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_MODULE_H
#define LOVE_MODULE_H

// LOVE
#include "runtime.h"
#include "Exception.h"
#include "Object.h"

namespace love
{
	/**
	* Abstract superclass for all modules. 
	**/
	class Module : public Object
	{
	public:

		/**
		* Destructor.
		**/
		virtual ~Module(){};

		/**
		* Gets the name of the module. This is used in case of errors
		* and other messages.
		* 
		* @return The full name of the module, eg. love.graphics.opengl. 
		**/
		virtual const char * getName() const = 0;

	}; // Module

} // love

#endif // LOVE_MODULE_H
