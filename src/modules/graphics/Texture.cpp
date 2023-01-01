/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
	totalGraphicsMemory = std::max(totalGraphicsMemory - graphicsMemorySize, (int64) 0);

	bytes = std::max(bytes, (int64) 0);
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

bool Texture::isValidSlice(int slice) const
{
	if (slice < 0)
		return false;

	if (texType == TEXTURE_CUBE)
		return slice < 6;
	else if (texType == TEXTURE_VOLUME)
		return slice < depth;
	else if (texType == TEXTURE_2D_ARRAY)
		return slice < layers;
	else if (slice > 0)
		return false;

	return true;
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

	const Matrix4 &tm = gfx->getTransform();
	bool is2D = tm.isAffine2DTransform();

	Graphics::StreamDrawCommand cmd;
	cmd.formats[0] = vertex::getSinglePositionFormat(is2D);
	cmd.formats[1] = CommonFormat::STf_RGBAub;
	cmd.indexMode = TriangleIndexMode::QUADS;
	cmd.vertexCount = 4;
	cmd.texture = this;

	Graphics::StreamVertexData data = gfx->requestStreamDraw(cmd);

	Matrix4 t(tm, localTransform);

	if (is2D)
		t.transformXY((Vector2 *) data.stream[0], q->getVertexPositions(), 4);
	else
		t.transformXY0((Vector3 *) data.stream[0], q->getVertexPositions(), 4);

	const Vector2 *texcoords = q->getVertexTexCoords();
	vertex::STf_RGBAub *vertexdata = (vertex::STf_RGBAub *) data.stream[1];

	Color32 c = toColor32(gfx->getColor());

	for (int i = 0; i < 4; i++)
	{
		vertexdata[i].s = texcoords[i].x;
		vertexdata[i].t = texcoords[i].y;
		vertexdata[i].color = c;
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

	Color32 c = toColor32(gfx->getColor());

	const Matrix4 &tm = gfx->getTransform();
	bool is2D = tm.isAffine2DTransform();

	Matrix4 t(tm, m);

	Graphics::StreamDrawCommand cmd;
	cmd.formats[0] = vertex::getSinglePositionFormat(is2D);
	cmd.formats[1] = CommonFormat::STPf_RGBAub;
	cmd.indexMode = TriangleIndexMode::QUADS;
	cmd.vertexCount = 4;
	cmd.texture = this;
	cmd.standardShaderType = Shader::STANDARD_ARRAY;

	Graphics::StreamVertexData data = gfx->requestStreamDraw(cmd);

	if (is2D)
		t.transformXY((Vector2 *) data.stream[0], q->getVertexPositions(), 4);
	else
		t.transformXY0((Vector3 *) data.stream[0], q->getVertexPositions(), 4);

	const Vector2 *texcoords = q->getVertexTexCoords();
	vertex::STPf_RGBAub *vertexdata = (vertex::STPf_RGBAub *) data.stream[1];

	for (int i = 0; i < 4; i++)
	{
		vertexdata[i].s = texcoords[i].x;
		vertexdata[i].t = texcoords[i].y;
		vertexdata[i].p = (float) layer;
		vertexdata[i].color = c;
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

float Texture::getDPIScale() const
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

int Texture::getTotalMipmapCount(int w, int h)
{
	return (int) log2(std::max(w, h)) + 1;
}

int Texture::getTotalMipmapCount(int w, int h, int d)
{
	return (int) log2(std::max(std::max(w, h), d)) + 1;
}

bool Texture::validateDimensions(bool throwException) const
{
	bool success = true;

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx == nullptr)
		return false;

	const Graphics::Capabilities &caps = gfx->getCapabilities();

	int max2Dsize   = (int) caps.limits[Graphics::LIMIT_TEXTURE_SIZE];
	int max3Dsize   = (int) caps.limits[Graphics::LIMIT_VOLUME_TEXTURE_SIZE];
	int maxcubesize = (int) caps.limits[Graphics::LIMIT_CUBE_TEXTURE_SIZE];
	int maxlayers   = (int) caps.limits[Graphics::LIMIT_TEXTURE_LAYERS];

	int largestdim = 0;
	const char *largestname = nullptr;

	if ((texType == TEXTURE_2D || texType == TEXTURE_2D_ARRAY) && (pixelWidth > max2Dsize || pixelHeight > max2Dsize))
	{
		success = false;
		largestdim = std::max(pixelWidth, pixelHeight);
		largestname = pixelWidth > pixelHeight ? "pixel width" : "pixel height";
	}
	else if (texType == TEXTURE_2D_ARRAY && layers > maxlayers)
	{
		success = false;
		largestdim = layers;
		largestname = "array layer count";
	}
	else if (texType == TEXTURE_CUBE && (pixelWidth > maxcubesize || pixelWidth != pixelHeight))
	{
		success = false;
		largestdim = std::max(pixelWidth, pixelHeight);
		largestname = pixelWidth > pixelHeight ? "pixel width" : "pixel height";

		if (throwException && pixelWidth != pixelHeight)
			throw love::Exception("Cubemap textures must have equal width and height.");
	}
	else if (texType == TEXTURE_VOLUME && (pixelWidth > max3Dsize || pixelHeight > max3Dsize || depth > max3Dsize))
	{
		success = false;
		largestdim = std::max(std::max(pixelWidth, pixelHeight), depth);
		if (largestdim == pixelWidth)
			largestname = "pixel width";
		else if (largestdim == pixelHeight)
			largestname = "pixel height";
		else
			largestname = "pixel depth";
	}

	if (throwException && largestname != nullptr)
		throw love::Exception("Cannot create texture: %s of %d is too large for this system.", largestname, largestdim);

	return success;
}

bool Texture::getConstant(const char *in, TextureType &out)
{
	return texTypes.find(in, out);
}

bool Texture::getConstant(TextureType in, const char *&out)
{
	return texTypes.find(in, out);
}

std::vector<std::string> Texture::getConstants(TextureType)
{
	return texTypes.getNames();
}

bool Texture::getConstant(const char *in, FilterMode &out)
{
	return filterModes.find(in, out);
}

bool Texture::getConstant(FilterMode in, const char *&out)
{
	return filterModes.find(in, out);
}

std::vector<std::string> Texture::getConstants(FilterMode)
{
	return filterModes.getNames();
}

bool Texture::getConstant(const char *in, WrapMode &out)
{
	return wrapModes.find(in, out);
}

bool Texture::getConstant(WrapMode in, const char *&out)
{
	return wrapModes.find(in, out);
}

std::vector<std::string> Texture::getConstants(WrapMode)
{
	return wrapModes.getNames();
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
