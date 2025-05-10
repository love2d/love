/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#include "common/config.h"
#include "wrap_Graphics.h"
#include "Texture.h"
#include "image/ImageData.h"
#include "image/Image.h"
#include "font/Rasterizer.h"
#include "filesystem/Filesystem.h"
#include "filesystem/wrap_Filesystem.h"
#include "video/VideoStream.h"
#include "image/wrap_Image.h"
#include "common/Reference.h"
#include "math/wrap_Transform.h"
#include "thread/wrap_Channel.h"

#include "opengl/Graphics.h"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <sstream>

#include <algorithm>

// Shove the wrap_Graphics.lua code directly into a raw string literal.
static const char graphics_lua[] =
#include "wrap_Graphics.lua"
;

namespace love
{
namespace graphics
{

#define instance() (Module::getInstance<Graphics>(Module::M_GRAPHICS))

static int luax_checkgraphicscreated(lua_State *L)
{
	if (!instance()->isCreated())
		return luaL_error(L, "love.graphics cannot function without a window.");
	return 0;
}

int w_reset(lua_State *)
{
	instance()->reset();
	return 0;
}

int w_clear(lua_State *L)
{
	OptionalColorD color(ColorD(0.0, 0.0, 0.0, 0.0));
	std::vector<OptionalColorD> colors;

	OptionalInt stencil(0);
	OptionalDouble depth(1.0);

	int argtype = lua_type(L, 1);
	int startidx = -1;

	if (argtype == LUA_TTABLE)
	{
		int maxn = lua_gettop(L);
		colors.reserve(maxn);

		for (int i = 0; i < maxn; i++)
		{
			argtype = lua_type(L, i + 1);

			if (argtype == LUA_TNUMBER || argtype == LUA_TBOOLEAN)
			{
				startidx = i + 1;
				break;
			}
			else if (argtype == LUA_TNIL || argtype == LUA_TNONE || luax_objlen(L, i + 1) == 0)
			{
				colors.push_back(OptionalColorD());
				continue;
			}

			for (int j = 1; j <= 4; j++)
				lua_rawgeti(L, i + 1, j);

			OptionalColorD c;
			c.hasValue = true;
			c.value.r = luaL_checknumber(L, -4);
			c.value.g = luaL_checknumber(L, -3);
			c.value.b = luaL_checknumber(L, -2);
			c.value.a = luaL_optnumber(L, -1, 1.0);
			colors.push_back(c);

			lua_pop(L, 4);
		}
	}
	else if (argtype == LUA_TBOOLEAN)
	{
		color.hasValue = luax_toboolean(L, 1);
		startidx = 2;
	}
	else if (argtype != LUA_TNONE && argtype != LUA_TNIL)
	{
		color.hasValue = true;
		color.value.r = (float) luaL_checknumber(L, 1);
		color.value.g = (float) luaL_checknumber(L, 2);
		color.value.b = (float) luaL_checknumber(L, 3);
		color.value.a = (float) luaL_optnumber(L, 4, 1.0);
		startidx = 5;
	}

	if (startidx >= 0)
	{
		argtype = lua_type(L, startidx);
		if (argtype == LUA_TBOOLEAN)
			stencil.hasValue = luax_toboolean(L, startidx);
		else if (argtype == LUA_TNUMBER)
			stencil.value = (int) luaL_checkinteger(L, startidx);

		argtype = lua_type(L, startidx + 1);
		if (argtype == LUA_TBOOLEAN)
			depth.hasValue = luax_toboolean(L, startidx + 1);
		else if (argtype == LUA_TNUMBER)
			depth.value = luaL_checknumber(L, startidx + 1);
	}

	if (colors.empty())
		luax_catchexcept(L, [&]() { instance()->clear(color, stencil, depth); });
	else
		luax_catchexcept(L, [&]() { instance()->clear(colors, stencil, depth); });

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
		size_t numbuffers = std::max((size_t) 1, instance()->getRenderTargets().colors.size());
		colorbuffers = std::vector<bool>(numbuffers, discardcolor);
	}

	bool depthstencil = luax_optboolean(L, 2, true);
	instance()->discard(colorbuffers, depthstencil);
	return 0;
}

int w_present(lua_State *L)
{
	luax_catchexcept(L, [&]() { instance()->present(L); });
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
	luax_pushboolean(L, graphics::isGammaCorrect());
	return 1;
}

int w_isLowPowerPreferred(lua_State *L)
{
	luax_pushboolean(L, graphics::isLowPowerPreferred());
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

int w_getPixelWidth(lua_State *L)
{
	lua_pushinteger(L, instance()->getPixelWidth());
	return 1;
}

int w_getPixelHeight(lua_State *L)
{
	lua_pushinteger(L, instance()->getPixelHeight());
	return 1;
}

int w_getPixelDimensions(lua_State *L)
{
	lua_pushinteger(L, instance()->getPixelWidth());
	lua_pushinteger(L, instance()->getPixelHeight());
	return 2;
}

int w_getDPIScale(lua_State *L)
{
	lua_pushnumber(L, instance()->getScreenDPIScale());
	return 1;
}

int w_getQuadIndexBuffer(lua_State *L)
{
	luax_pushtype(L, instance()->getQuadIndexBuffer());
	return 1;
}

static Graphics::RenderTarget checkRenderTarget(lua_State *L, int idx)
{
	lua_rawgeti(L, idx, 1);
	Graphics::RenderTarget target(luax_checktexture(L, -1), 0);
	lua_pop(L, 1);

	TextureType type = target.texture->getTextureType();
	if (type == TEXTURE_2D_ARRAY || type == TEXTURE_VOLUME)
		target.slice = luax_checkintflag(L, idx, "layer") - 1;
	else if (type == TEXTURE_CUBE)
		target.slice = luax_checkintflag(L, idx, "face") - 1;

	target.mipmap = luax_intflag(L, idx, "mipmap", 1) - 1;

	return target;
}

int w_setCanvas(lua_State *L)
{
	// called with none -> reset to default buffer
	if (lua_isnoneornil(L, 1))
	{
		instance()->setRenderTarget();
		return 0;
	}

	bool is_table = lua_istable(L, 1);
	Graphics::RenderTargets targets;

	if (is_table)
	{
		lua_rawgeti(L, 1, 1);
		bool table_of_tables = lua_istable(L, -1);
		lua_pop(L, 1);

		for (int i = 1; i <= (int) luax_objlen(L, 1); i++)
		{
			lua_rawgeti(L, 1, i);

			if (table_of_tables)
				targets.colors.push_back(checkRenderTarget(L, -1));
			else
			{
				targets.colors.emplace_back(luax_checktexture(L, -1), 0);

				if (targets.colors.back().texture->getTextureType() != TEXTURE_2D)
					return luaL_error(L, "Non-2D textures must use the table-of-tables variant of setCanvas.");
			}

			lua_pop(L, 1);
		}

		uint32 tempdepthflag   = Graphics::TEMPORARY_RT_DEPTH;
		uint32 tempstencilflag = Graphics::TEMPORARY_RT_STENCIL;

		lua_getfield(L, 1, "depthstencil");
		int dstype = lua_type(L, -1);
		if (dstype == LUA_TTABLE)
			targets.depthStencil = checkRenderTarget(L, -1);
		else if (dstype == LUA_TBOOLEAN)
			targets.temporaryRTFlags |= luax_toboolean(L, -1) ? (tempdepthflag | tempstencilflag) : 0;
		else if (dstype != LUA_TNONE && dstype != LUA_TNIL)
			targets.depthStencil.texture = luax_checktexture(L, -1);
		lua_pop(L, 1);

		if (targets.depthStencil.texture == nullptr && (targets.temporaryRTFlags & tempdepthflag) == 0)
			targets.temporaryRTFlags |= luax_boolflag(L, 1, "depth", false) ? tempdepthflag : 0;

		if (targets.depthStencil.texture == nullptr && (targets.temporaryRTFlags & tempstencilflag) == 0)
			targets.temporaryRTFlags |= luax_boolflag(L, 1, "stencil", false) ? tempstencilflag : 0;
	}
	else
	{
		for (int i = 1; i <= lua_gettop(L); i++)
		{
			Graphics::RenderTarget target(luax_checktexture(L, i), 0);
			TextureType type = target.texture->getTextureType();

			if (i == 1 && type != TEXTURE_2D)
			{
				target.slice = (int) luaL_checkinteger(L, i + 1) - 1;
				target.mipmap = (int) luaL_optinteger(L, i + 2, 1) - 1;
				targets.colors.push_back(target);
				break;
			}
			else if (type == TEXTURE_2D && lua_isnumber(L, i + 1))
			{
				target.mipmap = (int) luaL_optinteger(L, i + 1, 1) - 1;
				i++;
			}

			if (i > 1 && type != TEXTURE_2D)
				return luaL_error(L, "This variant of setCanvas only supports 2D texture types.");

			targets.colors.push_back(target);
		}
	}

	luax_catchexcept(L, [&]() {
		if (targets.getFirstTarget().texture != nullptr)
			instance()->setRenderTargets(targets);
		else
			instance()->setRenderTarget();
	});
	
	return 0;
}

static void pushRenderTarget(lua_State *L, const Graphics::RenderTarget &rt)
{
	lua_createtable(L, 1, 2);

	luax_pushtype(L, rt.texture);
	lua_rawseti(L, -2, 1);

	TextureType type = rt.texture->getTextureType();

	if (type == TEXTURE_2D_ARRAY || type == TEXTURE_VOLUME)
	{
		lua_pushnumber(L, rt.slice + 1);
		lua_setfield(L, -2, "layer");
	}
	else if (type == TEXTURE_CUBE)
	{
		lua_pushnumber(L, rt.slice + 1);
		lua_setfield(L, -2, "face");
	}

	lua_pushnumber(L, rt.mipmap + 1);
	lua_setfield(L, -2, "mipmap");
}

int w_getCanvas(lua_State *L)
{
	Graphics::RenderTargets targets = instance()->getRenderTargets();
	int ntargets = (int) targets.colors.size();

	if (ntargets == 0)
	{
		lua_pushnil(L);
		return 1;
	}

	bool shouldUseTablesVariant = targets.depthStencil.texture != nullptr;

	if (!shouldUseTablesVariant)
	{
		for (const auto &rt : targets.colors)
		{
			if (rt.mipmap != 0 || rt.texture->getTextureType() != TEXTURE_2D)
			{
				shouldUseTablesVariant = true;
				break;
			}
		}
	}

	if (shouldUseTablesVariant)
	{
		lua_createtable(L, ntargets, 0);

		for (int i = 0; i < ntargets; i++)
		{
			pushRenderTarget(L, targets.colors[i]);
			lua_rawseti(L, -2, i + 1);
		}

		if (targets.depthStencil.texture != nullptr)
		{
			pushRenderTarget(L, targets.depthStencil);
			lua_setfield(L, -2, "depthstencil");
		}

		return 1;
	}
	else
	{
		for (const auto &rt : targets.colors)
			luax_pushtype(L, rt.texture);

		return ntargets;
	}
}

static void screenshotFunctionCallback(const Graphics::ScreenshotInfo *info, love::image::ImageData *i, void *gd)
{
	if (info == nullptr)
		return;

	lua_State *L = (lua_State *) gd;
	Reference *ref = (Reference *) info->data;

	if (i != nullptr && L != nullptr)
	{
		if (ref == nullptr)
			luaL_error(L, "Internal error in screenshot callback.");

		ref->push(L);
		delete ref;
		luax_pushtype(L, i);
		lua_call(L, 1, 0);
	}
	else
		delete ref;
}

struct ScreenshotFileInfo
{
	std::string filename;
	image::FormatHandler::EncodedFormat format;
};

static void screenshotFileCallback(const Graphics::ScreenshotInfo *info, love::image::ImageData *i, void * /*gd*/)
{
	if (info == nullptr)
		return;

	ScreenshotFileInfo *fileinfo = (ScreenshotFileInfo *) info->data;

	if (i != nullptr && fileinfo != nullptr)
	{
		try
		{
			i->encode(fileinfo->format, fileinfo->filename.c_str(), true);
		}
		catch (love::Exception &e)
		{
			printf("Screenshot encoding or saving failed: %s", e.what());
			// Do nothing...
		}
	}

	delete fileinfo;
}

static void screenshotChannelCallback(const Graphics::ScreenshotInfo *info, love::image::ImageData *i, void * /*gd*/)
{
	if (info == nullptr)
		return;

	auto *channel = (love::thread::Channel *) info->data;

	if (channel != nullptr)
	{
		if (i != nullptr)
			channel->push(Variant(&love::image::ImageData::type, i));

		channel->release();
	}
}

int w_captureScreenshot(lua_State *L)
{
	Graphics::ScreenshotInfo info;

	if (lua_isfunction(L, 1))
	{
		lua_pushvalue(L, 1);
		info.data = luax_refif(L, LUA_TFUNCTION);
		lua_pop(L, 1);
		info.callback = screenshotFunctionCallback;
	}
	else if (lua_isstring(L, 1))
	{
		std::string filename = luax_checkstring(L, 1);
		std::string ext;

		size_t dotpos = filename.rfind('.');

		if (dotpos != std::string::npos)
			ext = filename.substr(dotpos + 1);

		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

		image::FormatHandler::EncodedFormat format;
		if (!image::ImageData::getConstant(ext.c_str(), format))
			return luax_enumerror(L, "encoded image format", image::ImageData::getConstants(format), ext.c_str());

		ScreenshotFileInfo *fileinfo = new ScreenshotFileInfo;
		fileinfo->filename = filename;
		fileinfo->format = format;

		info.data = fileinfo;
		info.callback = screenshotFileCallback;
	}
	else if (luax_istype(L, 1, love::thread::Channel::type))
	{
		auto *channel = love::thread::luax_checkchannel(L, 1);
		channel->retain();
		info.data = channel;
		info.callback = screenshotChannelCallback;
	}
	else
		return luax_typerror(L, 1, "function, string, or Channel");

	luax_catchexcept(L,
		[&]() { instance()->captureScreenshot(info); },
		[&](bool except) { if (except) info.callback(&info, nullptr, nullptr); }
	);

	return 0;
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

	Rect rect;
	rect.x = (int) luaL_checkinteger(L, 1);
	rect.y = (int) luaL_checkinteger(L, 2);
	rect.w = (int) luaL_checkinteger(L, 3);
	rect.h = (int) luaL_checkinteger(L, 4);

	if (rect.w < 0 || rect.h < 0)
		return luaL_error(L, "Can't set scissor with negative width and/or height.");

	instance()->setScissor(rect);
	return 0;
}

int w_intersectScissor(lua_State *L)
{
	Rect rect;
	rect.x = (int) luaL_checkinteger(L, 1);
	rect.y = (int) luaL_checkinteger(L, 2);
	rect.w = (int) luaL_checkinteger(L, 3);
	rect.h = (int) luaL_checkinteger(L, 4);

	if (rect.w < 0 || rect.h < 0)
		return luaL_error(L, "Can't set scissor with negative width and/or height.");

	instance()->intersectScissor(rect);
	return 0;
}

int w_getScissor(lua_State *L)
{
	Rect rect;
	if (!instance()->getScissor(rect))
		return 0;

	lua_pushinteger(L, rect.x);
	lua_pushinteger(L, rect.y);
	lua_pushinteger(L, rect.w);
	lua_pushinteger(L, rect.h);

	return 4;
}

int w_setStencilMode(lua_State *L)
{
	if (lua_gettop(L) <= 1 && lua_isnoneornil(L, 1))
	{
		luax_catchexcept(L, [&]() { instance()->setStencilMode(); });
		return 0;
	}

	StencilMode mode = STENCIL_MODE_OFF;
	const char *modestr = luaL_checkstring(L, 1);
	if (!getConstant(modestr, mode))
		return luax_enumerror(L, "stencil mode", getConstants(mode), modestr);

	int value = (int) luaL_optinteger(L, 2, 1);

	luax_catchexcept(L, [&]() { instance()->setStencilMode(mode, value); });
	return 0;
}

int w_getStencilMode(lua_State *L)
{
	int value = 0;
	StencilMode mode = instance()->getStencilMode(value);

	const char *modestr;
	if (!getConstant(mode, modestr))
		return luaL_error(L, "Unknown stencil mode.");

	lua_pushstring(L, modestr);
	lua_pushinteger(L, value);
	return 2;
}

int w_setStencilState(lua_State *L)
{
	if (lua_gettop(L) <= 1 && lua_isnoneornil(L, 1))
	{
		luax_catchexcept(L, [&](){ instance()->setStencilState(); });
		return 0;
	}

	StencilState s;

	const char *actionstr = luaL_checkstring(L, 1);
	if (!getConstant(actionstr, s.action))
		return luax_enumerror(L, "stencil draw action", getConstants(s.action), actionstr);

	const char *comparestr = luaL_checkstring(L, 2);
	if (!getConstant(comparestr, s.compare))
		return luax_enumerror(L, "compare mode", getConstants(s.compare), comparestr);

	s.value = (int) luaL_optinteger(L, 3, 0);
	s.readMask = (uint32) luaL_optnumber(L, 4, LOVE_UINT32_MAX);
	s.writeMask = (uint32) luaL_optnumber(L, 5, LOVE_UINT32_MAX);

	luax_catchexcept(L, [&](){ instance()->setStencilState(s); });
	return 0;
}

int w_getStencilState(lua_State *L)
{
	const StencilState &s = instance()->getStencilState();

	const char *actionstr;
	if (!getConstant(s.action, actionstr))
		return luaL_error(L, "Unknown stencil draw action.");

	const char *comparestr;
	if (!getConstant(s.compare, comparestr))
		return luaL_error(L, "Unknown compare mode.");

	lua_pushstring(L, actionstr);
	lua_pushstring(L, comparestr);
	lua_pushinteger(L, s.value);
	lua_pushnumber(L, s.readMask);
	lua_pushnumber(L, s.writeMask);
	return 5;
}

static void parseDPIScale(Data *d, float *dpiscale)
{
	auto fd = dynamic_cast<love::filesystem::FileData *>(d);
	if (fd == nullptr)
		return;

	// Parse a density scale of 2.0 from "image@2x.png".
	const std::string &fname = fd->getName();

	size_t namelen = fname.length();
	size_t atpos = fname.rfind('@');

	if (atpos != std::string::npos && atpos + 2 < namelen
		&& (fname[namelen - 1] == 'x' || fname[namelen - 1] == 'X'))
	{
		char *end = nullptr;
		long density = strtol(fname.c_str() + atpos + 1, &end, 10);
		if (end != nullptr && density > 0 && dpiscale != nullptr)
			*dpiscale = (float) density;
	}
}

static std::pair<StrongRef<image::ImageData>, StrongRef<image::CompressedImageData>>
getImageData(lua_State *L, int idx, bool allowcompressed, float *dpiscale)
{
	StrongRef<image::ImageData> idata;
	StrongRef<image::CompressedImageData> cdata;

	if (luax_istype(L, idx, image::ImageData::type))
		idata.set(image::luax_checkimagedata(L, idx));
	else if (luax_istype(L, idx, image::CompressedImageData::type))
		cdata.set(image::luax_checkcompressedimagedata(L, idx));
	else if (filesystem::luax_cangetdata(L, idx))
	{
		// Convert to ImageData / CompressedImageData.
		auto imagemodule = Module::getInstance<image::Image>(Module::M_IMAGE);
		if (imagemodule == nullptr)
			luaL_error(L, "Cannot load images without the love.image module.");

		StrongRef<Data> fdata(filesystem::luax_getdata(L, idx), Acquire::NORETAIN);

		if (dpiscale != nullptr)
			parseDPIScale(fdata, dpiscale);

		if (allowcompressed && imagemodule->isCompressed(fdata))
			luax_catchexcept(L, [&]() { cdata.set(imagemodule->newCompressedData(fdata), Acquire::NORETAIN); });
		else
			luax_catchexcept(L, [&]() { idata.set(imagemodule->newImageData(fdata), Acquire::NORETAIN); });
	}
	else
		idata.set(image::luax_checkimagedata(L, idx));

	return std::make_pair(idata, cdata);
}

static int w__pushNewTexture(lua_State *L, Texture::Slices *slices, const Texture::Settings &settings)
{
	StrongRef<Texture> i;
	luax_catchexcept(L,
		[&]() { i.set(instance()->newTexture(settings, slices), Acquire::NORETAIN); },
		[&](bool) { if (slices) slices->clear(); }
	);

	luax_pushtype(L, i);
	return 1;
}

static void luax_checktexturesettings(lua_State *L, int idx, bool opt, bool checkType, bool checkDimensions, OptionalBool forceRenderTarget, Texture::Settings &s, bool &setdpiscale)
{
	setdpiscale = false;
	if (forceRenderTarget.hasValue)
		s.renderTarget = forceRenderTarget.value;

	if (opt && lua_isnoneornil(L, idx))
		return;

	luax_checktablefields<Texture::SettingType>(L, idx, "texture setting name", Texture::getConstant);

	if (!forceRenderTarget.hasValue)
		s.renderTarget = luax_boolflag(L, idx, Texture::getConstant(Texture::SETTING_RENDER_TARGET), s.renderTarget);

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_DEBUGNAME));
	if (!lua_isnoneornil(L, -1))
	{
		s.debugName = luaL_checkstring(L, -1);
	}
	lua_pop(L, 1);

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_FORMAT));
	if (!lua_isnoneornil(L, -1))
	{
		const char *str = luaL_checkstring(L, -1);
		if (!getConstant(str, s.format))
			luax_enumerror(L, "pixel format", str);
	}
	lua_pop(L, 1);

	if (checkType)
	{
		lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_TYPE));
		if (!lua_isnoneornil(L, -1))
		{
			const char *str = luaL_checkstring(L, -1);
			if (!Texture::getConstant(str, s.type))
				luax_enumerror(L, "texture type", Texture::getConstants(s.type), str);
		}
		lua_pop(L, 1);
	}

	if (checkDimensions)
	{
		s.width = luax_checkintflag(L, idx, Texture::getConstant(Texture::SETTING_WIDTH));
		s.height = luax_checkintflag(L, idx, Texture::getConstant(Texture::SETTING_HEIGHT));
		if (s.type == TEXTURE_2D_ARRAY || s.type == TEXTURE_VOLUME)
			s.layers = luax_checkintflag(L, idx, Texture::getConstant(Texture::SETTING_LAYERS));
	}
	else
	{
		s.width = luax_intflag(L, idx, Texture::getConstant(Texture::SETTING_WIDTH), s.width);
		s.height = luax_intflag(L, idx, Texture::getConstant(Texture::SETTING_HEIGHT), s.height);
		if (s.type == TEXTURE_2D_ARRAY || s.type == TEXTURE_VOLUME)
			s.layers = luax_intflag(L, idx, Texture::getConstant(Texture::SETTING_LAYERS), s.layers);
	}

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_MIPMAPS));
	if (!lua_isnoneornil(L, -1))
	{
		if (lua_type(L, -1) == LUA_TBOOLEAN)
			s.mipmaps = luax_toboolean(L, -1) ? Texture::MIPMAPS_MANUAL : Texture::MIPMAPS_NONE;
		else
		{
			const char *str = luaL_checkstring(L, -1);
			if (!Texture::getConstant(str, s.mipmaps))
				luax_enumerror(L, "Texture mipmap mode", Texture::getConstants(s.mipmaps), str);
		}
	}
	lua_pop(L, 1);

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_MIPMAP_COUNT));
	if (!lua_isnoneornil(L, -1))
		s.mipmapCount = (int) luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	s.linear = luax_boolflag(L, idx, Texture::getConstant(Texture::SETTING_LINEAR), s.linear);
	s.msaa = luax_intflag(L, idx, Texture::getConstant(Texture::SETTING_MSAA), s.msaa);

	s.computeWrite = luax_boolflag(L, idx, Texture::getConstant(Texture::SETTING_COMPUTE_WRITE), s.computeWrite);

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_VIEW_FORMATS));
	if (!lua_isnoneornil(L, -1))
	{
		if (lua_type(L, -1) != LUA_TTABLE)
			luaL_argerror(L, idx, "expected field 'viewformats' to be a table type");

		for (int i = 1; i <= (int)luax_objlen(L, -1); i++)
		{
			lua_rawgeti(L, -1, i);
			const char *str = luaL_checkstring(L, -1);
			PixelFormat viewformat = PIXELFORMAT_UNKNOWN;
			if (!getConstant(str, viewformat))
				luax_enumerror(L, "pixel format", str);
			s.viewFormats.push_back(viewformat);
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_READABLE));
	if (!lua_isnoneornil(L, -1))
		s.readable.set(luax_checkboolean(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, idx, Texture::getConstant(Texture::SETTING_DPI_SCALE));
	if (lua_isnumber(L, -1))
	{
		s.dpiScale = (float) lua_tonumber(L, -1);
		setdpiscale = true;
	}
	lua_pop(L, 1);
}

