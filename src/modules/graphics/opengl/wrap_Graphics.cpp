/**
 * Copyright (c) 2006-2013 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "wrap_Graphics.h"
#include "OpenGL.h"
#include "graphics/DrawQable.h"
#include "image/ImageData.h"
#include "font/Rasterizer.h"

#include "scripts/graphics.lua.h"
#include <cassert>

using love::window::WindowFlags;

namespace love
{
namespace graphics
{
namespace opengl
{

static Graphics *instance = 0;

bool luax_boolflag(lua_State *L, int table_index, const char *key, bool defaultValue)
{
	lua_getfield(L, table_index, key);

	bool retval;
	if (lua_isnoneornil(L, -1))
		retval = defaultValue;
	else
		retval = lua_toboolean(L, -1);

	lua_pop(L, 1);
	return retval;
}

int luax_intflag(lua_State *L, int table_index, const char *key, int defaultValue)
{
	lua_getfield(L, table_index, key);

	int retval;
	if (!lua_isnumber(L, -1))
		retval = defaultValue;
	else
		retval = lua_tonumber(L, -1);

	lua_pop(L, 1);
	return retval;
}

int w_checkMode(lua_State *L)
{
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);
	bool fs = luax_toboolean(L, 3);
	luax_pushboolean(L, instance->checkMode(w, h, fs));
	return 1;
}

int w_setMode(lua_State *L)
{
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);
	if (lua_isnoneornil(L, 3))
	{
		luax_pushboolean(L, instance->setMode(w, h, 0));
		return 1;
	}

	luaL_checktype(L, 3, LUA_TTABLE);

	WindowFlags flags;

	flags.fullscreen = luax_boolflag(L, 3, "fullscreen", false);
	flags.vsync = luax_boolflag(L, 3, "vsync", true);
	flags.fsaa = luax_intflag(L, 3, "fsaa", 0);
	flags.resizable = luax_boolflag(L, 3, "resizable", false);
	flags.borderless = luax_boolflag(L, 3, "borderless", false);
	flags.centered = luax_boolflag(L, 3, "centered", true);

	luax_pushboolean(L, instance->setMode(w, h, &flags));
	return 1;
}

int w_getMode(lua_State *L)
{
	int w, h;
	WindowFlags flags;
	instance->getMode(w, h, flags);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	lua_newtable(L);

	luax_pushboolean(L, flags.fullscreen);
	lua_setfield(L, -2, "fullscreen");

	luax_pushboolean(L, flags.vsync);
	lua_setfield(L, -2, "vsync");

	lua_pushnumber(L, flags.fsaa);
	lua_setfield(L, -2, "fsaa");

	luax_pushboolean(L, flags.resizable);
	lua_setfield(L, -2, "resizable");

	luax_pushboolean(L, flags.borderless);
	lua_setfield(L, -2, "borderless");

	luax_pushboolean(L, flags.centered);
	lua_setfield(L, -2, "centered");

	return 3;
}

int w_toggleFullscreen(lua_State *L)
{
	luax_pushboolean(L, instance->toggleFullscreen());
	return 1;
}

int w_reset(lua_State *)
{
	instance->reset();
	return 0;
}

int w_clear(lua_State *)
{
	instance->clear();
	return 0;
}

int w_present(lua_State *)
{
	instance->present();
	return 0;
}

int w_setIcon(lua_State *L)
{
	Image *image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
	instance->setIcon(image);
	return 0;
}

int w_setCaption(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	instance->setCaption(str);
	return 0;
}

int w_getCaption(lua_State *L)
{
	return instance->getCaption(L);
}

int w_getWidth(lua_State *L)
{
	lua_pushnumber(L, instance->getWidth());
	return 1;
}

int w_getHeight(lua_State *L)
{
	lua_pushnumber(L, instance->getHeight());
	return 1;
}

int w_getDimensions(lua_State *L)
{
	lua_pushnumber(L, instance->getWidth());
	lua_pushnumber(L, instance->getHeight());
	return 2;
}

int w_isCreated(lua_State *L)
{
	luax_pushboolean(L, instance->isCreated());
	return 1;
}

int w_getModes(lua_State *L)
{
	return instance->getModes(L);
}

int w_setScissor(lua_State *L)
{
	if (lua_gettop(L) == 0)
	{
		instance->setScissor();
		return 0;
	}

	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int w = luaL_checkint(L, 3);
	int h = luaL_checkint(L, 4);

	if (w < 0 || h < 0)
		return luaL_error(L, "Can't set scissor with negative width and/or height.");

	instance->setScissor(x, y, w, h);
	return 0;
}

int w_getScissor(lua_State *L)
{
	return instance->getScissor(L);
}

int w_newStencil(lua_State *L)
{
	// just return the function
	if (!lua_isfunction(L, 1))
		return luaL_typerror(L, 1, "function");
	lua_settop(L, 1);
	return 1;
}

static int setStencil(lua_State *L, bool invert)
{
	// no argument -> clear mask
	if (lua_isnoneornil(L, 1))
	{
		instance->discardStencil();
		return 0;
	}

	if (!lua_isfunction(L, 1))
		return luaL_typerror(L, 1, "mask");

	instance->defineStencil();
	lua_call(L, lua_gettop(L) - 1, 0); // call mask(...)
	instance->useStencil(invert);

	return 0;
}

int w_setStencil(lua_State *L)
{
	return setStencil(L, false);
}

int w_setInvertedStencil(lua_State *L)
{
	return setStencil(L, true);
}

int w_newImage(lua_State *L)
{
	// Convert to File, if necessary.
	if (lua_isstring(L, 1))
		luax_convobj(L, 1, "filesystem", "newFile");

	// Convert to ImageData, if necessary.
	if (luax_istype(L, 1, FILESYSTEM_FILE_T))
		luax_convobj(L, 1, "image", "newImageData");

	love::image::ImageData *data = luax_checktype<love::image::ImageData>(L, 1, "ImageData", IMAGE_IMAGE_DATA_T);

	// Create the image.
	Image *image = 0;
	try
	{
		image = instance->newImage(data);
	}
	catch(love::Exception &e)
	{
		luaL_error(L, e.what());
	}

	if (image == 0)
		return luaL_error(L, "Could not load image.");


	// Push the type.
	luax_newtype(L, "Image", GRAPHICS_IMAGE_T, (void *)image);

	return 1;
}

int w_newQuad(lua_State *L)
{
	float x = (float) luaL_checknumber(L, 1);
	float y = (float) luaL_checknumber(L, 2);
	float w = (float) luaL_checknumber(L, 3);
	float h = (float) luaL_checknumber(L, 4);
	float sw = (float) luaL_checknumber(L, 5);
	float sh = (float) luaL_checknumber(L, 6);

	Quad *quad = instance->newQuad(x, y, w, h, sw, sh);

	luax_newtype(L, "Quad", GRAPHICS_QUAD_T, (void *)quad);
	return 1;
}

int w_newFont(lua_State *L)
{
	Data *font_data = NULL;
	// Convert to File, if necessary.
	if (lua_isstring(L, 1))
		luax_convobj(L, 1, "filesystem", "newFile");

	// Convert to Data, if necessary.
	if (luax_istype(L, 1, FILESYSTEM_FILE_T))
	{
		love::filesystem::File *f = luax_checktype<love::filesystem::File>(L, 1, "File", FILESYSTEM_FILE_T);
		try
		{
			font_data = f->read();
		}
		catch(love::Exception &e)
		{
			return luaL_error(L, e.what());
		}
		lua_remove(L, 1); // get rid of the file
		luax_newtype(L, "Data", DATA_T, (void *)font_data);
		lua_insert(L, 1); // put it at the bottom of the stack
	}

	// Convert to Rasterizer, if necessary.
	if (luax_istype(L, 1, DATA_T))
	{
		int idxs[] = {1, 2};
		int ret = luax_pconvobj(L, idxs, 2, "font", "newRasterizer");
		if (ret != 0)
		{
			if (font_data)
				font_data->release();
			return lua_error(L);
		}
	}

	if (font_data)
		font_data->release();

	love::font::Rasterizer *rasterizer = luax_checktype<love::font::Rasterizer>(L, 1, "Rasterizer", FONT_RASTERIZER_T);

	Font *font = NULL;
	try
	{
		// Create the font.
		font = instance->newFont(rasterizer, instance->getDefaultFilter());
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, e.what());
	}

	if (font == 0)
		return luaL_error(L, "Could not load font.");

	// Push the type.
	luax_newtype(L, "Font", GRAPHICS_FONT_T, (void *)font);

	return 1;
}

int w_newImageFont(lua_State *L)
{
	// filter for glyphs, defaults to linear/linear
	Image::Filter img_filter;
	bool setFilter = false;

	// For the filter modes..
	int startIndex = 2;

	// Convert to ImageData if necessary.
	if (lua_isstring(L, 1) || luax_istype(L, 1, FILESYSTEM_FILE_T) || (luax_istype(L, 1, DATA_T) && !luax_istype(L, 1, IMAGE_IMAGE_DATA_T)))
		luax_convobj(L, 1, "image", "newImageData");
	else if (luax_istype(L, 1, GRAPHICS_IMAGE_T))
	{
		Image *i = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
		img_filter = i->getFilter();
		setFilter = true;
		love::image::ImageData *id = i->getData();
		luax_newtype(L, "ImageData", IMAGE_IMAGE_DATA_T, (void *)id, false);
		lua_replace(L, 1);
	}

	// Convert to Rasterizer if necessary.
	if (luax_istype(L, 1, IMAGE_IMAGE_DATA_T))
	{
		int idxs[] = {1, 2};
		luax_convobj(L, idxs, 2, "font", "newRasterizer");
		startIndex = 3; // There's a glyphs args in there, move up
	}

	love::font::Rasterizer *rasterizer = luax_checktype<love::font::Rasterizer>(L, 1, "Rasterizer", FONT_RASTERIZER_T);

	if (lua_isstring(L, startIndex) && lua_isstring(L, startIndex+1))
	{
		Image::FilterMode min;
		Image::FilterMode mag;
		const char *minstr = luaL_checkstring(L, startIndex);
		const char *magstr = luaL_checkstring(L, startIndex+1);
		if (!Image::getConstant(minstr, min))
			return luaL_error(L, "Invalid filter mode: %s", minstr);
		if (!Image::getConstant(magstr, mag))
			return luaL_error(L, "Invalid filter mode: %s", magstr);

		img_filter.min = min;
		img_filter.mag = mag;
		setFilter = true;
	}

	if (!setFilter)
		img_filter = instance->getDefaultFilter();

	// Create the font.
	Font *font = instance->newFont(rasterizer, img_filter);

	if (font == 0)
		return luaL_error(L, "Could not load font.");

	// Push the type.
	luax_newtype(L, "Font", GRAPHICS_FONT_T, (void *)font);

	return 1;
}

int w_newSpriteBatch(lua_State *L)
{
	Image *image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
	int size = luaL_optint(L, 2, 1000);
	SpriteBatch::UsageHint usage = SpriteBatch::USAGE_DYNAMIC;
	if (lua_gettop(L) > 2)
	{
		if (!SpriteBatch::getConstant(luaL_checkstring(L, 3), usage))
			usage = SpriteBatch::USAGE_DYNAMIC;
	}
	SpriteBatch *t = NULL;
	try
	{
		t = instance->newSpriteBatch(image, size, usage);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, e.what());
	}
	luax_newtype(L, "SpriteBatch", GRAPHICS_SPRITE_BATCH_T, (void *)t);
	return 1;
}

int w_newParticleSystem(lua_State *L)
{
	Image *image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
	int size = luaL_checkint(L, 2);
	ParticleSystem *t = instance->newParticleSystem(image, size);
	luax_newtype(L, "ParticleSystem", GRAPHICS_PARTICLE_SYSTEM_T, (void *)t);
	return 1;
}

int w_newCanvas(lua_State *L)
{
	// check if width and height are given. else default to screen dimensions.
	int width       = luaL_optint(L, 1, instance->getWidth());
	int height      = luaL_optint(L, 2, instance->getHeight());
	const char *str = luaL_optstring(L, 3, "normal");

	Canvas::TextureType texture_type;
	if (!Canvas::getConstant(str, texture_type))
		return luaL_error(L, "Invalid canvas type: %s", str);

	Canvas *canvas = NULL;
	try
	{
		canvas = instance->newCanvas(width, height, texture_type);
	}
	catch(Exception &e)
	{
		return luaL_error(L, e.what());
	}

	if (NULL == canvas)
		return luaL_error(L, "Canvas not created, but no error thrown. I don't even...");

	luax_newtype(L, "Canvas", GRAPHICS_CANVAS_T, (void *)canvas);
	return 1;
}

int w_newShader(lua_State *L)
{
	if (!Shader::isSupported())
		return luaL_error(L, "Sorry, your graphics card does not support shaders.");

	// clamp stack to 2 elements
	lua_settop(L, 2);

	// read any filepath arguments
	for (int i = 1; i <= 2; i++)
	{
		if (!lua_isstring(L, i))
			continue;

		// call love.filesystem.isFile(arg_i)
		luax_getfunction(L, "filesystem", "isFile");
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);

		bool isFile = luax_toboolean(L, -1);
		lua_pop(L, 1);

		if (isFile)
		{
			luax_getfunction(L, "filesystem", "read");
			lua_pushvalue(L, i);
			lua_call(L, 1, 1);
			lua_replace(L, i);
		}
	}

	bool has_arg1 = lua_isstring(L, 1);
	bool has_arg2 = lua_isstring(L, 2);

	// require at least one string argument
	if (!(has_arg1 || has_arg2))
		luaL_checkstring(L, 1);

	luax_getfunction(L, "graphics", "_shaderCodeToGLSL");

	// push vertexcode and pixelcode strings to the top of the stack
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);

	// call effectCodeToGLSL, returned values will be at the top of the stack
	if (lua_pcall(L, 2, 2, 0) != 0)
		return luaL_error(L, "%s", lua_tostring(L, -1));

	Shader::ShaderSources sources;

	// vertex shader code
	if (lua_isstring(L, -2))
	{
		std::string vertexcode(luaL_checkstring(L, -2));
		sources[Shader::TYPE_VERTEX] = vertexcode;
	}
	else if (has_arg1 && has_arg2)
		return luaL_error(L, "Could not parse vertex shader code (missing 'position' function?)");

	// pixel shader code
	if (lua_isstring(L, -1))
	{
		std::string pixelcode(luaL_checkstring(L, -1));
		sources[Shader::TYPE_PIXEL] = pixelcode;
	}
	else if (has_arg1 && has_arg2)
		return luaL_error(L, "Could not parse pixel shader code (missing 'effect' function?)");

	if (sources.empty())
	{
		// Original args had source code, but effectCodeToGLSL couldn't translate it
		for (int i = 1; i <= 2; i++)
		{
			if (lua_isstring(L, i))
				return luaL_argerror(L, i, "missing 'position' or 'effect' function?");
		}
	}

	try
	{
		Shader *shader = instance->newShader(sources);
		luax_newtype(L, "Shader", GRAPHICS_SHADER_T, (void *)shader);
	}
	catch (const love::Exception &e)
	{
		// memory is freed in Graphics::newShader
		luax_getfunction(L, "graphics", "_transformGLSLErrorMessages");
		lua_pushstring(L, e.what());
		lua_pcall(L, 1, 1, 0);
		const char *err = lua_tostring(L, -1);
		return luaL_error(L, "%s", err);
	}

	return 1;
}

int w_setColor(lua_State *L)
{
	Color c;
	if (lua_istable(L, 1))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 1, i);

		c.r = (unsigned char)luaL_checkint(L, -4);
		c.g = (unsigned char)luaL_checkint(L, -3);
		c.b = (unsigned char)luaL_checkint(L, -2);
		c.a = (unsigned char)luaL_optint(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (unsigned char)luaL_checkint(L, 1);
		c.g = (unsigned char)luaL_checkint(L, 2);
		c.b = (unsigned char)luaL_checkint(L, 3);
		c.a = (unsigned char)luaL_optint(L, 4, 255);
	}
	instance->setColor(c);
	return 0;
}

int w_getColor(lua_State *L)
{
	Color c = instance->getColor();
	lua_pushinteger(L, c.r);
	lua_pushinteger(L, c.g);
	lua_pushinteger(L, c.b);
	lua_pushinteger(L, c.a);
	return 4;
}

int w_setBackgroundColor(lua_State *L)
{
	Color c;
	if (lua_istable(L, 1))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 1, i);

		c.r = (unsigned char)luaL_checkint(L, -4);
		c.g = (unsigned char)luaL_checkint(L, -3);
		c.b = (unsigned char)luaL_checkint(L, -2);
		c.a = (unsigned char)luaL_optint(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (unsigned char)luaL_checkint(L, 1);
		c.g = (unsigned char)luaL_checkint(L, 2);
		c.b = (unsigned char)luaL_checkint(L, 3);
		c.a = (unsigned char)luaL_optint(L, 4, 255);
	}
	instance->setBackgroundColor(c);
	return 0;
}

int w_getBackgroundColor(lua_State *L)
{
	Color c = instance->getBackgroundColor();
	lua_pushinteger(L, c.r);
	lua_pushinteger(L, c.g);
	lua_pushinteger(L, c.b);
	lua_pushinteger(L, c.a);
	return 4;
}

int w_setFont(lua_State *L)
{
	Font *font = luax_checktype<Font>(L, 1, "Font", GRAPHICS_FONT_T);
	instance->setFont(font);
	return 0;
}

int w_getFont(lua_State *L)
{
	Font *f = instance->getFont();

	if (f == 0)
		return 0;

	f->retain();
	luax_newtype(L, "Font", GRAPHICS_FONT_T, (void *)f);
	return 1;
}

int w_setColorMask(lua_State *L)
{
	bool mask[4];
	for (int i = 0; i < 4; i++)
		mask[i] = luax_toboolean(L, i + 1);

	// r, g, b, a
	instance->setColorMask(mask[0], mask[1], mask[2], mask[3]);

	return 0;
}

int w_getColorMask(lua_State *L)
{
	const bool *mask = instance->getColorMask();

	for (int i = 0; i < 4; i++)
		luax_pushboolean(L, mask[i]);

	return 4;
}

int w_setBlendMode(lua_State *L)
{
	Graphics::BlendMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Invalid blend mode: %s", str);

	try
	{
		instance->setBlendMode(mode);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, e.what());
	}
	return 0;
}

int w_getBlendMode(lua_State *L)
{
	try
	{
		Graphics::BlendMode mode = instance->getBlendMode();
		const char *str;
		if (!Graphics::getConstant(mode, str))
			return luaL_error(L, "Unknown blend mode");
		lua_pushstring(L, str);
		return 1;
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
}

int w_setDefaultFilter(lua_State *L)
{
	Image::FilterMode min;
	Image::FilterMode mag;

	const char *minstr = luaL_checkstring(L, 1);
	const char *magstr = luaL_optstring(L, 2, minstr);

	if (!Image::getConstant(minstr, min))
		return luaL_error(L, "Invalid filter mode: %s", minstr);
	if (!Image::getConstant(magstr, mag))
		return luaL_error(L, "Invalid filter mode: %s", magstr);

	float anisotropy = (float) luaL_optnumber(L, 3, 1.0);

	Image::Filter f;
	f.min = min;
	f.mag = mag;
	f.anisotropy = anisotropy;

	instance->setDefaultFilter(f);
	
	return 0;
}

int w_getDefaultFilter(lua_State *L)
{
	const Image::Filter &f = instance->getDefaultFilter();
	const char *minstr;
	const char *magstr;
	if (!Image::getConstant(f.min, minstr))
		return luaL_error(L, "Unknown minification filter mode");
	if (!Image::getConstant(f.mag, magstr))
		return luaL_error(L, "Unknown magnification filter mode");
	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, f.anisotropy);
	return 3;
}

int w_setDefaultMipmapFilter(lua_State *L)
{
	Image::FilterMode filter = Image::FILTER_NONE;
	if (!lua_isnoneornil(L, 1))
	{
		const char *str = luaL_checkstring(L, 1);
		if (!Image::getConstant(str, filter))
			return luaL_error(L, "Invalid filter mode: %s", str);
	}

	float sharpness = luaL_optnumber(L, 2, 0);

	instance->setDefaultMipmapFilter(filter, sharpness);

	return 0;
}

int w_getDefaultMipmapFilter(lua_State *L)
{
	Image::FilterMode filter;
	float sharpness;

	instance->getDefaultMipmapFilter(&filter, &sharpness);

	const char *str;
	if (Image::getConstant(filter, str))
		lua_pushstring(L, str);
	else
		lua_pushnil(L);
	
	lua_pushnumber(L, sharpness);

	return 2;
}

int w_setLineWidth(lua_State *L)
{
	float width = (float)luaL_checknumber(L, 1);
	instance->setLineWidth(width);
	return 0;
}

int w_setLineStyle(lua_State *L)
{
	Graphics::LineStyle style;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, style))
		return luaL_error(L, "Invalid line style: %s", str);

	instance->setLineStyle(style);
	return 0;
}

int w_setLine(lua_State *L)
{
	float width = (float)luaL_checknumber(L, 1);

	Graphics::LineStyle style = Graphics::LINE_SMOOTH;

	if (lua_gettop(L) >= 2)
	{
		const char *str = luaL_checkstring(L, 2);
		if (!Graphics::getConstant(str, style))
			return luaL_error(L, "Invalid line style: %s", str);
	}

	instance->setLine(width, style);
	return 0;
}

int w_getLineWidth(lua_State *L)
{
	lua_pushnumber(L, instance->getLineWidth());
	return 1;
}

int w_getLineStyle(lua_State *L)
{
	Graphics::LineStyle style = instance->getLineStyle();
	const char *str;
	if (!Graphics::getConstant(style, str))
		return luaL_error(L, "Unknown line style");
	lua_pushstring(L, str);
	return 1;
}

int w_setPointSize(lua_State *L)
{
	float size = (float)luaL_checknumber(L, 1);
	instance->setPointSize(size);
	return 0;
}

int w_setPointStyle(lua_State *L)
{
	Graphics::PointStyle style;

	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, style))
		return luaL_error(L, "Invalid point style: %s", str);

	instance->setPointStyle(style);
	return 0;
}

int w_setPoint(lua_State *L)
{
	float size = (float)luaL_checknumber(L, 1);

	Graphics::PointStyle style = Graphics::POINT_SMOOTH;

	if (lua_gettop(L) >= 2)
	{
		const char *str = luaL_checkstring(L, 2);
		if (!Graphics::getConstant(str, style))
			return luaL_error(L, "Invalid point style: %s", str);
	}

	instance->setPoint(size, style);
	return 0;
}

int w_getPointSize(lua_State *L)
{
	lua_pushnumber(L, instance->getPointSize());
	return 1;
}

int w_getPointStyle(lua_State *L)
{
	Graphics::PointStyle style = instance->getPointStyle();
	const char *str;
	if (!Graphics::getConstant(style, str))
		return luaL_error(L, "Unknown point style");
	lua_pushstring(L, str);
	return 1;
}

int w_getMaxPointSize(lua_State *L)
{
	lua_pushnumber(L, instance->getMaxPointSize());
	return 1;
}

int w_newScreenshot(lua_State *L)
{
	love::image::Image *image = luax_getmodule<love::image::Image>(L, "image", MODULE_IMAGE_T);
	bool copyAlpha = luax_optboolean(L, 1, false);
	love::image::ImageData *i = instance->newScreenshot(image, copyAlpha);
	luax_newtype(L, "ImageData", IMAGE_IMAGE_DATA_T, (void *)i);
	return 1;
}

int w_setCanvas(lua_State *L)
{
	// discard stencil testing
	instance->discardStencil();

	// called with none -> reset to default buffer
	// nil is an error, to help people with typoes
	if (lua_isnone(L,1))
	{
		Canvas::bindDefaultCanvas();
		return 0;
	}

	Canvas *canvas = luax_checkcanvas(L, 1);

	try
	{
		// this unbinds the previous fbo
		canvas->startGrab();
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	return 0;
}

int w_setCanvases(lua_State *L)
{
	// discard stencil testing
	instance->discardStencil();

	// called with none -> reset to default buffer
	// nil is an error, to help people with typoes
	if (lua_isnone(L,1))
	{
		Canvas::bindDefaultCanvas();
		return 0;
	}

	bool is_table = lua_istable(L, 1);
	std::vector<Canvas *> attachments;

	Canvas *canvas = 0;

	if (is_table)
	{
		// grab the first canvas in the array and attach the rest
		lua_rawgeti(L, 1, 1);
		canvas = luax_checkcanvas(L, -1);
		lua_pop(L, 1);

		for (int i = 2; i <= lua_objlen(L, 1); i++)
		{
			lua_rawgeti(L, 1, i);
			attachments.push_back(luax_checkcanvas(L, -1));
			lua_pop(L, 1);
		}
	}
	else
	{
		canvas = luax_checkcanvas(L, 1);
		for (int i = 2; i <= lua_gettop(L); i++)
			attachments.push_back(luax_checkcanvas(L, i));
	}

	try
	{
		canvas->startGrab(attachments);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	return 0;
}

int w_getCanvas(lua_State *L)
{
	Canvas *canvas = Canvas::current;
	int n = 1;

	if (canvas)
	{
		canvas->retain();
		luax_newtype(L, "Canvas", GRAPHICS_CANVAS_T, (void *) canvas);

		const std::vector<Canvas *> &attachments = canvas->getAttachedCanvases();
		for (size_t i = 0; i < attachments.size(); i++)
		{
			attachments[i]->retain();
			luax_newtype(L, "Canvas", GRAPHICS_CANVAS_T, (void *) attachments[i]);
			n++;
		}
	}
	else
		lua_pushnil(L);

	return n;
}

int w_setShader(lua_State *L)
{
	if (lua_isnoneornil(L,1))
	{
		Shader::detach();
		return 0;
	}

	Shader *shader = luax_checkshader(L, 1);
	shader->attach();
	return 0;
}

int w_getShader(lua_State *L)
{
	Shader *shader = Shader::current;
	if (shader)
	{
		shader->retain();
		luax_newtype(L, "Shader", GRAPHICS_SHADER_T, (void *) shader);
	}
	else
		lua_pushnil(L);

	return 1;
}

int w_isSupported(lua_State *L)
{
	bool supported = true;
	size_t len = lua_gettop(L);
	Graphics::Support support;
	for (unsigned int i = 1; i <= len; i++)
	{
		const char *str = luaL_checkstring(L, i);
		if (!Graphics::getConstant(str, support))
		{
			supported = false;
			break;
		}
		switch (support)
		{
		case Graphics::SUPPORT_CANVAS:
			if (!Canvas::isSupported())
				supported = false;
			break;
		case Graphics::SUPPORT_HDR_CANVAS:
			if (!Canvas::isHDRSupported())
				supported = false;
			break;
		case Graphics::SUPPORT_MULTI_CANVAS:
			if (!Canvas::isMultiCanvasSupported())
				supported = false;
			break;
		case Graphics::SUPPORT_SHADER:
			if (!Shader::isSupported())
				supported = false;
			break;
		case Graphics::SUPPORT_NPOT:
			if (!Image::hasNpot())
				supported = false;
			break;
		case Graphics::SUPPORT_SUBTRACTIVE:
			if (!((GLEE_VERSION_1_4 || GLEE_ARB_imaging) || (GLEE_EXT_blend_minmax && GLEE_EXT_blend_subtract)))
				supported = false;
			break;
		case Graphics::SUPPORT_MIPMAP:
			if (!Image::hasMipmapSupport())
				supported = false;
			break;
		default:
			supported = false;
		}
		if (!supported)
			break;
	}
	lua_pushboolean(L, supported);
	return 1;
}

/**
 * Draws an Image at the specified coordinates, with rotation and
 * scaling along both axes.
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param angle The amount of rotation.
 * @param sx The scale factor along the x-axis. (1 = normal).
 * @param sy The scale factor along the y-axis. (1 = normal).
 * @param ox The offset along the x-axis.
 * @param oy The offset along the y-axis.
 * @param kx Shear along the x-axis.
 * @param ky Shear along the y-axis.
 **/
