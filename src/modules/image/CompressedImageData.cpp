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

#include "CompressedImageData.h"
#include "common/Exception.h"

namespace love
{
namespace image
{

love::Type CompressedImageData::type("CompressedImageData", &Data::type);

CompressedImageData::CompressedImageData(const std::list<FormatHandler *> &formats, Data *filedata)
	: format(PIXELFORMAT_UNKNOWN)
{
	FormatHandler *parser = nullptr;

	for (FormatHandler *handler : formats)
	{
		if (handler->canParseCompressed(filedata))
		{
			parser = handler;
			break;
		}
	}

	if (parser == nullptr)
		throw love::Exception("Could not parse compressed data: Unknown format.");

	memory = parser->parseCompressed(filedata, dataImages, format);

	if (memory == nullptr)
		throw love::Exception("Could not parse compressed data.");

	if (format == PIXELFORMAT_UNKNOWN)
		throw love::Exception("Could not parse compressed data: Unknown format.");

	if (dataImages.size() == 0 || memory->getSize() == 0)
		throw love::Exception("Could not parse compressed data: No valid data?");

	// This throws away some information the decoder could give us, but we
	// can't really rely on it I think...
	format = getLinearPixelFormat(format);
}

CompressedImageData::CompressedImageData(const CompressedImageData &c)
	: format(c.format)
{
	memory.set(c.memory->clone(), Acquire::NORETAIN);

	for (const auto &i : c.dataImages)
	{
		auto slice = new CompressedSlice(i->getFormat(), i->getWidth(), i->getHeight(), memory, i->getOffset(), i->getSize());
		dataImages.push_back(slice);
		slice->release();
	}
}

CompressedImageData *CompressedImageData::clone() const
{
	return new CompressedImageData(*this);
}

CompressedImageData::~CompressedImageData()
{
}

size_t CompressedImageData::getSize() const
{
	return memory->getSize();
}

void *CompressedImageData::getData() const
{
	return memory->getData();
}

int CompressedImageData::getMipmapCount() const
{
	return (int) dataImages.size();
}

int CompressedImageData::getSliceCount(int /*mip*/) const
{
	return 1;
}

size_t CompressedImageData::getSize(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getSize();
}

void *CompressedImageData::getData(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getData();
}

int CompressedImageData::getWidth(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getWidth();
}

int CompressedImageData::getHeight(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getHeight();
}

PixelFormat CompressedImageData::getFormat() const
{
	return format;
}

void CompressedImageData::setLinear(bool linear)
{
	for (auto &slice : dataImages)
		slice->setLinear(linear);
}

bool CompressedImageData::isLinear() const
{
	return dataImages.empty() ? false : dataImages[0]->isLinear();
}

CompressedSlice *CompressedImageData::getSlice(int slice, int miplevel) const
{
	checkSliceExists(slice, miplevel);

	return dataImages[miplevel].get();
}

void CompressedImageData::checkSliceExists(int slice, int miplevel) const
{
	if (slice != 0)
		throw love::Exception("Slice index %d does not exists", slice + 1);

	if (miplevel < 0 || miplevel >= (int) dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel + 1);
}

} // image
} // love
