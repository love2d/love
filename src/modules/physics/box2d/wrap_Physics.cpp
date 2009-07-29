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

// LOVE
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{
	static Physics * instance = 0;

	int _wrap_newWorld(lua_State * L)
	{
		if(lua_gettop(L) == 2)
		{
			float x = (float)luaL_checknumber(L, 1);
			float y = (float)luaL_checknumber(L, 2);
			World * w = instance->newWorld(x, y);
			luax_newtype(L, "World", LOVE_PHYSICS_WORLD_BITS, (void*)w);
			return 1;
		}
		else if(lua_gettop(L) == 6)
		{
			float lx = (float)luaL_checknumber(L, 1);
			float ly = (float)luaL_checknumber(L, 2);
			float ux = (float)luaL_checknumber(L, 3);
			float uy = (float)luaL_checknumber(L, 4);
			float gx = (float)luaL_checknumber(L, 5);
			float gy = (float)luaL_checknumber(L, 6);
			bool sleep = luax_toboolean(L, 7);
			World * w = instance->newWorld(lx, ly, ux, uy, gx, gy, sleep);
			luax_newtype(L, "World", LOVE_PHYSICS_WORLD_BITS, (void*)w);
			return 1;
		}
		else
			return luaL_error(L, "Incorrect number of parameters");
	}

	int _wrap_newBody(lua_State * L)
	{
		World * world = luax_checktype<World>(L, 1, "World", LOVE_PHYSICS_WORLD_BITS);
		float x = (float)luaL_optnumber(L, 2, 0.0);
		float y = (float)luaL_optnumber(L, 3, 0.0);
		float m = (float)luaL_optnumber(L, 4, 1.0);
		float i = (float)luaL_optnumber(L, 5, 1.0);
		Body * body = instance->newBody(world, x, y, m, i);
		luax_newtype(L, "Body", LOVE_PHYSICS_BODY_BITS, (void*)body);
		return 1;
	}

	int _wrap_newCircleShape(lua_State * L)
	{
		Body * body = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		int top = lua_gettop(L);

		if(top == 2)
		{
			float radius = (float)luaL_checknumber(L, 2);
			CircleShape * shape = instance->newCircleShape(body, radius);
			luax_newtype(L, "CircleShape", LOVE_PHYSICS_CIRCLE_SHAPE_BITS, (void*)shape);
			return 1;
		}
		else if(top == 4)
		{
			float x = (float)luaL_checknumber(L, 2);
			float y = (float)luaL_checknumber(L, 3);
			float radius = (float)luaL_checknumber(L, 4);
			CircleShape * shape = instance->newCircleShape(body, x, y, radius);
			luax_newtype(L, "CircleShape", LOVE_PHYSICS_CIRCLE_SHAPE_BITS, (void*)shape);
			return 1;
		}
		else
			return luaL_error(L, "Incorrect number of parameters");
	}

	int _wrap_newRectangleShape(lua_State * L)
	{
		Body * body = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		int top = lua_gettop(L);

		if(top == 3)
		{
			float w = (float)luaL_checknumber(L, 2);
			float h = (float)luaL_checknumber(L, 3);
			PolygonShape * shape = instance->newRectangleShape(body, w, h);
			luax_newtype(L, "PolygonShape", LOVE_PHYSICS_POLYGON_SHAPE_BITS, (void*)shape);
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
			luax_newtype(L, "PolygonShape", LOVE_PHYSICS_POLYGON_SHAPE_BITS, (void*)shape);
			return 1;
		}
		else
			return luaL_error(L, "Incorrect number of parameters");
	}

	int _wrap_newPolygonShape(lua_State * L)
	{
		return instance->newPolygonShape(L);
	}

	int _wrap_newDistanceJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", LOVE_PHYSICS_BODY_BITS);
		float x1 = (float)luaL_checknumber(L, 3);
		float y1 = (float)luaL_checknumber(L, 4);
		float x2 = (float)luaL_checknumber(L, 5);
		float y2 = (float)luaL_checknumber(L, 6);
		DistanceJoint * j = instance->newDistanceJoint(body1, body2, x1, y1, x2, y2);
		luax_newtype(L, "DistanceJoint", LOVE_PHYSICS_DISTANCE_JOINT_BITS, (void*)j);
		return 1;
	}

	int _wrap_newMouseJoint(lua_State * L)
	{
		Body * body = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		MouseJoint * j = instance->newMouseJoint(body, x, y);
		luax_newtype(L, "MouseJoint", LOVE_PHYSICS_MOUSE_JOINT_BITS, (void*)j);
		return 1;
	}

	int _wrap_newRevoluteJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", LOVE_PHYSICS_BODY_BITS);
		float x = (float)luaL_checknumber(L, 3);
		float y = (float)luaL_checknumber(L, 4);
		RevoluteJoint * j = instance->newRevoluteJoint(body1, body2, x, y);
		luax_newtype(L, "RevoluteJoint", LOVE_PHYSICS_REVOLUTE_JOINT_BITS, (void*)j);
		return 1;
	}

	int _wrap_newPrismaticJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", LOVE_PHYSICS_BODY_BITS);
		float x = (float)luaL_checknumber(L, 3);
		float y = (float)luaL_checknumber(L, 4);
		float ax = (float)luaL_checknumber(L, 5);
		float ay = (float)luaL_checknumber(L, 6);
		PrismaticJoint * j = instance->newPrismaticJoint(body1, body2, x, y, ax, ay);
		luax_newtype(L, "PrismaticJoint", LOVE_PHYSICS_PRISMATIC_JOINT_BITS, (void*)j);
		return 1;
	}
	
	int _wrap_newPulleyJoint(lua_State * L)
	{
		Body * body1 = luax_checktype<Body>(L, 1, "Body", LOVE_PHYSICS_BODY_BITS);
		Body * body2 = luax_checktype<Body>(L, 2, "Body", LOVE_PHYSICS_BODY_BITS);
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
		luax_newtype(L, "PulleyJoint", LOVE_PHYSICS_PULLEY_JOINT_BITS, (void*)j);
		return 1;
	}
	
	int _wrap_newGearJoint(lua_State * L)
	{
		Joint * joint1 = luax_checktype<Joint>(L, 1, "Joint", LOVE_PHYSICS_JOINT_BITS);
		Joint * joint2 = luax_checktype<Joint>(L, 2, "Joint", LOVE_PHYSICS_JOINT_BITS);
		float ratio = (float)luaL_optnumber(L, 3, 1.0);

		GearJoint * j = instance->newGearJoint(joint1, joint2, ratio);
		luax_newtype(L, "GearJoint", LOVE_PHYSICS_GEAR_JOINT_BITS, (void*)j);
		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg wrap_Physics_functions[] = {
		{ "newWorld", _wrap_newWorld },
		{ "newBody", _wrap_newBody },
		{ "newCircleShape", _wrap_newCircleShape },
		{ "newRectangleShape", _wrap_newRectangleShape },
		{ "newPolygonShape", _wrap_newPolygonShape },
		{ "newDistanceJoint", _wrap_newDistanceJoint },
		{ "newMouseJoint", _wrap_newMouseJoint },
		{ "newRevoluteJoint", _wrap_newRevoluteJoint },
		{ "newPrismaticJoint", _wrap_newPrismaticJoint },
		{ "newPulleyJoint", _wrap_newPulleyJoint },
		{ "newGearJoint", _wrap_newGearJoint },
		{ 0, 0 },
	};

	static const lua_CFunction wrap_Physics_types[] = {
		wrap_World_open,
		wrap_Contact_open,
		wrap_Body_open,
		wrap_Shape_open,
		wrap_CircleShape_open,
		wrap_PolygonShape_open,
		wrap_Joint_open,
		wrap_MouseJoint_open,
		wrap_DistanceJoint_open,
		wrap_PrismaticJoint_open,
		wrap_RevoluteJoint_open,
		wrap_PulleyJoint_open,
		wrap_GearJoint_open,
		0
	};

	int wrap_Physics_open(lua_State * L)
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

		return luax_register_module(L, wrap_Physics_functions, wrap_Physics_types, "physics");
	}

} // box2d
} // physics
} // love
