/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "Image.h"
#include "wrap_SpriteBatch.h"

namespace love
{
namespace graphics
{
namespace opengl
{

SpriteBatch *luax_checkspritebatch(lua_State *L, int idx)
{
	return luax_checktype<SpriteBatch>(L, idx, "SpriteBatch", GRAPHICS_SPRITE_BATCH_T);
}

int w_SpriteBatch_add(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	float x = (float)luaL_optnumber(L, 2, 0.0f);
	float y = (float)luaL_optnumber(L, 3, 0.0f);
	float angle = (float)luaL_optnumber(L, 4, 0.0f);
	float sx = (float)luaL_optnumber(L, 5, 1.0f);
	float sy = (float)luaL_optnumber(L, 6, sx);
	float ox = (float)luaL_optnumber(L, 7, 0);
	float oy = (float)luaL_optnumber(L, 8, 0);
	float kx = (float)luaL_optnumber(L, 9, 0);
	float ky = (float)luaL_optnumber(L, 10, 0);
	lua_pushnumber(L, t->add(x, y, angle, sx, sy, ox, oy, kx, ky));
	return 1;
}

int w_SpriteBatch_addg(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	Geometry *g = luax_checktype<Geometry>(L, 2, "Geometry", GRAPHICS_GEOMETRY_T);

	float x = (float)luaL_optnumber(L, 3, 0.0f);
	float y = (float)luaL_optnumber(L, 4, 0.0f);
	float angle = (float)luaL_optnumber(L, 5, 0.0f);
	float sx = (float)luaL_optnumber(L, 6, 1.0f);
	float sy = (float)luaL_optnumber(L, 7, sx);
	float ox = (float)luaL_optnumber(L, 8, 0);
	float oy = (float)luaL_optnumber(L, 9, 0);
	float kx = (float)luaL_optnumber(L, 10, 0);
	float ky = (float)luaL_optnumber(L, 11, 0);
	try
	{
		lua_pushnumber(L, t->addg(g, x, y, angle, sx, sy, ox, oy, kx, ky));
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	return 1;
}

int w_SpriteBatch_set(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	int index = luaL_checkinteger(L, 2);
	float x = (float)luaL_optnumber(L, 3, 0.0f);
	float y = (float)luaL_optnumber(L, 4, 0.0f);
	float angle = (float)luaL_optnumber(L, 5, 0.0f);
	float sx = (float)luaL_optnumber(L, 6, 1.0f);
	float sy = (float)luaL_optnumber(L, 7, sx);
	float ox = (float)luaL_optnumber(L, 8, 0);
	float oy = (float)luaL_optnumber(L, 9, 0);
	float kx = (float)luaL_optnumber(L, 10, 0);
	float ky = (float)luaL_optnumber(L, 11, 0);
	t->add(x, y, angle, sx, sy, ox, oy, kx, ky, index);
	return 0;
}

int w_SpriteBatch_setg(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	int index = luaL_checkinteger(L, 2);
	Geometry *g = luax_checktype<Geometry>(L, 3, "Geometry", GRAPHICS_GEOMETRY_T);

	float x = (float)luaL_optnumber(L, 4, 0.0f);
	float y = (float)luaL_optnumber(L, 5, 0.0f);
	float angle = (float)luaL_optnumber(L, 6, 0.0f);
	float sx = (float)luaL_optnumber(L, 7, 1.0f);
	float sy = (float)luaL_optnumber(L, 8, sx);
	float ox = (float)luaL_optnumber(L, 9, 0);
	float oy = (float)luaL_optnumber(L, 10, 0);
	float kx = (float)luaL_optnumber(L, 11, 0);
	float ky = (float)luaL_optnumber(L, 12, 0);
	try
	{
		t->addg(g, x, y, angle, sx, sy, ox, oy, kx, ky, index);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	return 0;
}

int w_SpriteBatch_clear(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	t->clear();
	return 0;
}

int w_SpriteBatch_bind(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	try
	{
		t->lock();
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	return 0;
}

int w_SpriteBatch_unbind(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	t->unlock();
	return 0;
}

int w_SpriteBatch_setImage(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	Image *image = luax_checktype<Image>(L, 2, "Image", GRAPHICS_IMAGE_T);
	t->setImage(image);
	return 0;
}

int w_SpriteBatch_getImage(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	Image *image = t->getImage();
	image->retain();
	luax_newtype(L, "Image", GRAPHICS_IMAGE_T, (void *)image);
	return 1;
}

int w_SpriteBatch_setColor(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	Color c;

	if (lua_gettop(L) <= 1)
	{
		t->setColor();
		return 0;
	}
	else if (lua_istable(L, 2))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 2, i);

		c.r = (unsigned char) luaL_checkint(L, -4);
		c.g = (unsigned char) luaL_checkint(L, -3);
		c.b = (unsigned char) luaL_checkint(L, -2);
		c.a = (unsigned char) luaL_optint(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (unsigned char)luaL_checkint(L, 2);
		c.g = (unsigned char)luaL_checkint(L, 3);
		c.b = (unsigned char)luaL_checkint(L, 4);
		c.a = (unsigned char)luaL_optint(L, 5, 255);
	}

	t->setColor(c);

	return 0;
}

int w_SpriteBatch_getCount(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	lua_pushinteger(L, t->getCount());
	return 1;
}

int w_SpriteBatch_getBufferSize(lua_State *L)
{
	SpriteBatch *t = luax_checkspritebatch(L, 1);
	lua_pushinteger(L, t->getBufferSize());
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "add", w_SpriteBatch_add },
	{ "addg", w_SpriteBatch_addg },
	{ "set", w_SpriteBatch_set },
	{ "setg", w_SpriteBatch_setg },
	{ "clear", w_SpriteBatch_clear },
	{ "bind", w_SpriteBatch_bind },
	{ "unbind", w_SpriteBatch_unbind },
	{ "setImage", w_SpriteBatch_setImage },
	{ "getImage", w_SpriteBatch_getImage },
	{ "setColor", w_SpriteBatch_setColor },
	{ "getCount", w_SpriteBatch_getCount },
	{ "getBufferSize", w_SpriteBatch_getBufferSize },
	{ 0, 0 }
};

extern "C" int luaopen_spritebatch(lua_State *L)
{
	return luax_register_type(L, "SpriteBatch", functions);
}

} // opengl
} // graphics
} // love
