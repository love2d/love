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

#include "Texture.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace metal
{

static MTLTextureType getMTLTextureType(TextureType type, int msaa)
{
	switch (type)
	{
		case TEXTURE_2D: return msaa > 1 ? MTLTextureType2DMultisample : MTLTextureType2D;
		case TEXTURE_VOLUME: return MTLTextureType3D;
		case TEXTURE_2D_ARRAY: return MTLTextureType2DArray;
		case TEXTURE_CUBE: return MTLTextureTypeCube;
		case TEXTURE_MAX_ENUM: return MTLTextureType2D;
	}
	return MTLTextureType2D;
}

Texture::Texture(love::graphics::Graphics *gfx, id<MTLDevice> device, const Settings &settings, const Slices *data)
	: love::graphics::Texture(gfx, settings, data)
	, texture(nil)
	, msaaTexture(nil)
	, sampler(nil)
{ @autoreleasepool {
	MTLTextureDescriptor *desc = [MTLTextureDescriptor new];

	int w = pixelWidth;
	int h = pixelHeight;

	auto formatdesc = Metal::convertPixelFormat(format, sRGB);

	desc.width = w;
	desc.height = h;
	desc.depth = depth;
	desc.arrayLength = layers;
	desc.mipmapLevelCount = mipmapCount;
	desc.textureType = getMTLTextureType(texType, 1);
	desc.pixelFormat = formatdesc.format;
	if (formatdesc.swizzled)
		desc.swizzle = formatdesc.swizzle;
	desc.storageMode = MTLStorageModePrivate;

	if (readable)
		desc.usage |= MTLTextureUsageShaderRead;
	if (renderTarget)
		desc.usage |= MTLTextureUsageRenderTarget;

	texture = [device newTextureWithDescriptor:desc];

	if (texture == nil)
		throw love::Exception("Out of graphics memory.");

	if (getRequestedMSAA() > 1)
	{
		// TODO: sampleCount validation
		desc.sampleCount = getRequestedMSAA();
		desc.textureType = getMTLTextureType(texType, (int)desc.sampleCount);
		desc.usage &= ~MTLTextureUsageShaderRead;

		// TODO: This needs to be cleared, etc.
		msaaTexture = [device newTextureWithDescriptor:desc];
		if (msaaTexture == nil)
		{
			texture = nil;
			throw love::Exception("Out of graphics memory.");
		}
	}

	int mipcount = getMipmapCount();

	int slicecount = 1;
	if (texType == TEXTURE_VOLUME)
		slicecount = getDepth();
	else if (texType == TEXTURE_2D_ARRAY)
		slicecount = getLayerCount();
	else if (texType == TEXTURE_CUBE)
		slicecount = 6;

	for (int mip = 0; mip < mipcount; mip++)
	{
		for (int slice = 0; slice < slicecount; slice++)
		{
			auto imgd = data != nullptr ? data->get(slice, mip) : nullptr;
			if (imgd != nullptr)
				uploadImageData(imgd, mip, slice, 0, 0);
		}
	}

	if (data == nullptr || data->get(0, 0) == nullptr)
	{
		// Initialize all slices to transparent black.
		if (!isPixelFormatDepthStencil(format))
		{
			std::vector<uint8> emptydata(getPixelFormatSliceSize(format, w, h));
			Rect r = {0, 0, w, h};
			for (int i = 0; i < slicecount; i++)
				uploadByteData(format, emptydata.data(), emptydata.size(), 0, i, r);
		}
		else
		{
			// TODO
		}
	}

	// Non-readable textures can't have mipmaps (enforced in the base class),
	// so generateMipmaps here is fine - when they aren't already initialized.
	if (getMipmapCount() > 1 && (data == nullptr || data->getMipmapCount() <= 1))
		generateMipmaps();

	setSamplerState(samplerState);
}}

Texture::~Texture()
{ @autoreleasepool {
	texture = nil;
	msaaTexture = nil;
	sampler = nil;
}}

void Texture::uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r, love::image::ImageDataBase *)
{ @autoreleasepool {
	auto gfx = Graphics::getInstance();
	id<MTLBuffer> buffer = [gfx->device newBufferWithBytes:data
													length:size
												   options:MTLResourceStorageModeShared];

	memcpy(buffer.contents, data, size);

	id<MTLBlitCommandEncoder> encoder = gfx->useBlitEncoder();

	int z = 0;
	if (texType == TEXTURE_VOLUME)
	{
		z = slice;
		slice = 0;
	}

	MTLBlitOption options = MTLBlitOptionNone;

	switch (pixelformat)
	{
#ifdef LOVE_IOS
	case PIXELFORMAT_PVR1_RGB2_UNORM:
	case PIXELFORMAT_PVR1_RGB4_UNORM:
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
		options |= MTLBlitOptionRowLinearPVRTC;
		break;
#endif
	default:
		break;
	}

	size_t rowSize = 0;
	if (isCompressed())
		rowSize = getPixelFormatCompressedBlockRowSize(format, r.w);
	else
		rowSize = getPixelFormatUncompressedRowSize(format, r.w);

	// TODO: Verify this is correct for compressed formats at small sizes.
	size_t sliceSize = getPixelFormatSliceSize(format, r.w, r.h);

	[encoder copyFromBuffer:buffer
			   sourceOffset:0
		  sourceBytesPerRow:rowSize
		sourceBytesPerImage:sliceSize
				 sourceSize:MTLSizeMake(r.w, r.h, 1)
				  toTexture:texture
		   destinationSlice:slice
		   destinationLevel:level
		  destinationOrigin:MTLOriginMake(r.x, r.y, z)
					options:options];
}}

void Texture::generateMipmaps()
{ @autoreleasepool {
	// TODO: alternate method for non-color-renderable and non-filterable
	// pixel formats.
	id<MTLBlitCommandEncoder> encoder = Graphics::getInstance()->useBlitEncoder();
	[encoder generateMipmapsForTexture:texture];
}}

love::image::ImageData *Texture::newImageData(love::image::Image *module, int slice, int mipmap, const Rect &rect)
{
	// TODO
	return nullptr;
}

void Texture::setSamplerState(const SamplerState &s)
{ @autoreleasepool {
	sampler = Graphics::getInstance()->getCachedSampler(s);
}}

} // metal
} // graphics
} // love
