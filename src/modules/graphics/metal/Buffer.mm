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

namespace love
{
namespace graphics
{
namespace metal
{

Buffer::Buffer(love::graphics::Graphics *gfx, id<MTLDevice> device, const Settings &settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
	: love::graphics::Buffer(gfx, settings, format, size, arraylength)
	, mappedRange()
{ @autoreleasepool {
	MTLResourceOptions opts = MTLResourceStorageModeManaged;
	buffer = [device newBufferWithLength:size options:opts];

	// TODO: synchronization etc
	memoryMap = (char *) buffer.contents;

	if (data != nullptr)
	{
		memcpy(map(), data, size);
		unmap();
	}
}}

Buffer::~Buffer()
{ @autoreleasepool {
	buffer = nil;
}}

void *Buffer::map()
{
	return memoryMap;
}

void Buffer::unmap()
{ @autoreleasepool {
	[buffer didModifyRange:{0, size}];
}}

void Buffer::setMappedRangeModified(size_t offset, size_t size)
{
	mappedRange = NSIntersectionRange(mappedRange, {offset, size});
}

void Buffer::fill(size_t offset, size_t size, const void *data)
{
	// TODO
}

void Buffer::copyTo(size_t offset, size_t size, love::graphics::Buffer *other, size_t otheroffset)
{
	// TODO
}

} // metal
} // graphics
} // love
