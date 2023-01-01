/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "wrap_Texture.h"
#include "math/MathModule.h"
#include "math/Transform.h"
#include "Graphics.h"

#include <string>
#include <algorithm>
#include <cmath>
#include <limits>

namespace love
{
namespace graphics
{

Shader *luax_checkshader(lua_State *L, int idx)
{
	return luax_checktype<Shader>(L, idx);
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

template <typename T, typename luaT, luaT (*checknum)(lua_State *, int)>
static void _updateNumbers(lua_State *L, int startidx, T *values, int components, int count)
{
	if (components == 1)
	{
		for (int i = 0; i < count; ++i)
			values[i] = (T) checknum(L, startidx + i);
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			luaL_checktype(L, startidx + i, LUA_TTABLE);

			for (int k = 1; k <= components; k++)
			{
				lua_rawgeti(L, startidx + i, k);
				values[i * components + k - 1] = (T) checknum(L, -1);
			}

			lua_pop(L, components);
		}
	}
}

int w_Shader_sendFloats(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info, bool colors)
{
	int count = _getCount(L, startidx, info);
	int components = info->components;
	float *values = info->floats;

	if (colors)
		_updateNumbers<float, lua_Number, luax_checknumberclamped01>(L, startidx, values, components, count);
	else
		_updateNumbers<float, lua_Number, luaL_checknumber>(L, startidx, values, components, count);

	if (colors && graphics::isGammaCorrect())
	{
		// alpha is always linear (when present).
		int gammacomponents = std::min(components, 3);

		for (int i = 0; i < count; i++)
		{
			for (int j = 0; j < gammacomponents; j++)
				values[i * components + j] = math::gammaToLinear(values[i * components + j]);
		}
	}

	luax_catchexcept(L, [&]() { shader->updateUniform(info, count); });
	return 0;
}

int w_Shader_sendInts(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);
	_updateNumbers<int, lua_Integer, luaL_checkinteger>(L, startidx, info->ints, info->components, count);
	luax_catchexcept(L, [&]() { shader->updateUniform(info, count); });
	return 0;
}

int w_Shader_sendUnsignedInts(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);
	_updateNumbers<unsigned int, lua_Integer, luaL_checkinteger>(L, startidx, info->uints, info->components, count);
	luax_catchexcept(L, [&]() { shader->updateUniform(info, count); });
	return 0;
}

int w_Shader_sendBooleans(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);
	int components = info->components;

	// We have to send booleans as ints.
	int *values = info->ints;

	if (components == 1)
	{
		for (int i = 0; i < count; i++)
		{
			luaL_checktype(L, startidx + i, LUA_TBOOLEAN);
			values[i] = (int) lua_toboolean(L, startidx + i);
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
				values[i * components + k - 1] = (int) lua_toboolean(L, -1);
			}

			lua_pop(L, components);
		}
	}

	luax_catchexcept(L, [&]() { shader->updateUniform(info, count); });
	return 0;
}

