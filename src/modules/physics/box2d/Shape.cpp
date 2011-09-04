/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "Shape.h"

// Module
#include "Body.h"
#include "World.h"

// STD
#include <bitset>

namespace love
{
namespace physics
{
namespace box2d
{
	Shape::Shape()
		: shape(NULL)
	{
	}

	Shape::~Shape()
	{
		shape = 0;
	}

	Shape::Type Shape::getType() const
	{
		switch(shape->GetType())
		{
		case b2Shape::e_circle:
			return SHAPE_CIRCLE;
		case b2Shape::e_polygon:
			return SHAPE_POLYGON;
		case b2Shape::e_edge:
			return SHAPE_EDGE;
		case b2Shape::e_chain:
			return SHAPE_CHAIN;
		default:
			return SHAPE_INVALID;
		}
	}

} // box2d
} // physics
} // love
