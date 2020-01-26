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

#include "Image.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace metal
{

Image::Image(id<MTLDevice> device, TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings)
	: love::graphics::Image(textype, format, width, height, slices, settings)
	, texture(nil)
	, sampler(nil)
{ @autoreleasepool {
	create(device);
}}

Image::Image(id<MTLDevice> device, const Slices &slices, const Settings &settings)
	: love::graphics::Image(slices, settings)
	, texture(nil)
	, sampler(nil)
{ @autoreleasepool {
	create(device);
}}

Image::~Image()
{ @autoreleasepool {
	texture = nil;
	sampler = nil;
}}

void Image::create(id<MTLDevice> device)
{
	MTLTextureDescriptor *desc = [MTLTextureDescriptor new];

	desc.width = pixelWidth;
	desc.height = pixelHeight;
	desc.depth = depth;
	desc.arrayLength = layers;
	desc.mipmapLevelCount = mipmapCount;
	desc.textureType = Metal::getTextureType(texType, 1);
	desc.pixelFormat = Metal::convertPixelFormat(format, sRGB);
	desc.usage = MTLTextureUsageShaderRead;
	desc.storageMode = MTLStorageModePrivate;

	texture = [device newTextureWithDescriptor:desc];

	if (texture == nil)
		throw love::Exception("Out of graphics memory.");

	// TODO: upload

	if (mipmapsType == MIPMAPS_GENERATED)
		generateMipmaps();
}

void Image::uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r)
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

	[encoder copyFromBuffer:buffer
			   sourceOffset:0
		  sourceBytesPerRow:getPixelFormatRowStride(pixelformat, r.w)
		sourceBytesPerImage:0 // TODO?
				 sourceSize:MTLSizeMake(r.w, r.h, 1)
				  toTexture:texture
		   destinationSlice:slice
		   destinationLevel:level
		  destinationOrigin:MTLOriginMake(r.x, r.y, z)
					options:options];
}}

void Image::generateMipmaps()
{ @autoreleasepool {
	id<MTLBlitCommandEncoder> encoder = Graphics::getInstance()->useBlitEncoder();
	[encoder generateMipmapsForTexture:texture];
}}

} // metal
} // graphics
} // love