int w_newCanvas(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture::Settings s;

	OptionalBool forceRenderTarget(true);
	bool setDPIScale = false;

	if (lua_istable(L, 1))
	{
		luax_checktexturesettings(L, 1, false, true, true, forceRenderTarget, s, setDPIScale);
	}
	else
	{
		// check if width and height are given. else default to screen dimensions.
		s.width  = (int) luaL_optinteger(L, 1, instance()->getWidth());
		s.height = (int) luaL_optinteger(L, 2, instance()->getHeight());

		int startidx = 3;

		if (lua_isnumber(L, 3))
		{
			s.layers = (int) luaL_checkinteger(L, 3);
			s.type = TEXTURE_2D_ARRAY;
			startidx = 4;
		}

		luax_checktexturesettings(L, startidx, true, true, false, forceRenderTarget, s, setDPIScale);
	}

	// Default to the screen's current pixel density scale.
	if (!setDPIScale)
		s.dpiScale = instance()->getScreenDPIScale();

	Texture *texture = nullptr;
	luax_catchexcept(L, [&](){ texture = instance()->newTexture(s); });

	luax_pushtype(L, texture);
	texture->release();
	return 1;
}

int w_newTexture(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture::Slices slices(TEXTURE_2D);
	Texture::Slices *slicesref = &slices;

	Texture::Settings settings;
	settings.type = TEXTURE_2D;
	bool dpiscaleset = false;

	if (lua_type(L, 1) == LUA_TNUMBER)
	{
		slicesref = nullptr;

		settings.width = (int) luaL_checkinteger(L, 1);
		settings.height = (int) luaL_checkinteger(L, 2);

		int startidx = 3;

		if (lua_type(L, 3) == LUA_TNUMBER)
		{
			settings.layers = (int) luaL_checkinteger(L, 3);
			settings.type = TEXTURE_2D_ARRAY;
			startidx = 4;
		}

		luax_checktexturesettings(L, startidx, true, true, false, OptionalBool(), settings, dpiscaleset);
	}
	else
	{
		luax_checktexturesettings(L, 2, true, false, false, OptionalBool(), settings, dpiscaleset);
		float *autodpiscale = dpiscaleset ? nullptr : &settings.dpiScale;

		if (lua_istable(L, 1))
		{
			int n = std::max(1, (int) luax_objlen(L, 1));
			for (int i = 0; i < n; i++)
			{
				lua_rawgeti(L, 1, i + 1);
				auto data = getImageData(L, -1, true, i == 0 ? autodpiscale : nullptr);
				if (data.first.get())
					slices.set(0, i, data.first);
				else
					slices.set(0, i, data.second->getSlice(0, 0));
			}
			lua_pop(L, n);
		}
		else
		{
			auto data = getImageData(L, 1, true, autodpiscale);
			if (data.first.get())
				slices.set(0, 0, data.first);
			else
				slices.add(data.second, 0, 0, false, settings.mipmaps != Texture::MIPMAPS_NONE);
		}
	}

	return w__pushNewTexture(L, slicesref, settings);
}

int w_newCubeTexture(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture::Slices slices(TEXTURE_CUBE);
	Texture::Slices *slicesref = &slices;

	Texture::Settings settings;
	settings.type = TEXTURE_CUBE;
	bool dpiscaleset = false;

	if (lua_type(L, 1) == LUA_TNUMBER)
	{
		slicesref = nullptr;
		settings.width = settings.height = (int) luaL_checkinteger(L, 1);
		luax_checktexturesettings(L, 2, true, false, false, OptionalBool(), settings, dpiscaleset);
	}
	else
	{
		luax_checktexturesettings(L, 2, true, false, false, OptionalBool(), settings, dpiscaleset);
		float *autodpiscale = dpiscaleset ? nullptr : &settings.dpiScale;

		auto imagemodule = Module::getInstance<love::image::Image>(Module::M_IMAGE);

		if (!lua_istable(L, 1))
		{
			auto data = getImageData(L, 1, true, autodpiscale);

			std::vector<StrongRef<love::image::ImageData>> faces;

			if (data.first.get())
			{
				luax_catchexcept(L, [&](){ faces = imagemodule->newCubeFaces(data.first); });

				for (int i = 0; i < (int) faces.size(); i++)
					slices.set(i, 0, faces[i]);
			}
			else
				slices.add(data.second, 0, 0, true, settings.mipmaps != Texture::MIPMAPS_NONE);
		}
		else
		{
			int tlen = (int) luax_objlen(L, 1);

			if (luax_isarrayoftables(L, 1))
			{
				if (tlen != 6)
					return luaL_error(L, "Cubemap images must have 6 faces.");

				for (int face = 0; face < tlen; face++)
				{
					lua_rawgeti(L, 1, face + 1);
					luaL_checktype(L, -1, LUA_TTABLE);

					int miplen = std::max(1, (int) luax_objlen(L, -1));

					for (int mip = 0; mip < miplen; mip++)
					{
						lua_rawgeti(L, -1, mip + 1);

						auto data = getImageData(L, -1, true, face == 0 && mip == 0 ? autodpiscale : nullptr);
						if (data.first.get())
							slices.set(face, mip, data.first);
						else
							slices.set(face, mip, data.second->getSlice(0, 0));

						lua_pop(L, 1);
					}
				}
			}
			else
			{
				bool usemipmaps = false;

				for (int i = 0; i < tlen; i++)
				{
					lua_rawgeti(L, 1, i + 1);

					auto data = getImageData(L, -1, true, i == 0 ? autodpiscale : nullptr);

					if (data.first.get())
					{
						if (usemipmaps || data.first->getWidth() != data.first->getHeight())
						{
							usemipmaps = true;

							std::vector<StrongRef<love::image::ImageData>> faces;
							luax_catchexcept(L, [&](){ faces = imagemodule->newCubeFaces(data.first); });

							for (int face = 0; face < (int) faces.size(); face++)
								slices.set(face, i, faces[i]);
						}
						else
							slices.set(i, 0, data.first);
					}
					else
						slices.add(data.second, i, 0, false, settings.mipmaps != Texture::MIPMAPS_NONE);
				}
			}

			lua_pop(L, tlen);
		}
	}

	return w__pushNewTexture(L, slicesref, settings);
}

