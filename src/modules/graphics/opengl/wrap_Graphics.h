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

#ifndef LOVE_GRAPHICS_OPENGL_WRAP_GRAPHICS_H
#define LOVE_GRAPHICS_OPENGL_WRAP_GRAPHICS_H

// LOVE
#include "common/config.h"
#include "wrap_Font.h"
#include "wrap_Image.h"
#include "graphics/wrap_Quad.h"
#include "wrap_SpriteBatch.h"
#include "wrap_ParticleSystem.h"
#include "wrap_Canvas.h"
#include "wrap_Shader.h"
#include "wrap_Mesh.h"
#include "wrap_Text.h"
#include "wrap_Video.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{

extern "C" LOVE_EXPORT int luaopen_love_graphics(lua_State *L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_GRAPHICS_H
