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

#include "wrap_Contact.h"

namespace love
{
namespace physics
{
namespace box2d
{

	Contact * luax_checkcontact(lua_State * L, int idx)
	{
		return luax_checktype<Contact>(L, idx, "Contact", LOVE_PHYSICS_CONTACT_BITS);
	}

	int _wrap_Contact_getPosition(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		return t->getPosition(L);
	}

	int _wrap_Contact_getVelocity(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		return t->getVelocity(L);
	}

	int _wrap_Contact_getNormal(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		return t->getNormal(L);
	}

	int _wrap_Contact_getSeparation(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		lua_pushnumber(L, t->getSeparation());
		return 1;
	}

	int _wrap_Contact_getFriction(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		lua_pushnumber(L, t->getFriction());
		return 1;
	}

	int _wrap_Contact_getRestitution(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		lua_pushnumber(L, t->getRestitution());
		return 1;
	}

	static const luaL_Reg wrap_Contact_functions[] = {
		{ "getPosition", _wrap_Contact_getPosition },
		{ "getVelocity", _wrap_Contact_getVelocity },
		{ "getNormal", _wrap_Contact_getNormal },
		{ "getSeparation", _wrap_Contact_getSeparation },
		{ "getFriction", _wrap_Contact_getFriction },
		{ "getRestitution", _wrap_Contact_getRestitution },
		{ 0, 0 }
	};

	int wrap_Contact_open(lua_State * L)
	{
		luax_register_type(L, "Contact", wrap_Contact_functions);
		return 0;
	}

} // box2d
} // physics
} // love