int w_draw(lua_State *L)
{
	Drawable *drawable = luax_checktype<Drawable>(L, 1, "Drawable", GRAPHICS_DRAWABLE_T);
	float x = (float)luaL_optnumber(L, 2, 0.0f);
	float y = (float)luaL_optnumber(L, 3, 0.0f);
	float angle = (float)luaL_optnumber(L, 4, 0.0f);
	float sx = (float)luaL_optnumber(L, 5, 1.0f);
	float sy = (float)luaL_optnumber(L, 6, sx);
	float ox = (float)luaL_optnumber(L, 7, 0);
	float oy = (float)luaL_optnumber(L, 8, 0);
	float kx = (float)luaL_optnumber(L, 9, 0);
	float ky = (float)luaL_optnumber(L, 10, 0);
	drawable->draw(x, y, angle, sx, sy, ox, oy, kx, ky);
	return 0;
}

/**
 * Draws an Quad of a DrawQable at the specified coordinates,
 * with rotation and scaling along both axes.
 *
 * @param q The Quad to draw.
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param angle The amount of rotation.
 * @param sx The scale factor along the x-axis. (1 = normal).
 * @param sy The scale factor along the y-axis. (1 = normal).
 * @param ox The offset along the x-axis.
 * @param oy The offset along the y-axis.
 * @param kx Shear along the x-axis.
 * @param ky Shear along the y-axis.
 **/
