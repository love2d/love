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

Texture::Texture(love::graphics::Graphics *gfxbase, id<MTLDevice> device, const Settings &settings, const Slices *data)
	: love::graphics::Texture(gfxbase, settings, data)
{ @autoreleasepool {
	auto gfx = (Graphics *) gfxbase;

	MTLTextureDescriptor *desc = [MTLTextureDescriptor new];

	int w = pixelWidth;
	int h = pixelHeight;

	desc.width = w;
	desc.height = h;
	desc.depth = depth;
	desc.arrayLength = layers;
	desc.mipmapLevelCount = mipmapCount;
	desc.textureType = getMTLTextureType(texType, 1);

	auto formatdesc = Metal::convertPixelFormat(device, format);
	desc.pixelFormat = formatdesc.format;
	if (formatdesc.swizzled)
	{
		// Swizzled formats are already only used on supported systems, this
		// just silences a compiler warning about it.
		if (@available(macOS 10.15, iOS 13, *))
			desc.swizzle = formatdesc.swizzle;
	}

	desc.storageMode = MTLStorageModePrivate;

	if (readable)
		desc.usage |= MTLTextureUsageShaderRead;
	if (renderTarget)
		desc.usage |= MTLTextureUsageRenderTarget;
	if (computeWrite)
		desc.usage |= MTLTextureUsageShaderWrite;

	for (PixelFormat viewformat : viewFormats)
	{
		if (getLinearPixelFormat(viewformat) != getLinearPixelFormat(format))
		{
			desc.usage |= MTLTextureUsagePixelFormatView;
			break;
		}
	}

	texture = [device newTextureWithDescriptor:desc];

	if (texture == nil)
		throw love::Exception("Out of graphics memory.");

	if (!debugName.empty())
		texture.label = @(debugName.c_str());

	actualMSAASamples = gfx->getClosestMSAASamples(getRequestedMSAA());

	if (actualMSAASamples > 1)
	{
		desc.sampleCount = actualMSAASamples;
		desc.textureType = getMTLTextureType(texType, actualMSAASamples);
		desc.usage &= ~MTLTextureUsageShaderRead;

		msaaTexture = [device newTextureWithDescriptor:desc];
		if (msaaTexture == nil)
		{
			texture = nil;
			throw love::Exception("Out of graphics memory.");
		}

		if (!debugName.empty())
			msaaTexture.label = [@(debugName.c_str()) stringByAppendingString:@" (MSAA buffer)"];
	}

	int mipcount = getMipmapCount();

	bool cangeneratemips = true;

	if (isPixelFormatDepthStencil(format) || isPixelFormatCompressed(format))
		cangeneratemips = false;

	// generateMipmapsForTexture is only supported for color renderable +
	// filterable formats.
	uint32 genmipsflags = PIXELFORMATUSAGEFLAGS_LINEAR | PIXELFORMATUSAGEFLAGS_RENDERTARGET;
	if (!gfx->isPixelFormatSupported(format, (PixelFormatUsageFlags) genmipsflags))
		cangeneratemips = false;

	bool shouldgeneratemips = false;

	std::vector<uint8> emptydata;
	MTLRenderPassDescriptor *passdesc = nil;

	// Initialize texture.
	for (int mip = 0; mip < mipcount; mip++)
	{
		for (int slice = 0; slice < getSliceCount(mip); slice++)
		{
			auto imgd = data != nullptr ? data->get(slice, mip) : nullptr;
			if (imgd != nullptr)
			{
				uploadImageData(imgd, mip, slice, 0, 0);
			}
			else if (mip > 0 && cangeneratemips)
			{
				// Handled in the generateMipmaps call below.
				shouldgeneratemips = true;
				continue;
			}
			else if (getMSAA() <= 1 && !isPixelFormatDepthStencil(format) && !isPixelFormatCompressed(format))
			{
				// Initialize to transparent black.
				if (emptydata.empty())
					emptydata.resize(getPixelFormatSliceSize(format, w, h));

				Rect r = {0, 0, getPixelWidth(mip), getPixelHeight(mip)};
				uploadByteData(emptydata.data(), emptydata.size(), mip, slice, r);
			}
			else if (isRenderTarget())
			{
				// Clear to transparent black.
				gfx->submitAllEncoders(Graphics::SUBMIT_STORE);
				id<MTLCommandBuffer> cmd = gfx->useCommandBuffer();

				if (passdesc == nil)
					passdesc = [MTLRenderPassDescriptor renderPassDescriptor];

				auto configattachment = [&](MTLRenderPassAttachmentDescriptor *attachment)
				{
					attachment.texture = texture;
					attachment.level = mip;
					attachment.slice = texType == TEXTURE_VOLUME ? 0 : slice;
					attachment.depthPlane = texType == TEXTURE_VOLUME ? slice : 0;
					attachment.loadAction = MTLLoadActionClear;
					attachment.storeAction = MTLStoreActionStore;

					if (actualMSAASamples > 1)
					{
						attachment.texture = msaaTexture;
						attachment.resolveTexture = texture;
						attachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
					}
				};

				if (isPixelFormatDepth(format))
				{
					configattachment(passdesc.depthAttachment);
					passdesc.depthAttachment.clearDepth = 1.0;
				}
				if (isPixelFormatStencil(format))
				{
					configattachment(passdesc.stencilAttachment);
					passdesc.stencilAttachment.clearStencil = 0;
				}
				if (!isPixelFormatDepthStencil(format))
				{
					configattachment(passdesc.colorAttachments[0]);
					passdesc.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
				}

				id<MTLRenderCommandEncoder> encoder = [cmd renderCommandEncoderWithDescriptor:passdesc];
				[encoder endEncoding];
			}
			else
			{
				// Shouldn't be possible to get here.
				throw love::Exception("Could not initialize texture to transparent black.");
			}
		}
	}

	updateGraphicsMemorySize(true);

	// Non-readable textures can't have mipmaps (enforced in the base class),
	// so generateMipmaps here is fine - when they aren't already initialized.
	if (shouldgeneratemips)
		generateMipmaps();

	setSamplerState(samplerState);
}}

