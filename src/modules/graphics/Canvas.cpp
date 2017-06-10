/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

#include "Canvas.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{

love::Type Canvas::type("Canvas", &Texture::type);
int Canvas::canvasCount = 0;

Canvas::Canvas(const Settings &settings)
	: Texture(settings.type)
{
	this->settings = settings;

	width = settings.width;
	height = settings.height;
	pixelWidth = (int) ((width * settings.pixeldensity) + 0.5);
	pixelHeight = (int) ((height * settings.pixeldensity) + 0.5);

	format = settings.format;

	if (texType == TEXTURE_VOLUME)
		depth = settings.layers;
	else if (texType == TEXTURE_2D_ARRAY)
		layers = settings.layers;
	else
		layers = 1;

	if (width <= 0 || height <= 0 || layers <= 0)
		throw love::Exception("Canvas dimensions must be greater than 0.");

	if (texType != TEXTURE_2D && settings.msaa > 1)
		throw love::Exception("MSAA is only supported for Canvases with the 2D texture type.");

	if (settings.readable.hasValue)
		readable = settings.readable.value;
	else
		readable = !isPixelFormatDepthStencil(format);

	if (readable && isPixelFormatDepthStencil(format) && settings.msaa > 1)
		throw love::Exception("Readable depth/stencil Canvases with MSAA are not currently supported.");

	if ((!readable || settings.msaa > 1) && settings.mipmaps != MIPMAPS_NONE)
		throw love::Exception("Non-readable and MSAA textures cannot have mipmaps.");

	if (settings.mipmaps != MIPMAPS_NONE)
	{
		mipmapCount = getMipmapCount(pixelWidth, pixelHeight);
		filter.mipmap = defaultMipmapFilter;
	}

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);

	if (!gfx->isCanvasFormatSupported(format, readable))
	{
		const char *fstr = "rgba8";
		const char *readablestr = "";
		if (readable != !isPixelFormatDepthStencil(format))
			readablestr = readable ? " readable" : " non-readable";
		love::getConstant(format, fstr);
		throw love::Exception("The %s%s canvas format is not supported by your graphics drivers.", fstr, readablestr);
	}

	if (getRequestedMSAA() > 1 && texType != TEXTURE_2D)
		throw love::Exception("MSAA is only supported for 2D texture types.");

	if (!readable && texType != TEXTURE_2D)
		throw love::Exception("Non-readable pixel formats are only supported for 2D texture types.");

	if (!gfx->isTextureTypeSupported(texType))
	{
		const char *textypestr = "unknown";
		Texture::getConstant(texType, textypestr);
		throw love::Exception("%s textures are not supported on this system!", textypestr);
	}

	int maxsize = 0;
	switch (texType)
	{
	case TEXTURE_2D:
		maxsize = gfx->getSystemLimit(Graphics::LIMIT_TEXTURE_SIZE);
		if (pixelWidth > maxsize)
			throw TextureTooLargeException("width", pixelWidth);
		else if (pixelHeight > maxsize)
			throw TextureTooLargeException("height", pixelHeight);
		break;
	case TEXTURE_VOLUME:
		maxsize = gfx->getSystemLimit(Graphics::LIMIT_VOLUME_TEXTURE_SIZE);
		if (pixelWidth > maxsize)
			throw TextureTooLargeException("width", pixelWidth);
		else if (pixelHeight > maxsize)
			throw TextureTooLargeException("height", pixelHeight);
		else if (depth > maxsize)
			throw TextureTooLargeException("depth", depth);
		break;
	case TEXTURE_2D_ARRAY:
		maxsize = gfx->getSystemLimit(Graphics::LIMIT_TEXTURE_SIZE);
		if (pixelWidth > maxsize)
			throw TextureTooLargeException("width", pixelWidth);
		else if (pixelHeight > maxsize)
			throw TextureTooLargeException("height", pixelHeight);
		else if (layers > gfx->getSystemLimit(Graphics::LIMIT_TEXTURE_LAYERS))
			throw TextureTooLargeException("array layer count", layers);
		break;
	case TEXTURE_CUBE:
		if (pixelWidth != pixelHeight)
			throw love::Exception("Cubemap textures must have equal width and height.");
		else if (pixelWidth > gfx->getSystemLimit(Graphics::LIMIT_CUBE_TEXTURE_SIZE))
			throw TextureTooLargeException("width", pixelWidth);
		break;
	default:
		break;
	}

	canvasCount++;
}

