/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#include "Body.h"

#include <common/math.h>

#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Body::Body(World * world, b2Vec2 p, float m, float i)
		: world(world)
	{
		world->retain();
		b2BodyDef def;
		def.position = world->scaleDown(p);
		def.massData.mass = m;
		def.massData.I = i;
		body = world->world->CreateBody(&def);
	}

	Body::~Body()
	{
		world->world->DestroyBody(body);
		world->release();
		body = 0;
	}

	float Body::getX()
	{
		return world->scaleUp(body->GetPosition().x);
	}

	float Body::getY()
	{
		return world->scaleUp(body->GetPosition().y);
	}

	int Body::getPosition(lua_State * L)
	{
		return pushVector(L, world->scaleUp(body->GetPosition()));
	}

	int Body::getLinearVelocity(lua_State * L)
	{
		return pushVector(L, world->scaleUp(body->GetLinearVelocity()));
	}

	float Body::getAngle()
	{
		return body->GetAngle();
	}

	int Body::getWorldCenter(lua_State * L)
	{
		return pushVector(L, world->scaleUp(body->GetWorldCenter()));
	}

	int Body::getLocalCenter(lua_State * L)
	{
		return pushVector(L, world->scaleUp(body->GetLocalCenter()));
	}

	float Body::getAngularVelocity() const
	{
		return body->GetAngularVelocity();
	}

	float Body::getMass() const
	{
		return body->GetMass();
	}

	float Body::getInertia() const
	{
		return body->GetInertia();
	}

	float Body::getAngularDamping() const
	{
		return body->m_angularDamping;
	}

	float Body::getLinearDamping() const
	{
		return body->m_linearDamping;
	}

	void Body::applyImpulse(float jx, float jy)
	{
		body->ApplyImpulse(b2Vec2(jx, jy), world->scaleDown(body->GetWorldCenter()));
	}

	void Body::applyImpulse(float jx, float jy, float rx, float ry)
	{
		body->ApplyImpulse(b2Vec2(jx, jy), world->scaleDown(b2Vec2(rx, ry)));
	}

	void Body::applyTorque(float t)
	{
		body->ApplyTorque(t);
	}

	void Body::applyForce(float fx, float fy, float rx, float ry)
	{
		body->ApplyForce(b2Vec2(fx, fy), world->scaleDown(b2Vec2(rx, ry)));
	}

	void Body::applyForce(float fx, float fy)
	{
		body->ApplyForce(b2Vec2(fx, fy), world->scaleDown(body->GetWorldCenter()));
	}

	void Body::setX(float x)
	{
		body->SetXForm(world->scaleDown(b2Vec2(x, getY())), getAngle());
	}

	void Body::setY(float y)
	{
		body->SetXForm(world->scaleDown(b2Vec2(getX(), y)), getAngle());
	}

	void Body::setLinearVelocity(float x, float y)
	{
		body->SetLinearVelocity(world->scaleDown(b2Vec2(x, y)));
	}

	void Body::setAngle(float d)
	{
		body->SetXForm(body->GetPosition(), d);
	}

	void Body::setAngularVelocity(float r)
	{
		body->SetAngularVelocity(r);
	}

	void Body::setPosition(float x, float y)
	{
		body->SetXForm(world->scaleDown(b2Vec2(x, y)), body->GetAngle());
	}

	void Body::setAngularDamping(float d)
	{
		body->m_angularDamping = d;
	}

	void Body::setLinearDamping(float d)
	{
		body->m_linearDamping = d;
	}

	void Body::setMassFromShapes()
	{
		body->SetMassFromShapes();
	}

	void Body::setMass(float x, float y, float m, float i)
	{
		b2MassData massData;
		massData.center = world->scaleDown(b2Vec2(x, y));
		massData.mass = m;
		massData.I = i;
		body->SetMass(&massData);
	}

	int Body::getWorldPoint(lua_State * L)
	{
		b2Vec2 v = world->scaleDown(getVector(L));
		return pushVector(L, world->scaleUp(body->GetWorldPoint(v)));
	}

	int Body::getWorldVector(lua_State * L)
	{
		b2Vec2 v = world->scaleDown(getVector(L));
		return pushVector(L, world->scaleUp(body->GetWorldVector(v)));
	}

	int Body::getLocalPoint(lua_State * L)
	{
		b2Vec2 v = world->scaleDown(getVector(L));
		return pushVector(L, world->scaleUp(body->GetLocalPoint(v)));
	}

	int Body::getLocalVector(lua_State * L)
	{
		b2Vec2 v = world->scaleDown(getVector(L));
		return pushVector(L, world->scaleUp(body->GetLocalVector(v)));
	}

	int Body::getLinearVelocityFromWorldPoint(lua_State * L)
	{
		b2Vec2 v = world->scaleDown(getVector(L));
		return pushVector(L, world->scaleUp(body->GetLinearVelocityFromWorldPoint(v)));
	}

	int Body::getLinearVelocityFromLocalPoint(lua_State * L)
	{
		b2Vec2 v = world->scaleDown(getVector(L));
		return pushVector(L, world->scaleUp(body->GetLinearVelocityFromLocalPoint(v)));
	}

	bool Body::isBullet() const
	{
		return body->IsBullet();
	}

	void Body::setBullet(bool bullet)
	{
		return body->SetBullet(bullet);
	}

	bool Body::isStatic() const
	{
		return body->IsStatic();
	}

	bool Body::isDynamic() const
	{
		return body->IsDynamic();
	}

	bool Body::isFrozen() const
	{
		return body->IsFrozen();
	}

	bool Body::isSleeping() const
	{
		return body->IsSleeping();
	}

	void Body::setAllowSleeping(bool allow)
	{
		body->AllowSleeping(true);
	}

	void Body::putToSleep()
	{
		body->PutToSleep();
	}

	void Body::wakeUp()
	{
		body->WakeUp();
	}

	b2Vec2 Body::getVector(lua_State * L)
	{
		love::luax_assert_argc(L, 2, 2);
		b2Vec2 v((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2));
		lua_pop(L, 2);
		return v;
	}

	int Body::pushVector(lua_State * L, const b2Vec2 & v)
	{
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		return 2;
	}

} // box2d
} // physics
} // love
