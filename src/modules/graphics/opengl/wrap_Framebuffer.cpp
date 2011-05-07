#include "Graphics.h"
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

	int w_Framebuffer_getImageData(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		love::image::Image * image = luax_getmodule<love::image::Image>(L, "image", MODULE_IMAGE_T);
		love::image::ImageData * img = fbo->getImageData( image );
		luax_newtype(L, "ImageData", IMAGE_IMAGE_DATA_T, (void *)img);
		return 1;
	}

	int w_Framebuffer_setFilter(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		const char * minstr = luaL_checkstring(L, 2);
		const char * magstr = luaL_checkstring(L, 3);

		Image::Filter f;
		if (!Image::getConstant(minstr, f.min))
			return luaL_error(L, "Invalid min filter mode: %s", minstr);
		if (!Image::getConstant(magstr, f.mag))
			return luaL_error(L, "Invalid max filter mode: %s", magstr);

		fbo->setFilter(f);
		return 0;
	}

	int w_Framebuffer_getFilter(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		Image::Filter f = fbo->getFilter();

		const char * minstr;
		const char * magstr;
		Image::getConstant(f.min, minstr);
		Image::getConstant(f.mag, magstr);

		lua_pushstring(L, minstr);
		lua_pushstring(L, magstr);

		return 2;
	}

	int w_Framebuffer_setWrap(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		const char * wrap_s = luaL_checkstring(L, 2);
		const char * wrap_t = luaL_checkstring(L, 3);

		Image::Wrap w;
		if (!Image::getConstant(wrap_s, w.s))
			return luaL_error(L, "Invalid wrap mode: %s", wrap_s);
		if (!Image::getConstant(wrap_t, w.t))
			return luaL_error(L, "Invalid wrap mode: %s", wrap_t);

		fbo->setWrap(w);
		return 0;
	}

	int w_Framebuffer_getWrap(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		Image::Wrap w = fbo->getWrap();

		const char * wrap_s;
		const char * wrap_t;
		Image::getConstant(w.s, wrap_s);
		Image::getConstant(w.t, wrap_t);

		lua_pushstring(L, wrap_s);
		lua_pushstring(L, wrap_t);

		return 2;
	}

	int w_Framebuffer_clear(lua_State * L)
	{
		Framebuffer * fbo = luax_checkfbo(L, 1);
		Color c;
		if (lua_isnoneornil(L, 2)) {
			c.r = 0;
			c.g = 0;
			c.b = 0;
			c.a = 0;
		} else if (lua_istable(L, 2)) {
			lua_pushinteger(L, 1);
			lua_gettable(L, 2);
			c.r = (unsigned char)luaL_checkint(L, -1);
			lua_pushinteger(L, 2);
			lua_gettable(L, 2);
			c.g = (unsigned char)luaL_checkint(L, -1);
			lua_pushinteger(L, 3);
			lua_gettable(L, 2);
			c.b = (unsigned char)luaL_checkint(L, -1);
			lua_pushinteger(L, 4);
			lua_gettable(L, 2);
			c.g = (unsigned char)luaL_optint(L, -1, 255);
			lua_pop(L, 4);
		} else {
			c.r = (unsigned char)luaL_checkint(L, 2);
			c.g = (unsigned char)luaL_checkint(L, 3);
			c.b = (unsigned char)luaL_checkint(L, 4);
			c.a = (unsigned char)luaL_optint(L, 5, 255);
		}
		fbo->clear(c);

		return 0;
	}

	static const luaL_Reg functions[] = {
		{ "renderTo", w_Framebuffer_renderTo },
		{ "getImageData", w_Framebuffer_getImageData },
		{ "setFilter", w_Framebuffer_setFilter },
		{ "getFilter", w_Framebuffer_getFilter },
		{ "setWrap", w_Framebuffer_setWrap },
		{ "getWrap", w_Framebuffer_getWrap },
		{ "clear", w_Framebuffer_clear },
		{ 0, 0 }
	};

	int luaopen_framebuffer(lua_State * L)
	{
		return luax_register_type(L, "Framebuffer", functions);
	}

} // opengl
} // graphics
} // love