Texture::Texture(love::graphics::Graphics *gfx, id<MTLDevice> device, love::graphics::Texture *base, const Texture::ViewSettings &viewsettings)
	: love::graphics::Texture(gfx, base, viewsettings)
{
	id<MTLTexture> basetex = ((Texture *) base)->texture;
	auto formatdesc = Metal::convertPixelFormat(device, format);
	int slices = texType == TEXTURE_CUBE ? 6 : getLayerCount();

	if (formatdesc.swizzled)
	{
		if (@available(macOS 10.15, iOS 13, *))
		{
			texture = [basetex newTextureViewWithPixelFormat:formatdesc.format
												 textureType:getMTLTextureType(texType, 1)
													  levels:NSMakeRange(parentView.startMipmap, mipmapCount)
													  slices:NSMakeRange(parentView.startLayer, slices)
													 swizzle:formatdesc.swizzle];
		}
	}
	else
	{
		texture = [basetex newTextureViewWithPixelFormat:formatdesc.format
											 textureType:getMTLTextureType(texType, 1)
												  levels:NSMakeRange(parentView.startMipmap, mipmapCount)
												  slices:NSMakeRange(parentView.startLayer, slices)];
	}

	if (texture == nil)
		throw love::Exception("Could not create Metal texture view.");

	if (!debugName.empty())
		texture.label = @(debugName.c_str());

	setSamplerState(samplerState);
}

Texture::~Texture()
{ @autoreleasepool {
	texture = nil;
	msaaTexture = nil;
	sampler = nil;
}}

