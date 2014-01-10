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

#ifndef LOVE_MODULE_H
#define LOVE_MODULE_H

// LOVE
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
	virtual ~Module();

	/**
	 * Gets the name of the module. This is used in case of errors
	 * and other messages.
	 *
	 * @return The full name of the module, eg. love.graphics.opengl.
	 **/
	virtual const char *getName() const = 0;

	/**
	 * Add module to internal registry. To be used /only/ in
	 * runtime.cpp:luax_register_module()
	 * @param instance The module instance.
	 */
	static void registerInstance(Module *instance);

	/**
	 * Retrieve module instance from internal registry. May return NULL
	 * if module not registered.
	 * @param name The full name of the module.
	 * @return Module instance or NULL if the module is not registered.
	 */
	static Module *getInstance(const std::string &name);

	/**
	 * Find the first module instance from the internal registry whose name
	 * starts with the supplied name. May return NULL if module is not
	 * registered or the supplied name is not part of any module name.
	 * @param name The partial name of the module.
	 * @return Module instance or NULL if the module is not registered.
	 **/
	static Module *findInstance(const std::string &name);

}; // Module

} // love

#endif // LOVE_MODULE_H
