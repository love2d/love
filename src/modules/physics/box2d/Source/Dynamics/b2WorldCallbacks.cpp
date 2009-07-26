/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "b2WorldCallbacks.h"
#include "../Collision/Shapes/b2Shape.h"

b2ContactFilter b2_defaultFilter;

// Return true if contact calculations should be performed between these two shapes.
// If you implement your own collision filter you may want to build from this implementation.
bool b2ContactFilter::ShouldCollide(b2Shape* shape1, b2Shape* shape2)
{
	const b2FilterData& filter1 = shape1->GetFilterData();
	const b2FilterData& filter2 = shape2->GetFilterData();

	if (filter1.groupIndex == filter2.groupIndex && filter1.groupIndex != 0)
	{
		return filter1.groupIndex > 0;
	}

	bool collide = (filter1.maskBits & filter2.categoryBits) != 0 && (filter1.categoryBits & filter2.maskBits) != 0;
	return collide;
}

b2DebugDraw::b2DebugDraw()
{
	m_drawFlags = 0;
}

void b2DebugDraw::SetFlags(uint32 flags)
{
	m_drawFlags = flags;
}

uint32 b2DebugDraw::GetFlags() const
{
	return m_drawFlags;
}

void b2DebugDraw::AppendFlags(uint32 flags)
{
	m_drawFlags |= flags;
}

void b2DebugDraw::ClearFlags(uint32 flags)
{
	m_drawFlags &= ~flags;
}
