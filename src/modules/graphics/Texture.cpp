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

	const uint32 BITS_4 = 0xF;

	return (minFilter << 0) | (magFilter << 1) | (mipmapFilter << 2)
	     | (wrapU << 4) | (wrapV << 7) | (wrapW << 10)
	     | (maxAnisotropy << 13) | ((BITS_4 & minLod) << 17) | ((BITS_4 & maxLod) << 21)
	     | (depthSampleMode.hasValue << 25) | (depthSampleMode.value << 26)
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

	s.maxAnisotropy = (key >> 13) & BITS_4;

	s.minLod = (key >> 17) & BITS_4;
	s.maxLod = (key >> 21) & BITS_4;

	s.depthSampleMode.hasValue = ((key >> 25) & BITS_1) != 0;
	s.depthSampleMode.value = (CompareMode) ((key >> 26) & BITS_4);

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

Texture::Texture(Graphics *gfx, const Settings &settings, const Slices *slices)
	: texType(settings.type)
	, format(settings.format)
	, renderTarget(settings.renderTarget)
	, computeWrite(settings.computeWrite)
	, readable(true)
	, viewFormats(settings.viewFormats)
	, mipmapsMode(settings.mipmaps)
	, width(settings.width)
	, height(settings.height)
	, depth(settings.type == TEXTURE_VOLUME ? settings.layers : 1)
	, layers(settings.type == TEXTURE_2D_ARRAY ? settings.layers : 1)
	, mipmapCount(1)
	, pixelWidth(0)
	, pixelHeight(0)
	, requestedMSAA(settings.msaa > 1 ? settings.msaa : 0)
	, samplerState()
	, graphicsMemorySize(0)
	, debugName(settings.debugName)
	, rootView({this, 0, 0})
	, parentView({this, 0, 0})
{
	const auto &caps = gfx->getCapabilities();
	int requestedMipmapCount = settings.mipmapCount;

	if (slices != nullptr && slices->getMipmapCount() > 0 && slices->getSliceCount() > 0)
	{
		texType = slices->getTextureType();

		if (requestedMSAA > 1)
			throw love::Exception("MSAA textures cannot be created from image data.");

		int dataMipmaps = 1;
		if (slices->validate() && slices->getMipmapCount() > 1)
		{
			dataMipmaps = slices->getMipmapCount();

			if (requestedMipmapCount > 0)
				requestedMipmapCount = std::min(requestedMipmapCount, dataMipmaps);
			else
				requestedMipmapCount = dataMipmaps;
		}

		love::image::ImageDataBase *slice = slices->get(0, 0);

		format = slice->getFormat();
		if (isGammaCorrect() && !slice->isLinear())
			format = getSRGBPixelFormat(format);

		pixelWidth = slice->getWidth();
		pixelHeight = slice->getHeight();

		if (texType == TEXTURE_2D_ARRAY)
			layers = slices->getSliceCount();
		else if (texType == TEXTURE_VOLUME)
			depth = slices->getSliceCount();

		width  = (int) (pixelWidth / settings.dpiScale + 0.5);
		height = (int) (pixelHeight / settings.dpiScale + 0.5);

		if (isCompressed() && dataMipmaps <= 1)
			mipmapsMode = MIPMAPS_NONE;
	}
	else
	{
		if (isCompressed())
			throw love::Exception("Compressed textures must be created with initial data.");

		pixelWidth = (int) ((width * settings.dpiScale) + 0.5);
		pixelHeight = (int) ((height * settings.dpiScale) + 0.5);
	}

	if (settings.readable.hasValue)
		readable = settings.readable.value;
	else
		readable = !renderTarget || !isPixelFormatDepthStencil(format);

	format = gfx->getSizedFormat(format);
	if (!isGammaCorrect() || settings.linear)
		format = getLinearPixelFormat(format);

	if (mipmapsMode == MIPMAPS_AUTO && isCompressed())
		mipmapsMode = MIPMAPS_MANUAL;

	if (mipmapsMode != MIPMAPS_NONE)
	{
		int totalMipmapCount = getTotalMipmapCount(pixelWidth, pixelHeight, depth);

		if (requestedMipmapCount > 0)
			mipmapCount = std::min(totalMipmapCount, requestedMipmapCount);
		else
			mipmapCount = totalMipmapCount;
	}

	const char *miperr = nullptr;
	if (mipmapsMode == MIPMAPS_AUTO && !supportsGenerateMipmaps(miperr))
	{
		const char *fstr = "unknown";
		love::getConstant(format, fstr);
		throw love::Exception("Automatic mipmap generation is not supported for textures with the %s pixel format.", fstr);
	}

	if (pixelWidth <= 0 || pixelHeight <= 0 || layers <= 0 || depth <= 0)
		throw love::Exception("Texture dimensions must be greater than 0.");

	if (texType != TEXTURE_2D && requestedMSAA > 1)
		throw love::Exception("MSAA is only supported for textures with the 2D texture type.");

	if (!renderTarget && requestedMSAA > 1)
		throw love::Exception("MSAA is only supported with render target textures.");

	if (readable && isPixelFormatDepthStencil(format) && settings.msaa > 1)
		throw love::Exception("Readable depth/stencil textures with MSAA are not currently supported.");

	if ((!readable || settings.msaa > 1) && mipmapsMode != MIPMAPS_NONE)
		throw love::Exception("Non-readable and MSAA textures cannot have mipmaps.");

	if (!readable && texType != TEXTURE_2D)
		throw love::Exception("Non-readable pixel formats are only supported for 2D texture types.");

	if (isCompressed() && renderTarget)
		throw love::Exception("Compressed textures cannot be render targets.");

	if (isPixelFormatDepthStencil(format) && !renderTarget)
		throw love::Exception("Depth or stencil pixel formats are only supported with render target textures.");

	if (isPixelFormatDepthStencil(format) && texType == TEXTURE_VOLUME)
		throw love::Exception("Volume texture types are not supported with depth or stencil pixel formats.");

	for (PixelFormat viewformat : viewFormats)
	{
		if (getLinearPixelFormat(viewformat) == getLinearPixelFormat(format))
			continue;

		if (isPixelFormatCompressed(format) || isPixelFormatCompressed(viewformat))
			throw love::Exception("Compressed textures cannot use different pixel formats for texture views, aside from sRGB versus linear variants of the same pixel format.");

		if (isPixelFormatColor(viewformat) != isPixelFormatColor(format))
			throw love::Exception("Color-format textures cannot use depth/stencil pixel formats and vice versa, in texture views.");

		// TODO: depth[24|32f]_stencil8 -> stencil8 can work.
		if (isPixelFormatDepthStencil(viewformat))
			throw love::Exception("Using different pixel formats for texture views is not currently supported for depth or stencil formats.");

		size_t viewbytes = getPixelFormatBlockSize(viewformat);
		size_t basebytes = getPixelFormatBlockSize(format);

		if (viewbytes != basebytes)
			throw love::Exception("Texture view pixel formats must have the same bits per pixel as the base texture's pixel format.");
	}

	validatePixelFormat(gfx);

	if (!caps.textureTypes[texType])
	{
		const char *textypestr = "unknown";
		Texture::getConstant(texType, textypestr);
		throw love::Exception("%s textures are not supported on this system.", textypestr);
	}

	validateDimensions(true);

	samplerState = gfx->getDefaultSamplerState();

	if (getMipmapCount() == 1)
		samplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;

	Quad::Viewport v = {0, 0, (double) width, (double) height};
	quad.set(new Quad(v, width, height), Acquire::NORETAIN);

	++textureCount;
}

