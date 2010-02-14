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

// LOVE
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{
	static Physics * instance = 0;

	int w_newWorld(lua_State * L)
	{
		int top = lua_gettop(L);
		if (top == 2)
		{
			float x = (float)luaL_checknumber(L, 1);
			float y = (float)luaL_checknumber(L, 2);
			World * w = instance->newWorld(x, y);
			luax_newtype(L, "World", PHYSICS_WORLD_T, (void*)w);
			return 1;
		}
		else if (top == 6 || top == 7)
		{
			float lx = (float)luaL_checknumber(L, 1);
			float ly = (float)luaL_checknumber(L, 2);
			float ux = (float)luaL_checknumber(L, 3);
			float uy = (float)luaL_checknumber(L, 4);
			float gx = (float)luaL_checknumber(L, 5);
			float gy = (float)luaL_checknumber(L, 6);
			bool sleep = luax_optboolean(L, 7, true);
			World * w = instance->newWorld(lx, ly, ux, uy, gx, gy, sleep);
			luax_newtype(L, "World", PHYSICS_WORLD_T, (void*)w);
			return 1;
		}
		else
			return luaL_error(L, "Incorrect number of parameters");
	}

	int w_newBody(lua_State * L)
	{
		World * world = luax_checktype<World>(L, 1, "World", PHYSICS_WORLD_T);
		float x = (float)luaL_optnumber(L, 2, 0.0);
		float y = (float)luaL_optnumber(L, 3, 0.0);
		float m = (float)luaL_optnumber(L, 4, 0.0);
		float i = (float)luaL_optnumber(L, 5, 0.0);
		Body * body = instance->newBody(world, x, y, m, i);
		luax_newtype(L, "Body", PHYSICS_BODY_T, (void*)body);
		return 1;
	}

	int w_newCircleShape(lua_State * L)
	{
		Body * body = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		int top = lua_gettop(L);

		if(top == 2)
		{
			float radius = (float)luaL_checknumber(L, 2);
			CircleShape * shape = instance->newCircleShape(body, radius);
			luax_newtype(L, "CircleShape", PHYSICS_CIRCLE_SHAPE_T, (void*)shape);
			return 1;
		}
		else if(top == 4)
		{
			float x = (float)luaL_checknumber(L, 2);
			float y = (float)luaL_checknumber(L, 3);
			float radius = (float)luaL_checknumber(L, 4);
			CircleShape * shape = instance->newCircleShape(body, x, y, radius);
			luax_newtype(L, "CircleShape", PHYSICS_CIRCLE_SHAPE_T, (void*)shape);
			return 1;
		}
		else
			return luaL_error(L, "Incorrect number of parameters");
	}

	int w_newRectangleShape(lua_State * L)
	{
		Body * body = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		int top = lua_gettop(L);

		if(top == 3)
		{
			float w = (float)luaL_checknumber(L, 2);
			float h = (float)luaL_checknumber(L, 3);
			PolygonShape * shape = instance->newRectangleShape(body, w, h);
			luax_newtype(L, "PolygonShape", PHYSICS_POLYGON_SHAPE_T, (void*)shape);
			return 1;
		}
		else if(top == 5 || top == 6)
		{
			float x = (float)luaL_checknumber(L, 2);
			float y = (float)luaL_checknumber(L, 3);
			float w = (float)luaL_checknumber(L, 4);
			float h = (float)luaL_checknumber(L, 5);
			float angle = (float)luaL_optnumber(L, 6, 0);
			PolygonShape * shape = instance->newRectangleShape(body, x, y, w, h, angle);
			luax_newtype(L, "PolygonShape", PHYSICS_POLYGON_SHAPE_T, (void*)shape);
			return 1;
		}
		else
			return luaL_error(L, "Incorrect number of parameters");
	}

	int w_newPolygonShape(lua_State * L)
	{
		return instance->newPolygonShape(L);
	}

	int w_newDistanceJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", PHYSICS_BODY_T);
		float x1 = (float)luaL_checknumber(L, 3);
		float y1 = (float)luaL_checknumber(L, 4);
		float x2 = (float)luaL_checknumber(L, 5);
		float y2 = (float)luaL_checknumber(L, 6);
		DistanceJoint * j = instance->newDistanceJoint(body1, body2, x1, y1, x2, y2);
		luax_newtype(L, "DistanceJoint", PHYSICS_DISTANCE_JOINT_T, (void*)j);
		return 1;
	}

	int w_newMouseJoint(lua_State * L)
	{
		Body * body = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		MouseJoint * j = instance->newMouseJoint(body, x, y);
		luax_newtype(L, "MouseJoint", PHYSICS_MOUSE_JOINT_T, (void*)j);
		return 1;
	}

	int w_newRevoluteJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", PHYSICS_BODY_T);
		float x = (float)luaL_checknumber(L, 3);
		float y = (float)luaL_checknumber(L, 4);
		RevoluteJoint * j = instance->newRevoluteJoint(body1, body2, x, y);
		luax_newtype(L, "RevoluteJoint", PHYSICS_REVOLUTE_JOINT_T, (void*)j);
		return 1;
	}

	int w_newPrismaticJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", PHYSICS_BODY_T);
		float x = (float)luaL_checknumber(L, 3);
		float y = (float)luaL_checknumber(L, 4);
		float ax = (float)luaL_checknumber(L, 5);
		float ay = (float)luaL_checknumber(L, 6);
		PrismaticJoint * j = instance->newPrismaticJoint(body1, body2, x, y, ax, ay);
		luax_newtype(L, "PrismaticJoint", PHYSICS_PRISMATIC_JOINT_T, (void*)j);
		return 1;
	}

	int w_newPulleyJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", PHYSICS_BODY_T);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", PHYSICS_BODY_T);
		float gx1 = (float)luaL_checknumber(L, 3);
		float gy1 = (float)luaL_checknumber(L, 4);
		float gx2 = (float)luaL_checknumber(L, 5);
		float gy2 = (float)luaL_checknumber(L, 6);
		float x1 = (float)luaL_checknumber(L, 7);
		float y1 = (float)luaL_checknumber(L, 8);
		float x2 = (float)luaL_checknumber(L, 9);
		float y2 = (float)luaL_checknumber(L, 10);
		float ratio = (float)luaL_optnumber(L, 11, 1.0);

		PulleyJoint * j = instance->newPulleyJoint(body1, body2, b2Vec2(gx1,gy1), b2Vec2(gx2,gy2), b2Vec2(x1,y1), b2Vec2(x2,y2), ratio);
		luax_newtype(L, "PulleyJoint", PHYSICS_PULLEY_JOINT_T, (void*)j);
		return 1;
	}

	int w_newGearJoint(lua_State * L)
	{
		Joint * joint1 = luax_checktype<Joint>(L, 1, "Joint", PHYSICS_JOINT_T);
		Joint * joint2 = luax_checktype<Joint>(L, 2, "Joint", PHYSICS_JOINT_T);
		float ratio = (float)luaL_optnumber(L, 3, 1.0);

		GearJoint * j = instance->newGearJoint(joint1, joint2, ratio);
		luax_newtype(L, "GearJoint", PHYSICS_GEAR_JOINT_T, (void*)j);
		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "newWorld", w_newWorld },
		{ "newBody", w_newBody },
		{ "newCircleShape", w_newCircleShape },
		{ "newRectangleShape", w_newRectangleShape },
		{ "newPolygonShape", w_newPolygonShape },
		{ "newDistanceJoint", w_newDistanceJoint },
		{ "newMouseJoint", w_newMouseJoint },
		{ "newRevoluteJoint", w_newRevoluteJoint },
		{ "newPrismaticJoint", w_newPrismaticJoint },
		{ "newPulleyJoint", w_newPulleyJoint },
		{ "newGearJoint", w_newGearJoint },
		{ 0, 0 },
	};

	static const lua_CFunction types[] = {
		luaopen_world,
		luaopen_contact,
		luaopen_body,
		luaopen_shape,
		luaopen_circleshape,
		luaopen_polygonshape,
		luaopen_joint,
		luaopen_mousejoint,
		luaopen_distancejoint,
		luaopen_prismaticjoint,
		luaopen_revolutejoint,
		luaopen_pulleyjoint,
		luaopen_gearjoint,
		0
	};

	int luaopen_love_physics(lua_State * L)
	{
		if(instance == 0)
		{
			try
			{
				instance = new Physics();
			}
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}
		else
			instance->retain();

		WrappedModule w;
		w.module = instance;
		w.name = "physics";
		w.flags = MODULE_T;
		w.functions = functions;
		w.types = types;

		return luax_register_module(L, w);
	}

} // box2d
} // physics
} // love
