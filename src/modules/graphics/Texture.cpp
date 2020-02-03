/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

uint64 SamplerState::toKey() const
{
	union { float f; uint32 i; } conv;
	conv.f = lodBias;

	return (minFilter << 0) | (magFilter << 1) | (mipmapFilter << 2)
	     | (wrapU << 4) | (wrapV << 7) | (wrapW << 10)
	     | (maxAnisotropy << 12) | (minLod << 16) | (maxLod << 20)
	     | (depthSampleMode.hasValue << 24) | (depthSampleMode.value << 25)
	     | ((uint64)conv.i << 32);
}

SamplerState SamplerState::fromKey(uint64 key)
{
	const uint32 BITS_1 = 0x1;
	const uint32 BITS_2 = 0x3;
	const uint32 BITS_3 = 0x7;
	const uint32 BITS_4 = 0xF;

	SamplerState s;

	s.minFilter = (FilterMode) ((key >> 0) & BITS_1);
	s.magFilter = (FilterMode) ((key >> 1) & BITS_1);
	s.mipmapFilter = (MipmapFilterMode) ((key >> 2) & BITS_2);

	s.wrapU = (WrapMode) ((key >> 4 ) & BITS_3);
	s.wrapV = (WrapMode) ((key >> 7 ) & BITS_3);
	s.wrapW = (WrapMode) ((key >> 10) & BITS_3);

	s.maxAnisotropy = (key >> 12) & BITS_4;

	s.minLod = (key >> 16) & BITS_4;
	s.maxLod = (key >> 20) & BITS_4;

	s.depthSampleMode.hasValue = ((key >> 24) & BITS_1) != 0;
	s.depthSampleMode.value = (CompareMode) ((key >> 25) & BITS_4);

	union { float f; uint32 i; } conv;
	conv.i = (uint32) (key >> 32);
	s.lodBias = conv.f;

	return s;
}

bool SamplerState::isClampZeroOrOne(WrapMode w)
{
	return w == WRAP_CLAMP_ONE || w == WRAP_CLAMP_ZERO;
}

static StringMap<SamplerState::FilterMode, SamplerState::FILTER_MAX_ENUM>::Entry filterModeEntries[] =
{
	{ "linear",  SamplerState::FILTER_LINEAR  },
	{ "nearest", SamplerState::FILTER_NEAREST },
};

static StringMap<SamplerState::FilterMode, SamplerState::FILTER_MAX_ENUM> filterModes(filterModeEntries, sizeof(filterModeEntries));

static StringMap<SamplerState::MipmapFilterMode, SamplerState::MIPMAP_FILTER_MAX_ENUM>::Entry mipmapFilterModeEntries[] =
{
	{ "none",    SamplerState::MIPMAP_FILTER_NONE    },
	{ "linear",  SamplerState::MIPMAP_FILTER_LINEAR  },
	{ "nearest", SamplerState::MIPMAP_FILTER_NEAREST },
};

static StringMap<SamplerState::MipmapFilterMode, SamplerState::MIPMAP_FILTER_MAX_ENUM> mipmapFilterModes(mipmapFilterModeEntries, sizeof(mipmapFilterModeEntries));

static StringMap<SamplerState::WrapMode, SamplerState::WRAP_MAX_ENUM>::Entry wrapModeEntries[] =
{
	{ "clamp",          SamplerState::WRAP_CLAMP           },
	{ "clampzero",      SamplerState::WRAP_CLAMP_ZERO      },
	{ "clampone",       SamplerState::WRAP_CLAMP_ONE       },
	{ "repeat",         SamplerState::WRAP_REPEAT          },
	{ "mirroredrepeat", SamplerState::WRAP_MIRRORED_REPEAT },
};

static StringMap<SamplerState::WrapMode, SamplerState::WRAP_MAX_ENUM> wrapModes(wrapModeEntries, sizeof(wrapModeEntries));

bool SamplerState::getConstant(const char *in, FilterMode &out)
{
	return filterModes.find(in, out);
}

bool SamplerState::getConstant(FilterMode in, const char *&out)
{
	return filterModes.find(in, out);
}

