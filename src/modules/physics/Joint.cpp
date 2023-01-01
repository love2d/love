/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "Joint.h"

namespace love
{
namespace physics
{

love::Type Joint::type("Joint", &Object::type);

Joint::~Joint()
{
}

bool Joint::getConstant(const char *in, Type &out)
{
	return types.find(in, out);
}

bool Joint::getConstant(Type in, const char  *&out)
{
	return types.find(in, out);
}

StringMap<Joint::Type, Joint::JOINT_MAX_ENUM>::Entry Joint::typeEntries[] =
{
	{"distance", Joint::JOINT_DISTANCE},
	{"revolute", Joint::JOINT_REVOLUTE},
	{"prismatic", Joint::JOINT_PRISMATIC},
	{"mouse", Joint::JOINT_MOUSE},
	{"pulley", Joint::JOINT_PULLEY},
	{"gear", Joint::JOINT_GEAR},
	{"friction", Joint::JOINT_FRICTION},
	{"weld", Joint::JOINT_WELD},
	{"wheel", Joint::JOINT_WHEEL},
	{"rope", Joint::JOINT_ROPE},
	{"motor", Joint::JOINT_MOTOR},
};

StringMap<Joint::Type, Joint::JOINT_MAX_ENUM> Joint::types(Joint::typeEntries, sizeof(Joint::typeEntries));

} // physics
} // love
