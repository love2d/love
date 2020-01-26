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

#include "Canvas.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace metal
{

Canvas::Canvas(id<MTLDevice> device, const Settings &settings)
	: love::graphics::Canvas(settings)
{ @autoreleasepool {
	MTLTextureDescriptor *desc = [MTLTextureDescriptor new];

	// TODO: sampleCount validation
	desc.sampleCount = getRequestedMSAA();

	desc.width = pixelWidth;
	desc.height = pixelHeight;
	desc.depth = depth;
	desc.arrayLength = layers;
	desc.mipmapLevelCount = mipmapCount;
	desc.textureType = Metal::getTextureType(texType, getRequestedMSAA());

	bool sRGB = false;
	desc.pixelFormat = Metal::convertPixelFormat(format, sRGB);

	desc.storageMode = MTLStorageModePrivate;
	desc.usage = MTLTextureUsageRenderTarget;

	if (isReadable())
		desc.usage |= MTLTextureUsageShaderRead;

	texture = [device newTextureWithDescriptor:desc];

	if (texture == nil)
		throw love::Exception("Out of graphics memory.");

	// TODO: initialize texture to transparent black.
}}

Canvas::~Canvas()
{ @autoreleasepool {
	texture = nil;
}}

void Canvas::generateMipmaps()
{ @autoreleasepool {
	id<MTLBlitCommandEncoder> encoder = Graphics::getInstance()->useBlitEncoder();
	[encoder generateMipmapsForTexture:texture];
}}

} // metal
} // graphics
} // love
