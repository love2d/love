/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_PHYSICS_JOINT_H
#define LOVE_PHYSICS_JOINT_H

// LOVE
#include "common/Object.h"
#include "common/StringMap.h"

namespace love
{
namespace physics
{

class Joint : public Object
{
public:

	enum Type
	{
		JOINT_INVALID,
		JOINT_DISTANCE,
		JOINT_REVOLUTE,
		JOINT_PRISMATIC,
		JOINT_MOUSE,
		JOINT_PULLEY,
		JOINT_GEAR,
		JOINT_FRICTION,
		JOINT_WELD,
		JOINT_WHEEL,
		JOINT_ROPE,
		JOINT_MOTOR,
		JOINT_MAX_ENUM
	};


	virtual ~Joint();

	static bool getConstant(const char *in, Type &out);
	static bool getConstant(Type in, const char  *&out);

private:

	static StringMap<Type, JOINT_MAX_ENUM>::Entry typeEntries[];
	static StringMap<Type, JOINT_MAX_ENUM> types;
};

} // physics
} // love

#endif // LOVE_PHYSICS_JOINT_H
