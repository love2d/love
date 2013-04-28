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

#include "wrap_Shader.h"
#include "wrap_Image.h"
#include "wrap_Canvas.h"
#include <string>
#include <iostream>

namespace love
{
namespace graphics
{
namespace opengl
{

Shader *luax_checkshader(lua_State *L, int idx)
{
	return luax_checktype<Shader>(L, idx, "Shader", GRAPHICS_SHADER_T);
}

int w_Shader_getWarnings(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	lua_pushstring(L, shader->getWarnings().c_str());
	return 1;
}

template <typename T>
static T *_getScalars(lua_State *L, int count, size_t &dimension)
{
	dimension = 1;

	T *values = new T[count];
	for (int i = 0; i < count; ++i)
	{
		if (lua_isnumber(L, 3 + i))
			values[i] = static_cast<T>(lua_tonumber(L, 3 + i));
		else if (lua_isboolean(L, 3 + i))
			values[i] = static_cast<T>(lua_toboolean(L, 3 + i));
		else
		{
			delete[] values;
			luaL_typerror(L, 3 + i, "number or boolean");
			return 0;
		}
	}

	return values;
}

template <typename T>
static T *_getVectors(lua_State *L, int count, size_t &dimension)
{
	dimension = lua_objlen(L, 3);
	T *values = new T[count * dimension];

	for (int i = 0; i < count; ++i)
	{
		if (!lua_istable(L, 3 + i))
		{
			delete[] values;
			luaL_typerror(L, 3 + i, "table");
			return 0;
		}
		if (lua_objlen(L, 3 + i) != dimension)
		{
			delete[] values;
			luaL_error(L, "Error in argument %d: Expected table size %d, got %d.",
						   3+i, dimension, lua_objlen(L, 3+i));
			return 0;
		}

		for (size_t k = 1; k <= dimension; ++k)
		{
			lua_rawgeti(L, 3 + i, k);
			if (lua_isnumber(L, -1))
				values[i * dimension + k - 1] = static_cast<T>(lua_tonumber(L, -1));
			else if (lua_isboolean(L, -1))
				values[i * dimension + k - 1] = static_cast<T>(lua_toboolean(L, -1));
			else
			{
				delete[] values;
				luaL_typerror(L, -1, "number or boolean");
				return 0;
			}
		}
		lua_pop(L, int(dimension));
	}

	return values;
}

int w_Shader_sendInt(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int count = lua_gettop(L) - 2;

	if (count < 1)
		return luaL_error(L, "No variable to send.");

	int *values = 0;
	size_t dimension = 1;

	if (lua_isnumber(L, 3) || lua_isboolean(L, 3))
		values = _getScalars<int>(L, count, dimension);
	else if (lua_istable(L, 3))
		values = _getVectors<int>(L, count, dimension);
	else
		return luaL_typerror(L, 3, "number, boolean, or table");

	if (!values)
		return luaL_error(L, "Error in arguments.");

	try
	{
		shader->sendInt(name, dimension, values, count);
	}
	catch (love::Exception &e)
	{
		delete[] values;
		return luaL_error(L, "%s", e.what());
	}
	
	delete[] values;
	
	return 0;
}

int w_Shader_sendFloat(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int count = lua_gettop(L) - 2;

	if (count < 1)
		return luaL_error(L, "No variable to send.");

	float *values = 0;
	size_t dimension = 1;

	if (lua_isnumber(L, 3) || lua_isboolean(L, 3))
		values = _getScalars<float>(L, count, dimension);
	else if (lua_istable(L, 3))
		values = _getVectors<float>(L, count, dimension);
	else
		return luaL_typerror(L, 3, "number, boolean, or table");

	if (!values)
		return luaL_error(L, "Error in arguments.");

	try
	{
		shader->sendFloat(name, dimension, values, count);
	}
	catch (love::Exception &e)
	{
		delete[] values;
		return luaL_error(L, "%s", e.what());
	}

	delete[] values;

	return 0;
}

int w_Shader_sendMatrix(lua_State *L)
{
	int count = lua_gettop(L) - 2;
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);

	if (!lua_istable(L, 3))
		return luaL_typerror(L, 3, "matrix table");

	lua_getfield(L, 3, "dimension");
	int dimension = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (dimension < 2 || dimension > 4)
		return luaL_error(L, "Invalid matrix size: %dx%d (only 2x2, 3x3 and 4x4 matrices are supported).",
						  count, count);

	float *values = new float[dimension * dimension * count];
	for (int i = 0; i < count; ++i)
	{
		lua_getfield(L, 3+i, "dimension");
		if (lua_tointeger(L, -1) != dimension)
		{
			// You unlock this door with the key of imagination. Beyond it is
			// another dimension: a dimension of sound, a dimension of sight,
			// a dimension of mind. You're moving into a land of both shadow
			// and substance, of things and ideas. You've just crossed over
			// into... the Twilight Zone.
			int other_dimension = lua_tointeger(L, -1);
			delete[] values;
			return luaL_error(L, "Invalid matrix size at argument %d: Expected size %dx%d, got %dx%d.",
							  3+i, dimension, dimension, other_dimension, other_dimension);
		}

		for (int k = 1; k <= dimension*dimension; ++k)
		{
			lua_rawgeti(L, 3+i, k);
			values[i * dimension * dimension + k - 1] = (float)lua_tonumber(L, -1);
		}

		lua_pop(L, 1 + dimension);
	}

	try
	{
		shader->sendMatrix(name, dimension, values, count);
	}
	catch(love::Exception &e)
	{
		delete[] values;
		return luaL_error(L, "%s", e.what());
	}

	delete[] values;
	return 0;
}

int w_Shader_sendImage(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);
	Image *img = luax_checkimage(L, 3);

	try
	{
		shader->sendImage(name, *img);
	}
	catch(love::Exception &e)
	{
		luaL_error(L, "%s", e.what());
	}

	return 0;
}

int w_Shader_sendCanvas(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);
	Canvas *canvas = luax_checkcanvas(L, 3);

	try
	{
		shader->sendCanvas(name, *canvas);
	}
	catch(love::Exception &e)
	{
		luaL_error(L, "%s", e.what());
	}

	return 0;
}


static const luaL_Reg functions[] =
{
	{ "getWarnings", w_Shader_getWarnings },
	{ "sendInt",     w_Shader_sendInt },
	{ "sendFloat",   w_Shader_sendFloat },
	{ "sendMatrix",  w_Shader_sendMatrix },
	{ "sendImage",   w_Shader_sendImage },
	{ "sendCanvas",  w_Shader_sendCanvas },
	{ 0, 0 }
};

extern "C" int luaopen_shader(lua_State *L)
{
	return luax_register_type(L, "Shader", functions);
}

} // opengl
} // graphics
} // love

