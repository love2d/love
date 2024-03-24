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

#include "Shape.h"

// Module
#include "Body.h"
#include "World.h"
#include "Physics.h"

// STD
#include <bitset>

namespace love
{
namespace physics
{
namespace box2d
{

Shape::Shape(Body *body, const b2Shape &shape)
	: shape(nullptr)
	, own(false)
	, shapeType(SHAPE_INVALID)
	, body(body)
	, fixture(nullptr)
{
	if (body)
	{
		b2FixtureDef def;
		def.shape = &shape;
		def.userData.pointer = (uintptr_t)this;

		// 0 density stops CreateFixture from calling b2Body::ResetMassData().
		def.density = body->hasCustomMassData() ? 0.0f : 1.0f;

		fixture = body->body->CreateFixture(&def);
		this->shape = fixture->GetShape();

		if (body->hasCustomMassData())
			setDensity(1.0f);

		retain(); // Shape::destroy does the release().
	}
	else
	{
		// Path to support deprecated APIs.
		auto physics = Module::getInstance<Physics>(Module::M_PHYSICS);
		this->shape = shape.Clone(physics->getBlockAllocator());
		own = true;
	}

	switch (this->shape->GetType())
	{
	case b2Shape::e_circle:
		shapeType = SHAPE_CIRCLE;
		break;
	case b2Shape::e_polygon:
		shapeType = SHAPE_POLYGON;
		break;
	case b2Shape::e_edge:
		shapeType = SHAPE_EDGE;
		break;
	case b2Shape::e_chain:
		shapeType = SHAPE_CHAIN;
		break;
	default:
		shapeType = SHAPE_INVALID;
		break;
	}
}

Shape::~Shape()
{
	if (shape && own)
	{
		auto physics = Module::getInstance<Physics>(Module::M_PHYSICS);
		auto allocator = physics->getBlockAllocator();

		// Taken from b2Fixture::Destroy. Not very pretty...
		switch (shapeType)
		{
		case SHAPE_CIRCLE:
		{
			b2CircleShape *s = (b2CircleShape*)shape;
			s->~b2CircleShape();
			allocator->Free(s, sizeof(b2CircleShape));
			break;
		}
		case SHAPE_EDGE:
		{
			b2EdgeShape *s = (b2EdgeShape*)shape;
			s->~b2EdgeShape();
			allocator->Free(s, sizeof(b2EdgeShape));
			break;
		}
		case SHAPE_POLYGON:
		{
			b2PolygonShape *s = (b2PolygonShape*)shape;
			s->~b2PolygonShape();
			allocator->Free(s, sizeof(b2PolygonShape));
			break;
		}
		case SHAPE_CHAIN:
		{
			b2ChainShape *s = (b2ChainShape*)shape;
			s->~b2ChainShape();
			allocator->Free(s, sizeof(b2ChainShape));
			break;
		}
		default:
			break;
		}
	}

	if (ref)
		delete ref;
}

void Shape::destroy(bool implicit)
{
	if (fixture == nullptr)
		return;

	if (body->world->world->IsLocked())
	{
		// Called during time step. Save reference for destruction afterwards.
		this->retain();
		body->world->destructShapes.push_back(this);
		return;
	}

	if (!implicit && fixture != nullptr)
		body->body->DestroyFixture(fixture);

	fixture = nullptr;
	shape = nullptr;
	body = nullptr;

	// Remove userdata reference to avoid it sticking around after GC
	if (ref)
		ref->unref();

	// Box2D fixture destroyed. Release its reference to the love Shape.
	release();
}

void Shape::throwIfFixtureNotValid() const
{
	if (fixture == nullptr)
		throw love::Exception("Shape must be active in the physics World to use this method.");
}

void Shape::throwIfShapeNotValid() const
{
	if (shape == nullptr)
		throw love::Exception("Cannot call this method on a destroyed Shape.");
}

Shape::Type Shape::getType() const
{
	return shapeType;
}

void Shape::setFriction(float friction)
{
	throwIfFixtureNotValid();
	fixture->SetFriction(friction);
}

void Shape::setRestitution(float restitution)
{
	throwIfFixtureNotValid();
	fixture->SetRestitution(restitution);
}

void Shape::setDensity(float density)
{
	throwIfFixtureNotValid();
	fixture->SetDensity(density);
	if (!body->hasCustomMassData())
		body->resetMassData();
}

void Shape::setSensor(bool sensor)
{
	throwIfFixtureNotValid();
	fixture->SetSensor(sensor);
}

float Shape::getFriction() const
{
	throwIfFixtureNotValid();
	return fixture->GetFriction();
}

float Shape::getRestitution() const
{
	throwIfFixtureNotValid();
	return fixture->GetRestitution();
}

float Shape::getDensity() const
{
	throwIfFixtureNotValid();
	return fixture->GetDensity();
}

bool Shape::isSensor() const
{
	throwIfFixtureNotValid();
	return fixture->IsSensor();
}

Body *Shape::getBody() const
{
	return body;
}

float Shape::getRadius() const
{
	throwIfShapeNotValid();
	return Physics::scaleUp(shape->m_radius);
}

int Shape::getChildCount() const
{
	throwIfShapeNotValid();
	return shape->GetChildCount();
}

void Shape::setFilterData(int *v)
{
	throwIfFixtureNotValid();
	b2Filter f;
	f.categoryBits = (uint16) v[0];
	f.maskBits = (uint16) v[1];
	f.groupIndex = (int16) v[2];
	fixture->SetFilterData(f);
}

void Shape::getFilterData(int *v)
{
	throwIfFixtureNotValid();
	b2Filter f = fixture->GetFilterData();
	v[0] = (int) f.categoryBits;
	v[1] = (int) f.maskBits;
	v[2] = (int) f.groupIndex;
}

int Shape::setCategory(lua_State *L)
{
	throwIfFixtureNotValid();
	b2Filter f = fixture->GetFilterData();
	f.categoryBits = (uint16)getBits(L);
	fixture->SetFilterData(f);
	return 0;
}

int Shape::setMask(lua_State *L)
{
	throwIfFixtureNotValid();
	b2Filter f = fixture->GetFilterData();
	f.maskBits = ~(uint16)getBits(L);
	fixture->SetFilterData(f);
	return 0;
}

void Shape::setGroupIndex(int index)
{
	throwIfFixtureNotValid();
	b2Filter f = fixture->GetFilterData();
	f.groupIndex = (uint16)index;
	fixture->SetFilterData(f);
}

int Shape::getGroupIndex() const
{
	throwIfFixtureNotValid();
	b2Filter f = fixture->GetFilterData();
	return f.groupIndex;
}

int Shape::getCategory(lua_State *L)
{
	throwIfFixtureNotValid();
	return pushBits(L, fixture->GetFilterData().categoryBits);
}

int Shape::getMask(lua_State *L)
{
	throwIfFixtureNotValid();
	return pushBits(L, ~(fixture->GetFilterData().maskBits));
}

uint16 Shape::getBits(lua_State *L)
{
	// Get number of args.
	bool istable = lua_istable(L, 1);
	int argc = istable ? (int) luax_objlen(L, 1) : lua_gettop(L);

	// The new bitset.
	std::bitset<16> b;

	for (int i = 1; i <= argc; i++)
	{
		size_t bpos = 0;

		if (istable)
		{
			lua_rawgeti(L, 1, i);
			bpos = (size_t) (lua_tointeger(L, -1) - 1);
			lua_pop(L, 1);
		}
		else
			bpos = (size_t) (lua_tointeger(L, i) - 1);

		if (bpos >= 16)
			luaL_error(L, "Values must be in range 1-16.");

		b.set(bpos, true);
	}

	return (uint16)b.to_ulong();
}

int Shape::pushBits(lua_State *L, uint16 bits)
{
	// Create a bitset.
	std::bitset<16> b((int)bits);

	// Push all set bits.
	for (int i = 0; i<16; i++)
		if (b.test(i))
			lua_pushinteger(L, i+1);

	// Count number of set bits.
	return (int)b.count();
}

int Shape::setUserData(lua_State *L)
{
	love::luax_assert_argc(L, 1, 1);

	if(!ref)
		ref = new Reference();

	ref->ref(L);

	return 0;
}

int Shape::getUserData(lua_State *L)
{
	if (ref != nullptr)
		ref->push(L);
	else
		lua_pushnil(L);

	return 1;
}

bool Shape::testPoint(float x, float y) const
{
	throwIfFixtureNotValid();
	return fixture->TestPoint(Physics::scaleDown(b2Vec2(x, y)));
}

bool Shape::testPoint(float x, float y, float r, float px, float py) const
{
	throwIfShapeNotValid();
	b2Vec2 point(px, py);
	b2Transform transform(Physics::scaleDown(b2Vec2(x, y)), b2Rot(r));
	return shape->TestPoint(transform, Physics::scaleDown(point));
}

int Shape::rayCast(lua_State *L) const
{
	float p1x = Physics::scaleDown((float)luaL_checknumber(L, 1));
	float p1y = Physics::scaleDown((float)luaL_checknumber(L, 2));
	float p2x = Physics::scaleDown((float)luaL_checknumber(L, 3));
	float p2y = Physics::scaleDown((float)luaL_checknumber(L, 4));
	float maxFraction = (float)luaL_checknumber(L, 5);

	b2RayCastInput input;
	b2RayCastOutput output;
	input.p1.Set(p1x, p1y);
	input.p2.Set(p2x, p2y);
	input.maxFraction = maxFraction;

	if (lua_isnoneornil(L, 7))
	{
		throwIfFixtureNotValid();
		int childIndex = (int) luaL_optinteger(L, 6, 1) - 1; // Convert from 1-based index
		if (!fixture->RayCast(&output, input, childIndex))
			return 0; // Nothing hit.
	}
	else
	{
		throwIfShapeNotValid();
		float x = Physics::scaleDown((float)luaL_checknumber(L, 6));
		float y = Physics::scaleDown((float)luaL_checknumber(L, 7));
		float r = (float)luaL_checknumber(L, 8);
		int childIndex = (int) luaL_optinteger(L, 9, 1) - 1; // Convert from 1-based index
	
		b2Transform transform(b2Vec2(x, y), b2Rot(r));
		
		if (!shape->RayCast(&output, input, transform, childIndex))
			return 0; // No hit.
	}

	lua_pushnumber(L, output.normal.x);
	lua_pushnumber(L, output.normal.y);
	lua_pushnumber(L, output.fraction);
	return 3;
}

int Shape::computeAABB(lua_State *L) const
{
	throwIfShapeNotValid();
	float x = Physics::scaleDown((float)luaL_checknumber(L, 1));
	float y = Physics::scaleDown((float)luaL_checknumber(L, 2));
	float r = (float)luaL_checknumber(L, 3);
	int childIndex = (int) luaL_optinteger(L, 4, 1) - 1; // Convert from 1-based index
	b2Transform transform(b2Vec2(x, y), b2Rot(r));
	b2AABB box;
	shape->ComputeAABB(&box, transform, childIndex);
	box = Physics::scaleUp(box);
	lua_pushnumber(L, box.lowerBound.x);
	lua_pushnumber(L, box.lowerBound.y);
	lua_pushnumber(L, box.upperBound.x);
	lua_pushnumber(L, box.upperBound.y);
	return 4;
}

int Shape::computeMass(lua_State *L) const
{
	throwIfShapeNotValid();
	float density = (float)luaL_checknumber(L, 1);
	b2MassData data;
	shape->ComputeMass(&data, density);
	b2Vec2 center = Physics::scaleUp(data.center);
	lua_pushnumber(L, center.x);
	lua_pushnumber(L, center.y);
	lua_pushnumber(L, data.mass);
	lua_pushnumber(L, Physics::scaleUp(Physics::scaleUp(data.I)));
	return 4;
}

int Shape::getBoundingBox(lua_State *L) const
{
	throwIfFixtureNotValid();
	int childIndex = (int) luaL_optinteger(L, 1, 1) - 1; // Convert from 1-based index
	b2AABB box;
	luax_catchexcept(L, [&]() { box = fixture->GetAABB(childIndex); });
	box = Physics::scaleUp(box);
	lua_pushnumber(L, box.lowerBound.x);
	lua_pushnumber(L, box.lowerBound.y);
	lua_pushnumber(L, box.upperBound.x);
	lua_pushnumber(L, box.upperBound.y);
	return 4;
}

int Shape::getMassData(lua_State *L) const
{
	throwIfFixtureNotValid();
	b2MassData data;
	fixture->GetMassData(&data);
	b2Vec2 center = Physics::scaleUp(data.center);
	lua_pushnumber(L, center.x);
	lua_pushnumber(L, center.y);
	lua_pushnumber(L, data.mass);
	lua_pushnumber(L, data.I);
	return 4;
}

} // box2d
} // physics
} // love
