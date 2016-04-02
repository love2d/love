/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "graphics/Texture.h"
#include "image/ImageData.h"
#include "image/Image.h"
#include "font/Rasterizer.h"
#include "filesystem/wrap_Filesystem.h"
#include "video/VideoStream.h"
#include "image/wrap_Image.h"

#include <cassert>
#include <cstring>

#include <algorithm>

// Shove the wrap_Graphics.lua code directly into a raw string literal.
static const char graphics_lua[] =
#include "wrap_Graphics.lua"
;

namespace love
{
namespace graphics
{
namespace opengl
{

#define instance() (Module::getInstance<Graphics>(Module::M_GRAPHICS))

static int luax_checkgraphicscreated(lua_State *L)
{
	if (!instance()->isCreated())
		return luaL_error(L, "love.graphics cannot function without a window!");
	return 0;
}

int w_reset(lua_State *)
{
	instance()->reset();
	return 0;
}

int w_clear(lua_State *L)
{
	Colorf color;

	if (lua_isnoneornil(L, 1))
		color.set(0, 0, 0, 0);
	else if (lua_istable(L, 1))
	{
		std::vector<Graphics::OptionalColorf> colors((size_t) lua_gettop(L));

		for (int i = 0; i < lua_gettop(L); i++)
		{
			if (lua_isnoneornil(L, i + 1) || luax_objlen(L, i + 1) == 0)
			{
				colors[i].enabled = false;
				continue;
			}

			for (int j = 1; j <= 4; j++)
				lua_rawgeti(L, i + 1, j);

			colors[i].enabled = true;
			colors[i].r = (float) luaL_checknumber(L, -4);
			colors[i].g = (float) luaL_checknumber(L, -3);
			colors[i].b = (float) luaL_checknumber(L, -2);
			colors[i].a = (float) luaL_optnumber(L, -1, 255);

			lua_pop(L, 4);
		}

		luax_catchexcept(L, [&]() { instance()->clear(colors); });
		return 0;
	}
	else
	{
		color.r = (float) luaL_checknumber(L, 1);
		color.g = (float) luaL_checknumber(L, 2);
		color.b = (float) luaL_checknumber(L, 3);
		color.a = (float) luaL_optnumber(L, 4, 255);
	}

	luax_catchexcept(L, [&]() { instance()->clear(color); });
	return 0;
}

int w_discard(lua_State *L)
{
	std::vector<bool> colorbuffers;

	if (lua_istable(L, 1))
	{
		for (size_t i = 1; i <= luax_objlen(L, 1); i++)
		{
			lua_rawgeti(L, 1, i);
			colorbuffers.push_back(luax_optboolean(L, -1, true));
			lua_pop(L, 1);
		}
	}
	else
	{
		bool discardcolor = luax_optboolean(L, 1, true);
		size_t numbuffers = std::max((size_t) 1, instance()->getCanvas().size());
		colorbuffers = std::vector<bool>(numbuffers, discardcolor);
	}

	bool stencil = luax_optboolean(L, 2, true);
	instance()->discard(colorbuffers, stencil);
	return 0;
}

int w_present(lua_State *)
{
	instance()->present();
	return 0;
}

int w_isCreated(lua_State *L)
{
	luax_pushboolean(L, instance()->isCreated());
	return 1;
}

int w_isActive(lua_State *L)
{
	luax_pushboolean(L, instance()->isActive());
	return 1;
}

int w_isGammaCorrect(lua_State *L)
{
	luax_pushboolean(L, instance()->isGammaCorrect());
	return 1;
}

int w_getWidth(lua_State *L)
{
	lua_pushinteger(L, instance()->getWidth());
	return 1;
}

int w_getHeight(lua_State *L)
{
	lua_pushinteger(L, instance()->getHeight());
	return 1;
}

int w_getDimensions(lua_State *L)
{
	lua_pushinteger(L, instance()->getWidth());
	lua_pushinteger(L, instance()->getHeight());
	return 2;
}

int w_setScissor(lua_State *L)
{
	int nargs = lua_gettop(L);

	if (nargs == 0 || (nargs == 4 && lua_isnil(L, 1) && lua_isnil(L, 2)
		&& lua_isnil(L, 3) && lua_isnil(L, 4)))
	{
		instance()->setScissor();
		return 0;
	}

	int x = (int) luaL_checknumber(L, 1);
	int y = (int) luaL_checknumber(L, 2);
	int w = (int) luaL_checknumber(L, 3);
	int h = (int) luaL_checknumber(L, 4);

	if (w < 0 || h < 0)
		return luaL_error(L, "Can't set scissor with negative width and/or height.");

	instance()->setScissor(x, y, w, h);
	return 0;
}

int w_intersectScissor(lua_State *L)
{
	int x = (int) luaL_checknumber(L, 1);
	int y = (int) luaL_checknumber(L, 2);
	int w = (int) luaL_checknumber(L, 3);
	int h = (int) luaL_checknumber(L, 4);

	if (w < 0 || h < 0)
		return luaL_error(L, "Can't set scissor with negative width and/or height.");

	instance()->intersectScissor(x, y, w, h);
	return 0;
}

int w_getScissor(lua_State *L)
{
	int x, y, w, h;
	if (!instance()->getScissor(x, y, w, h))
		return 0;

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, w);
	lua_pushinteger(L, h);

	return 4;
}

int w_stencil(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);

	Graphics::StencilAction action = Graphics::STENCIL_REPLACE;

	if (!lua_isnoneornil(L, 2))
	{
		const char *actionstr = luaL_checkstring(L, 2);
		if (!Graphics::getConstant(actionstr, action))
			return luaL_error(L, "Invalid stencil draw action: %s", actionstr);
	}

	int stencilvalue = (int) luaL_optnumber(L, 3, 1);

	// Fourth argument: whether to keep the contents of the stencil buffer.
	if (lua_toboolean(L, 4) == 0)
		instance()->clearStencil();

	instance()->drawToStencilBuffer(action, stencilvalue);

	// Call stencilfunc()
	lua_pushvalue(L, 1);
	lua_call(L, 0, 0);

