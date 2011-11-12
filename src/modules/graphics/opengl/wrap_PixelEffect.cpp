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

#include "wrap_PixelEffect.h"
#include "wrap_Image.h"
#include "wrap_Canvas.h"
#include <string>
#include <iostream>
using namespace std;

namespace love
{
namespace graphics
{
namespace opengl
{
	PixelEffect * luax_checkpixeleffect(lua_State * L, int idx)
	{
		return luax_checktype<PixelEffect>(L, idx, "PixelEffect", GRAPHICS_PIXELEFFECT_T);
	}

	int w_PixelEffect_getWarnings(lua_State * L)
	{
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		lua_pushstring(L, effect->getWarnings().c_str());
		return 1;
	}

	static int _sendScalars(lua_State * L, PixelEffect * effect, const char * name, int count)
	{
		float * values = new float[count];
		for (int i = 0; i < count; ++i) {
			if (!lua_isnumber(L, 3 + i)) { 
				delete[] values;
				return luaL_typerror(L, 3 + i, "number");
			}
			values[i] = (float)lua_tonumber(L, 3 + i);
		}

		try {
			effect->sendFloat(name, 1, values, count);
		} catch(love::Exception& e) {
			delete[] values;
			return luaL_error(L, e.what());
		}

		delete[] values;
		return 0;
	}

	static int _sendVectors(lua_State * L, PixelEffect * effect, const char * name, int count)
	{
		size_t dimension = lua_objlen(L, 3);
		float * values = new float[count * dimension];

		for (int i = 0; i < count; ++i) {
			if (!lua_istable(L, 3 + i)) {
				delete[] values;
				return luaL_typerror(L, 3 + i, "table");
			}
			if (lua_objlen(L, 3 + i) != dimension) {
				delete[] values;
				return luaL_error(L, "Error in argument %d: Expected table size %d, got %d.",
				                     3+i, dimension, lua_objlen(L, 3+i));
			}

			for (size_t k = 1; k <= dimension; ++k) {
				lua_rawgeti(L, 3 + i, k);
				values[i * dimension + k - 1] = (float)lua_tonumber(L, -1);
			}
			lua_pop(L, dimension);
		}

		try {
			effect->sendFloat(name, dimension, values, count);
		} catch(love::Exception& e) {
			delete[] values;
			return luaL_error(L, e.what());
		}

		delete[] values;
		return 0;
	}

	int w_PixelEffect_sendFloat(lua_State * L)
	{
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		const char* name = luaL_checkstring(L, 2);
		int count = lua_gettop(L) - 2;

		if (count < 1)
			return luaL_error(L, "No variable to send.");

		if (lua_isnumber(L, 3))
			return _sendScalars(L, effect, name, count);
		else if (lua_istable(L, 3))
			return _sendVectors(L, effect, name, count);

		return luaL_typerror(L, 3, "number or table");
	}

	int w_PixelEffect_sendMatrix(lua_State * L)
	{
		int count = lua_gettop(L) - 2;
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		const char* name = luaL_checkstring(L, 2);

		if (!lua_istable(L, 3))
			return luaL_typerror(L, 3, "matrix table");

		lua_getfield(L, 3, "dimension");
		int dimension = lua_tointeger(L, -1);
		lua_pop(L, 1);

		if (dimension < 2 || dimension > 4)
			return luaL_error(L, "Invalid matrix size: %dx%d (only 2x2, 3x3 and 4x4 matrices are supported).",
			                     count, count);

		float * values = new float[dimension * dimension * count];
		for (int i = 0; i < count; ++i) {
			lua_getfield(L, 3+i, "dimension");
			if (lua_tointeger(L, -1) != dimension) {
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

			for (int k = 1; k <= dimension*dimension; ++k) {
				lua_rawgeti(L, 3+i, k);
				values[i * dimension * dimension + k - 1] = (float)lua_tonumber(L, -1);
			}

			lua_pop(L, 1 + dimension);
		}

		try {
			effect->sendMatrix(name, dimension, values, count);
		} catch(love::Exception& e) {
			delete[] values;
			return luaL_error(L, e.what());
		}

		delete[] values;
		return 0;
	}

	int w_PixelEffect_sendImage(lua_State * L)
	{
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		const char* name = luaL_checkstring(L, 2);
		Image* img = luax_checkimage(L, 3);

		try {
			effect->sendImage(name, *img);
		} catch(love::Exception& e) {
			luaL_error(L, e.what());
		}

		return 0;
	}

	int w_PixelEffect_sendCanvas(lua_State * L)
	{
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		const char* name = luaL_checkstring(L, 2);
		Canvas* canvas = luax_checkcanvas(L, 3);

		try {
			effect->sendCanvas(name, *canvas);
		} catch(love::Exception& e) {
			luaL_error(L, e.what());
		}

		return 0;
	}


	static const luaL_Reg functions[] = {
		{ "getWarnings", w_PixelEffect_getWarnings },
		{ "sendFloat",   w_PixelEffect_sendFloat },
		{ "sendMatrix",  w_PixelEffect_sendMatrix },
		{ "sendImage",   w_PixelEffect_sendImage },
		{ "sendCanvas",  w_PixelEffect_sendCanvas },
		{ 0, 0 }
	};

	int luaopen_pixeleffect(lua_State * L)
	{
		return luax_register_type(L, "PixelEffect", functions);
	}

} // opengl
} // graphics
} // love

