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

#include "wrap_Contact.h"

namespace love
{
namespace physics
{
namespace box2d
{

	Contact * luax_checkcontact(lua_State * L, int idx)
	{
		return luax_checktype<Contact>(L, idx, "Contact", PHYSICS_CONTACT_T);
	}

	int w_Contact_getPosition(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		return t->getPosition(L);
	}

	int w_Contact_getVelocity(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		return t->getVelocity(L);
	}

	int w_Contact_getNormal(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		return t->getNormal(L);
	}

	int w_Contact_getSeparation(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		lua_pushnumber(L, t->getSeparation());
		return 1;
	}

	int w_Contact_getFriction(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		lua_pushnumber(L, t->getFriction());
		return 1;
	}

	int w_Contact_getRestitution(lua_State * L)
	{
		Contact * t = luax_checkcontact(L, 1);
		lua_pushnumber(L, t->getRestitution());
		return 1;
	}

	int luaopen_contact(lua_State * L)
	{
		static const luaL_Reg functions[] = {
			{ "getPosition", w_Contact_getPosition },
			{ "getVelocity", w_Contact_getVelocity },
			{ "getNormal", w_Contact_getNormal },
			{ "getSeparation", w_Contact_getSeparation },
			{ "getFriction", w_Contact_getFriction },
			{ "getRestitution", w_Contact_getRestitution },
			{ 0, 0 }
		};

		return luax_register_type(L, "Contact", functions);
	}

} // box2d
} // physics
} // love