int w_newArrayTexture(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture::Slices slices(TEXTURE_2D_ARRAY);
	Texture::Slices *slicesref = &slices;

	Texture::Settings settings;
	settings.type = TEXTURE_2D_ARRAY;
	bool dpiscaleset = false;

	if (lua_type(L, 1) == LUA_TNUMBER)
	{
		slicesref = nullptr;
		settings.width = (int) luaL_checkinteger(L, 1);
		settings.height = (int) luaL_checkinteger(L, 2);
		settings.layers = (int) luaL_checkinteger(L, 3);
		luax_checktexturesettings(L, 4, true, false, false, OptionalBool(), settings, dpiscaleset);
	}
	else
	{
		luax_checktexturesettings(L, 2, true, false, false, OptionalBool(), settings, dpiscaleset);
		float *autodpiscale = dpiscaleset ? nullptr : &settings.dpiScale;

		if (lua_istable(L, 1))
		{
			int tlen = std::max(1, (int) luax_objlen(L, 1));

			if (luax_isarrayoftables(L, 1))
			{
				for (int slice = 0; slice < tlen; slice++)
				{
					lua_rawgeti(L, 1, slice + 1);
					luaL_checktype(L, -1, LUA_TTABLE);

					int miplen = std::max(1, (int) luax_objlen(L, -1));

					for (int mip = 0; mip < miplen; mip++)
					{
						lua_rawgeti(L, -1, mip + 1);

						auto data = getImageData(L, -1, true, slice == 0 && mip == 0 ? autodpiscale : nullptr);
						if (data.first.get())
							slices.set(slice, mip, data.first);
						else
							slices.set(slice, mip, data.second->getSlice(0, 0));

						lua_pop(L, 1);
					}
				}
			}
			else
			{
				for (int slice = 0; slice < tlen; slice++)
				{
					lua_rawgeti(L, 1, slice + 1);
					auto data = getImageData(L, -1, true, slice == 0 ? autodpiscale : nullptr);
					if (data.first.get())
						slices.set(slice, 0, data.first);
					else
						slices.add(data.second, slice, 0, false, settings.mipmaps != Texture::MIPMAPS_NONE);
				}
			}

			lua_pop(L, tlen);
		}
		else
		{
			auto data = getImageData(L, 1, true, autodpiscale);
			if (data.first.get())
				slices.set(0, 0, data.first);
			else
				slices.add(data.second, 0, 0, true, settings.mipmaps != Texture::MIPMAPS_NONE);
		}
	}

	return w__pushNewTexture(L, slicesref, settings);
}

int w_newVolumeTexture(lua_State *L)
{
	luax_checkgraphicscreated(L);

	auto imagemodule = Module::getInstance<love::image::Image>(Module::M_IMAGE);

	Texture::Slices slices(TEXTURE_VOLUME);
	Texture::Slices *slicesref = &slices;

	Texture::Settings settings;
	settings.type = TEXTURE_VOLUME;
	bool dpiscaleset = false;

	if (lua_type(L, 1) == LUA_TNUMBER)
	{
		slicesref = nullptr;
		settings.width = (int) luaL_checkinteger(L, 1);
		settings.height = (int) luaL_checkinteger(L, 2);
		settings.layers = (int) luaL_checkinteger(L, 3);
		luax_checktexturesettings(L, 4, true, false, false, OptionalBool(), settings, dpiscaleset);
	}
	else
	{
		luax_checktexturesettings(L, 2, true, false, false, OptionalBool(), settings, dpiscaleset);
		float *autodpiscale = dpiscaleset ? nullptr : &settings.dpiScale;

		if (lua_istable(L, 1))
		{
			int tlen = std::max(1, (int) luax_objlen(L, 1));

			if (luax_isarrayoftables(L, 1))
			{
				for (int mip = 0; mip < tlen; mip++)
				{
					lua_rawgeti(L, 1, mip + 1);
					luaL_checktype(L, -1, LUA_TTABLE);

					int slicelen = std::max(1, (int) luax_objlen(L, -1));

					for (int slice = 0; slice < slicelen; slice++)
					{
						lua_rawgeti(L, -1, slice + 1);

						auto data = getImageData(L, -1, true, slice == 0 && mip == 0 ? autodpiscale : nullptr);
						if (data.first.get())
							slices.set(slice, mip, data.first);
						else
							slices.set(slice, mip, data.second->getSlice(0, 0));

						lua_pop(L, 1);
					}
				}
			}
			else
			{
				for (int layer = 0; layer < tlen; layer++)
				{
					lua_rawgeti(L, 1, layer + 1);
					auto data = getImageData(L, -1, true, layer == 0 ? autodpiscale : nullptr);
					if (data.first.get())
						slices.set(layer, 0, data.first);
					else
						slices.add(data.second, layer, 0, false, settings.mipmaps != Texture::MIPMAPS_NONE);
				}
			}

			lua_pop(L, tlen);
		}
		else
		{
			auto data = getImageData(L, 1, true, autodpiscale);

			if (data.first.get())
			{
				std::vector<StrongRef<love::image::ImageData>> layers;
				luax_catchexcept(L, [&](){ layers = imagemodule->newVolumeLayers(data.first); });

				for (int i = 0; i < (int) layers.size(); i++)
					slices.set(i, 0, layers[i]);
			}
			else
				slices.add(data.second, 0, 0, true, settings.mipmaps != Texture::MIPMAPS_NONE);
		}
	}

	return w__pushNewTexture(L, slicesref, settings);
}

int w_newImage(lua_State *L)
{
	//luax_markdeprecated(L, 1, "love.graphics.newImage", API_FUNCTION, DEPRECATED_RENAMED, "love.graphics.newTexture");
	return w_newTexture(L);
}

int w_newCubeImage(lua_State *L)
{
	//luax_markdeprecated(L, 1, "love.graphics.newCubeImage", API_FUNCTION, DEPRECATED_RENAMED, "love.graphics.newCubeTexture");
	return w_newCubeTexture(L);
}

int w_newArrayImage(lua_State *L)
{
	//luax_markdeprecated(L, 1, "love.graphics.newArrayImage", API_FUNCTION, DEPRECATED_RENAMED, "love.graphics.newArrayTexture");
	return w_newArrayTexture(L);
}

int w_newVolumeImage(lua_State *L)
{
	//luax_markdeprecated(L, 1, "love.graphics.newVolumeImage", API_FUNCTION, DEPRECATED_RENAMED, "love.graphics.newVolumeTexture");
	return w_newVolumeTexture(L);
}

