#include "wrap_Framebuffer.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Framebuffer * luax_checkfbo(lua_State * L, int idx)
	{
		return luax_checktype<Framebuffer>(L, idx, "Framebuffer", GRAPHICS_FRAMEBUFFER_T);
	}

	int w_Framebuffer_renderTo(lua_State * L)
	{
		// As startGrab() clears the framebuffer, better not allow
		// grabbing inside another grabbing
		if (Framebuffer::current != NULL) {
			Framebuffer::bindDefaultBuffer();
			return luaL_error(L, "Current render target not the default framebuffer!");
		}

		Framebuffer * fbo = luax_checkfbo(L, 1);
		if (!lua_isfunction(L, 2))
			return luaL_error(L, "Need a function to render to fbo");

		fbo->startGrab();
		lua_settop(L, 2); // make sure the function is on top of the stack
		lua_call(L, 0, 0);
		fbo->stopGrab();

		return 0;
	}

	static const luaL_Reg functions[] = {
		{ "renderTo", w_Framebuffer_renderTo },
		{ 0, 0 }
	};

	int luaopen_framebuffer(lua_State * L)
	{
		return luax_register_type(L, "Framebuffer", functions);
	}

} // opengl
} // graphics
} // love
