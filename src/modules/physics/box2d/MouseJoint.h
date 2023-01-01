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

#ifndef LOVE_PHYSICS_BOX2D_MOUSE_JOINT_H
#define LOVE_PHYSICS_BOX2D_MOUSE_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * The MouseJoint is a joint type that
 * is suitable for controlling objects with the mouse.
 *
 * One end is anchored in the dynamic body, and the other id
 * anchor to a static ground body. The anchor offset can then be
 * moved to the current mouse position.
 **/
class MouseJoint : public Joint
{
public:

	static love::Type type;

	/**
	 * Creates a MouseJoint which connects body1 to the target point.
	 **/
	MouseJoint(Body *body1, float x, float y);

	virtual ~MouseJoint();

	/**
	 * Sets the target of anchor2. (You usually want
	 * to set this to the current mouse.)
	 **/
	void setTarget(float x, float y);

	/**
	 * Gets the current anchor2 target.
	 **/
	int getTarget(lua_State *L);

	/**
	 * Sets the maximum constraint force that can be exerted
	 * to move the candidate body.
	 **/
	void setMaxForce(float force);

	/**
	 * Gets the maximum constraint force that can be exerted
	 * to move the candidate body.
	 **/
	float getMaxForce() const;

	/**
	 * Sets the response speed.
	 **/
	void setFrequency(float hz);

	/**
	 * Gets the response speed.
	 **/
	float getFrequency() const;

	/**
	 * Sets the damping ratio.
	 * 0 = no damping, 1 = critical damping.
	 **/
	void setDampingRatio(float d);

	/**
	 * Gets the damping ratio.
	 * 0 = no damping, 1 = critical damping.
	 **/
	float getDampingRatio() const;

	virtual Body *getBodyA() const;
	virtual Body *getBodyB() const;

private:
	// The Box2D MouseJoint object.
	b2MouseJoint *joint;
};

} // box2d
} // physics
} // love


#endif // LOVE_PHYSICS_BOX2D_MOUSE_JOINT_H