int w_newTextureView(lua_State *L)
{
	Texture *base = luax_checktexture(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	Texture::ViewSettings settings;

	lua_getfield(L, 2, "format");
	if (!lua_isnoneornil(L, -1))
	{
		const char *str = luaL_checkstring(L, -1);
		if (!getConstant(str, settings.format.value))
			luax_enumerror(L, "pixel format", str);
		settings.format.hasValue = true;
	}
	lua_pop(L, 1);

	lua_getfield(L, 2, "type");
	if (!lua_isnoneornil(L, -1))
	{
		const char *str = luaL_checkstring(L, -1);
		if (!Texture::getConstant(str, settings.type.value))
			luax_enumerror(L, "texture type", Texture::getConstants(settings.type.value), str);
		settings.type.hasValue = true;
	}
	lua_pop(L, 1);

	lua_getfield(L, 2, "mipmapstart");
	if (!lua_isnoneornil(L, -1))
		settings.mipmapStart.set(luax_checkint(L, -1) - 1);
	lua_pop(L, 1);

	lua_getfield(L, 2, "mipmapcount");
	if (!lua_isnoneornil(L, -1))
		settings.mipmapCount.set(luax_checkint(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, 2, "layerstart");
	if (!lua_isnoneornil(L, -1))
		settings.layerStart.set(luax_checkint(L, -1) - 1);
	lua_pop(L, 1);

	lua_getfield(L, 2, "layers");
	if (!lua_isnoneornil(L, -1))
		settings.layerCount.set(luax_checkint(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, 2, "debugname");
	if (!lua_isnoneornil(L, -1))
		settings.debugName = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	Texture *t = nullptr;
	luax_catchexcept(L, [&]() { t = instance()->newTextureView(base, settings); });

	luax_pushtype(L, t);
	t->release();
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

	double sw = 0.0f;
	double sh = 0.0f;
	int layer = 0;

	if (luax_istype(L, 5, Texture::type))
	{
		Texture *texture = luax_checktexture(L, 5);
		sw = texture->getWidth();
		sh = texture->getHeight();
	}
	else if (luax_istype(L, 6, Texture::type))
	{
		layer = (int) luaL_checkinteger(L, 5) - 1;
		Texture *texture = luax_checktexture(L, 6);
		sw = texture->getWidth();
		sh = texture->getHeight();
	}
	else if (!lua_isnoneornil(L, 7))
	{
		layer = (int) luaL_checkinteger(L, 5) - 1;
		sw = luaL_checknumber(L, 6);
		sh = luaL_checknumber(L, 7);
	}
	else
	{
		sw = luaL_checknumber(L, 5);
		sh = luaL_checknumber(L, 6);
	}

	Quad *quad = instance()->newQuad(v, sw, sh);
	quad->setLayer(layer);

	luax_pushtype(L, quad);
	quad->release();
	return 1;
}

int w_newFont(lua_State *L)
{
	luax_checkgraphicscreated(L);

	graphics::Font *font = nullptr;

	// Convert to Rasterizer, if necessary.
	if (!luax_istype(L, 1, love::font::Rasterizer::type))
	{
		std::vector<int> idxs;
		for (int i = 0; i < lua_gettop(L); i++)
			idxs.push_back(i + 1);

		luax_convobj(L, idxs, "font", "newRasterizer");
	}

	love::font::Rasterizer *rasterizer = luax_checktype<love::font::Rasterizer>(L, 1);

	luax_catchexcept(L, [&]() {
		font = instance()->newFont(rasterizer); }
	);

	// Push the type.
	luax_pushtype(L, font);
	font->release();
	return 1;
}

int w_newImageFont(lua_State *L)
{
	luax_checkgraphicscreated(L);

	// Convert to Rasterizer if necessary.
	if (!luax_istype(L, 1, love::font::Rasterizer::type))
	{
		luaL_checktype(L, 2, LUA_TSTRING);

		std::vector<int> idxs;
		for (int i = 0; i < lua_gettop(L); i++)
			idxs.push_back(i + 1);

		luax_convobj(L, idxs, "font", "newImageRasterizer");
	}

	love::font::Rasterizer *rasterizer = luax_checktype<love::font::Rasterizer>(L, 1);

	// Create the font.
	Font *font = instance()->newFont(rasterizer);

	// Push the type.
	luax_pushtype(L, font);
	font->release();
	return 1;
}

int w_newSpriteBatch(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture *texture = luax_checktexture(L, 1);
	int size = (int) luaL_optinteger(L, 2, 1000);
	BufferDataUsage usage = BUFFERDATAUSAGE_DYNAMIC;
	if (lua_gettop(L) > 2)
	{
		const char *usagestr = luaL_checkstring(L, 3);
		if (!getConstant(usagestr, usage))
			return luax_enumerror(L, "usage hint", getConstants(usage), usagestr);
	}

	SpriteBatch *t = nullptr;
	luax_catchexcept(L,
		[&](){ t = instance()->newSpriteBatch(texture, size, usage); }
	);

	luax_pushtype(L, t);
	t->release();
	return 1;
}

int w_newParticleSystem(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Texture *texture = luax_checktexture(L, 1);
	lua_Number size = luaL_optnumber(L, 2, 1000);
	ParticleSystem *t = nullptr;
	if (size < 1.0 || size > ParticleSystem::MAX_PARTICLES)
		return luaL_error(L, "Invalid ParticleSystem size");

	luax_catchexcept(L,
		[&](){ t = instance()->newParticleSystem(texture, int(size)); }
	);

	luax_pushtype(L, t);
	t->release();
	return 1;
}

static int w_getShaderSource(lua_State *L, int startidx, std::vector<std::string> &stages, Shader::CompileOptions &options)
{
	using namespace love::filesystem;

	luax_checkgraphicscreated(L);

	auto fs = Module::getInstance<Filesystem>(Module::M_FILESYSTEM);

	// read any filepath arguments
	for (int i = startidx; i < startidx + 2; i++)
	{
		if (!lua_isstring(L, i))
		{
			if (luax_cangetfiledata(L, i))
			{
				FileData *fd = luax_getfiledata(L, i);

				lua_pushlstring(L, (const char *) fd->getData(), fd->getSize());
				fd->release();

				lua_replace(L, i);
			}
			continue;
		}

		size_t slen = 0;
		const char *str = lua_tolstring(L, i, &slen);

		Filesystem::Info info = {};
		if (fs != nullptr && fs->getInfo(str, info))
		{
			FileData *fd = nullptr;
			luax_catchexcept(L, [&](){ fd = fs->read(str); });

			lua_pushlstring(L, (const char *) fd->getData(), fd->getSize());
			fd->release();

			lua_replace(L, i);
		}
		else
		{
			// Check if the argument looks like a filepath - we want a nicer
			// error for misspelled filepath arguments.
			if (slen > 0 && slen < 64 && !strchr(str, '\n'))
			{
				const char *ext = strchr(str, '.');
				if (ext != nullptr && !strchr(ext, ';') && !strchr(ext, ' '))
					return luaL_error(L, "Could not open file %s. Does not exist.", str);
			}
		}
	}

	bool has_arg1 = lua_isstring(L, startidx + 0) != 0;
	bool has_arg2 = lua_isstring(L, startidx + 1) != 0;

	// require at least one string argument
	if (!(has_arg1 || has_arg2))
		luaL_checkstring(L, startidx);

	if (has_arg1)
		stages.push_back(luax_checkstring(L, startidx + 0));
	if (has_arg2)
		stages.push_back(luax_checkstring(L, startidx + 1));

	int optionsidx = has_arg2 ? startidx + 2 : startidx + 1;
	if (!lua_isnoneornil(L, optionsidx))
	{
		luaL_checktype(L, optionsidx, LUA_TTABLE);
		lua_getfield(L, optionsidx, "defines");
		if (!lua_isnoneornil(L, -1))
		{
			if (!lua_istable(L, -1))
				luaL_argerror(L, optionsidx, "expected 'defines' field to be a table");

			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				std::string defname;
				std::string defval;

				if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TSTRING)
					defname = luaL_checkstring(L, -1);
				else if (lua_type(L, -2) != LUA_TSTRING)
					luaL_argerror(L, optionsidx, "all fields in the 'defines' table must use string keys.");
				else
				{
					defname = luaL_checkstring(L, -2);
					if (lua_type(L, -1) == LUA_TBOOLEAN)
						defval = luax_toboolean(L, -1) ? "1" : "0";
					else
					{
						const char *val = lua_tostring(L, -1);
						if (val == nullptr)
							luaL_argerror(L, optionsidx, "'defines' table values must be strings, numbers, or booleans.");
						defval = val;
					}
				}

				options.defines[defname] = defval;

				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, optionsidx, "debugname");
		if (!lua_isnoneornil(L, -1))
			options.debugName = luax_checkstring(L, -1);
		lua_pop(L, 1);
	}

	return 0;
}

int w_newShader(lua_State *L)
{
	std::vector<std::string> stages;
	Shader::CompileOptions options;
	w_getShaderSource(L, 1, stages, options);

	bool should_error = false;
	try
	{
		Shader *shader = instance()->newShader(stages, options);
		if (shader->isUsingDeprecatedTextureFunctions())
			luax_markdeprecated(L, 1, "texture2D() or textureCube() function calls in shader code", API_CUSTOM, DEPRECATED_REPLACED, "texture() function calls");
		if (shader->isUsingDeprecatedTextureUniform())
			luax_markdeprecated(L, 1, "'texture' uniform variable name in shader code", API_CUSTOM, DEPRECATED_NO_REPLACEMENT, "");
		if (!shader->getUnsetVertexInputLocationsString().empty())
		{
			std::string str = "vertex input attribute(s) " + shader->getUnsetVertexInputLocationsString() + " without a 'location' layout qualifier in shader code";
			luax_markdeprecated(L, 1, str.c_str(), API_CUSTOM, DEPRECATED_REPLACED, "layout(location = #) qualifier for vertex inputs");
		}
		luax_pushtype(L, shader);
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

int w_newComputeShader(lua_State* L)
{
	std::vector<std::string> stages;
	Shader::CompileOptions options;
	w_getShaderSource(L, 1, stages, options);

	bool should_error = false;
	try
	{
		Shader *shader = instance()->newComputeShader(stages[0], options);
		if (shader->isUsingDeprecatedTextureFunctions())
			luax_markdeprecated(L, 1, "texture2D() or textureCube() function calls in shader code", API_CUSTOM, DEPRECATED_REPLACED, "texture() function calls");
		if (shader->isUsingDeprecatedTextureUniform())
			luax_markdeprecated(L, 1, "'texture' uniform variable name in shader code", API_CUSTOM, DEPRECATED_NO_REPLACEMENT, "");
		if (!shader->getUnsetVertexInputLocationsString().empty())
		{
			std::string str = "vertex input attribute(s) " + shader->getUnsetVertexInputLocationsString() + " without a 'location' layout qualifier in shader code";
			luax_markdeprecated(L, 1, str.c_str(), API_CUSTOM, DEPRECATED_REPLACED, "layout(location = #) qualifier for vertex inputs");
		}
		luax_pushtype(L, shader);
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

int w_validateShader(lua_State *L)
{
	bool gles = luax_checkboolean(L, 1);

	std::vector<std::string> stages;
	Shader::CompileOptions options;
	w_getShaderSource(L, 2, stages, options);

	bool success = true;
	std::string err;
	try
	{
		success = instance()->validateShader(gles, stages, options, err);
	}
	catch (love::Exception &e)
	{
		success = false;
		err = e.what();
	}

	luax_pushboolean(L, success);

	if (!success)
	{
		luax_pushstring(L, err);
		return 2;
	}

	return 1;
}

static BufferDataUsage luax_optdatausage(lua_State *L, int idx, BufferDataUsage def)
{
	const char *usagestr = lua_isnoneornil(L, idx) ? nullptr : luaL_checkstring(L, idx);

	if (usagestr && !getConstant(usagestr, def))
		luax_enumerror(L, "usage hint", getConstants(def), usagestr);

	return def;
}

static void luax_optbuffersettings(lua_State *L, int idx, Buffer::Settings &settings)
{
	if (lua_isnoneornil(L, idx))
		return;

	luaL_checktype(L, idx, LUA_TTABLE);

	lua_getfield(L, idx, "usage");
	settings.dataUsage = luax_optdatausage(L, -1, settings.dataUsage);
	lua_pop(L, 1);

	lua_getfield(L, idx, "debugname");
	if (!lua_isnoneornil(L, -1))
		settings.debugName = luax_checkstring(L, -1);
	lua_pop(L, 1);
}

static Buffer::DataDeclaration luax_checkdatadeclaration(lua_State* L, int formattableidx, int arrayindex, int declindex, bool requirename, bool requirelocation)
{
	Buffer::DataDeclaration decl("", DATAFORMAT_MAX_ENUM);

	lua_getfield(L, declindex, "name");
	if (requirename && lua_type(L, -1) != LUA_TSTRING)
	{
		std::ostringstream ss;
		ss << "'name' field expected in array element #";
		ss << arrayindex;
		ss << " of format table";
		std::string str = ss.str();
		luaL_argerror(L, formattableidx, str.c_str());
	}
	else if (!lua_isnoneornil(L, -1))
		decl.name = luax_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, declindex, "format");
	if (lua_type(L, -1) != LUA_TSTRING)
	{
		std::ostringstream ss;
		ss << "'format' field expected in array element #";
		ss << arrayindex;
		ss << " of format table";
		std::string str = ss.str();
		luaL_argerror(L, formattableidx, str.c_str());
	}
	const char* formatstr = luaL_checkstring(L, -1);
	if (!getConstant(formatstr, decl.format))
		luax_enumerror(L, "data format", getConstants(decl.format), formatstr);
	lua_pop(L, 1);

	decl.arrayLength = luax_intflag(L, declindex, "arraylength", 0);

	lua_getfield(L, declindex, "location");
	if (requirelocation && lua_type(L, -1) != LUA_TNUMBER)
	{
		std::ostringstream ss;
		ss << "'location' field expected in array element #";
		ss << arrayindex;
		ss << " of format table";
		std::string str = ss.str();
		luaL_argerror(L, formattableidx, str.c_str());
	}
	else if (!lua_isnoneornil(L, -1))
		decl.bindingLocation = luax_checkint(L, -1);
	lua_pop(L, 1);

	return decl;
}

static void luax_checkbufferformat(lua_State *L, int idx, const Buffer::Settings &settings, std::vector<Buffer::DataDeclaration> &format)
{
	if (lua_type(L, idx) == LUA_TSTRING)
	{
		Buffer::DataDeclaration decl("", DATAFORMAT_MAX_ENUM);
		const char *formatstr = luaL_checkstring(L, idx);
		if (!getConstant(formatstr, decl.format))
			luax_enumerror(L, "data format", getConstants(decl.format), formatstr);
		format.push_back(decl);
		return;
	}

	bool requirelocation = (settings.usageFlags & BUFFERUSAGE_VERTEX) != 0;

	luaL_checktype(L, idx, LUA_TTABLE);
	int tablelen = luax_objlen(L, idx);

	for (int i = 1; i <= tablelen; i++)
	{
		lua_rawgeti(L, idx, i);
		luaL_checktype(L, -1, LUA_TTABLE);

		Buffer::DataDeclaration decl = luax_checkdatadeclaration(L, idx, i, -1, false, requirelocation);

		format.push_back(decl);
		lua_pop(L, 1);
	}
}

static void luax_validatebuffervertexbindings(lua_State *L, Buffer *buffer)
{
	if (buffer->hasLegacyVertexBindings())
	{
		std::string names;

		for (const auto &member : buffer->getDataMembers())
		{
			if (member.decl.bindingLocation < 0)
			{
				if (names.empty())
					names = member.decl.name;
				else
					names += ", " + member.decl.name;
			}
		}

		luax_markdeprecated(L, 1, "vertex format 'name' fields in Meshes and Buffers", API_CUSTOM, DEPRECATED_REPLACED, "'location' field containing a binding location number value.");
	}
}

static Buffer *luax_newbuffer(lua_State *L, int idx, Buffer::Settings settings, const std::vector<Buffer::DataDeclaration> &format)
{
	size_t arraylength = 0;
	size_t bytesize = 0;
	Data *data = nullptr;
	const void *initialdata = nullptr;

	int ncomponents = 0;
	for (const Buffer::DataDeclaration &decl : format)
		ncomponents += getDataFormatInfo(decl.format).components;

	if (luax_istype(L, idx, Data::type))
	{
		data = luax_checktype<Data>(L, idx);
		initialdata = data->getData();
		bytesize = data->getSize();
	}

	bool tableoftables = false;

	if (lua_istable(L, idx))
	{
		arraylength = luax_objlen(L, idx);

		lua_rawgeti(L, idx, 1);
		tableoftables = lua_istable(L, -1);
		lua_pop(L, 1);

		if (!tableoftables)
		{
			if (arraylength % ncomponents != 0)
				luaL_error(L, "Array length in flat array variant of newBuffer must be a multiple of the total number of components (%d)", ncomponents);
			arraylength /= ncomponents;
		}
	}
	else if (data == nullptr)
	{
		lua_Integer len = luaL_checkinteger(L, idx);
		if (len <= 0)
			luaL_argerror(L, idx, "number of elements must be greater than 0");
		arraylength = (size_t) len;
		settings.zeroInitialize = true;
	}

	Buffer *b = nullptr;
	luax_catchexcept(L, [&] { b = instance()->newBuffer(settings, format, initialdata, bytesize, arraylength); });

	luax_validatebuffervertexbindings(L, b);

	if (lua_istable(L, idx))
	{
		Buffer::Mapper mapper(*b);
		char *data = (char *) mapper.data;
		const auto &members = b->getDataMembers();
		size_t stride = b->getArrayStride();

		if (tableoftables)
		{
			for (size_t i = 0; i < arraylength; i++)
			{
				// get arraydata[index]
				lua_rawgeti(L, 2, i + 1);
				luaL_checktype(L, -1, LUA_TTABLE);

				// get arraydata[index][j]
				for (int j = 1; j <= ncomponents; j++)
					lua_rawgeti(L, -j, j);

				int idx = -ncomponents;

				for (const Buffer::DataMember &member : members)
				{
					luax_writebufferdata(L, idx, member.decl.format, data + member.offset);
					idx += member.info.components;
				}

				lua_pop(L, ncomponents + 1);
				data += stride;
			}
		}
		else // Flat array
		{
			for (size_t i = 0; i < arraylength; i++)
			{
				// get arraydata[arrayindex * ncomponents + componentindex]
				for (int componentindex = 1; componentindex <= ncomponents; componentindex++)
					lua_rawgeti(L, 2, i * ncomponents + componentindex);

				int idx = -ncomponents;

				for (const Buffer::DataMember &member : members)
				{
					luax_writebufferdata(L, idx, member.decl.format, data + member.offset);
					idx += member.info.components;
				}

				lua_pop(L, ncomponents);
				data += stride;
			}
		}
	}

	return b;
}

int w_newBuffer(lua_State *L)
{
	Buffer::Settings settings(0, BUFFERDATAUSAGE_DYNAMIC);

	luaL_checktype(L, 3, LUA_TTABLE);

	for (int i = 0; i < BUFFERUSAGE_MAX_ENUM; i++)
	{
		BufferUsage bufferusage = (BufferUsage) i;
		const char *tname = nullptr;
		if (!getConstant(bufferusage, tname))
			continue;
		if (luax_boolflag(L, 3, tname, false))
			settings.usageFlags = (BufferUsageFlags)(settings.usageFlags | (1u << i));
	}

	luax_optbuffersettings(L, 3, settings);

	std::vector<Buffer::DataDeclaration> format;
	luax_checkbufferformat(L, 1, settings, format);

	Buffer *b = luax_newbuffer(L, 2, settings, format);

	luax_pushtype(L, b);
	b->release();
	return 1;
}

static PrimitiveType luax_checkmeshdrawmode(lua_State *L, int idx)
{
	const char *modestr = luaL_checkstring(L, idx);

	PrimitiveType mode = PRIMITIVE_TRIANGLES;
	if (!getConstant(modestr, mode))
		luax_enumerror(L, "mesh draw mode", getConstants(mode), modestr);

	return mode;
}

static Mesh *newStandardMesh(lua_State *L)
{
	Mesh *t = nullptr;

	PrimitiveType drawmode = luax_checkmeshdrawmode(L, 2);
	BufferDataUsage usage = luax_optdatausage(L, 3, BUFFERDATAUSAGE_DYNAMIC);

	std::vector<Buffer::DataDeclaration> format = Mesh::getDefaultVertexFormat();

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

			v.color.r = (unsigned char) (luax_optnumberclamped01(L, -4, 1.0) * 255.0);
			v.color.g = (unsigned char) (luax_optnumberclamped01(L, -3, 1.0) * 255.0);
			v.color.b = (unsigned char) (luax_optnumberclamped01(L, -2, 1.0) * 255.0);
			v.color.a = (unsigned char) (luax_optnumberclamped01(L, -1, 1.0) * 255.0);

			lua_pop(L, 9);
			vertices.push_back(v);
		}

		luax_catchexcept(L, [&](){ t = instance()->newMesh(format, vertices.data(), vertices.size() * sizeof(Vertex), drawmode, usage); });
	}
	else
	{
		int count = (int) luaL_checkinteger(L, 1);
		luax_catchexcept(L, [&](){ t = instance()->newMesh(format, count, drawmode, usage); });
	}

	return t;
}

static Mesh *newCustomMesh(lua_State *L)
{
	Mesh *t = nullptr;

	// First argument is the vertex format, second is a table of vertices or
	// the number of vertices.
	std::vector<Buffer::DataDeclaration> vertexformat;

	PrimitiveType drawmode = luax_checkmeshdrawmode(L, 3);
	BufferDataUsage usage = luax_optdatausage(L, 4, BUFFERDATAUSAGE_DYNAMIC);

	lua_rawgeti(L, 1, 1);
	if (!lua_istable(L, -1))
	{
		luaL_argerror(L, 1, "table of tables expected");
		return nullptr;
	}
	lua_pop(L, 1);

	// Per-vertex attribute data formats.
	for (int i = 1; i <= (int) luax_objlen(L, 1); i++)
	{
		lua_rawgeti(L, 1, i);

		Buffer::DataDeclaration decl("", DATAFORMAT_MAX_ENUM);

		lua_getfield(L, -1, "format");
		bool hasformatfield = !lua_isnoneornil(L, -1);
		lua_pop(L, 1);

		if (hasformatfield || luax_objlen(L, -1) == 0)
			decl = luax_checkdatadeclaration(L, 1, i, -1, false, true);
		else
		{
			// Legacy format arguments: {name, datatype, components}
			for (int j = 1; j <= 3; j++)
				lua_rawgeti(L, -j, j);

			decl.name = luaL_checkstring(L, -3);
			const char *tname = luaL_checkstring(L, -2);
			int components = (int)luaL_checkinteger(L, -1);

			// Check deprecated format names.
			if (strcmp(tname, "byte") == 0 || strcmp(tname, "unorm8") == 0)
			{
				if (components == 4)
					decl.format = DATAFORMAT_UNORM8_VEC4;
				else
					luaL_error(L, "Invalid component count (%d) for vertex data type %s", components, tname);
			}
			else if (strcmp(tname, "unorm16") == 0)
			{
				if (components == 2)
					decl.format = DATAFORMAT_UNORM16_VEC2;
				else if (components == 4)
					decl.format = DATAFORMAT_UNORM16_VEC4;
				else
					luaL_error(L, "Invalid component count (%d) for vertex data type %s", components, tname);

			}
			else if (strcmp(tname, "float") == 0)
			{
				if (components == 1)
					decl.format = DATAFORMAT_FLOAT;
				else if (components == 2)
					decl.format = DATAFORMAT_FLOAT_VEC2;
				else if (components == 3)
					decl.format = DATAFORMAT_FLOAT_VEC3;
				else if (components == 4)
					decl.format = DATAFORMAT_FLOAT_VEC4;
				else
					luaL_error(L, "Invalid component count (%d) for vertex data type %s", components, tname);
			}

			if (decl.format == DATAFORMAT_MAX_ENUM)
				luax_enumerror(L, "vertex data type", {"float", "byte", "unorm8", "unorm16"}, tname);

			lua_pop(L, 3);

			luax_markdeprecated(L, 1, "vertex format array values in love.graphics.newMesh", API_CUSTOM, DEPRECATED_REPLACED, "named table fields 'format' and 'location'");
		}

		lua_pop(L, 1);
		vertexformat.push_back(decl);
	}

	if (lua_isnumber(L, 2))
	{
		int vertexcount = (int) luaL_checkinteger(L, 2);
		luax_catchexcept(L, [&](){ t = instance()->newMesh(vertexformat, vertexcount, drawmode, usage); });
	}
	else if (luax_istype(L, 2, Data::type))
	{
		// Vertex data comes directly from a Data object.
		Data *data = luax_checktype<Data>(L, 2);
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

		size_t numvertices = luax_objlen(L, 2);

		luax_catchexcept(L, [&](){ t = instance()->newMesh(vertexformat, numvertices, drawmode, usage); });

		char *data = (char *) t->getVertexData();
		size_t stride = t->getVertexStride();
		const auto &members = t->getVertexFormat();

		for (size_t vertindex = 0; vertindex < numvertices; vertindex++)
		{
			// get vertices[vertindex]
			lua_rawgeti(L, 2, vertindex + 1);
			luaL_checktype(L, -1, LUA_TTABLE);

			int n = 0;
			for (size_t i = 0; i < vertexformat.size(); i++)
			{
				const auto &member = members[i];
				const auto &info = getDataFormatInfo(member.decl.format);

				// get vertices[vertindex][n]
				for (int c = 0; c < info.components; c++)
				{
					n++;
					lua_rawgeti(L, -(c + 1), n);
				}

				size_t offset = vertindex * stride + member.offset;

				// Fetch the values from Lua and store them in data buffer.
				luax_writebufferdata(L, -info.components, member.decl.format, data + offset);

				lua_pop(L, info.components);
			}

			lua_pop(L, 1); // pop vertices[vertindex]
		}

		t->setVertexDataModified(0, stride * numvertices);
		t->flush();
	}

	if (t->getVertexBuffer() != nullptr)
		luax_validatebuffervertexbindings(L, t->getVertexBuffer());

	return t;
}

static bool luax_isbufferattributetable(lua_State* L, int idx)
{
	if (lua_type(L, idx) != LUA_TTABLE)
		return false;

	lua_rawgeti(L, idx, 1);
	if (lua_type(L, -1) != LUA_TTABLE)
	{
		lua_pop(L, 1);
		return false;
	}

	lua_getfield(L, -1, "buffer");
	bool isbuffer = luax_istype(L, -1, Buffer::type);
	lua_pop(L, 2);
	return isbuffer;
}

static Mesh::BufferAttribute luax_checkbufferattributetable(lua_State *L, int idx)
{
	Mesh::BufferAttribute attrib;

	attrib.step = STEP_PER_VERTEX;
	attrib.enabled = true;

	lua_getfield(L, idx, "buffer");
	attrib.buffer = luax_checkbuffer(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, idx, "location");
	attrib.bindingLocation = luax_checkint(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, idx, "name");
	if (!lua_isnoneornil(L, -1))
		attrib.name = luax_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, idx, "step");
	if (!lua_isnoneornil(L, -1))
	{
		const char *stepstr = luaL_checkstring(L, -1);
		if (!getConstant(stepstr, attrib.step))
			luax_enumerror(L, "vertex attribute step", getConstants(attrib.step), stepstr);
	}
	lua_pop(L, 1);

	lua_getfield(L, idx, "locationinbuffer");
	if (!lua_isnoneornil(L, -1))
		attrib.bindingLocationInBuffer = luax_checkint(L, -1);
	else
		attrib.bindingLocationInBuffer = attrib.bindingLocation;
	lua_pop(L, 1);

	lua_getfield(L, idx, "nameinbuffer");	
	if (!lua_isnoneornil(L, -1))
		attrib.nameInBuffer = luax_checkstring(L, -1);
	else
		attrib.nameInBuffer = attrib.name;
	lua_pop(L, 1);

	lua_getfield(L, idx, "startindex");
	attrib.startArrayIndex = (int) luaL_optinteger(L, -1, 1) - 1;
	lua_pop(L, 1);

	return attrib;
}

static Mesh* newMeshFromBuffers(lua_State *L)
{
	std::vector<Mesh::BufferAttribute> attributes;
	for (size_t i = 1; i <= luax_objlen(L, 1); i++)
	{
		lua_rawgeti(L, 1, i);
		attributes.push_back(luax_checkbufferattributetable(L, -1));
		lua_pop(L, 1);
	}

	PrimitiveType drawmode = luax_checkmeshdrawmode(L, 2);

	Mesh *t = nullptr;
	luax_catchexcept(L, [&]() { t = instance()->newMesh(attributes, drawmode); });

	if (t->getVertexBuffer() != nullptr)
		luax_validatebuffervertexbindings(L, t->getVertexBuffer());

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
	if (luax_isbufferattributetable(L, 1))
		t = newMeshFromBuffers(L);
	else if (arg1type == LUA_TTABLE && (arg2type == LUA_TTABLE || arg2type == LUA_TNUMBER || arg2type == LUA_TUSERDATA))
		t = newCustomMesh(L);
	else
		t = newStandardMesh(L);

	luax_pushtype(L, t);
	t->release();
	return 1;
}

int w_newTextBatch(lua_State *L)
{
	luax_checkgraphicscreated(L);

	graphics::Font *font = luax_checkfont(L, 1);
	TextBatch *t = nullptr;

	if (lua_isnoneornil(L, 2))
		luax_catchexcept(L, [&](){ t = instance()->newTextBatch(font); });
	else
	{
		std::vector<love::font::ColoredString> text;
		luax_checkcoloredstring(L, 2, text);

		luax_catchexcept(L, [&](){ t = instance()->newTextBatch(font, text); });
	}

	luax_pushtype(L, t);
	t->release();
	return 1;
}

int w_newText(lua_State *L)
{
	luax_markdeprecated(L, 1, "love.graphics.newText", API_FUNCTION, DEPRECATED_RENAMED, "love.graphics.newTextBatch");
	return w_newTextBatch(L);
}

int w_newVideo(lua_State *L)
{
	luax_checkgraphicscreated(L);

	if (!luax_istype(L, 1, love::video::VideoStream::type))
		luax_convobj(L, 1, "video", "newVideoStream");

	auto stream = luax_checktype<love::video::VideoStream>(L, 1);
	float dpiscale = (float) luaL_optnumber(L, 2, 1.0);
	Video *video = nullptr;

	luax_catchexcept(L, [&]() { video = instance()->newVideo(stream, dpiscale); });

	luax_pushtype(L, video);
	video->release();
	return 1;
}

int w_readbackBuffer(lua_State *L)
{
	Buffer *b = luax_checkbuffer(L, 1);
	lua_Integer offset = luaL_optinteger(L, 2, 0);
	lua_Integer size = luaL_optinteger(L, 3, b->getSize() - offset);

	data::ByteData *dest = nullptr;
	size_t destoffset = 0;
	if (!lua_isnoneornil(L, 4))
	{
		dest = luax_checktype<data::ByteData>(L, 4);
		destoffset = (size_t) luaL_optinteger(L, 5, 0);
	}

	love::data::ByteData *data = nullptr;
	luax_catchexcept(L, [&]() { data = instance()->readbackBuffer(b, offset, size, dest, destoffset); });

	luax_pushtype(L, data);
	data->release();
	return 1;
}

int w_readbackBufferAsync(lua_State *L)
{
	Buffer *b = luax_checkbuffer(L, 1);
	lua_Integer offset = luaL_optinteger(L, 2, 0);
	lua_Integer size = luaL_optinteger(L, 3, b->getSize() - offset);

	data::ByteData *dest = nullptr;
	size_t destoffset = 0;
	if (!lua_isnoneornil(L, 4))
	{
		dest = luax_checktype<data::ByteData>(L, 4);
		destoffset = (size_t) luaL_optinteger(L, 5, 0);
	}

	GraphicsReadback *r = nullptr;
	luax_catchexcept(L, [&]() { r = instance()->readbackBufferAsync(b, offset, size, dest, destoffset); });

	luax_pushtype(L, r);
	r->release();
	return 1;
}

int w_readbackTexture(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);

	int slice = 0;
	if (t->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 2) - 1;

	int mipmap = (int) luaL_optinteger(L, 3, 1) - 1;

	Rect rect = {0, 0, t->getPixelWidth(mipmap), t->getPixelHeight(mipmap)};
	if (!lua_isnoneornil(L, 4))
	{
		rect.x = (int) luaL_checkinteger(L, 4);
		rect.y = (int) luaL_checkinteger(L, 5);
		rect.w = (int) luaL_checkinteger(L, 6);
		rect.h = (int) luaL_checkinteger(L, 7);
	}

	image::ImageData *dest = nullptr;
	int destx = 0;
	int desty = 0;

	if (!lua_isnoneornil(L, 8))
	{
		dest = luax_checktype<image::ImageData>(L, 8);
		destx = (int) luaL_optinteger(L, 9, 0);
		desty = (int) luaL_optinteger(L, 10, 0);
	}

	image::ImageData *imagedata = nullptr;
	luax_catchexcept(L, [&]() { imagedata = instance()->readbackTexture(t, slice, mipmap, rect, dest, destx, desty); });

	luax_pushtype(L, imagedata);
	imagedata->release();
	return 1;
}

int w_readbackTextureAsync(lua_State *L)
{
	Texture *t = luax_checktexture(L, 1);

	int slice = 0;
	if (t->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 2) - 1;

	int mipmap = (int) luaL_optinteger(L, 3, 1) - 1;

	Rect rect = {0, 0, t->getPixelWidth(mipmap), t->getPixelHeight(mipmap)};
	if (!lua_isnoneornil(L, 4))
	{
		rect.x = (int) luaL_checkinteger(L, 4);
		rect.y = (int) luaL_checkinteger(L, 5);
		rect.w = (int) luaL_checkinteger(L, 6);
		rect.h = (int) luaL_checkinteger(L, 7);
	}

	image::ImageData *dest = nullptr;
	int destx = 0;
	int desty = 0;

	if (!lua_isnoneornil(L, 8))
	{
		dest = luax_checktype<image::ImageData>(L, 8);
		destx = (int) luaL_optinteger(L, 9, 0);
		desty = (int) luaL_optinteger(L, 10, 0);
	}

	GraphicsReadback *r = nullptr;
	luax_catchexcept(L, [&]() { r = instance()->readbackTextureAsync(t, slice, mipmap, rect, dest, destx, desty); });

	luax_pushtype(L, r);
	r->release();
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
		c.a = (float) luaL_optnumber(L, -1, 1.0);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (float) luaL_checknumber(L, 1);
		c.g = (float) luaL_checknumber(L, 2);
		c.b = (float) luaL_checknumber(L, 3);
		c.a = (float) luaL_optnumber(L, 4, 1.0);
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
		c.a = (float) luaL_optnumber(L, -1, 1.0);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (float) luaL_checknumber(L, 1);
		c.g = (float) luaL_checknumber(L, 2);
		c.b = (float) luaL_checknumber(L, 3);
		c.a = (float) luaL_optnumber(L, 4, 1.0);
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
	luax_markdeprecated(L, 1, "love.graphics.setNewFont", API_FUNCTION, DEPRECATED_NO_REPLACEMENT, nullptr);

	int ret = w_newFont(L);
	Font *font = luax_checktype<Font>(L, -1);
	instance()->setFont(font);
	return ret;
}

int w_setFont(lua_State *L)
{
	Font *font = luax_checktype<Font>(L, 1);
	instance()->setFont(font);
	return 0;
}

int w_getFont(lua_State *L)
{
	luax_checkgraphicscreated(L);

	Font *f = nullptr;
	luax_catchexcept(L, [&](){ f = instance()->getFont(); });

	luax_pushtype(L, f);
	return 1;
}

int w_setColorMask(lua_State *L)
{
	ColorChannelMask mask;

	if (lua_isnoneornil(L, 1))
	{
		mask.r = mask.g = mask.b = mask.a = true;
	}
	else if (lua_gettop(L) <= 1)
	{
		// Set all color components if a single argument is given.
		mask.r = mask.g = mask.b = mask.a = luax_checkboolean(L, 1);
	}
	else
	{
		mask.r = luax_checkboolean(L, 1);
		mask.g = luax_checkboolean(L, 2);
		mask.b = luax_checkboolean(L, 3);
		mask.a = luax_checkboolean(L, 4);
	}

	instance()->setColorMask(mask);

	return 0;
}

int w_getColorMask(lua_State *L)
{
	ColorChannelMask mask = instance()->getColorMask();

	luax_pushboolean(L, mask.r);
	luax_pushboolean(L, mask.g);
	luax_pushboolean(L, mask.b);
	luax_pushboolean(L, mask.a);

	return 4;
}

int w_setBlendMode(lua_State *L)
{
	BlendMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!getConstant(str, mode))
		return luax_enumerror(L, "blend mode", getConstants(mode), str);

	BlendAlpha alphamode = BLENDALPHA_MULTIPLY;
	if (!lua_isnoneornil(L, 2))
	{
		const char *alphastr = luaL_checkstring(L, 2);
		if (!getConstant(alphastr, alphamode))
			return luax_enumerror(L, "blend alpha mode", getConstants(alphamode), alphastr);
	}

	luax_catchexcept(L, [&](){ instance()->setBlendMode(mode, alphamode); });
	return 0;
}