	instance()->stopDrawToStencilBuffer();
	return 0;
}

int w_setStencilTest(lua_State *L)
{
	// COMPARE_ALWAYS effectively disables stencil testing.
	Graphics::CompareMode compare = Graphics::COMPARE_ALWAYS;
	int comparevalue = 0;

	if (!lua_isnoneornil(L, 1))
	{
		const char *comparestr = luaL_checkstring(L, 1);
		if (!Graphics::getConstant(comparestr, compare))
			return luaL_error(L, "Invalid compare mode: %s", comparestr);

		comparevalue = (int) luaL_checknumber(L, 2);
	}

	instance()->setStencilTest(compare, comparevalue);
	return 0;
}

int w_getStencilTest(lua_State *L)
{
	Graphics::CompareMode compare = Graphics::COMPARE_ALWAYS;
	int comparevalue = 1;

	instance()->getStencilTest(compare, comparevalue);

	const char *comparestr;
	if (!Graphics::getConstant(compare, comparestr))
		return luaL_error(L, "Unknown compare mode.");

	lua_pushstring(L, comparestr);
	lua_pushnumber(L, comparevalue);
	return 2;
}

static const char *imageFlagName(Image::FlagType flagtype)
{
	const char *name = nullptr;
	Image::getConstant(flagtype, name);
	return name;
}

int w_newImage(lua_State *L)
{
	luax_checkgraphicscreated(L);

	std::vector<love::image::ImageData *> data;
	std::vector<love::image::CompressedImageData *> cdata;

	Image::Flags flags;
	if (!lua_isnoneornil(L, 2))
	{
		luaL_checktype(L, 2, LUA_TTABLE);
		flags.mipmaps = luax_boolflag(L, 2, imageFlagName(Image::FLAG_TYPE_MIPMAPS), flags.mipmaps);
		flags.linear = luax_boolflag(L, 2, imageFlagName(Image::FLAG_TYPE_LINEAR), flags.linear);
	}

	bool releasedata = false;

	// Convert to ImageData / CompressedImageData, if necessary.
	if (lua_isstring(L, 1) || luax_istype(L, 1, FILESYSTEM_FILE_ID) || luax_istype(L, 1, FILESYSTEM_FILE_DATA_ID))
	{
		auto imagemodule = Module::getInstance<love::image::Image>(Module::M_IMAGE);
		if (imagemodule == nullptr)
			return luaL_error(L, "Cannot load images without the love.image module.");

		love::filesystem::FileData *fdata = love::filesystem::luax_getfiledata(L, 1);

		if (imagemodule->isCompressed(fdata))
		{
			luax_catchexcept(L,
				[&]() { cdata.push_back(imagemodule->newCompressedData(fdata)); },
				[&](bool) { fdata->release(); }
			);
		}
		else
		{
			luax_catchexcept(L,
				[&]() { data.push_back(imagemodule->newImageData(fdata)); },
				[&](bool) { fdata->release(); }
			);
		}

		// Lua's GC won't release the image data, so we should do it ourselves.
		releasedata = true;
	}
	else if (luax_istype(L, 1, IMAGE_COMPRESSED_IMAGE_DATA_ID))
		cdata.push_back(love::image::luax_checkcompressedimagedata(L, 1));
	else
		data.push_back(love::image::luax_checkimagedata(L, 1));

	if (lua_istable(L, 2))
	{
		lua_getfield(L, 2, imageFlagName(Image::FLAG_TYPE_MIPMAPS));

		// Add all manually specified mipmap images to the array of imagedata.
		// i.e. flags = {mipmaps = {mip1, mip2, ...}}.
		if (lua_istable(L, -1))
		{
			for (size_t i = 1; i <= luax_objlen(L, -1); i++)
			{
				lua_rawgeti(L, -1, i);

				if (!data.empty())
				{
					if (!luax_istype(L, -1, IMAGE_IMAGE_DATA_ID))
						luax_convobj(L, -1, "image", "newImageData");

					data.push_back(love::image::luax_checkimagedata(L, -1));
				}
				else if (!cdata.empty())
				{
					if (!luax_istype(L, -1, IMAGE_COMPRESSED_IMAGE_DATA_ID))
						luax_convobj(L, -1, "image", "newCompressedData");

					cdata.push_back(love::image::luax_checkcompressedimagedata(L, -1));
				}

				lua_pop(L, 1);
			}
		}

		lua_pop(L, 1);
	}

	// Create the image.
	Image *image = nullptr;
	luax_catchexcept(L,
		[&]() {
			if (!cdata.empty())
				image = instance()->newImage(cdata, flags);
			else if (!data.empty())
				image = instance()->newImage(data, flags);
		},
		[&](bool) {
			if (releasedata)
			{
				for (auto d : data)
					d->release();
				for (auto d : cdata)
					d->release();
			}
		}
	);

	if (image == nullptr)
		return luaL_error(L, "Could not load image.");

	// Push the type.
	luax_pushtype(L, GRAPHICS_IMAGE_ID, image);
	image->release();
	return 1;
}

int w_newQuad(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Quad::Viewport v;
	v.x = luaL_checknumber(L, 1);
	v.y = luaL_checknumber(L, 2);
	v.w = luaL_checknumber(L, 3);
	v.h = luaL_checknumber(L, 4);

	double sw = luaL_checknumber(L, 5);
	double sh = luaL_checknumber(L, 6);

	Quad *quad = instance()->newQuad(v, sw, sh);
	luax_pushtype(L, GRAPHICS_QUAD_ID, quad);
	quad->release();
	return 1;
}

int w_newFont(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Font *font = nullptr;

	// Convert to Rasterizer, if necessary.
	if (!luax_istype(L, 1, FONT_RASTERIZER_ID))
	{
		std::vector<int> idxs;
		for (int i = 0; i < lua_gettop(L); i++)
			idxs.push_back(i + 1);

		luax_convobj(L, &idxs[0], (int) idxs.size(), "font", "newRasterizer");
	}

	love::font::Rasterizer *rasterizer = luax_checktype<love::font::Rasterizer>(L, 1, FONT_RASTERIZER_ID);

	luax_catchexcept(L, [&]() {
		font = instance()->newFont(rasterizer, instance()->getDefaultFilter()); }
	);

	// Push the type.
	luax_pushtype(L, GRAPHICS_FONT_ID, font);
	font->release();
	return 1;
}

int w_newImageFont(lua_State *L)
{
	luax_checkgraphicscreated(L);

	// filter for glyphs
	Texture::Filter filter = instance()->getDefaultFilter();

	// Convert to ImageData if necessary.
	if (luax_istype(L, 1, GRAPHICS_IMAGE_ID))
	{
		Image *i = luax_checktype<Image>(L, 1, GRAPHICS_IMAGE_ID);
		filter = i->getFilter();
		const auto &idlevels = i->getImageData();
		if (idlevels.empty())
			return luaL_argerror(L, 1, "Image must not be compressed.");
		luax_pushtype(L, IMAGE_IMAGE_DATA_ID, idlevels[0].get());
		lua_replace(L, 1);
	}

	// Convert to Rasterizer if necessary.
	if (!luax_istype(L, 1, FONT_RASTERIZER_ID))
	{
		luaL_checktype(L, 2, LUA_TSTRING);

		std::vector<int> idxs;
		for (int i = 0; i < lua_gettop(L); i++)
			idxs.push_back(i + 1);

		luax_convobj(L, &idxs[0], (int) idxs.size(), "font", "newImageRasterizer");
	}

	love::font::Rasterizer *rasterizer = luax_checktype<love::font::Rasterizer>(L, 1, FONT_RASTERIZER_ID);

	// Create the font.
	Font *font = instance()->newFont(rasterizer, filter);

	// Push the type.
	luax_pushtype(L, GRAPHICS_FONT_ID, font);
	font->release();
	return 1;
}

int w_newSpriteBatch(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture *texture = luax_checktexture(L, 1);
	int size = (int) luaL_optnumber(L, 2, 1000);
	Mesh::Usage usage = Mesh::USAGE_DYNAMIC;
	if (lua_gettop(L) > 2)
	{
		const char *usagestr = luaL_checkstring(L, 3);
		if (!Mesh::getConstant(usagestr, usage))
			return luaL_error(L, "Invalid SpriteBatch usage hint: %s", usagestr);
	}

	SpriteBatch *t = nullptr;
	luax_catchexcept(L,
		[&](){ t = instance()->newSpriteBatch(texture, size, usage); }
	);

	luax_pushtype(L, GRAPHICS_SPRITE_BATCH_ID, t);
	t->release();
	return 1;
}

int w_newParticleSystem(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture *texture = luax_checktexture(L, 1);
	lua_Number size = luaL_optnumber(L, 2, 1000);
	ParticleSystem *t = 0;
	if (size < 1.0 || size > ParticleSystem::MAX_PARTICLES)
		return luaL_error(L, "Invalid ParticleSystem size");

	luax_catchexcept(L,
		[&](){ t = instance()->newParticleSystem(texture, int(size)); }
	);

	luax_pushtype(L, GRAPHICS_PARTICLE_SYSTEM_ID, t);
	t->release();
	return 1;
}

int w_newCanvas(lua_State *L)
{
	luax_checkgraphicscreated(L);

	// check if width and height are given. else default to screen dimensions.
	int width       = (int) luaL_optnumber(L, 1, instance()->getWidth());
	int height      = (int) luaL_optnumber(L, 2, instance()->getHeight());
	const char *str = luaL_optstring(L, 3, "normal");
	int msaa        = (int) luaL_optnumber(L, 4, 0);

	Canvas::Format format;
	if (!Canvas::getConstant(str, format))
		return luaL_error(L, "Invalid Canvas format: %s", str);

	Canvas *canvas = nullptr;
	luax_catchexcept(L,
		[&](){ canvas = instance()->newCanvas(width, height, format, msaa); }
	);

	if (canvas == nullptr)
		return luaL_error(L, "Canvas not created, but no error thrown. I don't even...");

	luax_pushtype(L, GRAPHICS_CANVAS_ID, canvas);
	canvas->release();
	return 1;
}

int w_newShader(lua_State *L)
{
	luax_checkgraphicscreated(L);

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
		else
		{
			// Check if the argument looks like a filepath - we want a nicer
			// error for misspelled filepath arguments.
			size_t slen = 0;
			const char *str = lua_tolstring(L, i, &slen);
			if (slen > 0 && slen < 256 && !strchr(str, '\n'))
			{
				const char *ext = strchr(str, '.');
				if (ext != nullptr && !strchr(ext, ';') && !strchr(ext, ' '))
					return luaL_error(L, "Could not open file %s. Does not exist.", str);
			}
		}
	}

