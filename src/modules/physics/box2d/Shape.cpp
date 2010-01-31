/**
* Copyright (c) 2006-2010 LOVE Development Team
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
	Shape::Shape(Body * body)
		: body(body)
	{
		body->retain();
		data = new shapeudata();
		data->ref = 0;
	}

	Shape::~Shape()
	{
		if(data->ref != 0)
			delete data->ref;

		delete data;
		data = 0;

		body->body->DestroyShape(shape);
		shape = 0;

		body->release();
	}

	Shape::Type Shape::getType() const
	{
		switch(shape->GetType())
		{
		case e_circleShape:
			return SHAPE_CIRCLE;
		case e_polygonShape:
			return SHAPE_POLYGON;
		default:
			return SHAPE_INVALID;
		}
	}

	void Shape::setFriction(float friction)
	{
		shape->m_friction = friction;
	}

	void Shape::setRestitution(float restitution)
	{
		shape->m_restitution = restitution;
	}

	void Shape::setDensity(float density)
	{
		shape->m_density = density;
	}

	void Shape::setSensor(bool sensor)
	{
		shape->m_isSensor = sensor;
	}

	float Shape::getFriction() const
	{
		return shape->GetFriction();
	}

	float Shape::getRestitution() const
	{
		return shape->GetRestitution();
	}

	float Shape::getDensity() const
	{
		return shape->m_density;
	}

	bool Shape::isSensor() const
	{
		return shape->IsSensor();
	}

	Body * Shape::getBody() const
	{
		return body;
	}

	bool Shape::testPoint(float x, float y) const
	{
		return shape->TestPoint(shape->GetBody()->GetXForm(), body->world->scaleDown(b2Vec2(x, y)));
	}

	int Shape::testSegment(lua_State * L)
	{
		love::luax_assert_argc(L, 4, 4);

		b2Segment s;

		s.p1.x = (float)lua_tonumber(L, 1);
		s.p1.y = (float)lua_tonumber(L, 2);
		s.p2.x = (float)lua_tonumber(L, 3);
		s.p2.y = (float)lua_tonumber(L, 4);

		s.p1 = body->world->scaleDown(s.p1);
		s.p2 = body->world->scaleDown(s.p2);

		float lambda;
		b2Vec2 normal;

		if(shape->TestSegment(shape->GetBody()->GetXForm(), &lambda, &normal, s, 1.0f))
		{
			lua_pushnumber(L, lambda);
			normal = body->world->scaleUp(normal);
			lua_pushnumber(L, normal.x);
			lua_pushnumber(L, normal.y);
			return 3;
		}

		return 0;
	}

	void Shape::setFilterData(int * v)
	{
		b2FilterData f;
		f.categoryBits = (unsigned short)v[0];
		f.maskBits = (unsigned short)v[1];
		f.groupIndex = v[2];
		shape->SetFilterData(f);
		shape->GetBody()->GetWorld()->Refilter(shape);
	}

	void Shape::getFilterData(int * v)
	{
		b2FilterData f = shape->GetFilterData();
		v[0] = (int)f.categoryBits;
		v[1] = (int)f.maskBits;
		v[2] = f.groupIndex;
	}

	int Shape::setCategory(lua_State * L)
	{
		b2FilterData f = shape->GetFilterData();
		f.categoryBits = (uint16)getBits(L);
		shape->SetFilterData(f);
		shape->GetBody()->GetWorld()->Refilter(shape);
		return 0;
	}

	int Shape::setMask(lua_State * L)
	{
		b2FilterData f = shape->GetFilterData();
		f.maskBits = ~(uint16)getBits(L);
		shape->SetFilterData(f);
		shape->GetBody()->GetWorld()->Refilter(shape);
		return 0;
	}

	void Shape::setGroupIndex(int index)
	{
		b2FilterData f = shape->GetFilterData();
		f.groupIndex = (uint16)index;
		shape->SetFilterData(f);
		shape->GetBody()->GetWorld()->Refilter(shape);
	}

	int Shape::getGroupIndex() const
	{
		b2FilterData f = shape->GetFilterData();
		return f.groupIndex;
	}

	int Shape::getCategory(lua_State * L)
	{
		return pushBits(L, shape->GetFilterData().categoryBits);
	}

	int Shape::getMask(lua_State * L)
	{
		return pushBits(L, ~(shape->GetFilterData().maskBits));
	}

	uint16 Shape::getBits(lua_State * L)
	{
		// Get number of args.
		int argc = lua_gettop(L);

		// The new bitset.
		std::bitset<16> b;

		for(int i = 1;i<=argc;i++)
		{
			size_t bpos = (size_t)(lua_tointeger(L, i)-1);
			if(bpos < 0 || bpos > 16)
				return luaL_error(L, "Values must be in range 1-16.");
			b.set(bpos, true);
		}

		return (uint16)b.to_ulong();
	}

	int Shape::pushBits(lua_State * L, uint16 bits)
	{
		// Create a bitset.
		std::bitset<16> b((unsigned long)bits);

		// Push all set bits.
		for(int i = 0;i<16;i++)
			if(b.test(i))
				lua_pushinteger(L, i+1);

		// Count number of set bits.
		return (int)b.count();
	}

	int Shape::setData(lua_State * L)
	{
		love::luax_assert_argc(L, 1, 1);

		if(data->ref != 0)
		{
			delete data->ref;
			data->ref = 0;
		}

		data->ref = new Reference(L);
		return 0;
	}

	int Shape::getData(lua_State * L)
	{
		love::luax_assert_argc(L, 0, 0);
		if(data->ref != 0)
			data->ref->push();
		else
			lua_pushnil(L);

		return 1;
	}

	int Shape::getBoundingBox(lua_State * L)
	{
		love::luax_assert_argc(L, 0, 0);
		b2AABB bb;
		shape->ComputeAABB(&bb, shape->GetBody()->GetXForm());
		bb = body->world->scaleUp(bb);

		// Top left.
		lua_pushnumber(L, bb.lowerBound.x);
		lua_pushnumber(L, bb.upperBound.y);

		// Bottom left.
		lua_pushnumber(L, bb.lowerBound.x);
		lua_pushnumber(L, bb.lowerBound.y);

		// Bottom right.
		lua_pushnumber(L, bb.upperBound.x);
		lua_pushnumber(L, bb.lowerBound.y);

		// Top right.
		lua_pushnumber(L, bb.upperBound.x);
		lua_pushnumber(L, bb.upperBound.y);

		return 8;
	}

} // box2d
} // physics
} // love