int w_Shader_sendMatrices(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	bool columnmajor = false;

	if (lua_type(L, startidx) == LUA_TSTRING)
	{
		const char *layoutstr = lua_tostring(L, startidx);
		math::Transform::MatrixLayout layout;
		if (!math::Transform::getConstant(layoutstr, layout))
			return luax_enumerror(L, "matrix layout", math::Transform::getConstants(layout), layoutstr);

		columnmajor = (layout == math::Transform::MATRIX_COLUMN_MAJOR);
		startidx++;
	}

	int count = _getCount(L, startidx, info);
	int columns = info->matrix.columns;
	int rows = info->matrix.rows;
	int elements = columns * rows;

	float *values = info->floats;

	for (int i = 0; i < count; i++)
	{
		if (columns == 4 && rows == 4 && luax_istype(L, startidx + i, math::Transform::type))
		{
			math::Transform *t = luax_totype<math::Transform>(L, startidx + i);
			memcpy(&values[i * 16], t->getMatrix().getElements(), sizeof(float) * 16);
			continue;
		}

		luaL_checktype(L, startidx + i, LUA_TTABLE);

		lua_rawgeti(L, startidx + i, 1);
		bool table_of_tables = lua_istable(L, -1);
		lua_pop(L, 1);

		if (table_of_tables)
		{
			int n = i * elements;

			if (columnmajor)
			{
				for (int column = 0; column < columns; column++)
				{
					lua_rawgeti(L, startidx + i, column + 1);

					for (int row = 0; row < rows; row++)
					{
						lua_rawgeti(L, -(row + 1), row + 1);
						values[n + (column * rows + row)] = (float) luaL_checknumber(L, -1);
					}

					lua_pop(L, rows + 1);
				}
			}
			else
			{
				for (int row = 0; row < rows; row++)
				{
					lua_rawgeti(L, startidx + i, row + 1);

					for (int column = 0; column < columns; column++)
					{
						// The table has the matrix elements laid out in row-major
						// order, but we need to store them column-major in memory.
						lua_rawgeti(L, -(column + 1), column + 1);
						values[n + (column * rows + row)] = (float) luaL_checknumber(L, -1);
					}

					lua_pop(L, columns + 1);
				}
			}
		}
		else
		{
			int n = i * elements;

			if (columnmajor)
			{
				for (int column = 0; column < columns; column++)
				{
					for (int row = 0; row < rows; row++)
					{
						lua_rawgeti(L, startidx + i, column * rows + row + 1);
						values[n + (column * rows + row)] = (float) luaL_checknumber(L, -1);
					}
				}
			}
			else
			{
				for (int column = 0; column < columns; column++)
				{
					for (int row = 0; row < rows; row++)
					{
						// The table has the matrix elements laid out in row-major
						// order, but we need to store them column-major in memory.
						lua_rawgeti(L, startidx + i, row * columns + column + 1);
						values[n + (column * rows + row)] = (float) luaL_checknumber(L, -1);
					}
				}
			}

			lua_pop(L, elements);
		}
	}

	shader->updateUniform(info, count);
	return 0;
}

int w_Shader_sendTextures(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info)
{
	int count = _getCount(L, startidx, info);

	std::vector<Texture *> textures;
	textures.reserve(count);

	for (int i = 0; i < count; i++)
	{
		Texture *tex = luax_checktexture(L, startidx + i);
		if (tex->getTextureType() != info->textureType)
			return luaL_argerror(L, startidx + i, "invalid texture type for uniform");
		textures.push_back(tex);
	}

	luax_catchexcept(L, [&]() { shader->sendTextures(info, textures.data(), count); });
	return 0;
}

static int w_Shader_sendLuaValues(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info, const char *name)
{
	switch (info->baseType)
	{
	case Shader::UNIFORM_FLOAT:
		return w_Shader_sendFloats(L, startidx, shader, info, false);
	case Shader::UNIFORM_MATRIX:
		return w_Shader_sendMatrices(L, startidx, shader, info);
	case Shader::UNIFORM_INT:
		return w_Shader_sendInts(L, startidx, shader, info);
	case Shader::UNIFORM_UINT:
		return w_Shader_sendUnsignedInts(L, startidx, shader, info);
	case Shader::UNIFORM_BOOL:
		return w_Shader_sendBooleans(L, startidx, shader, info);
	case Shader::UNIFORM_SAMPLER:
		return w_Shader_sendTextures(L, startidx, shader, info);
	default:
		return luaL_error(L, "Unknown variable type for shader uniform '%s", name);
	}
}

