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

#include "Buffer.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{

Buffer::Buffer(size_t size, BufferTypeFlags typeflags, vertex::Usage usage, uint32 mapflags)
	: size(size)
	, typeFlags(typeflags)
	, usage(usage)
	, mapFlags(mapflags)
	, mapped(false)
{
}

Buffer::Buffer(Graphics *gfx, const Settings &settings, const std::vector<DataMember> &format, size_t arraylength)
	: Buffer(0, settings.typeFlags, settings.usage, settings.mapFlags)
{
	bool uniformbuffer = settings.typeFlags & BUFFERFLAG_UNIFORM;
	bool indexbuffer = settings.typeFlags & BUFFERFLAG_INDEX;
	bool vertexbuffer = settings.typeFlags & BUFFERFLAG_VERTEX;
	bool ssbuffer = settings.typeFlags & BUFFERFLAG_SHADER_STORAGE;

	if (indexbuffer && format.size() > 1)
		throw love::Exception("test");

	for (const auto &member : format)
	{
		if (indexbuffer)
		{
			if (member.type != DATA_UINT16 && member.type != DATA_UINT32)
				throw love::Exception("test");
		}

		if (uniformbuffer)
		{
			
		}
	}
}

Buffer::~Buffer()
{
}

} // graphics
} // love
