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

#include "Texture.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{

love::Type Texture::type("Texture", &Drawable::type);

Texture::Filter Texture::defaultFilter;
Texture::FilterMode Texture::defaultMipmapFilter = Texture::FILTER_LINEAR;
float Texture::defaultMipmapSharpness = 0.0f;

Texture::Texture()
	: format(PIXELFORMAT_UNKNOWN)
	, width(0)
	, height(0)
	, pixelWidth(0)
	, pixelHeight(0)
	, filter(defaultFilter)
	, wrap()
	, vertices()
{
}

Texture::~Texture()
{
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

bool Texture::getConstant(const char *in, FilterMode &out)
{
	return filterModes.find(in, out);
}

bool Texture::getConstant(FilterMode in, const char  *&out)
{
	return filterModes.find(in, out);
}

bool Texture::getConstant(const char *in, WrapMode &out)
{
	return wrapModes.find(in, out);
}

bool Texture::getConstant(WrapMode in, const char  *&out)
{
	return wrapModes.find(in, out);
}

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
