/**
* Copyright (c) 2006-2009 LOVE Development Team
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
#include "wrap_Font.h"
#include "wrap_Image.h"
#include "wrap_Frame.h"
#include "wrap_SpriteBatch.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	int _wrap_checkMode(lua_State * L);
	int _wrap_setMode(lua_State * L);
	int _wrap_toggleFullscreen(lua_State * L);
	int _wrap_reset(lua_State * L);
	int _wrap_clear(lua_State * L);
	int _wrap_present(lua_State * L);
	int _wrap_setCaption(lua_State * L);
	int _wrap_getCaption(lua_State * L);
	int _wrap_getWidth(lua_State * L);
	int _wrap_getHeight(lua_State * L);
	int _wrap_isCreated(lua_State * L);
	int _wrap_setScissor(lua_State * L);
	int _wrap_getScissor(lua_State * L);
	int _wrap_newImage(lua_State * L);
	int _wrap_newFrame(lua_State * L);
	int _wrap_newFont(lua_State * L);
	int _wrap_newImageFont(lua_State * L);
	int _wrap_newSpriteBatch(lua_State * L);
	int _wrap_setColor(lua_State * L);
	int _wrap_getColor(lua_State * L);
	int _wrap_setBackgroundColor(lua_State * L);
	int _wrap_getBackgroundColor(lua_State * L);
	int _wrap_setFont(lua_State * L);
	int _wrap_getFont(lua_State * L);
	int _wrap_setBlendMode(lua_State * L);
	int _wrap_setColorMode(lua_State * L);
	int _wrap_getBlendMode(lua_State * L);
	int _wrap_getColorMode(lua_State * L);
	int _wrap_setLineWidth(lua_State * L);
	int _wrap_setLineStyle(lua_State * L);
	int _wrap_setLine(lua_State * L);
	int _wrap_setLineStipple(lua_State * L);
	int _wrap_getLineWidth(lua_State * L);
	int _wrap_getLineStyle(lua_State * L);
	int _wrap_getLineStipple(lua_State * L);
	int _wrap_setPointSize(lua_State * L);
	int _wrap_setPointStyle(lua_State * L);
	int _wrap_setPoint(lua_State * L);
	int _wrap_getPointSize(lua_State * L);
	int _wrap_getPointStyle(lua_State * L);
	int _wrap_getMaxPointSize(lua_State * L);
	int _wrap_draw(lua_State * L);
	int _wrap_draws(lua_State * L);
	int _wrap_drawTest(lua_State * L);
	int _wrap_print1(lua_State * L);
	int _wrap_printf1(lua_State * L);
	int _wrap_point(lua_State * L);
	int _wrap_line(lua_State * L);
	int _wrap_triangle(lua_State * L);
	int _wrap_rectangle(lua_State * L);
	int _wrap_quad(lua_State * L);
	int _wrap_circle(lua_State * L);
	int _wrap_push(lua_State * L);
	int _wrap_pop(lua_State * L);
	int _wrap_rotate(lua_State * L);
	int _wrap_scale(lua_State * L);
	int _wrap_translate(lua_State * L);
	int wrap_Graphics_open(lua_State * L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_GRAPHICS_H
