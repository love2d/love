/**
* Copyright (c) 2006-2009 LOVE Development Team
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

namespace love
{
namespace graphics
{
namespace opengl
{
	static Graphics * instance = 0;

	int w_checkMode(lua_State * L)
	{
		int w = luaL_checkint(L, 1);
		int h = luaL_checkint(L, 2);
		bool fs = luax_toboolean(L, 3);
		luax_pushboolean(L, instance->checkMode(w, h, fs));
		return 1;
	}

	int w_setMode(lua_State * L)
	{
		int w = luaL_checkint(L, 1);
		int h = luaL_checkint(L, 2);
		bool fs = luax_optboolean(L, 3, false);
		bool vsync = luax_optboolean(L, 4, true);
		int fsaa = luaL_optint(L, 5, 0);
		luax_pushboolean(L, instance->setMode(w, h, fs, vsync, fsaa));
		return 1;
	}

	int w_toggleFullscreen(lua_State * L)
	{
		luax_pushboolean(L, instance->toggleFullscreen());
		return 1;
	}

	int w_reset(lua_State * L)
	{
		instance->reset();
		return 0;
	}

	int w_clear(lua_State * L)
	{
		instance->clear();
		return 0;
	}

	int w_present(lua_State * L)
	{
		instance->present();
		return 0;
	}

	int w_setCaption(lua_State * L)
	{
		const char * str = luaL_checkstring(L, 1);
		instance->setCaption(str);
		return 0;
	}

	int w_getCaption(lua_State * L)
	{
		return instance->getCaption(L);
	}

	int w_getWidth(lua_State * L)
	{
		lua_pushnumber(L, instance->getWidth());
		return 1;
	}

	int w_getHeight(lua_State * L)
	{
		lua_pushnumber(L, instance->getHeight());
		return 1;
	}

	int w_isCreated(lua_State * L)
	{
		luax_pushboolean(L, instance->isCreated());
		return 1;
	}

	int w_getModes(lua_State * L)
	{
		return instance->getModes(L);
	}

	int w_setScissor(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->setScissor();
			return 0;
		}
	
		int x = luaL_checkint(L, 1);
		int y = luaL_checkint(L, 2);
		int w = luaL_checkint(L, 3);
		int h = luaL_checkint(L, 4);

		instance->setScissor(x, y, w, h);
		return 0;
	}

	int w_getScissor(lua_State * L)
	{
		return instance->getScissor(L);
	}

	int w_newImage(lua_State * L)
	{
		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_convobj(L, 1, "filesystem", "newFile");

		// Convert to ImageData, if necessary.
		if(luax_istype(L, 1, FILESYSTEM_FILE_T))
			luax_convobj(L, 1, "image", "newImageData");

		love::image::ImageData * data = luax_checktype<love::image::ImageData>(L, 1, "ImageData", IMAGE_IMAGE_DATA_T);

		// Create the image.
		Image * image = instance->newImage(data);
			
		if(image == 0)
			return luaL_error(L, "Could not load image.");


		// Push the type.
		luax_newtype(L, "Image", GRAPHICS_IMAGE_T, (void*)image);

		return 1;
	}

	int w_newGlyph(lua_State * L)
	{
		love::font::GlyphData * data = luax_checktype<love::font::GlyphData>(L, 1, "GlyphData", FONT_GLYPH_DATA_T);

		// Create the image.
		Glyph * t = new Glyph(data);
		t->load();
			
		// Push the type.
		luax_newtype(L, "Glyph", GRAPHICS_GLYPH_T, (void*)t);

		return 1;
	}
	
	int w_newQuad(lua_State * L)
	{
		int x = luaL_checkint(L, 1);
		int y = luaL_checkint(L, 2);
		int w = luaL_checkint(L, 3);
		int h = luaL_checkint(L, 4);
		int sw = luaL_checkint(L, 5);
		int sh = luaL_checkint(L, 6);

		Quad * frame = instance->newQuad(x, y, w, h, sw, sh);

		if (frame == 0)
			return luaL_error(L, "Could not create frame.");
		
		luax_newtype(L, "Quad", GRAPHICS_QUAD_T, (void*)frame);
		return 1;
	}

	int w_newFont(lua_State * L)
	{
		Data * d = 0;

		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_convobj(L, 1, "filesystem", "newFile");

		if(luax_istype(L, 1, FILESYSTEM_FILE_T))
		{
			// Check the value.
			love::filesystem::File * file = luax_checktype<love::filesystem::File>(L, 1, "File", FILESYSTEM_FILE_T);
			d = file->read();
		}
		else if(luax_istype(L, 1, DATA_T))
		{
			d = luax_checktype<Data>(L, 1, "Data", DATA_T);
		} else { // This is not the type you're looking for.
			return luaL_error(L, "love.graphics.newFont() requires a string, File, or font data as argument #1");
		}

		// Second optional parameter can be a number:
		int size = luaL_optint(L, 2, 12);

		Font * font;
		try {
			font = instance->newFont(d, size);
		} catch (Exception & e) {
			return luaL_error(L, e.what());
		}

		if(font == 0)
			return luaL_error(L, "Could not load the font");
		
		luax_newtype(L, "Font", GRAPHICS_FONT_T, (void*)font);
		
		return 1;
	}

	int w_newImageFont(lua_State * L)
	{
		// Convert to File, if necessary.
		if(lua_isstring(L, 1))
			luax_convobj(L, 1, "filesystem", "newFile");

		// Convert to Image, if necessary.
		if(luax_istype(L, 1, FILESYSTEM_FILE_T))
			luax_convobj(L, 1, "graphics", "newImage");

		// Check the value.
		Image * image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);

		const char * glyphs = luaL_checkstring(L, 2);

		Font * font = instance->newImageFont(image, glyphs);

		if(font == 0)
			return luaL_error(L, "Could not load the font");

		luax_newtype(L, "Font", GRAPHICS_FONT_T, (void*)font);
		
		return 1;
	}

	int w_newSpriteBatch(lua_State * L)
	{
		Image * image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
		int size = luaL_optint(L, 2, 1000);
		int usage = luaL_optint(L, 3, SpriteBatch::USAGE_DYNAMIC);
		SpriteBatch * t = instance->newSpriteBatch(image, size, usage);
		luax_newtype(L, "SpriteBatch", GRAPHICS_SPRITE_BATCH_T, (void*)t);
		return 1;
	}

	int w_newParticleSystem(lua_State * L)
	{
		Image * image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
		int size = luaL_checkint(L, 2);
		ParticleSystem * t = instance->newParticleSystem(image, size);
		luax_newtype(L, "ParticleSystem", GRAPHICS_PARTICLE_SYSTEM_T, (void*)t);
		return 1;
	}

	int w_setColor(lua_State * L)
	{
		Color c;
		c.r = (unsigned char)luaL_checkint(L, 1);
		c.g = (unsigned char)luaL_checkint(L, 2);
		c.b = (unsigned char)luaL_checkint(L, 3);
		c.a = (unsigned char)luaL_optint(L, 4, 255);
		instance->setColor(c);
		return 0;
	}

	int w_getColor(lua_State * L)
	{
		Color c = instance->getColor();
		lua_pushinteger(L, c.r);
		lua_pushinteger(L, c.g);
		lua_pushinteger(L, c.b);
		lua_pushinteger(L, c.a);
		return 4;
	}

	int w_setBackgroundColor(lua_State * L)
	{
		Color c;
		c.r = (unsigned char)luaL_checkint(L, 1);
		c.g = (unsigned char)luaL_checkint(L, 2);
		c.b = (unsigned char)luaL_checkint(L, 3);
		c.a = 255;
		instance->setBackgroundColor(c);
		return 0;
	}

	int w_getBackgroundColor(lua_State * L)
	{
		Color c = instance->getBackgroundColor();
		lua_pushinteger(L, c.r);
		lua_pushinteger(L, c.g);
		lua_pushinteger(L, c.b);
		lua_pushinteger(L, c.a);
		return 4;
	}

	int w_setFont(lua_State * L)
	{
		// The second parameter is an optional int.
		int size = luaL_optint(L, 2, 12);

		// If the first parameter is a string, convert it to a file.
		if(lua_isstring(L, 1))
			luax_convobj(L, 1, "filesystem", "newFile");

		// If the first parameter is a File, use another setFont function.
		if(luax_istype(L, 1, FILESYSTEM_FILE_T))
		{
			love::filesystem::File * file = luax_checktype<love::filesystem::File>(L, 1, "File", FILESYSTEM_FILE_T);
			instance->setFont(file->read(), size);
			return 0;
		}
		else if(luax_istype(L, 1, DATA_T))
		{
			Data * data = luax_checktype<Data>(L, 1, "Data", DATA_T);
			instance->setFont(data, size);
			return 0;
		}

		Font * font = luax_checktype<Font>(L, 1, "Font", GRAPHICS_FONT_T);
		instance->setFont(font);
		return 0;
	}

	int w_getFont(lua_State * L)
	{
		Font * f = instance->getFont();

		if(f == 0)
			return 0;

		f->retain();
		luax_newtype(L, "Font", GRAPHICS_FONT_T, (void*)f);
		return 1;
	}

	int w_setBlendMode(lua_State * L)
	{
		int mode = luaL_checkint(L, 1);
		instance->setBlendMode(mode);
		return 0;
	}

	int w_setColorMode(lua_State * L)
	{
		int mode = luaL_checkint(L, 1);
		instance->setColorMode(mode);
		return 0;
	}

	int w_getBlendMode(lua_State * L)
	{
		lua_pushinteger(L, instance->getBlendMode());
		return 1;
	}

	int w_getColorMode(lua_State * L)
	{
		lua_pushinteger(L, instance->getColorMode());
		return 1;
	}

	int w_setLineWidth(lua_State * L)
	{
		float width = (float)luaL_checknumber(L, 1);
		instance->setLineWidth(width);
		return 0;
	}

	int w_setLineStyle(lua_State * L)
	{
		int style = luaL_checkint(L, 1);
		instance->setLineStyle(style);
		return 0;
	}

	int w_setLine(lua_State * L)
	{
		float width = (float)luaL_checknumber(L, 1);
		int style = luaL_optint(L, 2, Graphics::LINE_SMOOTH);
		instance->setLine(width, style);
		return 0;
	}

	int w_setLineStipple(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->setLineStipple();
			return 0;
		}

		unsigned short pattern = (unsigned short)luaL_checkint(L, 1);
		int repeat = luaL_optint(L, 2, 1);
		instance->setLineStipple(pattern, repeat);
		return 0;
	}

	int w_getLineWidth(lua_State * L)
	{
		lua_pushnumber(L, instance->getLineWidth());
		return 1;
	}

	int w_getLineStyle(lua_State * L)
	{
		lua_pushinteger(L, instance->getLineStyle());
		return 1;
	}

	int w_getLineStipple(lua_State * L)
	{
		return instance->getLineStipple(L);
	}

	int w_setPointSize(lua_State * L)
	{
		float size = (float)luaL_checknumber(L, 1);
		instance->setPointSize(size);
		return 0;
	}

	int w_setPointStyle(lua_State * L)
	{
		int style = luaL_checkint(L, 1);
		instance->setPointStyle(style);
		return 0;
	}

	int w_setPoint(lua_State * L)
	{
		float size = (float)luaL_checknumber(L, 1);
		int style = luaL_optint(L, 2, Graphics::POINT_SMOOTH);
		instance->setPoint(size, style);
		return 0;
	}

	int w_getPointSize(lua_State * L)
	{
		lua_pushnumber(L, instance->getPointSize());
		return 1;
	}

	int w_getPointStyle(lua_State * L)
	{
		lua_pushinteger(L, instance->getPointStyle());
		return 1;
	}

	int w_getMaxPointSize(lua_State * L)
	{
		lua_pushnumber(L, instance->getMaxPointSize());
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
	**/
	int w_draw(lua_State * L)
	{
		Drawable * drawable = luax_checktype<Drawable>(L, 1, "Drawable", GRAPHICS_DRAWABLE_T);
		float x = (float)luaL_optnumber(L, 2, 0.0f);
		float y = (float)luaL_optnumber(L, 3, 0.0f);
		float angle = (float)luaL_optnumber(L, 4, 0.0f);
		float sx = (float)luaL_optnumber(L, 5, 1.0f);
		float sy = (float)luaL_optnumber(L, 6, sx);
		float ox = (float)luaL_optnumber(L, 7, 0);
		float oy = (float)luaL_optnumber(L, 8, 0);
		drawable->draw(x, y, angle, sx, sy, ox, oy);
		return 0;
	}

	/**
	* Draws an Quad of an Image at the specified coordinates, 
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
	**/
	int w_drawq(lua_State * L)
	{
		Image * image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
		Quad * q = luax_checkframe(L, 2);
		float x = (float)luaL_checknumber(L, 3);
		float y = (float)luaL_checknumber(L, 4);
		float angle = (float)luaL_optnumber(L, 5, 0);
		float sx = (float)luaL_optnumber(L, 6, 1);
		float sy = (float)luaL_optnumber(L, 7, sx);
		float ox = (float)luaL_optnumber(L, 8, 0);
		float oy = (float)luaL_optnumber(L, 9, 0);
		image->drawq(q, x, y, angle, sx, sy, ox, oy);
		return 0;
	}

	int w_drawTest(lua_State * L)
	{
		Image * image = luax_checktype<Image>(L, 1, "Image", GRAPHICS_IMAGE_T);
		float x = (float)luaL_optnumber(L, 2, 0.0f);
		float y = (float)luaL_optnumber(L, 3, 0.0f);
		float angle = (float)luaL_optnumber(L, 4, 0.0f);
		float sx = (float)luaL_optnumber(L, 5, 1.0f);
		float sy = (float)luaL_optnumber(L, 6, sx);
		float ox = (float)luaL_optnumber(L, 7, 0);
		float oy = (float)luaL_optnumber(L, 8, 0);
		instance->drawTest(image, x, y, angle, sx, sy, ox, oy);
		return 0;
	}

	int w_print1(lua_State * L)
	{
		const char * str = luaL_checkstring(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		float angle = (float)luaL_optnumber(L, 4, 0.0f);
		float sx = (float)luaL_optnumber(L, 5, 1.0f);
		float sy = (float)luaL_optnumber(L, 6, sx);

		switch(lua_gettop(L))
		{
		case 3:
			instance->print(str, x, y);
			break;
		case 4:
			instance->print(str, x, y, angle);
			break;
		case 5:
			instance->print(str, x, y, angle, sx);
			break;
		case 6:
			instance->print(str, x, y, angle, sx, sy);
			break;
		default:
			return luaL_error(L, "Incorrect number of parameters");
		}
		return 0;
	}

	int w_printf1(lua_State * L)
	{
		const char * str = luaL_checkstring(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		float wrap = (float)luaL_checknumber(L, 4);
		int align = luaL_optint(L, 5, 0);
		instance->printf(str, x, y, wrap, align);
		return 0;
	}

	int w_point(lua_State * L)
	{
		float x = (float)luaL_checknumber(L, 1);
		float y = (float)luaL_checknumber(L, 2);
		instance->point(x, y);
		return 0;
	}

	int w_line(lua_State * L)
	{
		float x1 = (float)luaL_checknumber(L, 1);
		float y1 = (float)luaL_checknumber(L, 2);
		float x2 = (float)luaL_checknumber(L, 3);
		float y2 = (float)luaL_checknumber(L, 4);
		instance->line(x1, y1, x2, y2);
		return 0;
	}

	int w_triangle(lua_State * L)
	{
		int type = luaL_checkint(L, 1);
		float x1 = (float)luaL_checknumber(L, 2);
		float y1 = (float)luaL_checknumber(L, 3);
		float x2 = (float)luaL_checknumber(L, 4);
		float y2 = (float)luaL_checknumber(L, 5);
		float x3 = (float)luaL_checknumber(L, 6);
		float y3 = (float)luaL_checknumber(L, 7);
		instance->triangle(type, x1, y1, x2, y2, x3, y3);
		return 0;
	}

	int w_rectangle(lua_State * L)
	{
		int type = luaL_checkint(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		float w = (float)luaL_checknumber(L, 4);
		float h = (float)luaL_checknumber(L, 5);
		instance->rectangle(type, x, y, w, h);
		return 0;
	}

	int w_quad(lua_State * L)
	{
		int type = luaL_checkint(L, 1);
		float x1 = (float)luaL_checknumber(L, 2);
		float y1 = (float)luaL_checknumber(L, 3);
		float x2 = (float)luaL_checknumber(L, 4);
		float y2 = (float)luaL_checknumber(L, 5);
		float x3 = (float)luaL_checknumber(L, 6);
		float y3 = (float)luaL_checknumber(L, 7);
		float x4 = (float)luaL_checknumber(L, 6);
		float y4 = (float)luaL_checknumber(L, 7);
		instance->quad(type, x1, y1, x2, y2, x3, y3, x4, y4);
		return 0;
	}

	int w_circle(lua_State * L)
	{
		int type = luaL_checkint(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		float radius = (float)luaL_checknumber(L, 4);
		int points = luaL_optint(L, 5, 10);
		instance->circle(type, x, y, radius, points);
		return 0;
	}

	int w_polygon(lua_State * L)
	{
		return instance->polygon(L);
	}

	int w_push(lua_State * L)
	{
		instance->push();
		return 0;
	}

	int w_pop(lua_State * L)
	{
		instance->pop();
		return 0;
	}

	int w_rotate(lua_State * L)
	{
		float deg = (float)luaL_checknumber(L, 1);
		instance->rotate(deg);
		return 0;
	}

	int w_scale(lua_State * L)
	{
		float sx = (float)luaL_optnumber(L, 1, 1.0f);
		float sy = (float)luaL_optnumber(L, 2, sx);
		instance->scale(sx, sy);
		return 0;
	}

	int w_translate(lua_State * L)
	{
		float x = (float)luaL_checknumber(L, 1);
		float y = (float)luaL_checknumber(L, 2);
		instance->translate(x, y);
		return 0;
	}


	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "checkMode", w_checkMode },
		{ "setMode", w_setMode },
		{ "toggleFullscreen", w_toggleFullscreen },
		{ "reset", w_reset },
		{ "clear", w_clear },
		{ "present", w_present },

		{ "newImage", w_newImage },
		{ "newGlyph", w_newGlyph },
		{ "newQuad", w_newQuad },
		{ "newFont", w_newFont },
		{ "newImageFont", w_newImageFont },
		{ "newSpriteBatch", w_newSpriteBatch },
		{ "newParticleSystem", w_newParticleSystem },

		{ "setColor", w_setColor },
		{ "getColor", w_getColor },
		{ "setBackgroundColor", w_setBackgroundColor },
		{ "getBackgroundColor", w_getBackgroundColor },

		{ "setFont", w_setFont },
		{ "getFont", w_getFont },

		{ "setBlendMode", w_setBlendMode },
		{ "setColorMode", w_setColorMode },
		{ "getBlendMode", w_getBlendMode },
		{ "getColorMode", w_getColorMode },
		{ "setLineWidth", w_setLineWidth },
		{ "setLineStyle", w_setLineStyle },
		{ "setLine", w_setLine },
		{ "setLineStipple", w_setLineStipple },
		{ "getLineWidth", w_getLineWidth },
		{ "getLineStyle", w_getLineStyle },
		{ "getLineStipple", w_getLineStipple },
		{ "setPointSize", w_setPointSize },
		{ "setPointStyle", w_setPointStyle },
		{ "setPoint", w_setPoint },
		{ "getPointSize", w_getPointSize },
		{ "getPointStyle", w_getPointStyle },
		{ "getMaxPointSize", w_getMaxPointSize },

		{ "draw", w_draw },
		{ "drawq", w_drawq },
		{ "drawTest", w_drawTest },

		{ "print1", w_print1 },
		{ "printf1", w_printf1 },

		{ "setCaption", w_setCaption },
		{ "getCaption", w_getCaption },

		{ "getWidth", w_getWidth },
		{ "getHeight", w_getHeight },

		{ "isCreated", w_isCreated },

		{ "getModes", w_getModes },

		{ "setScissor", w_setScissor },
		{ "getScissor", w_getScissor },

		{ "point", w_point },
		{ "line", w_line },
		{ "triangle", w_triangle },
		{ "rectangle", w_rectangle },
		{ "quad", w_quad },
		{ "circle", w_circle },

		{ "polygon", w_polygon },

		{ "push", w_push },
		{ "pop", w_pop },
		{ "rotate", w_rotate },
		{ "scale", w_scale },

		{ "translate", w_translate },

		{ 0, 0 }
	};

	// Types for this module.
	static const lua_CFunction types[] = {
		luaopen_font, 
		luaopen_image, 
		luaopen_glyph,
		luaopen_frame, 
		luaopen_spritebatch,
		luaopen_particlesystem,
		0		
	};

	// List of constants.
	static const LuaConstant constants[] = {

		{ "align_left", Graphics::ALIGN_LEFT },
		{ "align_right", Graphics::ALIGN_RIGHT },
		{ "align_center", Graphics::ALIGN_CENTER },

		{ "blend_alpha", Graphics::BLEND_ALPHA },
		{ "blend_additive", Graphics::BLEND_ADDITIVE },
		{ "color_replace", Graphics::COLOR_REPLACE },
		{ "color_modulate", Graphics::COLOR_MODULATE },

		{ "draw_line", Graphics::DRAW_LINE },
		{ "draw_fill", Graphics::DRAW_FILL },

		{ "line_smooth", Graphics::LINE_SMOOTH },
		{ "line_rough", Graphics::LINE_ROUGH },

		{ "point_smooth", Graphics::POINT_SMOOTH },
		{ "point_rough", Graphics::POINT_ROUGH },

		{ "filter_linear", Image::FILTER_LINEAR },
		{ "filter_nearest", Image::FILTER_NEAREST },

		{ "wrap_clamp", Image::WRAP_CLAMP },
		{ "wrap_repeat", Image::WRAP_REPEAT },

		/**

		// Vertex buffer geometry types.

		{ "type_points", TYPE_POINTS },
		{ "type_lines", TYPE_LINES },
		{ "type_line_strip", TYPE_LINE_STRIP },
		{ "type_triangles", TYPE_TRIANGLES },
		{ "type_triangle_strip", TYPE_TRIANGLE_STRIP },
		{ "type_triangle_fan", TYPE_TRIANGLE_FAN },
		{ "type_num", TYPE_NUM },
		
		// Vertex buffer usage hints.

		{ "usage_array", USAGE_ARRAY },
		{ "usage_dynamic", USAGE_DYNAMIC },
		{ "usage_static", USAGE_STATIC },
		{ "usage_stream", USAGE_STREAM },
		{ "usage_num", USAGE_NUM },
		**/


		{ 0, 0 }
	};

	int luaopen_love_graphics(lua_State * L)
	{
		if(instance == 0)
		{
			try 
			{
				instance = new Graphics();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, instance);
		luax_register_module(L, functions, types, constants, "graphics");		

#		include <scripts/graphics.lua.h>

		return 0;
	}

} // opengl
} // graphics
} // love
