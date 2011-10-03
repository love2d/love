#include "Graphics.h"
#include "wrap_Canvas.h"

namespace love
{
namespace graphics
{
namespace opengl
{
	Canvas * luax_checkcanvas(lua_State * L, int idx)
	{
		return luax_checktype<Canvas>(L, idx, "Canvas", GRAPHICS_CANVAS_T);
	}

	int w_Canvas_renderTo(lua_State * L)
	{
		// As startGrab() clears the framebuffer, better not allow
		// grabbing inside another grabbing
		if (Canvas::current != NULL) {
			Canvas::bindDefaultCanvas();
			return luaL_error(L, "Current render target not the default canvas!");
		}

		Canvas * canvas = luax_checkcanvas(L, 1);
		if (!lua_isfunction(L, 2))
			return luaL_error(L, "Need a function to render to canvas.");

		canvas->startGrab();
		lua_settop(L, 2); // make sure the function is on top of the stack
		lua_call(L, 0, 0);
		canvas->stopGrab();

		return 0;
	}

	int w_Canvas_getImageData(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		love::image::Image * image = luax_getmodule<love::image::Image>(L, "image", MODULE_IMAGE_T);
		love::image::ImageData * img = canvas->getImageData( image );
		luax_newtype(L, "ImageData", IMAGE_IMAGE_DATA_T, (void *)img);
		return 1;
	}

	int w_Canvas_setFilter(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		const char * minstr = luaL_checkstring(L, 2);
		const char * magstr = luaL_checkstring(L, 3);

		Image::Filter f;
		if (!Image::getConstant(minstr, f.min))
			return luaL_error(L, "Invalid min filter mode: %s", minstr);
		if (!Image::getConstant(magstr, f.mag))
			return luaL_error(L, "Invalid max filter mode: %s", magstr);

		canvas->setFilter(f);
		return 0;
	}

	int w_Canvas_getFilter(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		Image::Filter f = canvas->getFilter();

		const char * minstr;
		const char * magstr;
		Image::getConstant(f.min, minstr);
		Image::getConstant(f.mag, magstr);

		lua_pushstring(L, minstr);
		lua_pushstring(L, magstr);

		return 2;
	}

	int w_Canvas_setWrap(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		const char * wrap_s = luaL_checkstring(L, 2);
		const char * wrap_t = luaL_checkstring(L, 3);

		Image::Wrap w;
		if (!Image::getConstant(wrap_s, w.s))
			return luaL_error(L, "Invalid wrap mode: %s", wrap_s);
		if (!Image::getConstant(wrap_t, w.t))
			return luaL_error(L, "Invalid wrap mode: %s", wrap_t);

		canvas->setWrap(w);
		return 0;
	}

	int w_Canvas_getWrap(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		Image::Wrap w = canvas->getWrap();

		const char * wrap_s;
		const char * wrap_t;
		Image::getConstant(w.s, wrap_s);
		Image::getConstant(w.t, wrap_t);

		lua_pushstring(L, wrap_s);
		lua_pushstring(L, wrap_t);

		return 2;
	}

	int w_Canvas_clear(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
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
		canvas->clear(c);

		return 0;
	}

	int w_Canvas_getWidth(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		lua_pushnumber(L, canvas->getWidth());
		return 1;
	}

	int w_Canvas_getHeight(lua_State * L)
	{
		Canvas * canvas = luax_checkcanvas(L, 1);
		lua_pushnumber(L, canvas->getHeight());
		return 1;
	}

	static const luaL_Reg functions[] = {
		{ "renderTo", w_Canvas_renderTo },
		{ "getImageData", w_Canvas_getImageData },
		{ "setFilter", w_Canvas_setFilter },
		{ "getFilter", w_Canvas_getFilter },
		{ "setWrap", w_Canvas_setWrap },
		{ "getWrap", w_Canvas_getWrap },
		{ "clear", w_Canvas_clear },
		{ "getWidth", w_Canvas_getWidth },
		{ "getHeight", w_Canvas_getHeight },
		{ 0, 0 }
	};

	int luaopen_canvas(lua_State * L)
	{
		return luax_register_type(L, "Canvas", functions);
	}

} // opengl
} // graphics
} // love
