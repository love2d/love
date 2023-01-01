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

#ifndef LOVE_PHYSICS_BOX2D_EDGE_SHAPE_H
#define LOVE_PHYSICS_BOX2D_EDGE_SHAPE_H

// Module
#include "Shape.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * An Edge is just a line segment. Edges are designed to
 * be connected and/or chained together.
 **/
class EdgeShape : public Shape
{
public:

	static love::Type type;

	/**
	 * Create a new EdgeShape from a Box2D edge shape.
	 * @param e The edge shape.
	 **/
	EdgeShape(b2EdgeShape *e, bool own = true);

	virtual ~EdgeShape();

	void setNextVertex(float x, float y);
	void setNextVertex();
	bool getNextVertex(float &x, float &y) const;

	void setPreviousVertex(float x, float y);
	void setPreviousVertex();
	bool getPreviousVertex(float &x, float &y) const;

	/**
	 * Returns the transformed points of the edge shape.
	 * This function is useful for debug drawing and such.
	 *
	 * The result can be directly passed into love.graphics.line().
	 **/
	int getPoints(lua_State *L);
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_EDGE_SHAPE_H