	bool has_arg1 = lua_isstring(L, 1) != 0;
	bool has_arg2 = lua_isstring(L, 2) != 0;

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

	Shader::ShaderSource source;

	// vertex shader code
	if (lua_isstring(L, -2))
		source.vertex = luax_checkstring(L, -2);
	else if (has_arg1 && has_arg2)
		return luaL_error(L, "Could not parse vertex shader code (missing 'position' function?)");

	// pixel shader code
	if (lua_isstring(L, -1))
		source.pixel = luax_checkstring(L, -1);
	else if (has_arg1 && has_arg2)
		return luaL_error(L, "Could not parse pixel shader code (missing 'effect' function?)");

	if (source.vertex.empty() && source.pixel.empty())
	{
		// Original args had source code, but effectCodeToGLSL couldn't translate it
		for (int i = 1; i <= 2; i++)
		{
			if (lua_isstring(L, i))
				return luaL_argerror(L, i, "missing 'position' or 'effect' function?");
		}
	}

	bool should_error = false;
	try
	{
		Shader *shader = instance()->newShader(source);
		luax_pushtype(L, GRAPHICS_SHADER_ID, shader);
		shader->release();
	}
	catch (love::Exception &e)
	{
		luax_getfunction(L, "graphics", "_transformGLSLErrorMessages");
		lua_pushstring(L, e.what());

		// Function pushes the new error string onto the stack.
		lua_pcall(L, 1, 1, 0);
		should_error = true;
	}

	if (should_error)
		return lua_error(L);

	return 1;
}

static Mesh::Usage luax_optmeshusage(lua_State *L, int idx, Mesh::Usage def)
{
	const char *usagestr = lua_isnoneornil(L, idx) ? nullptr : luaL_checkstring(L, idx);

	if (usagestr && !Mesh::getConstant(usagestr, def))
		luaL_error(L, "Invalid mesh usage hint: %s", usagestr);

	return def;
}

static Mesh::DrawMode luax_optmeshdrawmode(lua_State *L, int idx, Mesh::DrawMode def)
{
	const char *modestr = lua_isnoneornil(L, idx) ? nullptr : luaL_checkstring(L, idx);

	if (modestr && !Mesh::getConstant(modestr, def))
		luaL_error(L, "Invalid mesh draw mode: %s", modestr);

	return def;
}

static Mesh *newStandardMesh(lua_State *L)
{
	Mesh *t = nullptr;

	Mesh::DrawMode drawmode = luax_optmeshdrawmode(L, 2, Mesh::DRAWMODE_FAN);
	Mesh::Usage usage = luax_optmeshusage(L, 3, Mesh::USAGE_DYNAMIC);

	// First argument is a table of standard vertices, or the number of
	// standard vertices.
	if (lua_istable(L, 1))
	{
		size_t vertexcount = luax_objlen(L, 1);
		std::vector<Vertex> vertices;
		vertices.reserve(vertexcount);

		// Get the vertices from the table.
		for (size_t i = 1; i <= vertexcount; i++)
		{
			lua_rawgeti(L, 1, (int) i);

			if (lua_type(L, -1) != LUA_TTABLE)
			{
				luax_typerror(L, 1, "table of tables");
				return nullptr;
			}

			for (int j = 1; j <= 8; j++)
				lua_rawgeti(L, -j, j);

			Vertex v;

			v.x = (float) luaL_checknumber(L, -8);
			v.y = (float) luaL_checknumber(L, -7);
			v.s = (float) luaL_optnumber(L, -6, 0.0);
			v.t = (float) luaL_optnumber(L, -5, 0.0);

			v.r = (unsigned char) luaL_optnumber(L, -4, 255);
			v.g = (unsigned char) luaL_optnumber(L, -3, 255);
			v.b = (unsigned char) luaL_optnumber(L, -2, 255);
			v.a = (unsigned char) luaL_optnumber(L, -1, 255);

			lua_pop(L, 9);
			vertices.push_back(v);
		}

		luax_catchexcept(L, [&](){ t = instance()->newMesh(vertices, drawmode, usage); });
	}
	else
	{
		int count = (int) luaL_checknumber(L, 1);
		luax_catchexcept(L, [&](){ t = instance()->newMesh(count, drawmode, usage); });
	}

	return t;
}

