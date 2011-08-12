#include "wrap_PixelEffect.h"
#include "wrap_Image.h"
#include "wrap_Canvas.h"
#include <string>

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

	int w_PixelEffect_sendFloat(lua_State * L)
	{
		size_t count = lua_gettop(L) - 2;
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		const char* name = luaL_checkstring(L, 2);

		if (count < 1 || count > 4)
			return luaL_error(L, "Invalid variable count (expected 1-4, got %d).", count);

		float values[4] = {0,0,0,0};
		for (unsigned int i = 0; i < count; ++i)
			values[i] = (float) luaL_checknumber(L, i+1 + 2);

		try {
			effect->sendFloat(name, count, values);
		} catch(love::Exception& e) {
			luaL_error(L, e.what());
		}

		return 0;
	}

	int w_PixelEffect_sendMatrix(lua_State * L)
	{
		size_t count = lua_gettop(L) - 3;
		PixelEffect * effect = luax_checkpixeleffect(L, 1);
		const char* name = luaL_checkstring(L, 2);
		int size = luaL_checkinteger(L, 3);

		if (size < 2 || size > 4)
			return luaL_error(L, "Invalid matrix size: %dx%d (only 2x2, 3x3 and 4x4 matrices are supported).", count, count);

		float values[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		for (unsigned int i = 0; i < count; ++i)
			values[i] = (float) luaL_checknumber(L, i+1 + 3);

		try {
			effect->sendFloat(name, size, values);
		} catch(love::Exception& e) {
			luaL_error(L, e.what());
		}

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