int w_getBlendMode(lua_State *L)
{
	const char *str;
	const char *alphastr;

	BlendAlpha alphamode;
	BlendMode mode = instance()->getBlendMode(alphamode);

	if (!getConstant(mode, str))
		return luaL_error(L, "Unknown blend mode");

	if (!getConstant(alphamode, alphastr))
		return luaL_error(L, "Unknown blend alpha mode");

	lua_pushstring(L, str);
	lua_pushstring(L, alphastr);
	return 2;
}

static BlendOperation luax_checkblendop(lua_State *L, int idx)
{
	BlendOperation op = BLENDOP_ADD;
	const char *str = luaL_checkstring(L, idx);
	if (!getConstant(str, op))
		luax_enumerror(L, "blend operation", getConstants(op), str);
	return op;
}

static BlendFactor luax_checkblendfactor(lua_State *L, int idx)
{
	BlendFactor factor = BLENDFACTOR_ZERO;
	const char *str = luaL_checkstring(L, idx);
	if (!getConstant(str, factor))
		luax_enumerror(L, "blend factor", getConstants(factor), str);
	return factor;
}

static void luax_pushblendop(lua_State *L, BlendOperation op)
{
	const char *str;
	if (!getConstant(op, str))
		luaL_error(L, "unknown blend operation");
	lua_pushstring(L, str);
}