std::vector<std::string> SamplerState::getConstants(FilterMode)
{
	return filterModes.getNames();
}

bool SamplerState::getConstant(const char *in, MipmapFilterMode &out)
{
	return mipmapFilterModes.find(in, out);
}

bool SamplerState::getConstant(MipmapFilterMode in, const char *&out)
{
	return mipmapFilterModes.find(in, out);
}

std::vector<std::string> SamplerState::getConstants(MipmapFilterMode)
{
	return mipmapFilterModes.getNames();
}

bool SamplerState::getConstant(const char *in, WrapMode &out)
{
	return wrapModes.find(in, out);
}

bool SamplerState::getConstant(WrapMode in, const char *&out)
{
	return wrapModes.find(in, out);
}

std::vector<std::string> SamplerState::getConstants(WrapMode)
{
	return wrapModes.getNames();
}

love::Type Texture::type("Texture", &Drawable::type);
int Texture::textureCount = 0;
int64 Texture::totalGraphicsMemory = 0;

Texture::Texture(TextureType texType)
	: texType(texType)
	, format(PIXELFORMAT_UNKNOWN)
	, renderTarget(false)
	, readable(true)
	, sRGB(false)
	, width(0)
	, height(0)
	, depth(1)
	, layers(1)
	, mipmapCount(1)
	, pixelWidth(0)
	, pixelHeight(0)
	, samplerState()
	, graphicsMemorySize(0)
{
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr)
		samplerState = gfx->getDefaultSamplerState();
	++textureCount;
}