static Mesh *newCustomMesh(lua_State *L)
{
	Mesh *t = nullptr;

	// First argument is the vertex format, second is a table of vertices or
	// the number of vertices.
	std::vector<Mesh::AttribFormat> vertexformat;

	Mesh::DrawMode drawmode = luax_optmeshdrawmode(L, 3, Mesh::DRAWMODE_FAN);
	Mesh::Usage usage = luax_optmeshusage(L, 4, Mesh::USAGE_DYNAMIC);

	lua_rawgeti(L, 1, 1);
	if (!lua_istable(L, -1))
	{
		luaL_argerror(L, 1, "table of tables expected");
		return nullptr;
	}
	lua_pop(L, 1);

	// Per-vertex attribute formats.
	for (int i = 1; i <= (int) luax_objlen(L, 1); i++)
	{
		lua_rawgeti(L, 1, i);

		// {name, datatype, components}
		for (int j = 1; j <= 3; j++)
			lua_rawgeti(L, -j, j);

		Mesh::AttribFormat format;
		format.name = luaL_checkstring(L, -3);

		const char *tname = luaL_checkstring(L, -2);
		if (!Mesh::getConstant(tname, format.type))
		{
			luaL_error(L, "Invalid Mesh vertex data type name: %s", tname);
			return nullptr;
		}

		format.components = (int) luaL_checknumber(L, -1);
		if (format.components <= 0 || format.components > 4)
		{
			luaL_error(L, "Number of vertex attribute components must be between 1 and 4 (got %d)", format.components);
			return nullptr;
		}

		lua_pop(L, 4);
		vertexformat.push_back(format);
	}

	if (lua_isnumber(L, 2))
	{
		int vertexcount = (int) luaL_checknumber(L, 2);
		luax_catchexcept(L, [&](){ t = instance()->newMesh(vertexformat, vertexcount, drawmode, usage); });
	}
	else if (luax_istype(L, 2, DATA_ID))
	{
		// Vertex data comes directly from a Data object.
		Data *data = luax_checktype<Data>(L, 2, DATA_ID);
		luax_catchexcept(L, [&](){ t = instance()->newMesh(vertexformat, data->getData(), data->getSize(), drawmode, usage); });
	}
	else
	{
		// Table of vertices.
		lua_rawgeti(L, 2, 1);
		if (!lua_istable(L, -1))
		{
			luaL_argerror(L, 2, "expected table of tables");
			return nullptr;
		}
		lua_pop(L, 1);

		int vertexcomponents = 0;
		for (const Mesh::AttribFormat &format : vertexformat)
			vertexcomponents += format.components;

		size_t numvertices = luax_objlen(L, 2);

		luax_catchexcept(L, [&](){ t = instance()->newMesh(vertexformat, numvertices, drawmode, usage); });

		// Maximum possible data size for a single vertex attribute.
		char data[sizeof(float) * 4];

		for (size_t vertindex = 0; vertindex < numvertices; vertindex++)
		{
			// get vertices[vertindex]
			lua_rawgeti(L, 2, vertindex + 1);
			luaL_checktype(L, -1, LUA_TTABLE);

			int n = 0;
			for (size_t i = 0; i < vertexformat.size(); i++)
			{
				int components = vertexformat[i].components;

				// get vertices[vertindex][n]
				for (int c = 0; c < components; c++)
				{
					n++;
					lua_rawgeti(L, -(c + 1), n);
				}

				// Fetch the values from Lua and store them in data buffer.
				luax_writeAttributeData(L, -components, vertexformat[i].type, components, data);

				lua_pop(L, components);

				luax_catchexcept(L,
					[&](){ t->setVertexAttribute(vertindex, i, data, sizeof(float) * 4); },
					[&](bool diderror){ if (diderror) t->release(); }
				);
			}

			lua_pop(L, 1); // pop vertices[vertindex]
		}

		t->flush();
	}

	return t;
}

int w_newMesh(lua_State *L)
{
	luax_checkgraphicscreated(L);

	// Check first argument: table or number of vertices.
	int arg1type = lua_type(L, 1);
	if (arg1type != LUA_TTABLE && arg1type != LUA_TNUMBER)
		luaL_argerror(L, 1, "table or number expected");

	Mesh *t = nullptr;

	int arg2type = lua_type(L, 2);
	if (arg1type == LUA_TTABLE && (arg2type == LUA_TTABLE || arg2type == LUA_TNUMBER || arg2type == LUA_TUSERDATA))
		t = newCustomMesh(L);
	else
		t = newStandardMesh(L);

	luax_pushtype(L, GRAPHICS_MESH_ID, t);
	t->release();
	return 1;
}

int w_newText(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Font *font = luax_checkfont(L, 1);
	Text *t = nullptr;

	if (lua_isnoneornil(L, 2))
		luax_catchexcept(L, [&](){ t = instance()->newText(font); });
	else
	{
		std::vector<Font::ColoredString> text;
		luax_checkcoloredstring(L, 2, text);

		luax_catchexcept(L, [&](){ t = instance()->newText(font, text); });
	}

	luax_pushtype(L, GRAPHICS_TEXT_ID, t);
	t->release();
	return 1;
}

int w_newVideo(lua_State *L)
{
	luax_checkgraphicscreated(L);

	if (!luax_istype(L, 1, VIDEO_VIDEO_STREAM_ID))
		luax_convobj(L, 1, "video", "newVideoStream");

	auto stream = luax_checktype<love::video::VideoStream>(L, 1, VIDEO_VIDEO_STREAM_ID);
	Video *video = nullptr;

	luax_catchexcept(L, [&]() { video = instance()->newVideo(stream); });
	luax_pushtype(L, GRAPHICS_VIDEO_ID, video);
	video->release();
	return 1;
}

int w_setColor(lua_State *L)
{
	Colorf c;
	if (lua_istable(L, 1))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 1, i);

		c.r = (float) luaL_checknumber(L, -4);
		c.g = (float) luaL_checknumber(L, -3);
		c.b = (float) luaL_checknumber(L, -2);
		c.a = (float) luaL_optnumber(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (float) luaL_checknumber(L, 1);
		c.g = (float) luaL_checknumber(L, 2);
		c.b = (float) luaL_checknumber(L, 3);
		c.a = (float) luaL_optnumber(L, 4, 255);
	}
	instance()->setColor(c);
	return 0;
}

int w_getColor(lua_State *L)
{
	Colorf c = instance()->getColor();
	lua_pushnumber(L, c.r);
	lua_pushnumber(L, c.g);
	lua_pushnumber(L, c.b);
	lua_pushnumber(L, c.a);
	return 4;
}

int w_setBackgroundColor(lua_State *L)
{
	Colorf c;
	if (lua_istable(L, 1))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 1, i);

		c.r = (float) luaL_checknumber(L, -4);
		c.g = (float) luaL_checknumber(L, -3);
		c.b = (float) luaL_checknumber(L, -2);
		c.a = (float) luaL_optnumber(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (float) luaL_checknumber(L, 1);
		c.g = (float) luaL_checknumber(L, 2);
		c.b = (float) luaL_checknumber(L, 3);
		c.a = (float) luaL_optnumber(L, 4, 255);
	}
	instance()->setBackgroundColor(c);
	return 0;
}

