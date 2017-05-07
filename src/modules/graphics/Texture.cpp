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

// LOVE
#include "common/config.h"
#include "Texture.h"
#include "Graphics.h"

// C
#include <cmath>
#include <algorithm>

#ifdef LOVE_ANDROID
// log2 is not declared in the math.h shipped with the Android NDK
static inline double log2(double n)
{
	// log(n)/log(2) is log2.
	return std::log(n) / std::log(2);
}
#endif


namespace love
{
namespace graphics
{

love::Type Texture::type("Texture", &Drawable::type);

Texture::Filter Texture::defaultFilter;
Texture::FilterMode Texture::defaultMipmapFilter = Texture::FILTER_LINEAR;
float Texture::defaultMipmapSharpness = 0.0f;
int64 Texture::totalGraphicsMemory = 0;

Texture::Texture(TextureType texType)
	: texType(texType)
	, format(PIXELFORMAT_UNKNOWN)
	, readable(true)
	, width(0)
	, height(0)
	, depth(1)
	, layers(1)
	, mipmapCount(1)
	, pixelWidth(0)
	, pixelHeight(0)
	, filter(defaultFilter)
	, wrap()
	, mipmapSharpness(defaultMipmapSharpness)
	, graphicsMemorySize(0)
{
}

Texture::~Texture()
{
	setGraphicsMemorySize(0);
}

void Texture::initQuad()
{
	Quad::Viewport v = {0, 0, (double) width, (double) height};
	quad.set(new Quad(v, width, height), Acquire::NORETAIN);
}

void Texture::setGraphicsMemorySize(int64 bytes)
{
	totalGraphicsMemory = std::max(totalGraphicsMemory - graphicsMemorySize, 0LL);

	bytes = std::max(bytes, 0LL);
	graphicsMemorySize = bytes;
	totalGraphicsMemory += bytes;
}

TextureType Texture::getTextureType() const
{
	return texType;
}

PixelFormat Texture::getPixelFormat() const
{
	return format;
}

bool Texture::isReadable() const
{
	return readable;
}

void Texture::draw(Graphics *gfx, const Matrix4 &m)
{
	draw(gfx, quad, m);
}

void Texture::draw(Graphics *gfx, Quad *q, const Matrix4 &localTransform)
{
	using namespace vertex;

	if (!readable)
		throw love::Exception("Textures with non-readable formats cannot be drawn.");

	if (texType == TEXTURE_2D_ARRAY)
	{
		drawLayer(gfx, q->getLayer(), q, localTransform);
		return;
	}

	Color c = toColor(gfx->getColor());

	Graphics::StreamDrawRequest req;
	req.formats[0] = CommonFormat::XYf_STf_RGBAub;
	req.indexMode = TriangleIndexMode::QUADS;
	req.vertexCount = 4;
	req.texture = this;

	Graphics::StreamVertexData data = gfx->requestStreamDraw(req);

	XYf_STf_RGBAub *verts = (XYf_STf_RGBAub *) data.stream[0];
	const XYf_STf *quadverts = q->getVertices();

	Matrix4 t(gfx->getTransform(), localTransform);
	t.transform(verts, quadverts, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadverts[i].s;
		verts[i].t = quadverts[i].t;
		verts[i].color = c;
	}
}

void Texture::drawLayer(Graphics *gfx, int layer, const Matrix4 &m)
{
	drawLayer(gfx, layer, quad, m);
}

void Texture::drawLayer(Graphics *gfx, int layer, Quad *q, const Matrix4 &m)
{
	using namespace vertex;

	if (!readable)
		throw love::Exception("Textures with non-readable formats cannot be drawn.");

	if (texType != TEXTURE_2D_ARRAY)
		throw love::Exception("drawLayer can only be used with Array Textures!");

	if (layer < 0 || layer >= layers)
		throw love::Exception("Invalid layer: %d (Texture has %d layers)", layer + 1, layers);

	Color c = toColor(gfx->getColor());

	Graphics::StreamDrawRequest req;
	req.formats[0] = CommonFormat::XYf_STPf_RGBAub;
	req.indexMode = TriangleIndexMode::QUADS;
	req.vertexCount = 4;
	req.texture = this;

	Graphics::StreamVertexData data = gfx->requestStreamDraw(req);

	XYf_STPf_RGBAub *verts = (XYf_STPf_RGBAub *) data.stream[0];
	const XYf_STf *quadverts = q->getVertices();

	Matrix4 t(gfx->getTransform(), m);
	t.transform(verts, quadverts, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadverts[i].s;
		verts[i].t = quadverts[i].t;
		verts[i].p = (float) layer;
		verts[i].color = c;
	}
}

int Texture::getWidth(int mip) const
{
	return std::max(width >> mip, 1);
}

int Texture::getHeight(int mip) const
{
	return std::max(height >> mip, 1);
}

int Texture::getDepth(int mip) const
{
	return std::max(depth >> mip, 1);
}

int Texture::getLayerCount() const
{
	return layers;
}

int Texture::getMipmapCount() const
{
	return mipmapCount;
}

int Texture::getPixelWidth(int mip) const
{
	return std::max(pixelWidth >> mip, 1);
}

int Texture::getPixelHeight(int mip) const
{
	return std::max(pixelHeight >> mip, 1);
}

float Texture::getPixelDensity() const
{
	return (float) pixelHeight / (float) height;
}

void Texture::setFilter(const Filter &f)
{
	if (!validateFilter(f, getMipmapCount() > 1))
	{
		if (f.mipmap != FILTER_NONE && getMipmapCount() == 1)
			throw love::Exception("Non-mipmapped texture cannot have mipmap filtering.");
		else
			throw love::Exception("Invalid texture filter.");
	}

	Graphics::flushStreamDrawsGlobal();

	filter = f;
}

const Texture::Filter &Texture::getFilter() const
{
	return filter;
}

const Texture::Wrap &Texture::getWrap() const
{
	return wrap;
}

float Texture::getMipmapSharpness() const
{
	return mipmapSharpness;
}

void Texture::setDepthSampleMode(Optional<CompareMode> mode)
{
	if (mode.hasValue && (!readable || !isPixelFormatDepthStencil(format)))
		throw love::Exception("Only readable depth textures can have a depth sample compare mode.");
}

Optional<CompareMode> Texture::getDepthSampleMode() const
{
	return depthCompareMode;
}

Quad *Texture::getQuad() const
{
	return quad;
}

bool Texture::validateFilter(const Filter &f, bool mipmapsAllowed)
{
	if (!mipmapsAllowed && f.mipmap != FILTER_NONE)
		return false;

	if (f.mag != FILTER_LINEAR && f.mag != FILTER_NEAREST)
		return false;

	if (f.min != FILTER_LINEAR && f.min != FILTER_NEAREST)
		return false;

	if (f.mipmap != FILTER_LINEAR && f.mipmap != FILTER_NEAREST && f.mipmap != FILTER_NONE)
		return false;

	return true;
}

int Texture::getMipmapCount(int w, int h)
{
	return (int) log2(std::max(w, h)) + 1;
}

int Texture::getMipmapCount(int w, int h, int d)
{
	return (int) log2(std::max(std::max(w, h), d)) + 1;
}

bool Texture::getConstant(const char *in, TextureType &out)
{
	return texTypes.find(in, out);
}

bool Texture::getConstant(TextureType in, const char *&out)
{
	return texTypes.find(in, out);
}

bool Texture::getConstant(const char *in, FilterMode &out)
{
	return filterModes.find(in, out);
}

bool Texture::getConstant(FilterMode in, const char *&out)
{
	return filterModes.find(in, out);
}

bool Texture::getConstant(const char *in, WrapMode &out)
{
	return wrapModes.find(in, out);
}

bool Texture::getConstant(WrapMode in, const char *&out)
{
	return wrapModes.find(in, out);
}

StringMap<TextureType, TEXTURE_MAX_ENUM>::Entry Texture::texTypeEntries[] =
{
	{ "2d", TEXTURE_2D },
	{ "volume", TEXTURE_VOLUME },
	{ "array", TEXTURE_2D_ARRAY },
	{ "cube", TEXTURE_CUBE },
};

StringMap<TextureType, TEXTURE_MAX_ENUM> Texture::texTypes(Texture::texTypeEntries, sizeof(Texture::texTypeEntries));

StringMap<Texture::FilterMode, Texture::FILTER_MAX_ENUM>::Entry Texture::filterModeEntries[] =
{
	{ "linear", FILTER_LINEAR },
	{ "nearest", FILTER_NEAREST },
	{ "none", FILTER_NONE },
};

StringMap<Texture::FilterMode, Texture::FILTER_MAX_ENUM> Texture::filterModes(Texture::filterModeEntries, sizeof(Texture::filterModeEntries));

StringMap<Texture::WrapMode, Texture::WRAP_MAX_ENUM>::Entry Texture::wrapModeEntries[] =
{
	{ "clamp", WRAP_CLAMP },
	{ "clampzero", WRAP_CLAMP_ZERO },
	{ "repeat", WRAP_REPEAT },
	{ "mirroredrepeat", WRAP_MIRRORED_REPEAT },
};

StringMap<Texture::WrapMode, Texture::WRAP_MAX_ENUM> Texture::wrapModes(Texture::wrapModeEntries, sizeof(Texture::wrapModeEntries));

} // graphics
} // love
