#ifndef LOVE_GRAPHICS_OPENGL_WRAP_FBO_H
#define LOVE_GRAPHICS_OPENGL_WRAP_FBO_H

// LOVE
#include <common/runtime.h>
#include "Framebuffer.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	//see Framebuffer.h
	Framebuffer * luax_checkfbo(lua_State * L, int idx);
	int w_Framebuffer_renderTo(lua_State * L);
	int w_Framebuffer_getImageData(lua_State * L);
	int luaopen_framebuffer(lua_State * L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_FBO_H