int w_getBackgroundColor(lua_State *L)
{
	Colorf c = instance()->getBackgroundColor();
	lua_pushnumber(L, c.r);
	lua_pushnumber(L, c.g);
	lua_pushnumber(L, c.b);
	lua_pushnumber(L, c.a);
	return 4;
}

int w_setNewFont(lua_State *L)
{
	int ret = w_newFont(L);
	Font *font = luax_checktype<Font>(L, -1, GRAPHICS_FONT_ID);
	instance()->setFont(font);
	return ret;
}

int w_setFont(lua_State *L)
{
	Font *font = luax_checktype<Font>(L, 1, GRAPHICS_FONT_ID);
	instance()->setFont(font);
	return 0;
}

int w_getFont(lua_State *L)
{
	Font *f = nullptr;
	luax_catchexcept(L, [&](){ f = instance()->getFont(); });

	luax_pushtype(L, GRAPHICS_FONT_ID, f);
	return 1;
}

int w_setColorMask(lua_State *L)
{
	Graphics::ColorMask mask;

	if (lua_gettop(L) <= 1 && lua_isnoneornil(L, 1))
	{
		// Enable all color components if no argument is given.
		mask.r = mask.g = mask.b = mask.a = true;
	}
	else
	{
		mask.r = luax_toboolean(L, 1);
		mask.g = luax_toboolean(L, 2);
		mask.b = luax_toboolean(L, 3);
		mask.a = luax_toboolean(L, 4);
	}

	instance()->setColorMask(mask);

	return 0;
}

int w_getColorMask(lua_State *L)
{
	Graphics::ColorMask mask = instance()->getColorMask();

	luax_pushboolean(L, mask.r);
	luax_pushboolean(L, mask.g);
	luax_pushboolean(L, mask.b);
	luax_pushboolean(L, mask.a);

	return 4;
}

int w_setBlendMode(lua_State *L)
{
	Graphics::BlendMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Invalid blend mode: %s", str);

	Graphics::BlendAlpha alphamode = Graphics::BLENDALPHA_MULTIPLY;
	if (!lua_isnoneornil(L, 2))
	{
		const char *alphastr = luaL_checkstring(L, 2);
		if (!Graphics::getConstant(alphastr, alphamode))
			return luaL_error(L, "Invalid blend alpha mode: %s", alphastr);
	}

	luax_catchexcept(L, [&](){ instance()->setBlendMode(mode, alphamode); });
	return 0;
}

int w_getBlendMode(lua_State *L)
{
	const char *str;
	const char *alphastr;

	Graphics::BlendAlpha alphamode;
	Graphics::BlendMode mode = instance()->getBlendMode(alphamode);

	if (!Graphics::getConstant(mode, str))
		return luaL_error(L, "Unknown blend mode");

	if (!Graphics::getConstant(alphamode, alphastr))
		return luaL_error(L, "Unknown blend alpha mode");

	lua_pushstring(L, str);
	lua_pushstring(L, alphastr);
	return 2;
}

int w_setDefaultFilter(lua_State *L)
{
	Texture::Filter f;

	const char *minstr = luaL_checkstring(L, 1);
	const char *magstr = luaL_optstring(L, 2, minstr);

	if (!Texture::getConstant(minstr, f.min))
		return luaL_error(L, "Invalid filter mode: %s", minstr);
	if (!Texture::getConstant(magstr, f.mag))
		return luaL_error(L, "Invalid filter mode: %s", magstr);

	f.anisotropy = (float) luaL_optnumber(L, 3, 1.0);

	instance()->setDefaultFilter(f);

	return 0;
}

int w_getDefaultFilter(lua_State *L)
{
	const Texture::Filter &f = instance()->getDefaultFilter();
	const char *minstr;
	const char *magstr;
	if (!Texture::getConstant(f.min, minstr))
		return luaL_error(L, "Unknown minification filter mode");
	if (!Texture::getConstant(f.mag, magstr))
		return luaL_error(L, "Unknown magnification filter mode");
	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, f.anisotropy);
	return 3;
}

int w_setDefaultMipmapFilter(lua_State *L)
{
	Texture::FilterMode filter = Texture::FILTER_NONE;
	if (!lua_isnoneornil(L, 1))
	{
		const char *str = luaL_checkstring(L, 1);
		if (!Texture::getConstant(str, filter))
			return luaL_error(L, "Invalid filter mode: %s", str);
	}

	float sharpness = (float) luaL_optnumber(L, 2, 0);

	instance()->setDefaultMipmapFilter(filter, sharpness);

	return 0;
}

int w_getDefaultMipmapFilter(lua_State *L)
{
	Texture::FilterMode filter;
	float sharpness;

	instance()->getDefaultMipmapFilter(&filter, &sharpness);

	const char *str;
	if (Texture::getConstant(filter, str))
		lua_pushstring(L, str);
	else
		lua_pushnil(L);

	lua_pushnumber(L, sharpness);

	return 2;
}

int w_setLineWidth(lua_State *L)
{
	float width = (float)luaL_checknumber(L, 1);
	instance()->setLineWidth(width);
	return 0;
}

int w_setLineStyle(lua_State *L)
{
	Graphics::LineStyle style;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, style))
		return luaL_error(L, "Invalid line style: %s", str);

	instance()->setLineStyle(style);
	return 0;
}

int w_setLineJoin(lua_State *L)
{
	Graphics::LineJoin join;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, join))
		return luaL_error(L, "Invalid line join mode: %s", str);

	instance()->setLineJoin(join);
	return 0;
}

int w_getLineWidth(lua_State *L)
{
	lua_pushnumber(L, instance()->getLineWidth());
	return 1;
}

int w_getLineStyle(lua_State *L)
{
	Graphics::LineStyle style = instance()->getLineStyle();
	const char *str;
	if (!Graphics::getConstant(style, str))
		return luaL_error(L, "Unknown line style");
	lua_pushstring(L, str);
	return 1;
}

int w_getLineJoin(lua_State *L)
{
	Graphics::LineJoin join = instance()->getLineJoin();
	const char *str;
	if (!Graphics::getConstant(join, str))
		return luaL_error(L, "Unknown line join");
	lua_pushstring(L, str);
	return 1;
}

int w_setPointSize(lua_State *L)
{
	float size = (float)luaL_checknumber(L, 1);
	instance()->setPointSize(size);
	return 0;
}

int w_getPointSize(lua_State *L)
{
	lua_pushnumber(L, instance()->getPointSize());
	return 1;
}

int w_setWireframe(lua_State *L)
{
	instance()->setWireframe(luax_toboolean(L, 1));
	return 0;
}

int w_isWireframe(lua_State *L)
{
	luax_pushboolean(L, instance()->isWireframe());
	return 1;
}

int w_newScreenshot(lua_State *L)
{
	love::image::Image *image = luax_getmodule<love::image::Image>(L, MODULE_IMAGE_ID);
	bool copyAlpha = luax_optboolean(L, 1, false);
	love::image::ImageData *i = 0;

	luax_catchexcept(L, [&](){ i = instance()->newScreenshot(image, copyAlpha); });

	luax_pushtype(L, IMAGE_IMAGE_DATA_ID, i);
	i->release();
	return 1;
}