Texture::Texture(Graphics *gfx, Texture *base, const ViewSettings &viewsettings)
	: texType(viewsettings.type.get(base->getTextureType()))
	, format(viewsettings.format.get(base->getPixelFormat()))
	, renderTarget(base->renderTarget)
	, computeWrite(base->computeWrite)
	, readable(base->readable)
	, viewFormats(base->viewFormats)
	, mipmapsMode(base->mipmapsMode)
	, width(1)
	, height(1)
	, depth(1)
	, layers(1)
	, mipmapCount(1)
	, pixelWidth(1)
	, pixelHeight(1)
	, requestedMSAA(base->requestedMSAA)
	, samplerState(base->samplerState)
	, quad(base->quad)
	, graphicsMemorySize(0)
	, debugName(viewsettings.debugName)
	, rootView({base->rootView.texture, 0, 0})
	, parentView({base, viewsettings.mipmapStart.get(0), viewsettings.layerStart.get(0)})
{
	width = base->getWidth(parentView.startMipmap);
	height = base->getHeight(parentView.startMipmap);

	if (texType == TEXTURE_VOLUME)
		depth = base->getDepth(parentView.startMipmap);

	if (texType == TEXTURE_2D_ARRAY)
	{
		int baselayers = base->getTextureType() == TEXTURE_CUBE ? 6 : base->getLayerCount();
		layers = viewsettings.layerCount.get(baselayers - parentView.startLayer);
	}

	mipmapCount = viewsettings.mipmapCount.get(base->getMipmapCount() - parentView.startMipmap);

	pixelWidth = base->getPixelWidth(parentView.startMipmap);
	pixelHeight = base->getPixelHeight(parentView.startMipmap);

	if (parentView.startMipmap < 0)
		throw love::Exception("Invalid mipmap start value for texture view (out of range).");

	if (mipmapCount < 0 || parentView.startMipmap + mipmapCount > base->getMipmapCount())
		throw love::Exception("Invalid mipmap start or count value for texture view (out of range).");

	if (parentView.startLayer < 0)
		throw love::Exception("Invalid layer start value for texture view (out of range).");

	int baseLayerCount = base->getTextureType() == TEXTURE_CUBE ? 6 : base->getLayerCount();
	if (layers < 0 || parentView.startLayer + layers > baseLayerCount)
		throw love::Exception("Invalid layer start or count value for texture view (out of range).");

	if (texType == TEXTURE_CUBE && parentView.startLayer + 6 > baseLayerCount)
		throw love::Exception("Cube texture view cannot fit in the base texture's layers with the given start layer.");

	ViewInfo nextView = { this, 0, 0 };
	while (nextView.texture != rootView.texture)
	{
		nextView = nextView.texture->parentView;
		rootView.startMipmap += nextView.startMipmap;
		rootView.startLayer += nextView.startLayer;
	}

	const auto &caps = gfx->getCapabilities();
	if (!caps.features[Graphics::FEATURE_GLSL4])
		throw love::Exception("Texture views are not supported on this system (GLSL 4 support is necessary.)");

	validatePixelFormat(gfx);

	if (!caps.textureTypes[texType])
	{
		const char *textypestr = "unknown";
		Texture::getConstant(texType, textypestr);
		throw love::Exception("%s textures are not supported on this system.", textypestr);
	}

	if (!readable)
		throw love::Exception("Texture views are not supported for non-readable textures.");

	if (base->getTextureType() == TEXTURE_2D)
	{
		if (texType != TEXTURE_2D && texType != TEXTURE_2D_ARRAY)
			throw love::Exception("Texture views created from a 2D texture must use the 2d or array texture type.");
	}
	else if (base->getTextureType() == TEXTURE_2D_ARRAY || base->getTextureType() == TEXTURE_CUBE)
	{
		if (texType != TEXTURE_2D && texType != TEXTURE_2D_ARRAY && texType != TEXTURE_CUBE)
			throw love::Exception("Texture views created from an array or cube texture must use the 2d, array, or cube texture type.");
	}
	else if (base->getTextureType() == TEXTURE_VOLUME)
	{
		if (texType != TEXTURE_VOLUME)
			throw love::Exception("Texture views created from a volume texture must use the volume texture type.");
	}
	else
	{
		throw love::Exception("Unknown texture type.");
	}

	if (format != base->getPixelFormat())
	{
		if (std::find(viewFormats.begin(), viewFormats.end(), format) == viewFormats.end())
			throw love::Exception("Using a different pixel format in a texture view requires the original texture to be created with a 'viewformats' setting that includes the given format in its list.");
	}

	const char *miperr = nullptr;
	if (mipmapsMode == MIPMAPS_AUTO && !supportsGenerateMipmaps(miperr))
		mipmapsMode = MIPMAPS_MANUAL;

	rootView.texture->retain();
	parentView.texture->retain();
}

