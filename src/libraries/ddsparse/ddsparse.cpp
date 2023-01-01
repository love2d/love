/**
 * Simple DDS data parser for compressed 2D textures.
 *
 * Copyright (c) 2013-2023 Sasha Szpakowski
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

#include "ddsparse.h"
#include "ddsinfo.h"

#include <algorithm>

namespace dds
{

using namespace dds::dxinfo;

// Creates a packed uint representation of a FourCC code.
#define MakeFourCC(a, b, c, d) ((uint32_t) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)))

#define ISBITMASK(r,g,b,a) (ddpf.rBitMask == r && ddpf.gBitMask == g && ddpf.bBitMask == b && ddpf.aBitMask == a)

// Function adapted from DirectXTex:
// https://github.com/microsoft/DirectXTex/blob/master/DDSTextureLoader/DDSTextureLoader.cpp#L623
static DXGIFormat getDXGIFormat(const DDSPixelFormat& ddpf)
{
	if (ddpf.flags & DDPF_RGB)
	{
		// Note that sRGB formats are written using the "DX10" extended header

		switch (ddpf.rgbBitCount)
		{
		case 32:
			if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				return DXGI_FORMAT_B8G8R8A8_UNORM;

			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				return DXGI_FORMAT_B8G8R8X8_UNORM;

			// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

			// Note that many common DDS reader/writers (including D3DX) swap the
			// the RED/BLUE masks for 10:10:10:2 formats. We assume
			// below that the 'backwards' header mask is being used since it is most
			// likely written by D3DX. The more robust solution is to use the 'DX10'
			// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

			// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
			if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				return DXGI_FORMAT_R10G10B10A2_UNORM;

			// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

			if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				return DXGI_FORMAT_R16G16_UNORM;

			if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
				// Only 32-bit color channel format in D3D9 was R32F
				return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
			break;

		case 24:
			// No 24bpp DXGI formats aka D3DFMT_R8G8B8
			break;

		case 16:
			if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				return DXGI_FORMAT_B5G5R5A1_UNORM;

			if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
				return DXGI_FORMAT_B5G6R5_UNORM;

			// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

			// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

			// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
			break;
		}
	}
	else if (ddpf.flags & DDPF_LUMINANCE)
	{
		if (ddpf.rgbBitCount == 8)
		{
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
				return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension

			// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
		}

		if (ddpf.rgbBitCount == 16)
		{
			if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
				return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension

			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
		}
	}
	else if (ddpf.flags & DDPF_ALPHA)
	{
		if (ddpf.rgbBitCount == 8)
			return DXGI_FORMAT_A8_UNORM;
	}
	else if (ddpf.flags & DDPF_BUMPDUDV)
	{
		if (ddpf.rgbBitCount == 16)
		{
			if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
				return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
		}

		if (ddpf.rgbBitCount == 32)
		{
			if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension

			if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension

			// No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
		}
	}
	else if (ddpf.flags & DDPF_FOURCC)
	{
		switch (ddpf.fourCC)
		{
		case MakeFourCC('D','X','T','1'):
			return DXGI_FORMAT_BC1_UNORM;

		case MakeFourCC('D','X','T','3'):
			return DXGI_FORMAT_BC2_UNORM;

		case MakeFourCC('D','X','T','5'):
			return DXGI_FORMAT_BC3_UNORM;

		// While pre-multiplied alpha isn't directly supported by the DXGI formats,
		// they are basically the same as these BC formats so they can be mapped
		case MakeFourCC('D','X','T','2'):
			return DXGI_FORMAT_BC2_UNORM;

		case MakeFourCC('D','X','T','4'):
			return DXGI_FORMAT_BC3_UNORM;

		case MakeFourCC('A','T','I','1'):
			return DXGI_FORMAT_BC4_UNORM;

		case MakeFourCC('B','C','4','U'):
			return DXGI_FORMAT_BC4_UNORM;

		case MakeFourCC('B','C','4','S'):
			return DXGI_FORMAT_BC4_SNORM;

		case MakeFourCC('A','T','I','2'):
			return DXGI_FORMAT_BC5_UNORM;

		case MakeFourCC('B','C','5','U'):
			return DXGI_FORMAT_BC5_UNORM;

		case MakeFourCC('B','C','5','S'):
			return DXGI_FORMAT_BC5_SNORM;

		// BC6H and BC7 are written using the "DX10" extended header

		case MakeFourCC('R','G','B','G'):
			return DXGI_FORMAT_R8G8_B8G8_UNORM;

		case MakeFourCC('G','R','G','B'):
			return DXGI_FORMAT_G8R8_G8B8_UNORM;

		// Check for D3DFORMAT enums being set here
		case 36: // D3DFMT_A16B16G16R16
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case 110: // D3DFMT_Q16W16V16U16
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case 111: // D3DFMT_R16F
			return DXGI_FORMAT_R16_FLOAT;

		case 112: // D3DFMT_G16R16F
			return DXGI_FORMAT_R16G16_FLOAT;

		case 113: // D3DFMT_A16B16G16R16F
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case 114: // D3DFMT_R32F
			return DXGI_FORMAT_R32_FLOAT;

		case 115: // D3DFMT_G32R32F
			return DXGI_FORMAT_R32G32_FLOAT;

		case 116: // D3DFMT_A32B32G32R32F
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}

	return DXGI_FORMAT_UNKNOWN;
}

static size_t getBitsPerPixel(DXGIFormat fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 32;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return 16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		return 0;
	}
}

static bool isBlockCompressed(DXGIFormat fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

bool isDDS(const void *data, size_t dataSize)
{
	const uint8_t *readData = (const uint8_t *) data;
	ptrdiff_t offset = 0;

	// Is the data large enough to hold the DDS header?
	if(dataSize < sizeof(uint32_t) + sizeof(DDSHeader))
		return false;

	// All DDS files start with "DDS ".
	if((*(uint32_t *) readData) != MakeFourCC('D','D','S',' '))
		return false;

	offset += sizeof(uint32_t);

	DDSHeader *header = (DDSHeader *) &readData[offset];

	// Verify header to validate DDS data.
	if (header->size != sizeof(DDSHeader) || header->format.size != sizeof(DDSPixelFormat))
		return false;

	offset += sizeof(DDSHeader);

	// Check for DX10 extension.
	if ((header->format.flags & DDPF_FOURCC) && (header->format.fourCC == MakeFourCC('D','X','1','0')))
	{
		// Data must be big enough for both headers plus the magic value.
		if (dataSize < (sizeof(uint32_t) + sizeof(DDSHeader) + sizeof(DDSHeader10)))
			return false;
	}

	return true;
}

DXGIFormat getDDSPixelFormat(const void *data, size_t dataSize)
{
	if (!isDDS(data, dataSize))
		return DXGI_FORMAT_UNKNOWN;

	const uint8_t *readData = (const uint8_t *) data;
	ptrdiff_t offset = sizeof(uint32_t);

	DDSHeader *header = (DDSHeader *) &readData[offset];
	offset += sizeof(DDSHeader);

	// Check for DX10 extension.
	if ((header->format.flags & DDPF_FOURCC) && (header->format.fourCC == MakeFourCC('D','X','1','0')))
	{
		DDSHeader10 *header10 = (DDSHeader10 *) &readData[offset];
		return header10->dxgiFormat;
	}

	return getDXGIFormat(header->format);
}

bool isCompressedDDS(const void *data, size_t dataSize)
{
	DXGIFormat format = getDDSPixelFormat(data, dataSize);
	return format != DXGI_FORMAT_UNKNOWN && isBlockCompressed(format);
}

Parser::Parser(const void *data, size_t dataSize)
	: format(DXGI_FORMAT_UNKNOWN)
{
	parseData(data, dataSize);
}

Parser::Parser(const Parser &other)
	: texData(other.texData)
	, format(other.format)
{
}

Parser::Parser()
	: format(DXGI_FORMAT_UNKNOWN)
{
}

Parser &Parser::operator = (const Parser &other)
{
	texData = other.texData;
	format = other.format;

	return *this;
}

Parser::~Parser()
{
}

DXGIFormat Parser::getFormat() const
{
	return format;
}

const Image *Parser::getImageData(size_t miplevel) const
{
	if (miplevel >= texData.size())
		return 0;

	return &texData[miplevel];
}

size_t Parser::getMipmapCount() const
{
	return texData.size();
}

size_t Parser::parseImageSize(DXGIFormat fmt, int width, int height) const
{
	size_t bytes = 0;
	size_t bytesPerBlock = 0;

	bool packed = false;
	bool blockCompressed = false;

	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		blockCompressed = true;
		bytesPerBlock = 8;
		break;
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		blockCompressed = true;
		bytesPerBlock = 16;
		break;
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		packed = true;
		bytesPerBlock = 4;
		break;
	default:
		break;
	}

	if (packed)
	{
		size_t rowBytes = (((size_t) width + 1u) >> 1) * bytesPerBlock;
		bytes = rowBytes * height;
	}
	else if (blockCompressed)
	{
		size_t numBlocksWide = width > 0 ? std::max(1, (width + 3) / 4) : 0;
		size_t numBlocksHigh = height > 0 ? std::max(1, (height + 3) / 4) : 0;
		bytes = numBlocksWide * bytesPerBlock * numBlocksHigh;
	}
	else
	{
		size_t bpp = getBitsPerPixel(fmt);
		if (bpp == 0)
			return 0;

		// Round up to the nearest byte.
		size_t rowBytes = ((size_t) width * bpp + 7u) / 8u;
		bytes = rowBytes * height;
	}

	return bytes;
}

bool Parser::parseTexData(const uint8_t *data, size_t dataSize, DXGIFormat fmt, int w, int h, int nb_mips)
{
	size_t offset = 0;
	std::vector<Image> newTexData;

	for (int i = 0; i < nb_mips; i++)
	{
		Image img;
		img.width = w;
		img.height = h;

		img.dataSize = parseImageSize(fmt, img.width, img.height);

		// Make sure the data size is valid.
		if (img.dataSize == 0 || (offset + img.dataSize) > dataSize)
			return false;

		// Store the memory address of the data representing this mip level.
		img.data = &data[offset];

		newTexData.push_back(img);

		// Move to the next mip level.
		offset += img.dataSize;

		w = std::max(w / 2, 1);
		h = std::max(h / 2, 1);
	}

	texData = newTexData;

	return true;
}

bool Parser::parseData(const void *data, size_t dataSize)
{
	if (!isDDS(data, dataSize))
		return false;

	const uint8_t *readData = (const uint8_t *) data;
	ptrdiff_t offset = sizeof(uint32_t);

	DDSHeader *header = (DDSHeader *) &readData[offset];
	offset += sizeof(DDSHeader);

	// Check for DX10 extension.
	if ((header->format.flags & DDPF_FOURCC) && (header->format.fourCC == MakeFourCC('D','X','1','0')))
	{
		DDSHeader10 *header10 = (DDSHeader10 *) &readData[offset];
		offset += sizeof(DDSHeader10);

		// We can't deal with 1D/3D textures.
		switch (header10->resourceDimension)
		{
		case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
		case D3D10_RESOURCE_DIMENSION_UNKNOWN:
			break;
		default:
			return false;
		}

		// We also can't deal with texture arrays and cubemaps.
		if (header10->arraySize > 1)
			return false;

		format = header10->dxgiFormat;
	}
	else
		format = getDXGIFormat(header->format);

	if (format == DXGI_FORMAT_UNKNOWN)
		return false;

	int w = header->width;
	int h = header->height;

	int nb_mips = std::max((int) header->mipMapCount, 1);

	return parseTexData(&readData[offset], dataSize - offset, format, w, h, nb_mips);
}

} // dds