int w_setCanvas(lua_State *L)
{
	// Disable stencil writes.
	instance()->stopDrawToStencilBuffer();

	// called with none -> reset to default buffer
	if (lua_isnoneornil(L, 1))
	{
		instance()->setCanvas();
		return 0;
	}

	bool is_table = lua_istable(L, 1);
	std::vector<Canvas *> canvases;

	if (is_table)
	{
		for (int i = 1; i <= (int) luax_objlen(L, 1); i++)
		{
			lua_rawgeti(L, 1, i);
			canvases.push_back(luax_checkcanvas(L, -1));
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 1; i <= lua_gettop(L); i++)
			canvases.push_back(luax_checkcanvas(L, i));
	}

	luax_catchexcept(L, [&]() {
		if (canvases.size() > 0)
			instance()->setCanvas(canvases);
		else
			instance()->setCanvas();
	});

	return 0;
}

int w_getCanvas(lua_State *L)
{
	const std::vector<Canvas *> canvases = instance()->getCanvas();
	int n = 0;

	for (Canvas *c : canvases)
	{
		luax_pushtype(L, GRAPHICS_CANVAS_ID, c);
		n++;
	}

	if (n == 0)
	{
		lua_pushnil(L);
		n = 1;
	}

	return n;
}

int w_setShader(lua_State *L)
{
	if (lua_isnoneornil(L,1))
	{
		instance()->setShader();
		return 0;
	}

	Shader *shader = luax_checkshader(L, 1);
	instance()->setShader(shader);
	return 0;
}

int w_getShader(lua_State *L)
{
	Shader *shader = instance()->getShader();
	if (shader)
		luax_pushtype(L, GRAPHICS_SHADER_ID, shader);
	else
		lua_pushnil(L);

	return 1;
}

int w_setDefaultShaderCode(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TTABLE);

	for (int i = 0; i < 2; i++)
	{
		for (int renderer = 0; renderer < Graphics::RENDERER_MAX_ENUM; renderer++)
		{
			const char *lang = renderer == Graphics::RENDERER_OPENGLES ? "glsles" : "glsl";

			lua_getfield(L, i + 1, lang);

			lua_getfield(L, -1, "vertex");
			lua_getfield(L, -2, "pixel");
			lua_getfield(L, -3, "videopixel");

			Shader::ShaderSource code;
			code.vertex = luax_checkstring(L, -3);
			code.pixel = luax_checkstring(L, -2);

			Shader::ShaderSource videocode;
			videocode.vertex = luax_checkstring(L, -3);
			videocode.pixel = luax_checkstring(L, -1);

			lua_pop(L, 4);

			Shader::defaultCode[renderer][i] = code;
			Shader::defaultVideoCode[renderer][i] = videocode;
		}
	}

	return 0;
}

