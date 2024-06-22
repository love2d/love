/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "wrap_GraphicsReadback.h"
#include "data/ByteData.h"
#include "image/ImageData.h"

namespace love
{
namespace graphics
{

GraphicsReadback *luax_checkgraphicsreadback(lua_State *L, int idx)
{
	return luax_checktype<GraphicsReadback>(L, idx);
}

int w_GraphicsReadback_isComplete(lua_State *L)
{
	GraphicsReadback *t = luax_checkgraphicsreadback(L, 1);
	luax_pushboolean(L, t->isComplete());
	return 1;
}

int w_GraphicsReadback_hasError(lua_State *L)
{
	GraphicsReadback *t = luax_checkgraphicsreadback(L, 1);
	luax_pushboolean(L, t->hasError());
	return 1;
}

int w_GraphicsReadback_wait(lua_State *L)
{
	GraphicsReadback *t = luax_checkgraphicsreadback(L, 1);
	t->wait();
	return 0;
}

int w_GraphicsReadback_update(lua_State *L)
{
	GraphicsReadback *t = luax_checkgraphicsreadback(L, 1);
	luax_catchexcept(L, [&]() { t->update(); });
	return 0;
}

int w_GraphicsReadback_getBufferData(lua_State *L)
{
	GraphicsReadback *t = luax_checkgraphicsreadback(L, 1);
	luax_pushtype(L, t->getBufferData());
	return 1;
}

int w_GraphicsReadback_getImageData(lua_State *L)
{
	GraphicsReadback *t = luax_checkgraphicsreadback(L, 1);
	luax_pushtype(L, t->getImageData());
	return 1;
}

static const luaL_Reg w_GraphicsReadback_functions[] =
{
	{ "isComplete", w_GraphicsReadback_isComplete },
	{ "hasError", w_GraphicsReadback_hasError },
	{ "wait", w_GraphicsReadback_wait },
	{ "update", w_GraphicsReadback_update },
	{ "getBufferData", w_GraphicsReadback_getBufferData },
	{ "getImageData", w_GraphicsReadback_getImageData },
	{ 0, 0 }
};

extern "C" int luaopen_graphicsreadback(lua_State *L)
{
	return luax_register_type(L, &GraphicsReadback::type, w_GraphicsReadback_functions, nullptr);
}

} // graphics
} // love