Texture::~Texture()
{
	updateGraphicsMemorySize(false);

	if (this == rootView.texture)
		--textureCount;

	if (rootView.texture != this && rootView.texture != nullptr)
		rootView.texture->release();
	if (parentView.texture != this && parentView.texture != nullptr)
		parentView.texture->release();
}

void Texture::updateGraphicsMemorySize(bool loaded)
{
	int64 memsize = 0;

	if (loaded)
	{
		for (int mip = 0; mip < getMipmapCount(); mip++)
		{
			int w = getPixelWidth(mip);
			int h = getPixelHeight(mip);
			int slices = getDepth(mip) * layers * (texType == TEXTURE_CUBE ? 6 : 1);
			memsize += getPixelFormatSliceSize(format, w, h) * slices;
		}

		if (getMSAA() > 1 && isReadable())
		{
			int slices = depth * layers * (texType == TEXTURE_CUBE ? 6 : 1);
			memsize += getPixelFormatSliceSize(format, pixelWidth, pixelHeight) * slices * getMSAA();
		}
		else if (getMSAA() > 1)
			memsize *= getMSAA();
	}

	totalGraphicsMemory = std::max(totalGraphicsMemory - graphicsMemorySize, (int64) 0);

	memsize = std::max(memsize, (int64) 0);
	graphicsMemorySize = memsize;
	totalGraphicsMemory += memsize;
}