static int w_Shader_sendData(lua_State *L, int startidx, Shader *shader, const Shader::UniformInfo *info, bool colors)
{
	if (info->baseType == Shader::UNIFORM_SAMPLER)
		return luaL_error(L, "Uniform sampler values (textures) cannot be sent to Shaders via Data objects.");

	math::Transform::MatrixLayout layout = math::Transform::MATRIX_ROW_MAJOR;
	int dataidx = startidx;
	if (info->baseType == Shader::UNIFORM_MATRIX)
	{
		if (lua_type(L, startidx) == LUA_TSTRING)
		{
			// (matrixlayout, data, ...)
			const char *layoutstr = lua_tostring(L, startidx);
			if (!math::Transform::getConstant(layoutstr, layout))
				return luax_enumerror(L, "matrix layout", math::Transform::getConstants(layout), layoutstr);

			startidx++;
			dataidx = startidx;
		}
		else if (lua_type(L, startidx + 1) == LUA_TSTRING)
		{
			// (data, matrixlayout, ...)
			// Should be deprecated in the future (doesn't match the argument
			// order of Shader:send(name, matrixlayout, table))
			const char *layoutstr = lua_tostring(L, startidx + 1);
			if (!math::Transform::getConstant(layoutstr, layout))
				return luax_enumerror(L, "matrix layout", math::Transform::getConstants(layout), layoutstr);

			startidx++;
		}
	}

	bool columnmajor = (layout == math::Transform::MATRIX_COLUMN_MAJOR);

	Data *data = luax_checktype<Data>(L, dataidx);
	size_t size = data->getSize();

	ptrdiff_t offset = (ptrdiff_t) luaL_optinteger(L, startidx + 1, 0);
	if (offset < 0)
		return luaL_error(L, "Offset cannot be negative.");
	else if ((size_t) offset >= size)
		return luaL_error(L, "Offset must be less than the size of the Data.");

	size_t uniformstride = info->dataSize / info->count;

	if (!lua_isnoneornil(L, startidx + 2))
	{
		lua_Integer sizearg = luaL_checkinteger(L, startidx + 2);
		if (sizearg <= 0)
			return luaL_error(L, "Size must be greater than 0.");
		else if ((size_t) sizearg > size - offset)
			return luaL_error(L, "Size and offset must fit within the Data's bounds.");
		else if (sizearg % uniformstride != 0)
			return luaL_error(L, "Size (%d) must be a multiple of the uniform's size in bytes (%d).", sizearg, uniformstride);
		else if ((size_t) sizearg > info->dataSize)
			return luaL_error(L, "Size must not be greater than the uniform's total size in bytes.");

		size = (size_t) sizearg;
	}
	else
	{
		size -= offset;
		size = std::min((size / uniformstride) * uniformstride, info->dataSize);
	}

	if (size == 0)
		return luaL_error(L, "Size to copy must be greater than 0.");

	int count = (int) (size / uniformstride);
	const char *mem = (const char *) data->getData() + offset;

	if (info->baseType != Shader::UNIFORM_MATRIX || columnmajor)
		memcpy(info->data, mem, size);
	else
	{
		int columns = info->matrix.columns;
		int rows = info->matrix.rows;

		const float *src = (const float *) mem;
		float *dst = info->floats;

		for (int i = 0; i < count; i++)
		{
			for (int row = 0; row < rows; row++)
			{
				for (int column = 0; column < columns; column++)
					dst[column * rows + row] = src[row * columns + column];
			}

			src += columns * rows;
			dst += columns * rows;
		}
	}

	if (colors && graphics::isGammaCorrect())
	{
		// alpha is always linear (when present).
		int components = info->components;
		int gammacomponents = std::min(components, 3);
		float *values = info->floats;

		for (int i = 0; i < count; i++)
		{
			for (int j = 0; j < gammacomponents; j++)
				values[i * components + j] = math::gammaToLinear(values[i * components + j]);
		}
	}

	shader->updateUniform(info, count);
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

	if (luax_istype(L, startidx, Data::type) || (info->baseType == Shader::UNIFORM_MATRIX && luax_istype(L, startidx + 1, Data::type)))
		return w_Shader_sendData(L, startidx, shader, info, false);
	else
		return w_Shader_sendLuaValues(L, startidx, shader, info, name);
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

	if (luax_istype(L, 3, Data::type))
		return w_Shader_sendData(L, 3, shader, info, true);
	else
		return w_Shader_sendFloats(L, 3, shader, info, true);
}

int w_Shader_hasUniform(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	const char *name = luaL_checkstring(L, 2);
	luax_pushboolean(L, shader->hasUniform(name));
	return 1;
}

static const luaL_Reg w_Shader_functions[] =
{
	{ "getWarnings", w_Shader_getWarnings },
	{ "send",        w_Shader_send },
	{ "sendColor",   w_Shader_sendColors },
	{ "hasUniform",  w_Shader_hasUniform },
	{ 0, 0 }
};

extern "C" int luaopen_shader(lua_State *L)
{
	return luax_register_type(L, &Shader::type, w_Shader_functions, nullptr);
}

} // graphics
} // love

