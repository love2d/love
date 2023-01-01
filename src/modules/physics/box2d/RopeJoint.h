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

#ifndef LOVE_PHYSICS_BOX2D_ROPE_JOINT_H
#define LOVE_PHYSICS_BOX2D_ROPE_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * The RopeJoint enforces a maximum distance between two points
 * on two bodies. It has no other effect.
 **/
class RopeJoint : public Joint
{
public:

	static love::Type type;

	/**
	 * Creates a RopeJoint connecting body1 to body2.
	 **/
	RopeJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, float maxLength, bool collideConnected);

	virtual ~RopeJoint();

	/**
	 * Gets the maximum length of the rope.
	 **/
	float getMaxLength() const;
	void setMaxLength(float length);

private:
	// The Box2D RopeJoint object.
	b2RopeJoint *joint;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_ROPE_JOINT_H
