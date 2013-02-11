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

#ifndef LOVE_GRAPHICS_OPENGL_WRAP_SPRITE_BATCH_H
#define LOVE_GRAPHICS_OPENGL_WRAP_SPRITE_BATCH_H

#include "common/runtime.h"
#include "SpriteBatch.h"

namespace love
{
namespace graphics
{
namespace opengl
{

SpriteBatch *luax_checkspritebatch(lua_State *L, int idx);
int w_SpriteBatch_add(lua_State *L);
int w_SpriteBatch_addq(lua_State *L);
int w_SpriteBatch_set(lua_State *L);
int w_SpriteBatch_setq(lua_State *L);
int w_SpriteBatch_remove(lua_State *L);
int w_SpriteBatch_clear(lua_State *L);
int w_SpriteBatch_lock(lua_State *L);
int w_SpriteBatch_unlock(lua_State *L);
int w_SpriteBatch_setImage(lua_State *L);
int w_SpriteBatch_getImage(lua_State *L);

extern "C" int luaopen_spritebatch(lua_State *L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_SPRITE_BATCH_H
