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

Texture::Texture(TextureType texType)
	: texType(texType)
	, format(PIXELFORMAT_UNKNOWN)
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
	, vertices()
{
}

Texture::~Texture()
{
}

void Texture::initVertices()
{
	for (int i = 0; i < 4; i++)
		vertices[i].color = Color(255, 255, 255, 255);

	// Vertices are ordered for use with triangle strips:
	// 0---2
	// | / |
	// 1---3
	vertices[0].x = 0.0f;
	vertices[0].y = 0.0f;
	vertices[1].x = 0.0f;
	vertices[1].y = (float) height;
	vertices[2].x = (float) width;
	vertices[2].y = 0.0f;
	vertices[3].x = (float) width;
	vertices[3].y = (float) height;

	vertices[0].s = 0.0f;
	vertices[0].t = 0.0f;
	vertices[1].s = 0.0f;
	vertices[1].t = 1.0f;
	vertices[2].s = 1.0f;
	vertices[2].t = 0.0f;
	vertices[3].s = 1.0f;
	vertices[3].t = 1.0f;
}

TextureType Texture::getTextureType() const
{
	return texType;
}

PixelFormat Texture::getPixelFormat() const
{
	return format;
}

void Texture::draw(Graphics *gfx, const Matrix4 &m)
{
	drawv(gfx, m, vertices);
}

void Texture::drawq(Graphics *gfx, Quad *quad, const Matrix4 &m)
{
	drawv(gfx, m, quad->getVertices());
}

void Texture::drawv(Graphics *gfx, const Matrix4 &localTransform, const Vertex *v)
{
	if (Shader::current)
		Shader::current->checkMainTextureType(texType);

	Matrix4 t(gfx->getTransform(), localTransform);

	Vertex verts[4] = {v[0], v[1], v[2], v[3]};
	t.transform(verts, v, 4);

	Color c = toColor(gfx->getColor());

	for (int i = 0; i < 4; i++)
		verts[i].color = c;

	Graphics::StreamDrawRequest req;
	req.formats[0] = vertex::CommonFormat::XYf_STf_RGBAub;
	req.indexMode = vertex::TriangleIndexMode::QUADS;
	req.vertexCount = 4;
	req.texture = this;

	Graphics::StreamVertexData data = gfx->requestStreamDraw(req);
	memcpy(data.stream[0], verts, sizeof(Vertex) * 4);
}

int Texture::getWidth() const
{
	return width;
}

int Texture::getHeight() const
{
	return height;
}

int Texture::getDepth() const
{
	return depth;
}

int Texture::getLayerCount() const
{
	return layers;
}

int Texture::getMipmapCount() const
{
	return mipmapCount;
}

int Texture::getPixelWidth() const
{
	return pixelWidth;
}

int Texture::getPixelHeight() const
{
	return pixelHeight;
}

float Texture::getPixelDensity() const
{
	return (float) pixelHeight / (float) height;
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

const Vertex *Texture::getVertices() const
{
	return vertices;
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
