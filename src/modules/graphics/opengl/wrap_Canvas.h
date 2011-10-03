#ifndef LOVE_GRAPHICS_OPENGL_WRAP_CANVAS_H
#define LOVE_GRAPHICS_OPENGL_WRAP_CANVAS_H

// LOVE
#include <common/runtime.h>
#include "Canvas.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	//see Canvas.h
	Canvas * luax_checkcanvas(lua_State * L, int idx);
	int w_Canvas_renderTo(lua_State * L);
	int w_Canvas_getImageData(lua_State * L);
	int w_Canvas_setFilter(lua_State * L);
	int w_Canvas_getFilter(lua_State * L);
	int w_Canvas_setWrap(lua_State * L);
	int w_Canvas_getWrap(lua_State * L);
	int w_Canvas_clear(lua_State * L);
	int w_Canvas_getWidth(lua_State * L);
	int w_Canvas_getHeight(lua_State * L);
	int luaopen_canvas(lua_State * L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_CANVAS_H
