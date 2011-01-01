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
			return luaL_error(L, "Invalid max filter mode: %s", minstr);

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

	static const luaL_Reg functions[] = {
		{ "renderTo", w_Framebuffer_renderTo },
		{ "getImageData", w_Framebuffer_getImageData },
		{ "setFilter", w_Framebuffer_setFilter },
		{ "getFilter", w_Framebuffer_getFilter },
		{ 0, 0 }
	};

	int luaopen_framebuffer(lua_State * L)
	{
		return luax_register_type(L, "Framebuffer", functions);
	}

} // opengl
} // graphics
} // love
