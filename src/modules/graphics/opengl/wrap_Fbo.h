#ifndef LOVE_GRAPHICS_OPENGL_WRAP_FBO_H
#define LOVE_GRAPHICS_OPENGL_WRAP_FBO_H

// LOVE
#include <common/runtime.h>
#include "Fbo.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	//see Fbo.h
	Fbo * luax_checkfbo(lua_State * L, int idx);
	int w_Fbo_render(lua_State * L);
	int w_Fbo_bind(lua_State * L);
	int w_Fbo_unbind(lua_State * L);
	int luaopen_fbo(lua_State * L);

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_WRAP_FBO_H
