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

	int w_Framebuffer_render(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		if (!lua_isfunction(L, 2))
			return luaL_error(L, "Need a function to render to fbo");

		fbo->bind();

		lua_settop(L, 2); // make sure the function is on top of the stack
		lua_pcall(L, 0, 0, 0);

		fbo->unbind();

		return 0;
	}

	int w_Framebuffer_bind(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		fbo->bind();
		return 0;
	}

	int w_Framebuffer_unbind(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		fbo->unbind();
		return 0;
	}

	static const luaL_Reg functions[] = {
		{ "render", w_Framebuffer_render },
		{ "bind", w_Framebuffer_bind },
		{ "unbind", w_Framebuffer_unbind },
		{ 0, 0 }
	};

	int luaopen_framebuffer(lua_State * L)
	{
		return luax_register_type(L, "Framebuffer", functions);
	}

} // opengl
} // graphics
} // love
