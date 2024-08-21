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

#include "GraphicsReadback.h"
#include "Buffer.h"
#include "Texture.h"
#include "Graphics.h"
#include "data/ByteData.h"
#include "image/ImageData.h"
#include "image/Image.h"

namespace love
{
namespace graphics
{

love::Type GraphicsReadback::type("GraphicsReadback", &Object::type);

GraphicsReadback::GraphicsReadback(Graphics */*gfx*/, ReadbackMethod method, Buffer *buffer, size_t offset, size_t size, love::data::ByteData *dest, size_t destoffset)
	: dataType(DATA_BUFFER)
	, method(method)
	, bufferData(dest)
{
	if (offset + size > buffer->getSize())
		throw love::Exception("Invalid offset or size for the given Buffer.");

	if (dest != nullptr && destoffset + size > dest->getSize())
		throw love::Exception("Invalid destination offset or size for the given ByteData.");

	bufferDataOffset = dest != nullptr ? destoffset : 0;
}

GraphicsReadback::GraphicsReadback(Graphics *gfx, ReadbackMethod method, Texture *texture, int slice, int mipmap, const Rect &rect, love::image::ImageData *dest, int destx, int desty)
	: dataType(DATA_TEXTURE)
	, method(method)
	, imageData(dest)
	, rect(rect)
{
	const auto &caps = gfx->getCapabilities();

	if (gfx->isRenderTargetActive(texture))
		throw love::Exception("readbackTexture cannot be called while that Texture is an active render target.");

	if (!texture->isReadable())
		throw love::Exception("readbackTexture requires a readable Texture.");

	int tw = texture->getPixelWidth(mipmap);
	int th = texture->getPixelHeight(mipmap);
	auto texType = texture->getTextureType();

	if (rect.x < 0 || rect.y < 0 || rect.w <= 0 || rect.h <= 0 || (rect.x + rect.w) > tw || (rect.y + rect.h) > th)
		throw love::Exception("Invalid rectangle dimensions.");

	if (slice < 0 || (texType == TEXTURE_VOLUME && slice >= texture->getDepth(mipmap))
		|| (texType == TEXTURE_2D_ARRAY && slice >= texture->getLayerCount())
		|| (texType == TEXTURE_CUBE && slice >= 6))
	{
		throw love::Exception("Invalid slice index.");
	}

	textureFormat = getLinearPixelFormat(texture->getPixelFormat());
	isFormatLinear = isGammaCorrect() && !isPixelFormatSRGB(texture->getPixelFormat());

	if (!image::ImageData::validPixelFormat(textureFormat))
	{
		const char *formatname = "unknown";
		love::getConstant(textureFormat, formatname);
		throw love::Exception("ImageData with the '%s' pixel format is not supported.", formatname);
	}

	bool isRT = texture->isRenderTarget();

	if (method == READBACK_ASYNC)
	{
		if (!isRT && !caps.features[Graphics::FEATURE_COPY_TEXTURE_TO_BUFFER])
			throw love::Exception("readbackTextureAsync with a non-render-target texture is not supported on this system.");
	}
	else
	{
		if (!isRT && !caps.features[Graphics::FEATURE_COPY_TEXTURE_TO_BUFFER])
			throw love::Exception("readbackTexture with a non-render-target texture is not supported on this system.");
	}

	if (dest != nullptr)
	{
		if (getLinearPixelFormat(dest->getFormat()) != textureFormat)
			throw love::Exception("Destination ImageData pixel format must match the source Texture's format.");

		if (destx < 0 || desty < 0)
			throw love::Exception("Invalid destination ImageData x/y coordinates.");

		if (destx + rect.w > dest->getWidth() || desty + rect.h > dest->getHeight())
			throw love::Exception("The specified rectangle does not fit within the destination ImageData's dimensions.");
	}

	imageDataX = dest != nullptr ? destx : 0;
	imageDataY = dest != nullptr ? desty : 0;
}

GraphicsReadback::~GraphicsReadback()
{
}

love::data::ByteData *GraphicsReadback::getBufferData() const
{
	if (!isComplete())
		return nullptr;
	return bufferData;
}

love::image::ImageData *GraphicsReadback::getImageData() const
{
	if (!isComplete())
		return nullptr;
	return imageData;
}

void *GraphicsReadback::prepareReadbackDest(size_t size)
{
	if (dataType == DATA_TEXTURE)
	{
		if (imageData.get())
		{
			// Not the cleanest, but should work since uncompressed formats always
			// have 1x1 blocks.
			int pixels = imageDataY * imageData->getWidth() + imageDataX;
			size_t offset = getPixelFormatUncompressedRowSize(textureFormat, pixels);

			return (uint8 *) imageData->getData() + offset;
		}
		else
		{
			auto module = Module::getInstance<image::Image>(Module::M_IMAGE);
			if (module == nullptr)
				throw love::Exception("The love.image module must be loaded for readbackTexture.");

			imageData.set(module->newImageData(rect.w, rect.h, textureFormat, nullptr), Acquire::NORETAIN);
			imageData->setLinear(isFormatLinear);
			return imageData->getData();
		}
	}
	else
	{
		if (!bufferData.get())
			bufferData.set(new love::data::ByteData(size, false), Acquire::NORETAIN);

		return (uint8 *) bufferData->getData() + bufferDataOffset;
	}
}

GraphicsReadback::Status GraphicsReadback::readbackBuffer(Buffer *buffer, size_t offset, size_t size)
{
	if (buffer == nullptr)
		return STATUS_ERROR;

	const void *data = buffer->map(Buffer::MAP_READ_ONLY, offset, size);

	if (data == nullptr)
		return STATUS_ERROR;

	bool success = true;

	try
	{
		void *dest = prepareReadbackDest(size);
		if (dest == nullptr)
			return STATUS_ERROR;

		if (imageData.get())
		{
			// Always lock the mutex since the user can't know when to do it.
			love::thread::Lock lock(imageData->getMutex());

			if (imageData->getWidth() != rect.w)
			{
				// Readback of compressed textures into ImageData isn't supported,
				// so this is fine.
				size_t stride = getPixelFormatUncompressedRowSize(textureFormat, imageData->getWidth());
				size_t rowsize = getPixelFormatUncompressedRowSize(textureFormat, rect.w);

				for (int i = 0; i < rect.h; i++)
				{
					memcpy(dest, data, rowsize);
					dest = (uint8 *) dest + stride;
					data = (uint8 *) data + rowsize;
				}
			}
			else
			{
				memcpy(dest, data, std::min(size, imageData->getSize()));
			}
		}
		else
		{
			memcpy(dest, data, std::min(size, bufferData->getSize()));
		}
	}
	catch (love::Exception &)
	{
		success = false;
	}

	buffer->unmap(offset, size);
	return success ? STATUS_COMPLETE : STATUS_ERROR;
}

} // graphics
} // love