Canvas::~Canvas()
{
	canvasCount--;
}

Canvas::MipmapMode Canvas::getMipmapMode() const
{
	return settings.mipmaps;
}

int Canvas::getRequestedMSAA() const
{
	return settings.msaa;
}

love::image::ImageData *Canvas::newImageData(love::image::Image *module, int slice, int mipmap, const Rect &r)
{
	if (!isReadable())
		throw love::Exception("Canvas:newImageData cannot be called on non-readable Canvases.");

	if (isPixelFormatDepthStencil(getPixelFormat()))
		throw love::Exception("Canvas:newImageData cannot be called on Canvases with depth/stencil pixel formats.");

	if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0 || (r.x + r.w) > getPixelWidth(mipmap) || (r.y + r.h) > getPixelHeight(mipmap))
		throw love::Exception("Invalid rectangle dimensions.");

	if (slice < 0 || (texType == TEXTURE_VOLUME && slice >= getDepth(mipmap))
		|| (texType == TEXTURE_2D_ARRAY && slice >= layers)
		|| (texType == TEXTURE_CUBE && slice >= 6))
	{
		throw love::Exception("Invalid slice index.");
	}

	Graphics *gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && gfx->isCanvasActive(this))
		throw love::Exception("Canvas:newImageData cannot be called while that Canvas is currently active.");

	PixelFormat dataformat;
	switch (getPixelFormat())
	{
		case PIXELFORMAT_RGB10A2: // FIXME: Conversions aren't supported in GLES
			dataformat = PIXELFORMAT_RGBA16;
			break;
		case PIXELFORMAT_R16F:
		case PIXELFORMAT_RG16F:
		case PIXELFORMAT_RGBA16F:
		case PIXELFORMAT_RG11B10F: // FIXME: Conversions aren't supported in GLES
			dataformat = PIXELFORMAT_RGBA16F;
			break;
		case PIXELFORMAT_R32F:
		case PIXELFORMAT_RG32F:
		case PIXELFORMAT_RGBA32F:
			dataformat = PIXELFORMAT_RGBA32F;
			break;
		default:
			dataformat = PIXELFORMAT_RGBA8;
			break;
	}

	return module->newImageData(r.w, r.h, dataformat);
}

void Canvas::draw(Graphics *gfx, Quad *q, const Matrix4 &t)
{
	if (gfx->isCanvasActive(this))
		throw love::Exception("Cannot render a Canvas to itself!");

	Texture::draw(gfx, q, t);
}

void Canvas::drawLayer(Graphics *gfx, int layer, Quad *quad, const Matrix4 &m)
{
	if (gfx->isCanvasActive(this, layer))
		throw love::Exception("Cannot render a Canvas to itself!");

	Texture::drawLayer(gfx, layer, quad, m);
}

bool Canvas::getConstant(const char *in, MipmapMode &out)
{
	return mipmapModes.find(in, out);
}

bool Canvas::getConstant(MipmapMode in, const char *&out)
{
	return mipmapModes.find(in, out);
}

StringMap<Canvas::MipmapMode, Canvas::MIPMAPS_MAX_ENUM>::Entry Canvas::mipmapEntries[] =
{
	{ "none",   MIPMAPS_NONE   },
	{ "manual", MIPMAPS_MANUAL },
	{ "auto",   MIPMAPS_AUTO   },
};

StringMap<Canvas::MipmapMode, Canvas::MIPMAPS_MAX_ENUM> Canvas::mipmapModes(Canvas::mipmapEntries, sizeof(Canvas::mipmapEntries));

} // graphics
} // love