int w_drawq(lua_State *L)
{
	DrawQable *dq = luax_checktype<DrawQable>(L, 1, "DrawQable", GRAPHICS_DRAWQABLE_T);
	Quad *q = luax_checkquad(L, 2);
	float x = (float)luaL_optnumber(L, 3, 0.0f);
	float y = (float)luaL_optnumber(L, 4, 0.0f);
	float angle = (float)luaL_optnumber(L, 5, 0);
	float sx = (float)luaL_optnumber(L, 6, 1);
	float sy = (float)luaL_optnumber(L, 7, sx);
	float ox = (float)luaL_optnumber(L, 8, 0);
	float oy = (float)luaL_optnumber(L, 9, 0);
	float kx = (float)luaL_optnumber(L, 10, 0);
	float ky = (float)luaL_optnumber(L, 11, 0);
	dq->drawq(q, x, y, angle, sx, sy, ox, oy, kx, ky);
	return 0;
}

int w_print(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float angle = (float)luaL_optnumber(L, 4, 0.0f);
	float sx = (float)luaL_optnumber(L, 5, 1.0f);
	float sy = (float)luaL_optnumber(L, 6, sx);
	float ox = (float)luaL_optnumber(L, 7, 0.0f);
	float oy = (float)luaL_optnumber(L, 8, 0.0f);
	float kx = (float)luaL_optnumber(L, 9, 0.0f);
	float ky = (float)luaL_optnumber(L, 10, 0.0f);
	try
	{
		instance->print(str, x, y, angle, sx, sy, ox, oy, kx,ky);
	}
	catch(love::Exception e)
	{
		return luaL_error(L, "Decoding error: %s", e.what());
	}
	return 0;
}