void Texture::draw(Graphics *gfx, const Matrix4 &m)
{
	draw(gfx, quad, m);
}

void Texture::draw(Graphics *gfx, Quad *q, const Matrix4 &localTransform)
{
	if (texType == TEXTURE_2D_ARRAY)
	{
		drawLayer(gfx, q->getLayer(), q, localTransform);
		return;
	}

	if (!readable)
		throw love::Exception("Textures with non-readable formats cannot be drawn.");

	if (renderTarget && gfx->isRenderTargetActive(this))
		throw love::Exception("Cannot render a Texture to itself.");

	const Matrix4 &tm = gfx->getTransform();
	bool is2D = tm.isAffine2DTransform();

	Graphics::BatchedDrawCommand cmd;
	cmd.formats[0] = getSinglePositionFormat(is2D);
	cmd.formats[1] = CommonFormat::STf_RGBAub;
	cmd.indexMode = TRIANGLEINDEX_QUADS;
	cmd.vertexCount = 4;
	cmd.texture = this;

	Graphics::BatchedVertexData data = gfx->requestBatchedDraw(cmd);

	Matrix4 t(tm, localTransform);

	if (is2D)
		t.transformXY((Vector2 *) data.stream[0], q->getVertexPositions(), 4);
	else
		t.transformXY0((Vector3 *) data.stream[0], q->getVertexPositions(), 4);

	const Vector2 *texcoords = q->getVertexTexCoords();
	STf_RGBAub *vertexdata = (STf_RGBAub *) data.stream[1];

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
	if (!readable)
		throw love::Exception("Textures with non-readable formats cannot be drawn.");

	if (renderTarget && gfx->isRenderTargetActive(this, layer))
		throw love::Exception("Cannot render a Texture to itself.");

	if (texType != TEXTURE_2D_ARRAY)
		throw love::Exception("drawLayer can only be used with Array Textures.");

	if (layer < 0 || layer >= layers)
		throw love::Exception("Invalid layer: %d (Texture has %d layers)", layer + 1, layers);

	Color32 c = toColor32(gfx->getColor());

	const Matrix4 &tm = gfx->getTransform();
	bool is2D = tm.isAffine2DTransform();

	Matrix4 t(tm, m);

	Graphics::BatchedDrawCommand cmd;
	cmd.formats[0] = getSinglePositionFormat(is2D);
	cmd.formats[1] = CommonFormat::STPf_RGBAub;
	cmd.indexMode = TRIANGLEINDEX_QUADS;
	cmd.vertexCount = 4;
	cmd.texture = this;
	cmd.standardShaderType = Shader::STANDARD_ARRAY;

	Graphics::BatchedVertexData data = gfx->requestBatchedDraw(cmd);

	if (is2D)
		t.transformXY((Vector2 *) data.stream[0], q->getVertexPositions(), 4);
	else
		t.transformXY0((Vector3 *) data.stream[0], q->getVertexPositions(), 4);

	const Vector2 *texcoords = q->getVertexTexCoords();
	STPf_RGBAub *vertexdata = (STPf_RGBAub *) data.stream[1];

	for (int i = 0; i < 4; i++)
	{
		vertexdata[i].s = texcoords[i].x;
		vertexdata[i].t = texcoords[i].y;
		vertexdata[i].p = (float) layer;
		vertexdata[i].color = c;
	}
}

