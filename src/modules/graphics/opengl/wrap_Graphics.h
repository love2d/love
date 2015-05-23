/**
 * Copyright (c) 2006-2015 LOVE Development Team
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
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{

int w_reset(lua_State *L);
int w_clear(lua_State *L);
int w_discard(lua_State *L);
int w_present(lua_State *L);
int w_isCreated(lua_State *L);
int w_isActive(lua_State *L);
int w_getWidth(lua_State *L);
int w_getHeight(lua_State *L);
int w_getDimensions(lua_State *L);
int w_setScissor(lua_State *L);
int w_getScissor(lua_State *L);
int w_stencil(lua_State *L);
int w_setStencilTest(lua_State *L);
int w_getStencilTest(lua_State *L);
int w_newImage(lua_State *L);
int w_newQuad(lua_State *L);
int w_newFont(lua_State *L);
int w_newImageFont(lua_State *L);
int w_newSpriteBatch(lua_State *L);
int w_newParticleSystem(lua_State *L);
int w_newCanvas(lua_State *L);
int w_newShader(lua_State *L);
int w_newMesh(lua_State *L);
int w_newText(lua_State *L);
int w_setColor(lua_State *L);
int w_getColor(lua_State *L);
int w_setBackgroundColor(lua_State *L);
int w_getBackgroundColor(lua_State *L);
int w_setNewFont(lua_State *L);
int w_setFont(lua_State *L);
int w_getFont(lua_State *L);
int w_setColorMask(lua_State *L);
int w_getColorMask(lua_State *L);
int w_setBlendMode(lua_State *L);
int w_getBlendMode(lua_State *L);
int w_setDefaultFilter(lua_State *L);
int w_getDefaultFilter(lua_State *L);
int w_setDefaultMipmapFilter(lua_State *L);
int w_getDefaultMipmapFilter(lua_State *L);
int w_setLineWidth(lua_State *L);
int w_setLineStyle(lua_State *L);
int w_setLineJoin(lua_State *L);
int w_getLineWidth(lua_State *L);
int w_getLineStyle(lua_State *L);
int w_getLineJoin(lua_State *L);
int w_setPointSize(lua_State *L);
int w_getPointSize(lua_State *L);
int w_setWireframe(lua_State *L);
int w_isWireframe(lua_State *L);
int w_newScreenshot(lua_State *L);
int w_setCanvas(lua_State *L);
int w_getCanvas(lua_State *L);
int w_setShader(lua_State *L);
int w_getShader(lua_State *L);
int w_setDefaultShaderCode(lua_State *L);
int w_getSupported(lua_State *L);
int w_getCanvasFormats(lua_State *L);
int w_getCompressedImageFormats(lua_State *L);
int w_getRendererInfo(lua_State *L);
int w_getSystemLimits(lua_State *L);
int w_getStats(lua_State *L);
int w_draw(lua_State *L);
int w_print(lua_State *L);
int w_printf(lua_State *L);
int w_point(lua_State *L);
int w_line(lua_State *L);
int w_rectangle(lua_State *L);
int w_circle(lua_State *L);
int w_ellipse(lua_State *L);
int w_arc(lua_State *L);
int w_roundedRectangle(lua_State *L);
int w_polygon(lua_State *L);
int w_push(lua_State *L);
int w_pop(lua_State *L);
int w_rotate(lua_State *L);
int w_scale(lua_State *L);
int w_translate(lua_State *L);
int w_shear(lua_State *L);
int w_origin(lua_State *L);
extern "C" LOVE_EXPORT int luaopen_love_graphics(lua_State *L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_GRAPHICS_H