int w_printf(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float wrap = (float)luaL_checknumber(L, 4);

	float angle = 0.0f;
	float sx = 1.0f, sy = 1.0f;
	float ox = 0.0f, oy = 0.0f;
	float kx = 0.0f, ky = 0.0f;

	Graphics::AlignMode align = Graphics::ALIGN_LEFT;

	if (lua_gettop(L) >= 5)
	{
		if (!lua_isnil(L, 5))
		{
			const char *str = luaL_checkstring(L, 5);
			if (!Graphics::getConstant(str, align))
				return luaL_error(L, "Incorrect alignment: %s", str);
		}

		angle = (float) luaL_optnumber(L, 6, 0.0f);
		sx = (float) luaL_optnumber(L, 7, 1.0f);
		sy = (float) luaL_optnumber(L, 8, sx);
		ox = (float) luaL_optnumber(L, 9, 0.0f);
		oy = (float) luaL_optnumber(L, 10, 0.0f);
		kx = (float) luaL_optnumber(L, 11, 0.0f);
		ky = (float) luaL_optnumber(L, 12, 0.0f);
	}

	try
	{
		instance->printf(str, x, y, wrap, align, angle, sx, sy, ox, oy, kx, ky);
	}
	catch(love::Exception e)
	{
		return luaL_error(L, "Decoding error: %s", e.what());
	}
	return 0;
}