static void luax_pushblendfactor(lua_State *L, BlendFactor factor)
{
	const char *str;
	if (!getConstant(factor, str))
		luaL_error(L, "unknown blend factor");
	lua_pushstring(L, str);
}

int w_setBlendState(lua_State *L)
{
	BlendState state;

	if (!lua_isnoneornil(L, 1))
	{
		state.enable = true;
		if (lua_gettop(L) >= 4)
		{
			state.operationRGB = luax_checkblendop(L, 1);
			state.operationA = luax_checkblendop(L, 2);
			state.srcFactorRGB = luax_checkblendfactor(L, 3);
			state.srcFactorA = luax_checkblendfactor(L, 4);
			state.dstFactorRGB = luax_checkblendfactor(L, 5);
			state.dstFactorA = luax_checkblendfactor(L, 6);
		}
		else
		{
			state.operationRGB = state.operationA = luax_checkblendop(L, 1);
			state.srcFactorRGB = state.srcFactorA = luax_checkblendfactor(L, 2);
			state.dstFactorRGB = state.dstFactorA = luax_checkblendfactor(L, 3);
		}
	}

	luax_catchexcept(L, [&](){ instance()->setBlendState(state); });
	return 0;
}

int w_getBlendState(lua_State *L)
{
	const BlendState &state = instance()->getBlendState();

	if (state.enable)
	{
		luax_pushblendop(L, state.operationRGB);
		luax_pushblendop(L, state.operationA);
		luax_pushblendfactor(L, state.srcFactorRGB);
		luax_pushblendfactor(L, state.srcFactorA);
		luax_pushblendfactor(L, state.dstFactorRGB);
		luax_pushblendfactor(L, state.dstFactorA);
	}
	else
	{
		for (int i = 0; i < 6; i++)
			lua_pushnil(L);
	}

	return 6;
}

int w_setDefaultFilter(lua_State *L)
{
	SamplerState s = instance()->getDefaultSamplerState();

	const char *minstr = luaL_checkstring(L, 1);
	const char *magstr = luaL_optstring(L, 2, minstr);

	if (!SamplerState::getConstant(minstr, s.minFilter))
		return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.minFilter), minstr);
	if (!SamplerState::getConstant(magstr, s.magFilter))
		return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.magFilter), magstr);

	s.maxAnisotropy = std::min(std::max(1, (int) luaL_optnumber(L, 3, 1.0)), LOVE_UINT8_MAX);

	instance()->setDefaultSamplerState(s);
	return 0;
}

int w_getDefaultFilter(lua_State *L)
{
	const SamplerState &s = instance()->getDefaultSamplerState();
	const char *minstr;
	const char *magstr;
	if (!SamplerState::getConstant(s.minFilter, minstr))
		return luaL_error(L, "Unknown minification filter mode");
	if (!SamplerState::getConstant(s.magFilter, magstr))
		return luaL_error(L, "Unknown magnification filter mode");
	lua_pushstring(L, minstr);
	lua_pushstring(L, magstr);
	lua_pushnumber(L, s.maxAnisotropy);
	return 3;
}

int w_setDefaultMipmapFilter(lua_State *L)
{
	SamplerState s = instance()->getDefaultSamplerState();
	s.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;
	if (!lua_isnoneornil(L, 1))
	{
		const char *str = luaL_checkstring(L, 1);
		if (!SamplerState::getConstant(str, s.mipmapFilter))
			return luax_enumerror(L, "filter mode", SamplerState::getConstants(s.mipmapFilter), str);
	}

	s.lodBias = -((float) luaL_optnumber(L, 2, 0.0));

	instance()->setDefaultSamplerState(s);
	return 0;
}

int w_getDefaultMipmapFilter(lua_State *L)
{
	const SamplerState &s = instance()->getDefaultSamplerState();

	const char *str;
	if (SamplerState::getConstant(s.mipmapFilter, str))
		lua_pushstring(L, str);
	else
		lua_pushnil(L);

	lua_pushnumber(L, -s.lodBias);

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
		return luax_enumerror(L, "line style", Graphics::getConstants(style), str);

	instance()->setLineStyle(style);
	return 0;
}

int w_setLineJoin(lua_State *L)
{
	Graphics::LineJoin join;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, join))
		return luax_enumerror(L, "line join", Graphics::getConstants(join), str);

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

int w_setDepthMode(lua_State *L)
{
	if (lua_isnoneornil(L, 1) && lua_isnoneornil(L, 2))
		luax_catchexcept(L, [&]() { instance()->setDepthMode(); });
	else
	{
		CompareMode compare = COMPARE_ALWAYS;
		const char *str = luaL_checkstring(L, 1);
		bool write = luax_checkboolean(L, 2);

		if (!getConstant(str, compare))
			return luax_enumerror(L, "compare mode", getConstants(compare), str);

		luax_catchexcept(L, [&]() { instance()->setDepthMode(compare, write); });
	}

	return 0;
}

int w_getDepthMode(lua_State *L)
{
	CompareMode compare = COMPARE_ALWAYS;
	bool write = false;
	instance()->getDepthMode(compare, write);

	const char *str;
	if (!getConstant(compare, str))
		return luaL_error(L, "Unknown compare mode");

	lua_pushstring(L, str);
	luax_pushboolean(L, write);
	return 2;
}

int w_setMeshCullMode(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	CullMode mode;

	if (!getConstant(str, mode))
		return luax_enumerror(L, "cull mode", getConstants(mode), str);

	luax_catchexcept(L, [&]() { instance()->setMeshCullMode(mode); });
	return 0;
}

int w_getMeshCullMode(lua_State *L)
{
	CullMode mode = instance()->getMeshCullMode();
	const char *str;
	if (!getConstant(mode, str))
		return luaL_error(L, "Unknown cull mode");
	lua_pushstring(L, str);
	return 1;
}

int w_setFrontFaceWinding(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	Winding winding;

	if (!getConstant(str, winding))
		return luax_enumerror(L, "vertex winding", getConstants(winding), str);

	luax_catchexcept(L, [&]() { instance()->setFrontFaceWinding(winding); });
	return 0;
}

int w_getFrontFaceWinding(lua_State *L)
{
	Winding winding = instance()->getFrontFaceWinding();
	const char *str;
	if (!getConstant(winding, str))
		return luaL_error(L, "Unknown vertex winding");
	lua_pushstring(L, str);
	return 1;
}

int w_setWireframe(lua_State *L)
{
	instance()->setWireframe(luax_checkboolean(L, 1));
	return 0;
}

int w_isWireframe(lua_State *L)
{
	luax_pushboolean(L, instance()->isWireframe());
	return 1;
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
		luax_pushtype(L, shader);
	else
		lua_pushnil(L);

	return 1;
}

