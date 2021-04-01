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

#import "Buffer.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace metal
{

static MTLPixelFormat getMTLPixelFormat(DataFormat format)
{
	switch (format)
	{
		case DATAFORMAT_FLOAT: return MTLPixelFormatR32Float;
		case DATAFORMAT_FLOAT_VEC2: return MTLPixelFormatRG32Float;
		case DATAFORMAT_FLOAT_VEC4: return MTLPixelFormatRGBA32Float;
		case DATAFORMAT_INT32: return MTLPixelFormatR32Sint;
		case DATAFORMAT_INT32_VEC2: return MTLPixelFormatRG32Sint;
		case DATAFORMAT_INT32_VEC4: return MTLPixelFormatRGBA32Sint;
		case DATAFORMAT_UINT32: return MTLPixelFormatR32Uint;
		case DATAFORMAT_UINT32_VEC2: return MTLPixelFormatRG32Uint;
		case DATAFORMAT_UINT32_VEC4: return MTLPixelFormatRGBA32Uint;
		case DATAFORMAT_UNORM8_VEC4: return MTLPixelFormatRGBA8Unorm;
		case DATAFORMAT_SNORM8_VEC4: return MTLPixelFormatRGBA8Snorm;
		case DATAFORMAT_INT8_VEC4: return MTLPixelFormatRGBA8Sint;
		case DATAFORMAT_UINT8_VEC4: return MTLPixelFormatRGBA8Uint;
		case DATAFORMAT_UNORM16_VEC2: return MTLPixelFormatRG16Unorm;
		case DATAFORMAT_UNORM16_VEC4: return MTLPixelFormatRGBA16Unorm;
		case DATAFORMAT_INT16_VEC2: return MTLPixelFormatRG16Sint;
		case DATAFORMAT_INT16_VEC4: return MTLPixelFormatRGBA16Sint;
		case DATAFORMAT_UINT16: return MTLPixelFormatR16Uint;
		case DATAFORMAT_UINT16_VEC2: return MTLPixelFormatRG16Uint;
		case DATAFORMAT_UINT16_VEC4: return MTLPixelFormatRGBA16Uint;
		default: return MTLPixelFormatInvalid;
	}
}

Buffer::Buffer(love::graphics::Graphics *gfx, id<MTLDevice> device, const Settings &settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
	: love::graphics::Buffer(gfx, settings, format, size, arraylength)
	, texture(nil)
	, mapBuffer(nil)
	, mappedRange()
{ @autoreleasepool {
	size = getSize();
	arraylength = getArrayLength();

	MTLResourceOptions opts = MTLResourceStorageModePrivate;
	buffer = [device newBufferWithLength:size options:opts];

	if (buffer == nil)
		throw love::Exception("Could not create buffer (out of VRAM?)");

	if (usageFlags & BUFFERUSAGEFLAG_TEXEL)
	{
		if (@available(iOS 12, macOS 10.14, *))
		{
			MTLPixelFormat pixformat = getMTLPixelFormat(getDataMember(0).decl.format);
			size_t width = arraylength * getDataMembers().size();
			auto desc = [MTLTextureDescriptor textureBufferDescriptorWithPixelFormat:pixformat
																			   width:width
																	 resourceOptions:opts
																			   usage:MTLTextureUsageShaderRead];
			texture = [buffer newTextureWithDescriptor:desc offset:0 bytesPerRow:size];
		}

		if (texture == nil)
			throw love::Exception("Could not create Metal texel buffer.");
	}

	if (data != nullptr)
		fill(0, size, data);
}}

Buffer::~Buffer()
{ @autoreleasepool {
	buffer = nil;
	texture = nil;
}}

void *Buffer::map(MapType /*map*/, size_t offset, size_t size)
{ @autoreleasepool {
	if (size == 0)
		return nullptr;

	Range r(offset, size);

	if (!Range(0, getSize()).contains(r))
		return nullptr;

	auto gfx = Graphics::getInstance();

	// TODO: Don't create a new buffer every time, also do something for stream
	// buffers.
	mapBuffer = [gfx->device newBufferWithLength:size options:MTLResourceStorageModeShared];

	if (mapBuffer != nil)
	{
		mappedRange = r;
		mapped = true;
		return mapBuffer.contents;
	}

	return nullptr;
}}

void Buffer::unmap(size_t usedoffset, size_t usedsize)
{ @autoreleasepool {
	if (mapBuffer == nil)
		return;

	Range r(usedoffset, usedsize);

	if (!mapped || !mappedRange.contains(r))
		return;

	auto gfx = Graphics::getInstance();
	auto encoder = gfx->useBlitEncoder();

	[encoder copyFromBuffer:mapBuffer
			   sourceOffset:(usedoffset - mappedRange.getOffset())
				   toBuffer:buffer
		  destinationOffset:usedoffset
					   size:usedsize];

	mapBuffer = nil;
	mapped = false;
}}

void Buffer::fill(size_t offset, size_t size, const void *data)
{ @autoreleasepool {
	if (size == 0)
		return;

	size_t buffersize = getSize();

	if (!Range(0, buffersize).contains(Range(offset, size)))
		return;

	// TODO: Don't create a new buffer every time, also do something for stream
	// buffers.
	auto gfx = Graphics::getInstance();
	auto encoder = gfx->useBlitEncoder();

	auto tempbuffer = [gfx->device newBufferWithLength:size options:MTLResourceStorageModeShared];
	memcpy(tempbuffer.contents, data, size);

	[encoder copyFromBuffer:tempbuffer
			   sourceOffset:0
				   toBuffer:buffer
		  destinationOffset:offset
					   size:size];
}}

void Buffer::copyTo(love::graphics::Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size)
{ @autoreleasepool {
	auto gfx = Graphics::getInstance();
	auto encoder = gfx->useBlitEncoder();

	[encoder copyFromBuffer:buffer
			   sourceOffset:sourceoffset
				   toBuffer:((Buffer *) dest)->buffer
		  destinationOffset:destoffset
					   size:size];
}}

} // metal
} // graphics
} // love
