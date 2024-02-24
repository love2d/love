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

	static love::Type type;

	enum ModuleType
	{
		M_UNKNOWN = -1, // Use this for modules outside of LOVE's source code.
		M_AUDIO,
		M_DATA,
		M_EVENT,
		M_FILESYSTEM,
		M_FONT,
		M_GRAPHICS,
		M_IMAGE,
		M_JOYSTICK,
		M_KEYBOARD,
		M_MATH,
		M_MOUSE,
		M_PHYSICS,
		M_SENSOR,
		M_SOUND,
		M_SYSTEM,
		M_THREAD,
		M_TIMER,
		M_TOUCH,
		M_VIDEO,
		M_WINDOW,
		M_MAX_ENUM
	};

	Module(ModuleType moduleType, const char *name);
	virtual ~Module();

    /**
     * Gets the base type of the module.
     **/
	ModuleType getModuleType() const { return moduleType; }

	/**
	 * Gets the name of the module. This is used in case of errors
	 * and other messages.
	 *
	 * @return The full name of the module, eg. love.graphics.opengl.
	 **/
	const char *getName() const { return name.c_str(); }

	/**
	 * Retrieve module instance from internal registry. May return NULL
	 * if module not registered.
	 * @param name The full name of the module.
	 * @return Module instance or NULL if the module is not registered.
	 */
	static Module *getInstance(const std::string &name);

	/**
	 * Retrieve module instance from the internal registry using the base
	 * module type. May return null if the module is not registered.
	 * @param type The base type of the module.
	 **/
	template <typename T>
	static T *getInstance(ModuleType type)
	{
		return type != M_UNKNOWN ? (T *) instances[type] : nullptr;
	}

private:

	static void registerInstance(Module *instance);

	ModuleType moduleType;
	std::string name;

	static Module *instances[M_MAX_ENUM];

}; // Module

} // love

#endif // LOVE_MODULE_H