int w_getSupported(lua_State *L)
{
	lua_createtable(L, 0, (int) Graphics::FEATURE_MAX_ENUM);

	for (int i = 0; i < (int) Graphics::FEATURE_MAX_ENUM; i++)
	{
		auto feature = (Graphics::Feature) i;
		const char *name = nullptr;

		if (!Graphics::getConstant(feature, name))
			continue;

		luax_pushboolean(L, instance()->isSupported(feature));
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getCanvasFormats(lua_State *L)
{
	lua_createtable(L, 0, (int) Canvas::FORMAT_MAX_ENUM);

	for (int i = 0; i < (int) Canvas::FORMAT_MAX_ENUM; i++)
	{
		Canvas::Format format = (Canvas::Format) i;
		const char *name = nullptr;

		if (!Canvas::getConstant(format, name))
			continue;

		luax_pushboolean(L, Canvas::isFormatSupported(format));
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getCompressedImageFormats(lua_State *L)
{
	lua_createtable(L, 0, (int) image::CompressedImageData::FORMAT_MAX_ENUM);

	for (int i = 0; i < (int) image::CompressedImageData::FORMAT_MAX_ENUM; i++)
	{
		image::CompressedImageData::Format format = (image::CompressedImageData::Format) i;
		const char *name = nullptr;

		if (format == image::CompressedImageData::FORMAT_UNKNOWN)
			continue;

		if (!image::CompressedImageData::getConstant(format, name))
			continue;

		luax_pushboolean(L, Image::hasCompressedTextureSupport(format, false));
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getRendererInfo(lua_State *L)
{
	Graphics::RendererInfo info;
	luax_catchexcept(L, [&](){ info = instance()->getRendererInfo(); });

	luax_pushstring(L, info.name);
	luax_pushstring(L, info.version);
	luax_pushstring(L, info.vendor);
	luax_pushstring(L, info.device);
	return 4;
}

int w_getSystemLimits(lua_State *L)
{
	lua_createtable(L, 0, (int) Graphics::LIMIT_MAX_ENUM);

	for (int i = 0; i < (int) Graphics::LIMIT_MAX_ENUM; i++)
	{
		Graphics::SystemLimit limittype = (Graphics::SystemLimit) i;
		const char *name = nullptr;

		if (!Graphics::getConstant(limittype, name))
			continue;

		lua_pushnumber(L, instance()->getSystemLimit(limittype));
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getStats(lua_State *L)
{
	Graphics::Stats stats = instance()->getStats();

	lua_createtable(L, 0, 7);

	lua_pushinteger(L, stats.drawCalls);
	lua_setfield(L, -2, "drawcalls");

	lua_pushinteger(L, stats.canvasSwitches);
	lua_setfield(L, -2, "canvasswitches");

	lua_pushinteger(L, stats.shaderSwitches);
	lua_setfield(L, -2, "shaderswitches");

	lua_pushinteger(L, stats.canvases);
	lua_setfield(L, -2, "canvases");

	lua_pushinteger(L, stats.images);
	lua_setfield(L, -2, "images");

	lua_pushinteger(L, stats.fonts);
	lua_setfield(L, -2, "fonts");

	lua_pushinteger(L, stats.textureMemory);
	lua_setfield(L, -2, "texturememory");

	return 1;
}

int w_draw(lua_State *L)
{
	Drawable *drawable = nullptr;
	Texture *texture = nullptr;
	Quad *quad = nullptr;
	int startidx = 2;

	if (luax_istype(L, 2, GRAPHICS_QUAD_ID))
	{
		texture = luax_checktexture(L, 1);
		quad = luax_totype<Quad>(L, 2, GRAPHICS_QUAD_ID);
		startidx = 3;
	}
	else if (lua_isnil(L, 2) && !lua_isnoneornil(L, 3))
	{
		return luax_typerror(L, 2, "Quad");
	}
	else
	{
		drawable = luax_checktype<Drawable>(L, 1, GRAPHICS_DRAWABLE_ID);
		startidx = 2;
	}

	float x  = (float) luaL_optnumber(L, startidx + 0, 0.0);
	float y  = (float) luaL_optnumber(L, startidx + 1, 0.0);
	float a  = (float) luaL_optnumber(L, startidx + 2, 0.0);
	float sx = (float) luaL_optnumber(L, startidx + 3, 1.0);
	float sy = (float) luaL_optnumber(L, startidx + 4, sx);
	float ox = (float) luaL_optnumber(L, startidx + 5, 0.0);
	float oy = (float) luaL_optnumber(L, startidx + 6, 0.0);
	float kx = (float) luaL_optnumber(L, startidx + 7, 0.0);
	float ky = (float) luaL_optnumber(L, startidx + 8, 0.0);

	luax_catchexcept(L, [&]() {
		if (texture && quad)
			texture->drawq(quad, x, y, a, sx, sy, ox, oy, kx, ky);
		else if (drawable)
			drawable->draw(x, y, a, sx, sy, ox, oy, kx, ky);
	});

	return 0;
}

int w_print(lua_State *L)
{
	std::vector<Font::ColoredString> str;
	luax_checkcoloredstring(L, 1, str);

	float x = (float)luaL_optnumber(L, 2, 0.0);
	float y = (float)luaL_optnumber(L, 3, 0.0);
	float angle = (float)luaL_optnumber(L, 4, 0.0f);
	float sx = (float)luaL_optnumber(L, 5, 1.0f);
	float sy = (float)luaL_optnumber(L, 6, sx);
	float ox = (float)luaL_optnumber(L, 7, 0.0f);
	float oy = (float)luaL_optnumber(L, 8, 0.0f);
	float kx = (float)luaL_optnumber(L, 9, 0.0f);
	float ky = (float)luaL_optnumber(L, 10, 0.0f);

	luax_catchexcept(L,
		[&](){ instance()->print(str, x, y, angle, sx, sy, ox, oy, kx,ky); }
	);
	return 0;
}

int w_printf(lua_State *L)
{
	std::vector<Font::ColoredString> str;
	luax_checkcoloredstring(L, 1, str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float wrap = (float)luaL_checknumber(L, 4);

	float angle = 0.0f;
	float sx = 1.0f, sy = 1.0f;
	float ox = 0.0f, oy = 0.0f;
	float kx = 0.0f, ky = 0.0f;

	Font::AlignMode align = Font::ALIGN_LEFT;

	if (lua_gettop(L) >= 5)
	{
		if (!lua_isnil(L, 5))
		{
			const char *str = luaL_checkstring(L, 5);
			if (!Font::getConstant(str, align))
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

	luax_catchexcept(L,
		[&](){ instance()->printf(str, x, y, wrap, align, angle, sx, sy, ox, oy, kx, ky); }
	);
	return 0;
}

int w_points(lua_State *L)
{
	// love.graphics.points has 3 variants:
	// - points(x1, y1, x2, y2, ...)
	// - points({x1, y1, x2, y2, ...})
	// - points({{x1, y1 [, r, g, b, a]}, {x2, y2 [, r, g, b, a]}, ...})

	int args = lua_gettop(L);
	bool is_table = false;
	bool is_table_of_tables = false;
	if (args == 1 && lua_istable(L, 1))
	{
		is_table = true;
		args = (int) luax_objlen(L, 1);

		lua_rawgeti(L, 1, 1);
		is_table_of_tables = lua_istable(L, -1);
		lua_pop(L, 1);
	}

	if (args % 2 != 0 && !is_table_of_tables)
		return luaL_error(L, "Number of vertex components must be a multiple of two");

	int numpoints = args / 2;
	if (is_table_of_tables)
		numpoints = args;

	float *coords = nullptr;
	uint8 *colors = nullptr;

	coords = new float[numpoints * 2];

	if (is_table_of_tables)
		colors = new uint8[numpoints * 4];

	if (is_table)
	{
		if (is_table_of_tables)
		{
			// points({{x1, y1 [, r, g, b, a]}, {x2, y2 [, r, g, b, a]}, ...})
			for (int i = 0; i < args; i++)
			{
				lua_rawgeti(L, 1, i + 1);
				for (int j = 1; j <= 6; j++)
					lua_rawgeti(L, -j, j);

				coords[i * 2 + 0] = luax_tofloat(L, -6);
				coords[i * 2 + 1] = luax_tofloat(L, -5);

				colors[i * 4 + 0] = (uint8) luaL_optnumber(L, -4, 255);
				colors[i * 4 + 1] = (uint8) luaL_optnumber(L, -3, 255);
				colors[i * 4 + 2] = (uint8) luaL_optnumber(L, -2, 255);
				colors[i * 4 + 3] = (uint8) luaL_optnumber(L, -1, 255);

				lua_pop(L, 7);
			}
		}
		else
		{
			// points({x1, y1, x2, y2, ...})
			for (int i = 0; i < args; i++)
			{
				lua_rawgeti(L, 1, i + 1);
				coords[i] = luax_tofloat(L, -1);
				lua_pop(L, 1);
			}
		}
	}
	else
	{
		for (int i = 0; i < args; i++)
			coords[i] = luax_tofloat(L, i + 1);
	}

	instance()->points(coords, colors, numpoints);

	delete[] coords;
	if (colors)
		delete[] colors;

	return 0;
}

int w_line(lua_State *L)
{
	int args = lua_gettop(L);
	bool is_table = false;
	if (args == 1 && lua_istable(L, 1))
	{
		args = (int) luax_objlen(L, 1);
		is_table = true;
	}

	if (args % 2 != 0)
		return luaL_error(L, "Number of vertex components must be a multiple of two");
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

	instance()->polyline(coords, args);

	delete[] coords;
	return 0;
}

int w_rectangle(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Invalid draw mode: %s", str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float w = (float)luaL_checknumber(L, 4);
	float h = (float)luaL_checknumber(L, 5);

	if (lua_isnoneornil(L, 6))
	{
		instance()->rectangle(mode, x, y, w, h);
		return 0;
	}

	float rx = (float)luaL_optnumber(L, 6, 0.0);
	float ry = (float)luaL_optnumber(L, 7, rx);

	int points;
	if (lua_isnoneornil(L, 8))
		points = std::max(rx, ry) > 20.0 ? (int)(std::max(rx, ry) / 2) : 10;
	else
		points = (int) luaL_checknumber(L, 8);

	instance()->rectangle(mode, x, y, w, h, rx, ry, points);
	return 0;
}

int w_circle(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Invalid draw mode: %s", str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float radius = (float)luaL_checknumber(L, 4);
	int points;
	if (lua_isnoneornil(L, 5))
		points = radius > 10 ? (int)(radius) : 10;
	else
		points = (int) luaL_checknumber(L, 5);

	instance()->circle(mode, x, y, radius, points);
	return 0;
}

int w_ellipse(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luaL_error(L, "Invalid draw mode: %s", str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float a = (float)luaL_checknumber(L, 4);
	float b = (float)luaL_optnumber(L, 5, a);

	int points;
	if (lua_isnoneornil(L, 6))
		points = a + b > 30 ? (int)((a + b) / 2) : 15;
	else
		points = (int) luaL_checknumber(L, 6);

	instance()->ellipse(mode, x, y, a, b, points);
	return 0;
}

int w_arc(lua_State *L)
{
	Graphics::DrawMode drawmode;
	const char *drawstr = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(drawstr, drawmode))
		return luaL_error(L, "Invalid draw mode: %s", drawstr);

	int startidx = 2;

	Graphics::ArcMode arcmode = Graphics::ARC_PIE;

	if (lua_type(L, 2) == LUA_TSTRING)
	{
		const char *arcstr = luaL_checkstring(L, 2);
		if (!Graphics::getConstant(arcstr, arcmode))
			return luaL_error(L, "Invalid arc mode: %s", arcstr);

		startidx = 3;
	}

	float x = (float) luaL_checknumber(L, startidx + 0);
	float y = (float) luaL_checknumber(L, startidx + 1);
	float radius = (float) luaL_checknumber(L, startidx + 2);
	float angle1 = (float) luaL_checknumber(L, startidx + 3);
	float angle2 = (float) luaL_checknumber(L, startidx + 4);

	int points = (int) radius;
	float angle = fabs(angle1 - angle2);

	// The amount of points is based on the fraction of the circle created by the arc.
	if (angle < 2.0f * (float) LOVE_M_PI)
		points *= angle / (2.0f * (float) LOVE_M_PI);

	points = std::max(points, 10);
	points = (int) luaL_optnumber(L, startidx + 5, points);

	instance()->arc(drawmode, arcmode, x, y, radius, angle1, angle2, points);
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
		args = (int) luax_objlen(L, 2);
		is_table = true;
	}

	if (args % 2 != 0)
		return luaL_error(L, "Number of vertex components must be a multiple of two");
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
	instance()->polygon(mode, coords, args+2);
	delete[] coords;

	return 0;
}

int w_push(lua_State *L)
{
	Graphics::StackType stype = Graphics::STACK_TRANSFORM;
	const char *sname = lua_isnoneornil(L, 1) ? nullptr : luaL_checkstring(L, 1);
	if (sname && !Graphics::getConstant(sname, stype))
		return luaL_error(L, "Invalid graphics stack type: %s", sname);

	luax_catchexcept(L, [&](){ instance()->push(stype); });
	return 0;
}

int w_pop(lua_State *L)
{
	luax_catchexcept(L, [&](){ instance()->pop(); });
	return 0;
}

int w_rotate(lua_State *L)
{
	float angle = (float)luaL_checknumber(L, 1);
	instance()->rotate(angle);
	return 0;
}

int w_scale(lua_State *L)
{
	float sx = (float)luaL_optnumber(L, 1, 1.0f);
	float sy = (float)luaL_optnumber(L, 2, sx);
	instance()->scale(sx, sy);
	return 0;
}

int w_translate(lua_State *L)
{
	float x = (float)luaL_checknumber(L, 1);
	float y = (float)luaL_checknumber(L, 2);
	instance()->translate(x, y);
	return 0;
}

int w_shear(lua_State *L)
{
	float kx = (float)luaL_checknumber(L, 1);
	float ky = (float)luaL_checknumber(L, 2);
	instance()->shear(kx, ky);
	return 0;
}

int w_origin(lua_State * /*L*/)
{
	instance()->origin();
	return 0;
}


// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "reset", w_reset },
	{ "clear", w_clear },
	{ "discard", w_discard },
	{ "present", w_present },

	{ "newImage", w_newImage },
	{ "newQuad", w_newQuad },
	{ "newFont", w_newFont },
	{ "newImageFont", w_newImageFont },
	{ "newSpriteBatch", w_newSpriteBatch },
	{ "newParticleSystem", w_newParticleSystem },
	{ "newCanvas", w_newCanvas },
	{ "newShader", w_newShader },
	{ "newMesh", w_newMesh },
	{ "newText", w_newText },
	{ "_newVideo", w_newVideo },

	{ "setColor", w_setColor },
	{ "getColor", w_getColor },
	{ "setBackgroundColor", w_setBackgroundColor },
	{ "getBackgroundColor", w_getBackgroundColor },

	{ "setNewFont", w_setNewFont },
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
	{ "setLineJoin", w_setLineJoin },
	{ "getLineWidth", w_getLineWidth },
	{ "getLineStyle", w_getLineStyle },
	{ "getLineJoin", w_getLineJoin },
	{ "setPointSize", w_setPointSize },
	{ "getPointSize", w_getPointSize },
	{ "setWireframe", w_setWireframe },
	{ "isWireframe", w_isWireframe },
	{ "newScreenshot", w_newScreenshot },
	{ "setCanvas", w_setCanvas },
	{ "getCanvas", w_getCanvas },

	{ "setShader", w_setShader },
	{ "getShader", w_getShader },
	{ "_setDefaultShaderCode", w_setDefaultShaderCode },

	{ "getSupported", w_getSupported },
	{ "getCanvasFormats", w_getCanvasFormats },
	{ "getCompressedImageFormats", w_getCompressedImageFormats },
	{ "getRendererInfo", w_getRendererInfo },
	{ "getSystemLimits", w_getSystemLimits },
	{ "getStats", w_getStats },

	{ "draw", w_draw },

	{ "print", w_print },
	{ "printf", w_printf },

	{ "isCreated", w_isCreated },
	{ "isActive", w_isActive },
	{ "isGammaCorrect", w_isGammaCorrect },
	{ "getWidth", w_getWidth },
	{ "getHeight", w_getHeight },
	{ "getDimensions", w_getDimensions },

	{ "setScissor", w_setScissor },
	{ "intersectScissor", w_intersectScissor },
	{ "getScissor", w_getScissor },

	{ "stencil", w_stencil },
	{ "setStencilTest", w_setStencilTest },
	{ "getStencilTest", w_getStencilTest },

	{ "points", w_points },
	{ "line", w_line },
	{ "rectangle", w_rectangle },
	{ "circle", w_circle },
	{ "ellipse", w_ellipse },
	{ "arc", w_arc },

	{ "polygon", w_polygon },

	{ "push", w_push },
	{ "pop", w_pop },
	{ "rotate", w_rotate },
	{ "scale", w_scale },
	{ "translate", w_translate },
	{ "shear", w_shear },
	{ "origin", w_origin },

	{ 0, 0 }
};

static int luaopen_drawable(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_DRAWABLE_ID, "Drawable", nullptr);
}

// Types for this module.
static const lua_CFunction types[] =
{
	luaopen_drawable,
	luaopen_texture,
	luaopen_font,
	luaopen_image,
	luaopen_quad,
	luaopen_spritebatch,
	luaopen_particlesystem,
	luaopen_canvas,
	luaopen_shader,
	luaopen_mesh,
	luaopen_text,
	luaopen_video,
	0
};

extern "C" int luaopen_love_graphics(lua_State *L)
{
	Graphics *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new Graphics(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "graphics";
	w.type = MODULE_GRAPHICS_ID;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	if (luaL_loadbuffer(L, (const char *)graphics_lua, sizeof(graphics_lua), "wrap_Graphics.lua") == 0)
		lua_call(L, 0, 0);

	return n;
}

} // opengl
} // graphics
} // love