void Texture::uploadImageData(love::image::ImageDataBase *d, int level, int slice, int x, int y)
{
	Rect rect = {x, y, d->getWidth(), d->getHeight()};
	uploadByteData(d->getData(), d->getSize(), level, slice, rect);
}

void Texture::replacePixels(love::image::ImageDataBase *d, int slice, int mipmap, int x, int y, bool reloadmipmaps)
{
	if (!isReadable())
		throw love::Exception("replacePixels can only be called on readable Textures.");

	if (getMSAA() > 1)
		throw love::Exception("replacePixels cannot be called on a MSAA Texture.");

	if (isPixelFormatDepthStencil(format))
		throw love::Exception("replacePixels cannot be called on depth or stencil Textures.");

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && gfx->isRenderTargetActive(this))
		throw love::Exception("replacePixels cannot be called on this Texture while it's an active render target.");

	// No effect if the texture hasn't been created yet.
	if (getHandle() == 0)
		return;

	// ImageData format might be linear but intended to be used as sRGB, so we
	// don't error if only the sRGBness is different.
	if (getLinearPixelFormat(d->getFormat()) != getLinearPixelFormat(getPixelFormat()))
		throw love::Exception("Pixel formats must match.");

	if (mipmap < 0 || mipmap >= getMipmapCount())
		throw love::Exception("Invalid texture mipmap index %d.", mipmap + 1);

	if (slice < 0 || (texType == TEXTURE_CUBE && slice >= 6)
		|| (texType == TEXTURE_VOLUME && slice >= getDepth(mipmap))
		|| (texType == TEXTURE_2D_ARRAY && slice >= getLayerCount()))
	{
		throw love::Exception("Invalid texture slice index %d.", slice + 1);
	}

	Rect rect = {x, y, d->getWidth(), d->getHeight()};

	int mipw = getPixelWidth(mipmap);
	int miph = getPixelHeight(mipmap);

	if (rect.x < 0 || rect.y < 0 || rect.w <= 0 || rect.h <= 0
		|| (rect.x + rect.w) > mipw || (rect.y + rect.h) > miph)
	{
		throw love::Exception("Invalid rectangle dimensions (x=%d, y=%d, w=%d, h=%d) for %dx%d Texture.", rect.x, rect.y, rect.w, rect.h, mipw, miph);
	}

	if (isPixelFormatCompressed(d->getFormat()) && (rect.x != 0 || rect.y != 0 || rect.w != mipw || rect.h != miph))
	{
		const PixelFormatInfo &info = getPixelFormatInfo(d->getFormat());
		int bw = (int) info.blockWidth;
		int bh = (int) info.blockHeight;
		if (rect.x % bw != 0 || rect.y % bh != 0 || rect.w % bw != 0 || rect.h % bh != 0)
		{
			const char *name = nullptr;
			love::getConstant(d->getFormat(), name);
			throw love::Exception("Compressed texture format %s only supports replacing a sub-rectangle with offset and dimensions that are a multiple of %d x %d.", name, bw, bh);
		}
	}

	Graphics::flushBatchedDrawsGlobal();

	uploadImageData(d, mipmap, slice, x, y);

	if (reloadmipmaps && mipmap == 0 && getMipmapCount() > 1)
		generateMipmaps();
}

void Texture::replacePixels(const void *data, size_t size, int slice, int mipmap, const Rect &rect, bool reloadmipmaps)
{
	if (!isReadable() || getMSAA() > 1)
		return;

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && gfx->isRenderTargetActive(this))
		return;

	Graphics::flushBatchedDrawsGlobal();

	uploadByteData(data, size, mipmap, slice, rect);

	if (reloadmipmaps && mipmap == 0 && getMipmapCount() > 1)
		generateMipmaps();
}

bool Texture::supportsGenerateMipmaps(const char *&outReason) const
{
	if (getMipmapsMode() == MIPMAPS_NONE)
	{
		outReason = "generateMipmaps can only be called on a Texture which was created with mipmaps enabled.";
		return false;
	}

	if (isPixelFormatCompressed(format))
	{
		outReason = "generateMipmaps cannot be called on a compressed Texture.";
		return false;
	}

	if (isPixelFormatDepthStencil(format))
	{
		outReason = "generateMipmaps cannot be called on a depth/stencil Texture.";
		return false;
	}

	if (isPixelFormatInteger(format))
	{
		outReason = "generateMipmaps cannot be called on an integer Texture.";
		return false;
	}

	// This should be linear | rt because that's what metal needs, but the above
	// code handles textures can't be used as RTs in metal.
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && !gfx->isPixelFormatSupported(format, PIXELFORMATUSAGEFLAGS_LINEAR))
	{
		outReason = "generateMipmaps cannot be called on textures with formats that don't support linear filtering on this system.";
		return false;
	}

	return true;
}