int w_point(lua_State *L)
{
	float x = (float)luaL_checknumber(L, 1);
	float y = (float)luaL_checknumber(L, 2);
	instance->point(x, y);
	return 0;
}

int w_line(lua_State *L)
{
	int args = lua_gettop(L);
	bool is_table = false;
	if (args == 1 && lua_istable(L, 1))
	{
		args = lua_objlen(L, 1);
		is_table = true;
	}
	if (args % 2 != 0)
		return luaL_error(L, "Number of vertices must be a multiple of two");
	else if (args < 4)
		return luaL_error(L, "Need at least two vertices to draw a line");

	float *coords = new float[args];
	if (is_table)
	{
		for (int i = 0; i < args; ++i)
		{
			lua_rawgeti(L, 1, i + 1);
			coords[i] = luax_tofloat(L, -1);
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < args; ++i)
			coords[i] = luax_tofloat(L, i + 1);
	}

	instance->polyline(coords, args);

	delete[] coords;
	return 0;
}

int w_rectangle(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Incorrect draw mode %s", str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float w = (float)luaL_checknumber(L, 4);
	float h = (float)luaL_checknumber(L, 5);
	instance->rectangle(mode, x, y, w, h);
	return 0;
}

int w_circle(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Incorrect draw mode %s", str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float radius = (float)luaL_checknumber(L, 4);
	int points;
	if (lua_isnoneornil(L, 5))
		points = radius > 10 ? (int)(radius) : 10;
	else
		points = luaL_checkint(L, 5);

	instance->circle(mode, x, y, radius, points);
	return 0;
}

