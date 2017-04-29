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