void Texture::generateMipmaps()
{
	const char *err = nullptr;
	if (!supportsGenerateMipmaps(err))
		throw love::Exception("%s", err);

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && gfx->isRenderTargetActive(this))
		throw love::Exception("generateMipmaps cannot be called on this Texture while it's an active render target.");

	generateMipmapsInternal();
}

bool Texture::isCompressed() const
{
	return isPixelFormatCompressed(format);
}

bool Texture::isFormatLinear() const
{
	return isGammaCorrect() && !isPixelFormatSRGB(format);
}

bool Texture::isValidSlice(int slice, int mip) const
{
	return slice >= 0 && slice < getSliceCount(mip);
}

int Texture::getSliceCount(int mip) const
{
	if (texType == TEXTURE_2D)
		return 1;
	else if (texType == TEXTURE_CUBE)
		return 6;
	else if (texType == TEXTURE_2D_ARRAY)
		return layers;
	else if (texType == TEXTURE_VOLUME)
		return getDepth(mip);
	return 1;
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

int Texture::getRequestedMSAA() const
{
	return requestedMSAA;
}

const SamplerState &Texture::getSamplerState() const
{
	return samplerState;
}

SamplerState Texture::validateSamplerState(SamplerState s) const
{
	if (!readable)
		return s;

	if (s.depthSampleMode.hasValue && !isPixelFormatDepth(format))
		throw love::Exception("Only depth textures can have a depth sample compare mode.");

	if (s.mipmapFilter != SamplerState::MIPMAP_FILTER_NONE && getMipmapCount() == 1)
		s.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;

	if (texType == TEXTURE_CUBE)
		s.wrapU = s.wrapV = s.wrapW = SamplerState::WRAP_CLAMP;

	if (s.minFilter == SamplerState::FILTER_LINEAR || s.magFilter == SamplerState::FILTER_LINEAR || s.mipmapFilter == SamplerState::MIPMAP_FILTER_LINEAR)
	{
		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		if (!gfx->isPixelFormatSupported(format, PIXELFORMATUSAGEFLAGS_LINEAR))
		{
			s.minFilter = s.magFilter = SamplerState::FILTER_NEAREST;
			if (s.mipmapFilter == SamplerState::MIPMAP_FILTER_LINEAR)
				s.mipmapFilter = SamplerState::MIPMAP_FILTER_NEAREST;
		}
	}

	Graphics::flushBatchedDrawsGlobal();

	return s;
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

void Texture::validatePixelFormat(Graphics *gfx) const
{
	uint32 usage = PIXELFORMATUSAGEFLAGS_NONE;
	if (renderTarget)
		usage |= PIXELFORMATUSAGEFLAGS_RENDERTARGET;
	if (readable)
		usage |= PIXELFORMATUSAGEFLAGS_SAMPLE;
	if (computeWrite)
		usage |= PIXELFORMATUSAGEFLAGS_COMPUTEWRITE;

	if (!gfx->isPixelFormatSupported(format, (PixelFormatUsageFlags) usage))
	{
		const char *fstr = "unknown";
		love::getConstant(format, fstr);

		const char *readablestr = "";
		if (readable != !isPixelFormatDepthStencil(format))
			readablestr = readable ? " readable" : " non-readable";

		const char *rtstr = "";
		if (computeWrite)
			rtstr = " as a compute shader-writable texture";
		else if (renderTarget)
			rtstr = " as a render target";

		throw love::Exception("The %s%s pixel format is not supported%s on this system.", fstr, readablestr, rtstr);
	}
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
		throw love::Exception("At least one ImageData or CompressedImageData is required.");

	if (textureType == TEXTURE_CUBE && slicecount != 6)
		throw love::Exception("Cube textures must have exactly 6 sides.");

	image::ImageDataBase *firstdata = get(0, 0);

	int w = firstdata->getWidth();
	int h = firstdata->getHeight();
	PixelFormat format = firstdata->getFormat();
	bool linear = firstdata->isLinear();

	if (textureType == TEXTURE_CUBE && w != h)
		throw love::Exception("Cube textures must have equal widths and heights for each cube face.");

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
				throw love::Exception("All texture layers must have the same mipmap count.");

			if (mipw != realw)
				throw love::Exception("Width of image data (slice %d, mipmap level %d) is incorrect (expected %d, got %d)", slice+1, mip+1, mipw, realw);

			if (miph != realh)
				throw love::Exception("Height of image data (slice %d, mipmap level %d) is incorrect (expected %d, got %d)", slice+1, mip+1, miph, realh);

			if (format != slicedata->getFormat())
				throw love::Exception("All texture slices and mipmaps must have the same pixel format.");

			if (linear != slicedata->isLinear())
				throw love::Exception("All texture slices and mipmaps must have the same linear setting.");
		}

		mipw = std::max(mipw / 2, 1);
		miph = std::max(miph / 2, 1);

		if (textureType == TEXTURE_VOLUME)
			mipslices = std::max(mipslices / 2, 1);
	}

	return true;
}

