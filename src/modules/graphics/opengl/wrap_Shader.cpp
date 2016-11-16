/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "graphics/wrap_Texture.h"
#include "math/MathModule.h"

#include <string>
#include <algorithm>
#include <cmath>

namespace love
{
namespace graphics
{
namespace opengl
{

Shader *luax_checkshader(lua_State *L, int idx)
{
	return luax_checktype<Shader>(L, idx, GRAPHICS_SHADER_ID);
}

int w_Shader_getWarnings(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	std::string warnings = shader->getWarnings();
	lua_pushstring(L, warnings.c_str());
	return 1;
}

static int _getCount(lua_State *L, int startidx, const Shader::UniformInfo *info)
{
	return std::min(std::max(lua_gettop(L) - startidx + 1, 1), info->count);
}

template <typename T>
static T *_getNumbers(lua_State *L, int startidx, Shader *shader, int components, int count)
{
	T *values = shader->getScratchBuffer<T>(components * count);

	if (components == 1)
	{
		for (int i = 0; i < count; ++i)
			values[i] = (T) luaL_checknumber(L, startidx + i);
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			luaL_checktype(L, startidx + i, LUA_TTABLE);

			for (int k = 1; k <= components; k++)
			{
				lua_rawgeti(L, startidx + i, k);
				values[i * components + k - 1] = (T) luaL_checknumber(L, -1);
			}

			lua_pop(L, components);
		}
	}

	return values;
}

int w_Shader_sendFloats(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info, bool colors)
{
	int count = _getCount(L, startidx, info);
	int components = info->components;

	float *values = _getNumbers<float>(L, startidx, shader, components, count);

	if (colors)
	{
		bool gammacorrect = love::graphics::isGammaCorrect();
		const auto &m = love::math::Math::instance;

		for (int i = 0; i < count; i++)
		{
			for (int j = 0; j < components; j++)
			{
				// the fourth component (alpha) is always already linear, if it exists.
				if (gammacorrect && j < 3)
					values[i * components + j] = m.gammaToLinear(values[i * components + j] / 255.0f);
				else
					values[i * components + j] /= 255.0f;
			}
		}
	}

	luax_catchexcept(L, [&]() { shader->sendFloats(info, values, count); });
	return 0;
}

int w_Shader_sendInts(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);
	int *values = _getNumbers<int>(L, startidx, shader, info->components, count);
	luax_catchexcept(L, [&]() { shader->sendInts(info, values, count); });
	return 0;
}

int w_Shader_sendBooleans(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);
	int components = info->components;

	// We have to send booleans as ints or floats.
	float *values = shader->getScratchBuffer<float>(components * count);

	if (components == 1)
	{
		for (int i = 0; i < count; i++)
		{
			luaL_checktype(L, startidx + i, LUA_TBOOLEAN);
			values[i] = (float) lua_toboolean(L, startidx + i);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			luaL_checktype(L, startidx + i, LUA_TTABLE);

			for (int k = 1; k <= components; k++)
			{
				lua_rawgeti(L, startidx + i, k);
				luaL_checktype(L, -1, LUA_TBOOLEAN);
				values[i * components + k - 1] = (float) lua_toboolean(L, -1);
			}

			lua_pop(L, components);
		}
	}

	luax_catchexcept(L, [&]() { shader->sendFloats(info, values, count); });
	return 0;
}

int w_Shader_sendMatrices(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);
	int dimension = info->components;
	int elements = dimension * dimension;

	float *values = shader->getScratchBuffer<float>(elements * count);

	for (int i = 0; i < count; i++)
	{
		luaL_checktype(L, startidx + i, LUA_TTABLE);

		lua_rawgeti(L, startidx + i, 1);
		bool table_of_tables = lua_istable(L, -1);
		lua_pop(L, 1);

		if (table_of_tables)
		{
			int n = 0;

			for (int j = 1; j <= dimension; j++)
			{
				lua_rawgeti(L, startidx + i, j);

				for (int k = 1; k <= dimension; k++)
				{
					lua_rawgeti(L, -k, k);
					values[i * elements + n] = (float) luaL_checknumber(L, -1);
					n++;
				}

				lua_pop(L, dimension + 1);
			}
		}
		else
		{
			for (int k = 1; k <= elements; k++)
			{
				lua_rawgeti(L, startidx + i, k);
				values[i * elements + (k - 1)] = (float) luaL_checknumber(L, -1);
			}

			lua_pop(L, elements);
		}
	}

	shader->sendMatrices(info, values, count);
	return 0;
}

int w_Shader_sendTexture(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	// We don't support arrays of textures (yet).
	Texture *texture = luax_checktexture(L, startidx);
	luax_catchexcept(L, [&]() { shader->sendTexture(info, texture); });
	return 0;
}

int w_Shader_send(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);

	const Shader::UniformInfo *info = shader->getUniformInfo(name);
	if (info == nullptr)
		return luaL_error(L, "Shader uniform '%s' does not exist.\nA common error is to define but not use the variable.", name);

	int startidx = 3;

	switch (info->baseType)
	{
	case Shader::UNIFORM_FLOAT:
		return w_Shader_sendFloats(L, startidx, shader, info, false);
	case Shader::UNIFORM_MATRIX:
		return w_Shader_sendMatrices(L, startidx, shader, info);
	case Shader::UNIFORM_INT:
		return w_Shader_sendInts(L, startidx, shader, info);
	case Shader::UNIFORM_BOOL:
		return w_Shader_sendBooleans(L, startidx, shader, info);
	case Shader::UNIFORM_SAMPLER:
		return w_Shader_sendTexture(L, startidx, shader, info);
	default:
		return luaL_error(L, "Unknown variable type for shader uniform '%s", name);
	}
}

int w_Shader_sendColors(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);

	const Shader::UniformInfo *info = shader->getUniformInfo(name);
	if (info == nullptr)
		return luaL_error(L, "Shader uniform '%s' does not exist.\nA common error is to define but not use the variable.", name);

	if (info->baseType != Shader::UNIFORM_FLOAT || info->components < 3)
		return luaL_error(L, "sendColor can only be used on vec3 or vec4 uniforms.");

	return w_Shader_sendFloats(L, 3, shader, info, true);
}

int w_Shader_getExternVariable(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);

	int components = 0;
	int arrayelements = 0;
	Shader::UniformType type = Shader::UNIFORM_UNKNOWN;

	type = shader->getExternVariable(name, components, arrayelements);

	// Check if the variable exists (function will set components to 0 if not.)
	if (components > 0)
	{
		const char *tname = nullptr;
		if (!Shader::getConstant(type, tname))
			return luaL_error(L, "Unknown extern variable type name.");

		lua_pushstring(L, tname);
		lua_pushinteger(L, components);
		lua_pushinteger(L, arrayelements);
	}
	else
	{
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
	}

	return 3;
}

static const luaL_Reg w_Shader_functions[] =
{
	{ "getWarnings", w_Shader_getWarnings },
	{ "sendInt",     w_Shader_send },
	{ "sendBoolean", w_Shader_send },
	{ "sendFloat",   w_Shader_send },
	{ "sendMatrix",  w_Shader_send },
	{ "sendTexture", w_Shader_send },
	{ "send",        w_Shader_send },
	{ "sendColor",   w_Shader_sendColors },
	{ "getExternVariable", w_Shader_getExternVariable },
	{ 0, 0 }
};

extern "C" int luaopen_shader(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_SHADER_ID, "Shader", w_Shader_functions, nullptr);
}

} // opengl
} // graphics
} // love

