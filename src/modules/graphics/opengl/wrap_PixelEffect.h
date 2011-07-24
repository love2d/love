#ifndef LOVE_GRAPHICS_OPENGL_WRAP_PROGRAM_H
#define LOVE_GRAPHICS_OPENGL_WRAP_PROGRAM_H

#include <common/runtime.h>
#include "PixelEffect.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	PixelEffect * luax_checkpixeleffect(lua_State * L, int idx);
	int w_PixelEffect_getWarnings(lua_State * L);
	int w_PixelEffect_sendFloat(lua_State * L);
	int w_PixelEffect_sendMatrix(lua_State * L);
	int w_PixelEffect_sendImage(lua_State * L);
	int luaopen_pixeleffect(lua_State * L);
} // opengl
} // graphics
} // love

#endif