Texture::~Texture()
{
	--textureCount;
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

bool Texture::isRenderTarget() const
{
	return renderTarget;
}

bool Texture::isReadable() const
{
	return readable;
}

bool Texture::isCompressed() const
{
	return isPixelFormatCompressed(format);
}

bool Texture::isFormatLinear() const
{
	return isGammaCorrect() && !sRGB && format != PIXELFORMAT_sRGBA8_UNORM;
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

	if (renderTarget && gfx->isRenderTargetActive(this))
		throw love::Exception("Cannot render a Texture to itself.");

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

	if (renderTarget && gfx->isRenderTargetActive(this, layer))
		throw love::Exception("Cannot render a Texture to itself.");

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

void Texture::setSamplerState(const SamplerState &s)
{
	if (s.depthSampleMode.hasValue && (!readable || !isPixelFormatDepthStencil(format)))
		throw love::Exception("Only readable depth textures can have a depth sample compare mode.");

	Graphics::flushStreamDrawsGlobal();

	samplerState = s;

	if (samplerState.mipmapFilter != SamplerState::MIPMAP_FILTER_NONE && getMipmapCount() == 1)
		samplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;

	if (texType == TEXTURE_CUBE)
		samplerState.wrapU = samplerState.wrapV = samplerState.wrapW = SamplerState::WRAP_CLAMP;
}

const SamplerState &Texture::getSamplerState() const
{
	return samplerState;
}

Quad *Texture::getQuad() const
{
	return quad;
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

Texture::Slices::Slices(TextureType textype)
	: textureType(textype)
{
}

void Texture::Slices::clear()
{
	data.clear();
}

void Texture::Slices::set(int slice, int mipmap, love::image::ImageDataBase *d)
{
	if (textureType == TEXTURE_VOLUME)
	{
		if (mipmap >= (int) data.size())
			data.resize(mipmap + 1);

		if (slice >= (int) data[mipmap].size())
			data[mipmap].resize(slice + 1);

		data[mipmap][slice].set(d);
	}
	else
	{
		if (slice >= (int) data.size())
			data.resize(slice + 1);

		if (mipmap >= (int) data[slice].size())
			data[slice].resize(mipmap + 1);

		data[slice][mipmap].set(d);
	}
}

love::image::ImageDataBase *Texture::Slices::get(int slice, int mipmap) const
{
	if (slice < 0 || slice >= getSliceCount(mipmap))
		return nullptr;

	if (mipmap < 0 || mipmap >= getMipmapCount(slice))
		return nullptr;

	if (textureType == TEXTURE_VOLUME)
		return data[mipmap][slice].get();
	else
		return data[slice][mipmap].get();
}

void Texture::Slices::add(love::image::CompressedImageData *cdata, int startslice, int startmip, bool addallslices, bool addallmips)
{
	int slicecount = addallslices ? cdata->getSliceCount() : 1;
	int mipcount = addallmips ? cdata->getMipmapCount() : 1;

	for (int mip = 0; mip < mipcount; mip++)
	{
		for (int slice = 0; slice < slicecount; slice++)
			set(startslice + slice, startmip + mip, cdata->getSlice(slice, mip));
	}
}

int Texture::Slices::getSliceCount(int mip) const
{
	if (textureType == TEXTURE_VOLUME)
	{
		if (mip < 0 || mip >= (int) data.size())
			return 0;

		return (int) data[mip].size();
	}
	else
		return (int) data.size();
}

int Texture::Slices::getMipmapCount(int slice) const
{
	if (textureType == TEXTURE_VOLUME)
		return (int) data.size();
	else
	{
		if (slice < 0 || slice >= (int) data.size())
			return 0;

		return data[slice].size();
	}
}

bool Texture::Slices::validate() const
{
	int slicecount = getSliceCount();
	int mipcount = getMipmapCount(0);

	if (slicecount == 0 || mipcount == 0)
		throw love::Exception("At least one ImageData or CompressedImageData is required!");

	if (textureType == TEXTURE_CUBE && slicecount != 6)
		throw love::Exception("Cube textures must have exactly 6 sides.");

	image::ImageDataBase *firstdata = get(0, 0);

	int w = firstdata->getWidth();
	int h = firstdata->getHeight();
	int depth = textureType == TEXTURE_VOLUME ? slicecount : 1;
	PixelFormat format = firstdata->getFormat();

	int expectedmips = Texture::getTotalMipmapCount(w, h, depth);

	if (mipcount != expectedmips && mipcount != 1)
		throw love::Exception("Image does not have all required mipmap levels (expected %d, got %d)", expectedmips, mipcount);

	if (textureType == TEXTURE_CUBE && w != h)
		throw love::Exception("Cube images must have equal widths and heights for each cube face.");

	int mipw = w;
	int miph = h;
	int mipslices = slicecount;

	for (int mip = 0; mip < mipcount; mip++)
	{
		if (textureType == TEXTURE_VOLUME)
		{
			slicecount = getSliceCount(mip);

			if (slicecount != mipslices)
				throw love::Exception("Invalid number of image data layers in mipmap level %d (expected %d, got %d)", mip+1, mipslices, slicecount);
		}

		for (int slice = 0; slice < slicecount; slice++)
		{
			auto slicedata = get(slice, mip);

			if (slicedata == nullptr)
				throw love::Exception("Missing image data (slice %d, mipmap level %d)", slice+1, mip+1);

			int realw = slicedata->getWidth();
			int realh = slicedata->getHeight();

			if (getMipmapCount(slice) != mipcount)
				throw love::Exception("All Image layers must have the same mipmap count.");

			if (mipw != realw)
				throw love::Exception("Width of image data (slice %d, mipmap level %d) is incorrect (expected %d, got %d)", slice+1, mip+1, mipw, realw);

			if (miph != realh)
				throw love::Exception("Height of image data (slice %d, mipmap level %d) is incorrect (expected %d, got %d)", slice+1, mip+1, miph, realh);

			if (format != slicedata->getFormat())
				throw love::Exception("All Image slices and mipmaps must have the same pixel format.");
		}

		mipw = std::max(mipw / 2, 1);
		miph = std::max(miph / 2, 1);

		if (textureType == TEXTURE_VOLUME)
			mipslices = std::max(mipslices / 2, 1);
	}

	return true;
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

StringMap<TextureType, TEXTURE_MAX_ENUM>::Entry Texture::texTypeEntries[] =
{
	{ "2d", TEXTURE_2D },
	{ "volume", TEXTURE_VOLUME },
	{ "array", TEXTURE_2D_ARRAY },
	{ "cube", TEXTURE_CUBE },
};

StringMap<TextureType, TEXTURE_MAX_ENUM> Texture::texTypes(Texture::texTypeEntries, sizeof(Texture::texTypeEntries));

} // graphics
} // love