int w_getSupported(lua_State *L)
{
	const Graphics::Capabilities &caps = instance()->getCapabilities();

	if (lua_istable(L, 1))
		lua_pushvalue(L, 1);
	else
		lua_createtable(L, 0, (int) Graphics::FEATURE_MAX_ENUM);

	for (int i = 0; i < (int) Graphics::FEATURE_MAX_ENUM; i++)
	{
		auto feature = (Graphics::Feature) i;
		const char *name = nullptr;

		if (!Graphics::getConstant(feature, name))
			continue;

		luax_pushboolean(L, caps.features[i]);
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getTextureFormats(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	bool rt = luax_checkboolflag(L, 1, Texture::getConstant(Texture::SETTING_RENDER_TARGET));
	bool computewrite = luax_boolflag(L, 1, Texture::getConstant(Texture::SETTING_COMPUTE_WRITE), false);

	OptionalBool readable;
	lua_getfield(L, 1, Texture::getConstant(Texture::SETTING_READABLE));
	if (!lua_isnoneornil(L, -1))
		readable.set(luax_checkboolean(L, -1));
	lua_pop(L, 1);

	if (lua_istable(L, 2))
		lua_pushvalue(L, 2);
	else
		lua_createtable(L, 0, (int) PIXELFORMAT_MAX_ENUM);

	for (int i = 0; i < (int) PIXELFORMAT_MAX_ENUM; i++)
	{
		PixelFormat format = (PixelFormat) i;
		const char *name = nullptr;

		if (format == PIXELFORMAT_UNKNOWN || !love::getConstant(format, name))
			continue;

		uint32 usage = PIXELFORMATUSAGEFLAGS_NONE;
		if (rt)
			usage |= PIXELFORMATUSAGEFLAGS_RENDERTARGET;
		if (readable.get(!isPixelFormatDepthStencil(format)))
			usage |= PIXELFORMATUSAGEFLAGS_SAMPLE;
		if (computewrite)
			usage |= PIXELFORMATUSAGEFLAGS_COMPUTEWRITE;

		luax_pushboolean(L, instance()->isPixelFormatSupported(format, (PixelFormatUsageFlags) usage));
		lua_setfield(L, -2, name);
	}

	return 1;
}

static int w__getFormats(lua_State *L, int idx, bool (*isFormatSupported)(PixelFormat), bool (*ignore)(PixelFormat))
{
	if (lua_istable(L, idx))
		lua_pushvalue(L, idx);
	else
		lua_createtable(L, 0, (int) PIXELFORMAT_MAX_ENUM);

	for (int i = 0; i < (int) PIXELFORMAT_MAX_ENUM; i++)
	{
		PixelFormat format = (PixelFormat) i;
		const char *name = nullptr;

		if (format == PIXELFORMAT_UNKNOWN || !love::getConstant(format, name) || ignore(format))
			continue;

		luax_pushboolean(L, isFormatSupported(format));
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getCanvasFormats(lua_State *L)
{
	luax_markdeprecated(L, 1, "love.graphics.getCanvasFormats", API_FUNCTION, DEPRECATED_REPLACED, "love.graphics.getTextureFormats");

	bool (*supported)(PixelFormat);

	int idx = 1;
	if (lua_type(L, 1) == LUA_TBOOLEAN)
	{
		idx = 2;
		if (luax_checkboolean(L, 1))
		{
			supported = [](PixelFormat format) -> bool
			{
				const uint32 usage = PIXELFORMATUSAGEFLAGS_SAMPLE | PIXELFORMATUSAGEFLAGS_RENDERTARGET;
				return instance()->isPixelFormatSupported(format, (PixelFormatUsageFlags) usage);
			};
		}
		else
		{
			supported = [](PixelFormat format) -> bool
			{
				return instance()->isPixelFormatSupported(format, PIXELFORMATUSAGEFLAGS_RENDERTARGET);
			};
		}
	}
	else
	{
		supported = [](PixelFormat format) -> bool
		{
			bool readable = !isPixelFormatDepthStencil(format);
			uint32 usage = PIXELFORMATUSAGEFLAGS_RENDERTARGET;
			if (readable)
				usage |= PIXELFORMATUSAGEFLAGS_SAMPLE;
			return instance()->isPixelFormatSupported(format, (PixelFormatUsageFlags) usage);
		};
	}

	return w__getFormats(L, idx, supported, isPixelFormatCompressed);
}

int w_getImageFormats(lua_State *L)
{
	luax_markdeprecated(L, 1, "love.graphics.getImageFormats", API_FUNCTION, DEPRECATED_REPLACED, "love.graphics.getTextureFormats");

	const auto supported = [](PixelFormat format) -> bool
	{
		return instance()->isPixelFormatSupported(format, PIXELFORMATUSAGEFLAGS_SAMPLE);
	};

	const auto ignore = [](PixelFormat format) -> bool
	{
		return !(image::ImageData::validPixelFormat(format) || isPixelFormatCompressed(format));
	};

	return w__getFormats(L, 1, supported, ignore);
}

int w_getTextureTypes(lua_State *L)
{
	const Graphics::Capabilities &caps = instance()->getCapabilities();

	if (lua_istable(L, 1))
		lua_pushvalue(L, 1);
	else
		lua_createtable(L, 0, (int) TEXTURE_MAX_ENUM);

	for (int i = 0; i < (int) TEXTURE_MAX_ENUM; i++)
	{
		TextureType textype = (TextureType) i;
		const char *name = nullptr;

		if (!Texture::getConstant(textype, name))
			continue;

		luax_pushboolean(L, caps.textureTypes[i]);
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
	const Graphics::Capabilities &caps = instance()->getCapabilities();

	if (lua_istable(L, 1))
		lua_pushvalue(L, 1);
	else
		lua_createtable(L, 0, (int) Graphics::LIMIT_MAX_ENUM);

	for (int i = 0; i < (int) Graphics::LIMIT_MAX_ENUM; i++)
	{
		Graphics::SystemLimit limittype = (Graphics::SystemLimit) i;
		const char *name = nullptr;

		if (!Graphics::getConstant(limittype, name))
			continue;

		lua_pushnumber(L, caps.limits[i]);
		lua_setfield(L, -2, name);
	}

	return 1;
}

int w_getStats(lua_State *L)
{
	Graphics::Stats stats = instance()->getStats();

	if (lua_istable(L, 1))
		lua_pushvalue(L, 1);
	else
		lua_createtable(L, 0, 7);

	lua_pushinteger(L, stats.drawCalls);
	lua_setfield(L, -2, "drawcalls");

	lua_pushinteger(L, stats.drawCallsBatched);
	lua_setfield(L, -2, "drawcallsbatched");

	lua_pushinteger(L, stats.renderTargetSwitches);
	lua_setfield(L, -2, "canvasswitches");

	lua_pushinteger(L, stats.shaderSwitches);
	lua_setfield(L, -2, "shaderswitches");

	lua_pushinteger(L, stats.textures);
	lua_setfield(L, -2, "textures");

	lua_pushinteger(L, stats.fonts);
	lua_setfield(L, -2, "fonts");

	lua_pushinteger(L, stats.buffers);
	lua_setfield(L, -2, "buffers");

	lua_pushnumber(L, (lua_Number) stats.textureMemory);
	lua_setfield(L, -2, "texturememory");

	lua_pushnumber(L, (lua_Number) stats.bufferMemory);
	lua_setfield(L, -2, "buffermemory");

	return 1;
}

int w_draw(lua_State *L)
{
	Drawable *drawable = nullptr;
	Texture *texture = nullptr;
	Quad *quad = luax_totype<Quad>(L, 2);
	int startidx = 2;

	if (quad != nullptr)
	{
		texture = luax_checktexture(L, 1);
		startidx = 3;
	}
	else if (lua_isnil(L, 2) && !lua_isnoneornil(L, 3))
	{
		return luax_typerror(L, 2, "Quad");
	}
	else
	{
		drawable = luax_checktype<Drawable>(L, 1);
		startidx = 2;
	}

	luax_checkstandardtransform(L, startidx, [&](const Matrix4 &m)
	{
		luax_catchexcept(L, [&]()
		{
			if (texture && quad)
				instance()->draw(texture, quad, m);
			else
				instance()->draw(drawable, m);
		});
	});

	return 0;
}

int w_drawLayer(lua_State *L)
{
	Texture *texture = luax_checktexture(L, 1);
	int layer = (int) luaL_checkinteger(L, 2) - 1;

	int startidx = 3;
	Quad *quad = luax_totype<Quad>(L, startidx);

	if (quad != nullptr)
	{
		texture = luax_checktexture(L, 1);
		startidx++;
	}
	else if (lua_isnil(L, startidx) && !lua_isnoneornil(L, startidx + 1))
	{
		return luax_typerror(L, startidx, "Quad");
	}

	luax_checkstandardtransform(L, startidx, [&](const Matrix4 &m)
	{
		luax_catchexcept(L, [&]()
		{
			if (quad)
				instance()->drawLayer(texture, layer, quad, m);
			else
				instance()->drawLayer(texture, layer, m);
		});
	});

	return 0;
}

int w_drawInstanced(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	int instancecount = (int) luaL_checkinteger(L, 2);

	luax_checkstandardtransform(L, 3, [&](const Matrix4 &m)
	{
		luax_catchexcept(L, [&]() { instance()->drawInstanced(t, m, instancecount); });
	});

	return 0;
}

int w_drawIndirect(lua_State *L)
{
	Mesh *t = luax_checkmesh(L, 1);
	Buffer *argsbuffer = luax_checkbuffer(L, 2);
	int argsindex = (int) luaL_checkinteger(L, 3) - 1;

	luax_checkstandardtransform(L, 4, [&](const Matrix4 &m)
	{
		luax_catchexcept(L, [&]() { instance()->drawIndirect(t, m, argsbuffer, argsindex); });
	});

	return 0;
}

int w_drawFromShader(lua_State *L)
{
	if (luax_istype(L, 1, Buffer::type))
	{
		// Indexed drawing.
		Buffer *t = luax_checkbuffer(L, 1);

		int indexcount = (int) luaL_checkinteger(L, 2);
		int instancecount = (int) luaL_optinteger(L, 3, 1);
		int indexstart = (int) luaL_optinteger(L, 4, 1) - 1;

		Texture *tex = nullptr;
		if (!lua_isnoneornil(L, 5))
			tex = luax_checktexture(L, 5);

		luax_catchexcept(L, [&]() { instance()->drawFromShader(t, indexcount, instancecount, indexstart, tex); });
	}
	else
	{
		const char *primstr = luaL_checkstring(L, 1);
		PrimitiveType primtype = PRIMITIVE_TRIANGLES;
		if (!getConstant(primstr, primtype))
			return luax_enumerror(L, "primitive type", getConstants(primtype), primstr);

		int vertexcount = (int) luaL_checkinteger(L, 2);
		int instancecount = (int) luaL_optinteger(L, 3, 1);

		Texture *tex = nullptr;
		if (!lua_isnoneornil(L, 4))
			tex = luax_checktexture(L, 4);

		luax_catchexcept(L, [&]() { instance()->drawFromShader(primtype, vertexcount, instancecount, tex); });
	}
	return 0;
}

int w_drawFromShaderIndirect(lua_State *L)
{
	if (luax_istype(L, 1, Buffer::type))
	{
		// Indexed drawing.
		Buffer *t = luax_checkbuffer(L, 1);
		Buffer *argsbuffer = luax_checkbuffer(L, 2);
		int argsindex = (int) luaL_optinteger(L, 3, 1) - 1;

		Texture *tex = nullptr;
		if (!lua_isnoneornil(L, 4))
			tex = luax_checktexture(L, 4);

		luax_catchexcept(L, [&]() { instance()->drawFromShaderIndirect(t, argsbuffer, argsindex, tex); });
	}
	else
	{
		const char *primstr = luaL_checkstring(L, 1);
		PrimitiveType primtype = PRIMITIVE_TRIANGLES;
		if (!getConstant(primstr, primtype))
			return luax_enumerror(L, "primitive type", getConstants(primtype), primstr);

		Buffer *argsbuffer = luax_checkbuffer(L, 2);
		int argsindex = (int) luaL_optinteger(L, 3, 1) - 1;

		Texture *tex = nullptr;
		if (!lua_isnoneornil(L, 4))
			tex = luax_checktexture(L, 4);

		luax_catchexcept(L, [&]() { instance()->drawFromShaderIndirect(primtype, argsbuffer, argsindex, tex); });
	}
	return 0;
}

int w_print(lua_State *L)
{
	std::vector<love::font::ColoredString> str;
	luax_checkcoloredstring(L, 1, str);

	if (luax_istype(L, 2, Font::type))
	{
		Font *font = luax_checkfont(L, 2);

		luax_checkstandardtransform(L, 3, [&](const Matrix4 &m)
		{
			luax_catchexcept(L, [&](){ instance()->print(str, font, m); });
		});
	}
	else
	{
		luax_checkstandardtransform(L, 2, [&](const Matrix4 &m)
		{
			luax_catchexcept(L, [&](){ instance()->print(str, m); });
		});
	}

	return 0;
}

int w_printf(lua_State *L)
{
	std::vector<love::font::ColoredString> str;
	luax_checkcoloredstring(L, 1, str);

	Font *font = nullptr;
	int startidx = 2;

	if (luax_istype(L, startidx, Font::type))
	{
		font = luax_checkfont(L, startidx);
		startidx++;
	}

	Font::AlignMode align = Font::ALIGN_LEFT;
	Matrix4 m;

	int formatidx = startidx + 2;

	if (luax_istype(L, startidx, math::Transform::type))
	{
		math::Transform *tf = luax_totype<math::Transform>(L, startidx);
		m = tf->getMatrix();
		formatidx = startidx + 1;
	}
	else
	{
		float x = (float)luaL_checknumber(L, startidx + 0);
		float y = (float)luaL_checknumber(L, startidx + 1);

		float angle = (float) luaL_optnumber(L, startidx + 4, 0.0f);
		float sx = (float) luaL_optnumber(L, startidx + 5, 1.0f);
		float sy = (float) luaL_optnumber(L, startidx + 6, sx);
		float ox = (float) luaL_optnumber(L, startidx + 7, 0.0f);
		float oy = (float) luaL_optnumber(L, startidx + 8, 0.0f);
		float kx = (float) luaL_optnumber(L, startidx + 9, 0.0f);
		float ky = (float) luaL_optnumber(L, startidx + 10, 0.0f);

		m = Matrix4(x, y, angle, sx, sy, ox, oy, kx, ky);
	}

	float wrap = (float)luaL_checknumber(L, formatidx);

	const char *astr = lua_isnoneornil(L, formatidx + 1) ? nullptr : luaL_checkstring(L, formatidx + 1);
	if (astr != nullptr && !Font::getConstant(astr, align))
		return luax_enumerror(L, "alignment", Font::getConstants(align), astr);

	if (font != nullptr)
		luax_catchexcept(L, [&](){ instance()->printf(str, font, wrap, align, m); });
	else
		luax_catchexcept(L, [&](){ instance()->printf(str, wrap, align, m); });

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

	int numpositions = args / 2;
	if (is_table_of_tables)
		numpositions = args;

	Vector2 *positions = nullptr;
	Colorf *colors = nullptr;

	if (is_table_of_tables)
	{
		size_t datasize = (sizeof(Vector2) + sizeof(Colorf)) * numpositions;
		uint8 *data = instance()->getScratchBuffer<uint8>(datasize);

		positions = (Vector2 *) data;
		colors = (Colorf *) (data + sizeof(Vector2) * numpositions);
	}
	else
		positions = instance()->getScratchBuffer<Vector2>(numpositions);

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

				positions[i].x = luax_checkfloat(L, -6);
				positions[i].y = luax_checkfloat(L, -5);

				colors[i].r = (float) luax_optnumberclamped01(L, -4, 1.0);
				colors[i].g = (float) luax_optnumberclamped01(L, -3, 1.0);
				colors[i].b = (float) luax_optnumberclamped01(L, -2, 1.0);
				colors[i].a = (float) luax_optnumberclamped01(L, -1, 1.0);

				lua_pop(L, 7);
			}
		}
		else
		{
			// points({x1, y1, x2, y2, ...})
			for (int i = 0; i < numpositions; i++)
			{
				lua_rawgeti(L, 1, i * 2 + 1);
				lua_rawgeti(L, 1, i * 2 + 2);
				positions[i].x = luax_checkfloat(L, -2);
				positions[i].y = luax_checkfloat(L, -1);
				lua_pop(L, 2);
			}
		}
	}
	else
	{
		for (int i = 0; i < numpositions; i++)
		{
			positions[i].x = luax_checkfloat(L, i * 2 + 1);
			positions[i].y = luax_checkfloat(L, i * 2 + 2);
		}
	}

	luax_catchexcept(L, [&](){ instance()->points(positions, colors, numpositions); });
	return 0;
}

int w_line(lua_State *L)
{
	int args = lua_gettop(L);
	int arg1type = lua_type(L, 1);
	bool is_table = false;

	if (args == 1 && arg1type == LUA_TTABLE)
	{
		args = (int) luax_objlen(L, 1);
		is_table = true;
	}

	if (arg1type != LUA_TTABLE && arg1type != LUA_TNUMBER)
		return luax_typerror(L, 1, "table or number");
	else if (args % 2 != 0)
		return luaL_error(L, "Number of vertex components must be a multiple of two.");
	else if (args < 4)
		return luaL_error(L, "Need at least two vertices to draw a line.");

	int numvertices = args / 2;

	Vector2 *coords = instance()->getScratchBuffer<Vector2>(numvertices);
	if (is_table)
	{
		for (int i = 0; i < numvertices; ++i)
		{
			lua_rawgeti(L, 1, (i * 2) + 1);
			lua_rawgeti(L, 1, (i * 2) + 2);
			coords[i].x = luax_checkfloat(L, -2);
			coords[i].y = luax_checkfloat(L, -1);
			lua_pop(L, 2);
		}
	}
	else
	{
		for (int i = 0; i < numvertices; ++i)
		{
			coords[i].x = luax_checkfloat(L, (i * 2) + 1);
			coords[i].y = luax_checkfloat(L, (i * 2) + 2);
		}
	}

	luax_catchexcept(L,
		[&](){ instance()->polyline(coords, numvertices); }
	);

	return 0;
}

int w_rectangle(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luax_enumerror(L, "draw mode", Graphics::getConstants(mode), str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float w = (float)luaL_checknumber(L, 4);
	float h = (float)luaL_checknumber(L, 5);

	if (lua_isnoneornil(L, 6))
	{
		luax_catchexcept(L, [&](){ instance()->rectangle(mode, x, y, w, h); });
		return 0;
	}

	float rx = (float)luaL_optnumber(L, 6, 0.0);
	float ry = (float)luaL_optnumber(L, 7, rx);

	if (lua_isnoneornil(L, 8))
		luax_catchexcept(L, [&](){ instance()->rectangle(mode, x, y, w, h, rx, ry); });
	else
	{
		int points = (int) luaL_checkinteger(L, 8);
		luax_catchexcept(L, [&](){ instance()->rectangle(mode, x, y, w, h, rx, ry, points); });
	}

	return 0;
}

int w_circle(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luax_enumerror(L, "draw mode", Graphics::getConstants(mode), str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float radius = (float)luaL_checknumber(L, 4);

	if (lua_isnoneornil(L, 5))
		luax_catchexcept(L, [&](){ instance()->circle(mode, x, y, radius); });
	else
	{
		int points = (int) luaL_checkinteger(L, 5);
		luax_catchexcept(L, [&](){ instance()->circle(mode, x, y, radius, points); });
	}

	return 0;
}

int w_ellipse(lua_State *L)
{
	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luax_enumerror(L, "draw mode", Graphics::getConstants(mode), str);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float a = (float)luaL_checknumber(L, 4);
	float b = (float)luaL_optnumber(L, 5, a);

	if (lua_isnoneornil(L, 6))
		luax_catchexcept(L, [&](){ instance()->ellipse(mode, x, y, a, b); });
	else
	{
		int points = (int) luaL_checkinteger(L, 6);
		luax_catchexcept(L, [&](){ instance()->ellipse(mode, x, y, a, b, points); });
	}

	return 0;
}

int w_arc(lua_State *L)
{
	Graphics::DrawMode drawmode;
	const char *drawstr = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(drawstr, drawmode))
		return luax_enumerror(L, "draw mode", Graphics::getConstants(drawmode), drawstr);

	int startidx = 2;

	Graphics::ArcMode arcmode = Graphics::ARC_PIE;

	if (lua_type(L, 2) == LUA_TSTRING)
	{
		const char *arcstr = luaL_checkstring(L, 2);
		if (!Graphics::getConstant(arcstr, arcmode))
			return luax_enumerror(L, "arc mode", Graphics::getConstants(arcmode), arcstr);

		startidx = 3;
	}

	float x = (float) luaL_checknumber(L, startidx + 0);
	float y = (float) luaL_checknumber(L, startidx + 1);
	float radius = (float) luaL_checknumber(L, startidx + 2);
	float angle1 = (float) luaL_checknumber(L, startidx + 3);
	float angle2 = (float) luaL_checknumber(L, startidx + 4);

	if (lua_isnoneornil(L, startidx + 5))
		luax_catchexcept(L, [&](){ instance()->arc(drawmode, arcmode, x, y, radius, angle1, angle2); });
	else
	{
		int points = (int) luaL_checkinteger(L, startidx + 5);
		luax_catchexcept(L, [&](){ instance()->arc(drawmode, arcmode, x, y, radius, angle1, angle2, points); });
	}

	return 0;
}

int w_polygon(lua_State *L)
{
	int args = lua_gettop(L) - 1;

	Graphics::DrawMode mode;
	const char *str = luaL_checkstring(L, 1);
	if (!Graphics::getConstant(str, mode))
		return luax_enumerror(L, "draw mode", Graphics::getConstants(mode), str);

	bool is_table = false;
	if (args == 1 && lua_istable(L, 2))
	{
		args = (int) luax_objlen(L, 2);
		is_table = true;
	}

	if (args % 2 != 0)
		return luaL_error(L, "Number of vertex components must be a multiple of two");
	else if (args < 6)
		return luaL_error(L, "Need at least three vertices to draw a polygon");

	int numvertices = args / 2;

	// fetch coords
	Vector2 *coords = instance()->getScratchBuffer<Vector2>(numvertices + 1);
	if (is_table)
	{
		for (int i = 0; i < numvertices; ++i)
		{
			lua_rawgeti(L, 2, (i * 2) + 1);
			lua_rawgeti(L, 2, (i * 2) + 2);
			coords[i].x = luax_checkfloat(L, -2);
			coords[i].y = luax_checkfloat(L, -1);
			lua_pop(L, 2);
		}
	}
	else
	{
		for (int i = 0; i < numvertices; ++i)
		{
			coords[i].x = luax_checkfloat(L, (i * 2) + 2);
			coords[i].y = luax_checkfloat(L, (i * 2) + 3);
		}
	}

	// make a closed loop
	coords[numvertices] = coords[0];

	luax_catchexcept(L, [&](){ instance()->polygon(mode, coords, numvertices+1); });
	return 0;
}

int w_dispatchThreadgroups(lua_State* L)
{
	Shader *shader = luax_checkshader(L, 1);
	int x = (int) luaL_checkinteger(L, 2);
	int y = (int) luaL_optinteger(L, 3, 1);
	int z = (int) luaL_optinteger(L, 4, 1);
	luax_catchexcept(L, [&](){ instance()->dispatchThreadgroups(shader, x, y, z); });
	return 0;
}

int w_dispatchIndirect(lua_State *L)
{
	Shader *shader = luax_checkshader(L, 1);
	Buffer *argsbuffer = luax_checkbuffer(L, 2);
	int argsindex = (int) luaL_optinteger(L, 3, 1) - 1;
	luax_catchexcept(L, [&]() { instance()->dispatchIndirect(shader, argsbuffer, argsindex); });
	return 0;
}

int w_copyBuffer(lua_State *L)
{
	Buffer *source = luax_checkbuffer(L, 1);
	Buffer *dest = luax_checkbuffer(L, 2);

	ptrdiff_t sourceoffset = luaL_optinteger(L, 3, 0);
	ptrdiff_t destoffset = luaL_optinteger(L, 4, 0);

	ptrdiff_t size = std::min(source->getSize() - sourceoffset, dest->getSize() - destoffset);
	if (!lua_isnoneornil(L, 5))
		size = luaL_checkinteger(L, 5);

	if (sourceoffset < 0 || destoffset < 0)
		return luaL_error(L, "copyBuffer offsets cannot be negative.");
	if (size <= 0)
		return luaL_error(L, "copyBuffer size must be greater than 0.");

	luax_catchexcept(L, [&](){ instance()->copyBuffer(source, dest, sourceoffset, destoffset, size); });
	return 0;
}

int w_copyBufferToTexture(lua_State *L)
{
	Buffer *source = luax_checkbuffer(L, 1);
	Texture *dest = luax_checktexture(L, 2);

	ptrdiff_t sourceoffset = luaL_optinteger(L, 3, 0);
	if (sourceoffset < 0)
		return luaL_error(L, "copyBufferToTexture source offset cannot be negative.");

	int sourcewidth = (int) luaL_optinteger(L, 4, 0);

	int slice = 0;
	int mipmap = 0;

	if (dest->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 5) - 1;

	mipmap = (int) luaL_optinteger(L, 6, 1) - 1;

	Rect rect = {0, 0, dest->getPixelWidth(mipmap), dest->getPixelHeight(mipmap)};
	if (!lua_isnoneornil(L, 7))
	{
		rect.x = (int) luaL_checkinteger(L, 7);
		rect.y = (int) luaL_checkinteger(L, 8);
		rect.w = (int) luaL_checkinteger(L, 9);
		rect.h = (int) luaL_checkinteger(L, 10);
	}

	luax_catchexcept(L, [&](){ instance()->copyBufferToTexture(source, dest, sourceoffset, sourcewidth, slice, mipmap, rect); });
	return 0;
}

int w_copyTextureToBuffer(lua_State *L)
{
	Texture *source = luax_checktexture(L, 1);
	Buffer *dest = luax_checkbuffer(L, 2);

	int slice = 0;
	int mipmap = 0;

	if (source->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 3) - 1;

	mipmap = (int) luaL_optinteger(L, 4, 1) - 1;

	Rect rect = {0, 0, source->getPixelWidth(mipmap), source->getPixelHeight(mipmap)};
	if (!lua_isnoneornil(L, 5))
	{
		rect.x = (int) luaL_checkinteger(L, 5);
		rect.y = (int) luaL_checkinteger(L, 6);
		rect.w = (int) luaL_checkinteger(L, 7);
		rect.h = (int) luaL_checkinteger(L, 8);
	}

	ptrdiff_t destoffset = luaL_optinteger(L, 9, 0);
	if (destoffset < 0)
		return luaL_error(L, "copyTextureToBuffer dest offset cannot be negative.");

	int destwidth = (int) luaL_optinteger(L, 10, 0);

	luax_catchexcept(L, [&](){ instance()->copyTextureToBuffer(source, dest, slice, mipmap, rect, destoffset, destwidth); });
	return 0;
}

int w_flushBatch(lua_State *)
{
	instance()->flushBatchedDraws();
	return 0;
}

int w_getStackDepth(lua_State *L)
{
	lua_pushnumber(L, instance()->getStackDepth());
	return 1;
}

int w_push(lua_State *L)
{
	Graphics::StackType stype = Graphics::STACK_TRANSFORM;
	const char *sname = lua_isnoneornil(L, 1) ? nullptr : luaL_checkstring(L, 1);
	if (sname && !Graphics::getConstant(sname, stype))
		return luax_enumerror(L, "graphics stack type", Graphics::getConstants(stype), sname);

	luax_catchexcept(L, [&](){ instance()->push(stype); });

	if (luax_istype(L, 2, math::Transform::type))
	{
		math::Transform *t = luax_totype<math::Transform>(L, 2);
		luax_catchexcept(L, [&]() { instance()->applyTransform(t->getMatrix()); });
	}

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

int w_applyTransform(lua_State *L)
{
	luax_checkstandardtransform(L, 1, [&](const Matrix4 &m)
	{
		luax_catchexcept(L, [&]() { instance()->applyTransform(m); });
	});
	return 0;
}

int w_replaceTransform(lua_State *L)
{
	luax_checkstandardtransform(L, 1, [&](const Matrix4 &m)
	{
		luax_catchexcept(L, [&]() { instance()->replaceTransform(m); });
	});
	return 0;
}

int w_transformPoint(lua_State *L)
{
	Vector2 p;
	p.x = (float) luaL_checknumber(L, 1);
	p.y = (float) luaL_checknumber(L, 2);
	p = instance()->transformPoint(p);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	return 2;
}

int w_inverseTransformPoint(lua_State *L)
{
	Vector2 p;
	p.x = (float) luaL_checknumber(L, 1);
	p.y = (float) luaL_checknumber(L, 2);
	p = instance()->inverseTransformPoint(p);
	lua_pushnumber(L, p.x);
	lua_pushnumber(L, p.y);
	return 2;
}

int w_setProjection(lua_State *L)
{
	math::Transform *transform = luax_totype<math::Transform>(L, 1);
	if (transform != nullptr)
	{
		instance()->setProjection(transform->getMatrix());
		return 0;
	}

	math::Transform::MatrixLayout layout = math::Transform::MATRIX_ROW_MAJOR;

	int idx = 1;
	if (lua_type(L, idx) == LUA_TSTRING)
	{
		const char* layoutstr = lua_tostring(L, idx);
		if (!math::Transform::getConstant(layoutstr, layout))
			return luax_enumerror(L, "matrix layout", math::Transform::getConstants(layout), layoutstr);

		idx++;
	}

	float elements[16];
	love::math::luax_checkmatrix(L, idx, layout, elements);

	instance()->setProjection(Matrix4(elements));
	return 0;
}

int w_resetProjection(lua_State */*L*/)
{
	instance()->resetProjection();
	return 0;
}


// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "reset", w_reset },
	{ "clear", w_clear },
	{ "discard", w_discard },
	{ "present", w_present },

	{ "newCanvas", w_newCanvas },
	{ "newTexture", w_newTexture },
	{ "newCubeTexture", w_newCubeTexture },
	{ "newArrayTexture", w_newArrayTexture },
	{ "newVolumeTexture", w_newVolumeTexture },
	{ "newTextureView", w_newTextureView },
	{ "newQuad", w_newQuad },
	{ "newFont", w_newFont },
	{ "newImageFont", w_newImageFont },
	{ "newSpriteBatch", w_newSpriteBatch },
	{ "newParticleSystem", w_newParticleSystem },
	{ "newShader", w_newShader },
	{ "newComputeShader", w_newComputeShader },
	{ "newBuffer", w_newBuffer },
	{ "newMesh", w_newMesh },
	{ "newTextBatch", w_newTextBatch },
	{ "_newVideo", w_newVideo },

	{ "readbackBuffer", w_readbackBuffer },
	{ "readbackBufferAsync", w_readbackBufferAsync },
	{ "readbackTexture", w_readbackTexture },
	{ "readbackTextureAsync", w_readbackTextureAsync },

	{ "validateShader", w_validateShader },

	{ "setCanvas", w_setCanvas },
	{ "getCanvas", w_getCanvas },

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
	{ "setBlendState", w_setBlendState },
	{ "getBlendState", w_getBlendState },
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
	{ "setDepthMode", w_setDepthMode },
	{ "getDepthMode", w_getDepthMode },
	{ "setMeshCullMode", w_setMeshCullMode },
	{ "getMeshCullMode", w_getMeshCullMode },
	{ "setFrontFaceWinding", w_setFrontFaceWinding },
	{ "getFrontFaceWinding", w_getFrontFaceWinding },
	{ "setWireframe", w_setWireframe },
	{ "isWireframe", w_isWireframe },

	{ "setShader", w_setShader },
	{ "getShader", w_getShader },

	{ "getSupported", w_getSupported },
	{ "getTextureFormats", w_getTextureFormats },
	{ "getRendererInfo", w_getRendererInfo },
	{ "getSystemLimits", w_getSystemLimits },
	{ "getTextureTypes", w_getTextureTypes },
	{ "getStats", w_getStats },

	{ "captureScreenshot", w_captureScreenshot },

	{ "draw", w_draw },
	{ "drawLayer", w_drawLayer },
	{ "drawInstanced", w_drawInstanced },
	{ "drawIndirect", w_drawIndirect },
	{ "drawFromShader", w_drawFromShader },
	{ "drawFromShaderIndirect", w_drawFromShaderIndirect },

	{ "print", w_print },
	{ "printf", w_printf },

	{ "dispatchThreadgroups", w_dispatchThreadgroups },
	{ "dispatchIndirect", w_dispatchIndirect },

	{ "copyBuffer", w_copyBuffer },
	{ "copyBufferToTexture", w_copyBufferToTexture },
	{ "copyTextureToBuffer", w_copyTextureToBuffer },

	{ "isCreated", w_isCreated },
	{ "isActive", w_isActive },
	{ "isGammaCorrect", w_isGammaCorrect },
	{ "isLowPowerPreferred", w_isLowPowerPreferred },
	{ "getWidth", w_getWidth },
	{ "getHeight", w_getHeight },
	{ "getDimensions", w_getDimensions },
	{ "getPixelWidth", w_getPixelWidth },
	{ "getPixelHeight", w_getPixelHeight },
	{ "getPixelDimensions", w_getPixelDimensions },
	{ "getDPIScale", w_getDPIScale },
	{ "getQuadIndexBuffer", w_getQuadIndexBuffer },

	{ "setScissor", w_setScissor },
	{ "intersectScissor", w_intersectScissor },
	{ "getScissor", w_getScissor },

	{ "setStencilMode", w_setStencilMode },
	{ "getStencilMode", w_getStencilMode },
	{ "setStencilState", w_setStencilState },
	{ "getStencilState", w_getStencilState },

	{ "points", w_points },
	{ "line", w_line },
	{ "rectangle", w_rectangle },
	{ "circle", w_circle },
	{ "ellipse", w_ellipse },
	{ "arc", w_arc },
	{ "polygon", w_polygon },

	{ "flushBatch", w_flushBatch },

	{ "getStackDepth", w_getStackDepth },
	{ "push", w_push },
	{ "pop", w_pop },
	{ "rotate", w_rotate },
	{ "scale", w_scale },
	{ "translate", w_translate },
	{ "shear", w_shear },
	{ "origin", w_origin },
	{ "applyTransform", w_applyTransform },
	{ "replaceTransform", w_replaceTransform },
	{ "transformPoint", w_transformPoint },
	{ "inverseTransformPoint", w_inverseTransformPoint },

	{ "setProjection", w_setProjection },
	{ "resetProjection", w_resetProjection },

	// Deprecated
	{ "newImage", w_newImage },
	{ "newArrayImage", w_newArrayImage },
	{ "newVolumeImage", w_newVolumeImage },
	{ "newCubeImage", w_newCubeImage },
	{ "newText", w_newText },
	{ "getCanvasFormats", w_getCanvasFormats },
	{ "getImageFormats", w_getImageFormats },

	{ 0, 0 }
};

static int luaopen_drawable(lua_State *L)
{
	return luax_register_type(L, &Drawable::type, nullptr);
}

// Types for this module.
static const lua_CFunction types[] =
{
	luaopen_drawable,
	luaopen_texture,
	luaopen_font,
	luaopen_quad,
	luaopen_graphicsbuffer,
	luaopen_graphicsreadback,
	luaopen_spritebatch,
	luaopen_particlesystem,
	luaopen_shader,
	luaopen_mesh,
	luaopen_textbatch,
	luaopen_video,
	0
};

extern "C" int luaopen_love_graphics(lua_State *L)
{
	Graphics *instance = Graphics::createInstance();

	if (instance == nullptr)
	{
		printf("Cannot create graphics: no supported renderer on this system.\n");
		return luaL_error(L, "Cannot create graphics: no supported renderer on this system.");
	}

	WrappedModule w;
	w.module = instance;
	w.name = "graphics";
	w.type = &Graphics::type;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	if (luaL_loadbuffer(L, (const char *)graphics_lua, sizeof(graphics_lua), "=[love \"wrap_Graphics.lua\"]") == 0)
		lua_call(L, 0, 0);
	else
		lua_error(L);

	return n;
}

} // graphics
} // love
