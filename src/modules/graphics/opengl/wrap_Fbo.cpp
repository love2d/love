#include "wrap_Fbo.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Fbo * luax_checkfbo(lua_State * L, int idx)
	{
		return luax_checktype<Fbo>(L, idx, "Fbo", GRAPHICS_FBO_T);
	}

	int w_Fbo_render(lua_State * L)
	{
		Fbo * fbo = luax_checkfbo(L, 1);
		if (!lua_isfunction(L, 2))
			return luaL_error(L, "Need a function to render to fbo");

		fbo->bind();

		lua_settop(L, 2); // make sure the function is on top of the stack
		lua_pcall(L, 0, 0, 0);

		fbo->unbind();

		return 0;
	}

	int w_Fbo_bind(lua_State * L)
	{
		Fbo * fbo = luax_checkfbo(L, 1);
		fbo->bind();
		return 0;
	}

	int w_Fbo_unbind(lua_State * L)
	{
		Fbo * fbo = luax_checkfbo(L, 1);
		fbo->unbind();
		return 0;
	}

	static const luaL_Reg functions[] = {
		{ "render", w_Fbo_render },
		{ "bind", w_Fbo_bind },
		{ "unbind", w_Fbo_unbind },
		{ 0, 0 }
	};

	int luaopen_fbo(lua_State * L)
	{
		return luax_register_type(L, "Fbo", functions);
	}

} // opengl
} // graphics
} // love