void Texture::uploadByteData(const void *data, size_t size, int level, int slice, const Rect &r)
{ @autoreleasepool {
	auto gfx = Graphics::getInstance();
	id<MTLBuffer> buffer = [gfx->device newBufferWithBytes:data
													length:size
												   options:MTLResourceStorageModeShared];

	id<MTLBlitCommandEncoder> encoder = gfx->useBlitEncoder();

	int z = 0;
	if (texType == TEXTURE_VOLUME)
	{
		z = slice;
		slice = 0;
	}

	MTLBlitOption options = MTLBlitOptionNone;

	switch (format)
	{
	case PIXELFORMAT_PVR1_RGB2_UNORM:
	case PIXELFORMAT_PVR1_RGB4_UNORM:
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
		if (@available(macOS 11.0, iOS 9.0, *))
			options |= MTLBlitOptionRowLinearPVRTC;
		break;
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

void Texture::generateMipmapsInternal()
{ @autoreleasepool {
	id<MTLBlitCommandEncoder> encoder = Graphics::getInstance()->useBlitEncoder();
	[encoder generateMipmapsForTexture:texture];
}}

void Texture::copyFromBuffer(love::graphics::Buffer *source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect &rect)
{ @autoreleasepool {
	id<MTLBlitCommandEncoder> encoder = Graphics::getInstance()->useBlitEncoder();
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)(void *) source->getHandle();

	size_t rowSize = 0;
	if (isCompressed())
		rowSize = getPixelFormatCompressedBlockRowSize(format, sourcewidth);
	else
		rowSize = getPixelFormatUncompressedRowSize(format, sourcewidth);

	int z = texType == TEXTURE_VOLUME ? slice : 0;

	MTLBlitOption options = MTLBlitOptionNone;
	if (isPixelFormatDepthStencil(format))
		options = MTLBlitOptionDepthFromDepthStencil;

	[encoder copyFromBuffer:buffer
			   sourceOffset:sourceoffset
		  sourceBytesPerRow:rowSize
		sourceBytesPerImage:size
				 sourceSize:MTLSizeMake(rect.w, rect.h, 1)
				  toTexture:texture
		   destinationSlice:texType == TEXTURE_VOLUME ? 0 : slice
		   destinationLevel:mipmap
		  destinationOrigin:MTLOriginMake(rect.x, rect.y, z)
					options:options];
}}

void Texture::copyToBuffer(love::graphics::Buffer *dest, int slice, int mipmap, const Rect &rect, size_t destoffset, int destwidth, size_t size)
{ @autoreleasepool {
	id<MTLBlitCommandEncoder> encoder = Graphics::getInstance()->useBlitEncoder();
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)(void *) dest->getHandle();

	size_t rowSize = 0;
	if (isCompressed())
		rowSize = getPixelFormatCompressedBlockRowSize(format, destwidth);
	else
		rowSize = getPixelFormatUncompressedRowSize(format, destwidth);

	int z = texType == TEXTURE_VOLUME ? slice : 0;

	MTLBlitOption options = MTLBlitOptionNone;
	if (isPixelFormatDepthStencil(format))
		options = MTLBlitOptionDepthFromDepthStencil;

	[encoder copyFromTexture:texture
				 sourceSlice:texType == TEXTURE_VOLUME ? 0 : slice
				 sourceLevel:mipmap
				sourceOrigin:MTLOriginMake(rect.x, rect.y, z)
				  sourceSize:MTLSizeMake(rect.w, rect.h, 1)
					toBuffer:buffer
		   destinationOffset:destoffset
	  destinationBytesPerRow:rowSize
	destinationBytesPerImage:size
					 options:options];
}}

void Texture::setSamplerState(const SamplerState &s)
{ @autoreleasepool {
	if (s.depthSampleMode.hasValue && !Graphics::getInstance()->isDepthCompareSamplerSupported())
		throw love::Exception("Depth comparison sampling in shaders is not supported on this system.");

	samplerState = validateSamplerState(s);
	sampler = Graphics::getInstance()->getCachedSampler(samplerState);
}}

} // metal
} // graphics
} // love