int w_arc(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Incorrect draw mode %s", str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float radius = (float)luaL_checknumber(L, 4);
	float angle1 = (float)luaL_checknumber(L, 5);
	float angle2 = (float)luaL_checknumber(L, 6);
	int points;
	if (lua_isnoneornil(L, 7))
		points = radius > 10 ? (int)(radius) : 10;
	else
		points = luaL_checkint(L, 7);

	instance->arc(mode, x, y, radius, angle1, angle2, points);
	return 0;
}

int w_polygon(lua_State *L)
{
	int args = lua_gettop(L) - 1;

	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Invalid draw mode: %s", str);

	bool is_table = false;
	float *coords;
	if (args == 1 && lua_istable(L, 2))
	{
		args = lua_objlen(L, 2);
		is_table = true;
	}

	if (args % 2 != 0)
		return luaL_error(L, "Number of vertices must be a multiple of two");
	else if (args < 6)
		return luaL_error(L, "Need at least three vertices to draw a polygon");

	// fetch coords
	coords = new float[args + 2];
	if (is_table)
	{
		for (int i = 0; i < args; ++i)
		{
			lua_rawgeti(L, 2, i + 1);
			coords[i] = luax_tofloat(L, -1);
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < args; ++i)
			coords[i] = luax_tofloat(L, i + 2);
	}

	// make a closed loop
	coords[args]   = coords[0];
	coords[args+1] = coords[1];
	instance->polygon(mode, coords, args+2);
	delete[] coords;

	return 0;
}