static StringMap<TextureType, TEXTURE_MAX_ENUM>::Entry texTypeEntries[] =
{
	{ "2d",     TEXTURE_2D       },
	{ "volume", TEXTURE_VOLUME   },
	{ "array",  TEXTURE_2D_ARRAY },
	{ "cube",   TEXTURE_CUBE     },
};

static StringMap<TextureType, TEXTURE_MAX_ENUM> texTypes(texTypeEntries, sizeof(texTypeEntries));

static StringMap<Texture::MipmapsMode, Texture::MIPMAPS_MAX_ENUM>::Entry mipmapEntries[] =
{
	{ "none",   Texture::MIPMAPS_NONE   },
	{ "manual", Texture::MIPMAPS_MANUAL },
	{ "auto",   Texture::MIPMAPS_AUTO   },
};

static StringMap<Texture::MipmapsMode, Texture::MIPMAPS_MAX_ENUM> mipmapModes(mipmapEntries, sizeof(mipmapEntries));

static StringMap<Texture::SettingType, Texture::SETTING_MAX_ENUM>::Entry settingTypeEntries[] =
{
	{ "width",        Texture::SETTING_WIDTH         },
	{ "height",       Texture::SETTING_HEIGHT        },
	{ "layers",       Texture::SETTING_LAYERS        },
	{ "mipmaps",      Texture::SETTING_MIPMAPS       },
	{ "mipmapcount",  Texture::SETTING_MIPMAP_COUNT  },
	{ "format",       Texture::SETTING_FORMAT        },
	{ "linear",       Texture::SETTING_LINEAR        },
	{ "type",         Texture::SETTING_TYPE          },
	{ "dpiscale",     Texture::SETTING_DPI_SCALE     },
	{ "msaa",         Texture::SETTING_MSAA          },
	{ "canvas",       Texture::SETTING_RENDER_TARGET },
	{ "computewrite", Texture::SETTING_COMPUTE_WRITE },
	{ "viewformats",  Texture::SETTING_VIEW_FORMATS  },
	{ "readable",     Texture::SETTING_READABLE      },
	{ "debugname",    Texture::SETTING_DEBUGNAME     },
};

static StringMap<Texture::SettingType, Texture::SETTING_MAX_ENUM> settingTypes(settingTypeEntries, sizeof(settingTypeEntries));

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

bool Texture::getConstant(const char *in, MipmapsMode &out)
{
	return mipmapModes.find(in, out);
}

bool Texture::getConstant(MipmapsMode in, const char *&out)
{
	return mipmapModes.find(in, out);
}

std::vector<std::string> Texture::getConstants(MipmapsMode)
{
	return mipmapModes.getNames();
}

bool Texture::getConstant(const char *in, SettingType &out)
{
	return settingTypes.find(in, out);
}

bool Texture::getConstant(SettingType in, const char *&out)
{
	return settingTypes.find(in, out);
}

const char *Texture::getConstant(SettingType in)
{
	const char *name = nullptr;
	getConstant(in, name);
	return name;
}

std::vector<std::string> Texture::getConstants(SettingType)
{
	return settingTypes.getNames();
}

} // graphics
} // love