int w_push(lua_State *L)
{
	try
	{
		instance->push();
	}
	catch(love::Exception e)
	{
		return luaL_error(L, e.what());
	}
	return 0;
}

int w_pop(lua_State *L)
{
	try
	{
		instance->pop();
	}
	catch(love::Exception e)
	{
		return luaL_error(L, e.what());
	}
	return 0;
}

int w_rotate(lua_State *L)
{
	float deg = (float)luaL_checknumber(L, 1);
	instance->rotate(deg);
	return 0;
}

int w_scale(lua_State *L)
{
	float sx = (float)luaL_optnumber(L, 1, 1.0f);
	float sy = (float)luaL_optnumber(L, 2, sx);
	instance->scale(sx, sy);
	return 0;
}

int w_translate(lua_State *L)
{
	float x = (float)luaL_checknumber(L, 1);
	float y = (float)luaL_checknumber(L, 2);
	instance->translate(x, y);
	return 0;
}

int w_shear(lua_State *L)
{
	float kx = (float)luaL_checknumber(L, 1);
	float ky = (float)luaL_checknumber(L, 2);
	instance->shear(kx, ky);
	return 0;
}

int w_hasFocus(lua_State *L)
{
	luax_pushboolean(L, instance->hasFocus());
	return 1;
}


// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "checkMode", w_checkMode },
	{ "setMode", w_setMode },
	{ "getMode", w_getMode },
	{ "toggleFullscreen", w_toggleFullscreen },
	{ "reset", w_reset },
	{ "clear", w_clear },
	{ "present", w_present },

	{ "newImage", w_newImage },
	{ "newQuad", w_newQuad },
	{ "newFont", w_newFont },
	{ "newImageFont", w_newImageFont },
	{ "newSpriteBatch", w_newSpriteBatch },
	{ "newParticleSystem", w_newParticleSystem },
	{ "newCanvas", w_newCanvas },
	{ "newShader", w_newShader },

	{ "setColor", w_setColor },
	{ "getColor", w_getColor },
	{ "setBackgroundColor", w_setBackgroundColor },
	{ "getBackgroundColor", w_getBackgroundColor },

	{ "setFont", w_setFont },
	{ "getFont", w_getFont },

	{ "setColorMask", w_setColorMask },
	{ "getColorMask", w_getColorMask },
	{ "setBlendMode", w_setBlendMode },
	{ "getBlendMode", w_getBlendMode },
	{ "setDefaultFilter", w_setDefaultFilter },
	{ "getDefaultFilter", w_getDefaultFilter },
	{ "setDefaultMipmapFilter", w_setDefaultMipmapFilter },
	{ "getDefaultMipmapFilter", w_getDefaultMipmapFilter },
	{ "setLineWidth", w_setLineWidth },
	{ "setLineStyle", w_setLineStyle },
	{ "setLine", w_setLine },
	{ "getLineWidth", w_getLineWidth },
	{ "getLineStyle", w_getLineStyle },
	{ "setPointSize", w_setPointSize },
	{ "setPointStyle", w_setPointStyle },
	{ "setPoint", w_setPoint },
	{ "getPointSize", w_getPointSize },
	{ "getPointStyle", w_getPointStyle },
	{ "getMaxPointSize", w_getMaxPointSize },
	{ "newScreenshot", w_newScreenshot },
	{ "setCanvas", w_setCanvas },
	{ "setCanvases", w_setCanvases },
	{ "getCanvas", w_getCanvas },
	{ "getCanvases", w_getCanvas },

	{ "setShader", w_setShader },
	{ "getShader", w_getShader },

	{ "isSupported", w_isSupported },

	{ "draw", w_draw },
	{ "drawq", w_drawq },

	{ "print", w_print },
	{ "printf", w_printf },

	{ "setCaption", w_setCaption },
	{ "getCaption", w_getCaption },

	{ "setIcon", w_setIcon },

	{ "getWidth", w_getWidth },
	{ "getHeight", w_getHeight },
	{ "getDimensions", w_getDimensions },

	{ "isCreated", w_isCreated },

	{ "getModes", w_getModes },

	{ "setScissor", w_setScissor },
	{ "getScissor", w_getScissor },

	{ "newStencil", w_newStencil },
	{ "setStencil", w_setStencil },
	{ "setInvertedStencil", w_setInvertedStencil },

	{ "point", w_point },
	{ "line", w_line },
	{ "rectangle", w_rectangle },
	{ "circle", w_circle },
	{ "arc", w_arc },

	{ "polygon", w_polygon },

	{ "push", w_push },
	{ "pop", w_pop },
	{ "rotate", w_rotate },
	{ "scale", w_scale },
	{ "translate", w_translate },
	{ "shear", w_shear },

	{ "hasFocus", w_hasFocus },

	{ 0, 0 }
};

// Types for this module.
static const lua_CFunction types[] =
{
	luaopen_font,
	luaopen_image,
	luaopen_quad,
	luaopen_spritebatch,
	luaopen_particlesystem,
	luaopen_canvas,
	luaopen_shader,
	0
};

extern "C" int luaopen_love_graphics(lua_State *L)
{
	if (instance == 0)
	{
		try
		{
			instance = new Graphics();
		}
		catch (love::Exception &e)
		{
			return luaL_error(L, e.what());
		}
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "graphics";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	if (luaL_loadbuffer(L, (const char *)graphics_lua, sizeof(graphics_lua), "graphics.lua") == 0)
		lua_call(L, 0, 0);

	return n;
}

} // opengl
} // graphics
} // love
