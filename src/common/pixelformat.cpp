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

#include "pixelformat.h"
#include "StringMap.h"

namespace love
{

static PixelFormatInfo formatInfo[] =
{
	// components, blockW, blockH, blockSize, color, depth, stencil, compressed
    { 0, 1, 1, 0, false, false, false, false }, // PIXELFORMAT_UNKNOWN

	{ 0, 1, 1, 0, true, false, false, false }, // PIXELFORMAT_NORMAL
	{ 0, 1, 1, 0, true, false, false, false }, // PIXELFORMAT_HDR

	{ 1, 1, 1, 1, true, false, false, false }, // PIXELFORMAT_R8_UNORM
	{ 1, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_R16_UNORM
	{ 1, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_R16_FLOAT
	{ 1, 1, 1, 4, true, false, false, false }, // PIXELFORMAT_R32_FLOAT

	{ 2, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_RG8_UNORM
	{ 2, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_LA8_UNORM
	{ 2, 1, 1, 4, true, false, false, false }, // PIXELFORMAT_RG16_UNORM
	{ 2, 1, 1, 4, true, false, false, false }, // PIXELFORMAT_RG16_FLOAT
	{ 2, 1, 1, 8, true, false, false, false }, // PIXELFORMAT_RG32_FLOAT

	{ 4, 1, 1, 4,  true, false, false, false }, // PIXELFORMAT_RGBA8_UNORM
	{ 4, 1, 1, 4,  true, false, false, false }, // PIXELFORMAT_sRGBA8_UNORM
	{ 4, 1, 1, 8,  true, false, false, false }, // PIXELFORMAT_RGBA16_UNORM
	{ 4, 1, 1, 8,  true, false, false, false }, // PIXELFORMAT_RGBA16_FLOAT
	{ 4, 1, 1, 16, true, false, false, false }, // PIXELFORMAT_RGBA32_FLOAT

	{ 4, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_RGBA4_UNORM
	{ 4, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_RGB5A1_UNORM
	{ 3, 1, 1, 2, true, false, false, false }, // PIXELFORMAT_RGB565_UNORM
	{ 4, 1, 1, 4, true, false, false, false }, // PIXELFORMAT_RGB10A2_UNORM
	{ 3, 1, 1, 4, true, false, false, false }, // PIXELFORMAT_RG11B10_FLOAT

	{ 1, 1, 1, 1, false, false, true , false }, // PIXELFORMAT_STENCIL8
	{ 1, 1, 1, 2, false, true,  false, false }, // PIXELFORMAT_DEPTH16_UNORM
	{ 1, 1, 1, 3, false, true,  false, false }, // PIXELFORMAT_DEPTH24_UNORM
	{ 1, 1, 1, 4, false, true,  false, false }, // PIXELFORMAT_DEPTH32_FLOAT
	{ 2, 1, 1, 4, false, true,  true , false }, // PIXELFORMAT_DEPTH24_UNORM_STENCIL8
	{ 2, 1, 1, 5, false, true,  true , false }, // PIXELFORMAT_DEPTH32_FLOAT_STENCIL8

	{ 3, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_DXT1_UNORM
	{ 4, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_DXT3_UNORM
	{ 4, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_DXT5_UNORM
	{ 1, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_BC4_UNORM
	{ 1, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_BC4_SNORM
	{ 2, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_BC5_UNORM
	{ 2, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_BC5_SNORM
	{ 3, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_BC6H_UFLOAT
	{ 3, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_BC6H_FLOAT
	{ 4, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_BC7_UNORM

	{ 3, 16, 8, 32, true, false, false, true }, // PIXELFORMAT_PVR1_RGB2_UNORM
	{ 3, 8,  8, 32, true, false, false, true }, // PIXELFORMAT_PVR1_RGB4_UNORM
	{ 4, 16, 8, 32, true, false, false, true }, // PIXELFORMAT_PVR1_RGBA2_UNORM
	{ 4, 8,  8, 32, true, false, false, true }, // PIXELFORMAT_PVR1_RGBA4_UNORM

	{ 3, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_ETC1_UNORM
	{ 3, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_ETC2_RGB_UNORM
	{ 4, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_ETC2_RGBA_UNORM
	{ 4, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_ETC2_RGBA1_UNORM
	{ 1, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_EAC_R_UNORM
	{ 1, 4, 4, 8,  true, false, false, true }, // PIXELFORMAT_EAC_R_SNORM
	{ 2, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_EAC_RG_UNORM
	{ 2, 4, 4, 16, true, false, false, true }, // PIXELFORMAT_EAC_RG_SNORM

	{ 4, 4,  4,  1, true, false, false, true }, // PIXELFORMAT_ASTC_4x4
	{ 4, 5,  4,  1, true, false, false, true }, // PIXELFORMAT_ASTC_5x4
	{ 4, 5,  5,  1, true, false, false, true }, // PIXELFORMAT_ASTC_5x5
	{ 4, 6,  5,  1, true, false, false, true }, // PIXELFORMAT_ASTC_6x5
	{ 4, 6,  6,  1, true, false, false, true }, // PIXELFORMAT_ASTC_6x6
	{ 4, 8,  5,  1, true, false, false, true }, // PIXELFORMAT_ASTC_8x5
	{ 4, 8,  6,  1, true, false, false, true }, // PIXELFORMAT_ASTC_8x6
	{ 4, 8,  8,  1, true, false, false, true }, // PIXELFORMAT_ASTC_8x8
	{ 4, 8,  5,  1, true, false, false, true }, // PIXELFORMAT_ASTC_10x5
	{ 4, 10, 6,  1, true, false, false, true }, // PIXELFORMAT_ASTC_10x6
	{ 4, 10, 8,  1, true, false, false, true }, // PIXELFORMAT_ASTC_10x8
	{ 4, 10, 10, 1, true, false, false, true }, // PIXELFORMAT_ASTC_10x10
	{ 4, 12, 10, 1, true, false, false, true }, // PIXELFORMAT_ASTC_12x10
	{ 4, 12, 12, 1, true, false, false, true }, // PIXELFORMAT_ASTC_12x12
};

static_assert(sizeof(formatInfo) / sizeof(PixelFormatInfo) == PIXELFORMAT_MAX_ENUM, "Update the formatInfo array when adding or removing a PixelFormat");

static StringMap<PixelFormat, PIXELFORMAT_MAX_ENUM>::Entry formatEntries[] =
{
    { "unknown", PIXELFORMAT_UNKNOWN },

	{ "normal",  PIXELFORMAT_NORMAL  },
	{ "hdr",     PIXELFORMAT_HDR     },

	{ "r8",      PIXELFORMAT_R8_UNORM     },
	{ "r16",     PIXELFORMAT_R16_UNORM    },
	{ "r16f",    PIXELFORMAT_R16_FLOAT    },
	{ "r32f",    PIXELFORMAT_R32_FLOAT    },

	{ "rg8",     PIXELFORMAT_RG8_UNORM    },
	{ "la8",     PIXELFORMAT_LA8_UNORM    },
	{ "rg16",    PIXELFORMAT_RG16_UNORM   },
	{ "rg16f",   PIXELFORMAT_RG16_FLOAT   },
	{ "rg32f",   PIXELFORMAT_RG32_FLOAT   },

	{ "rgba8",   PIXELFORMAT_RGBA8_UNORM  },
	{ "srgba8",  PIXELFORMAT_sRGBA8_UNORM },
	{ "rgba16",  PIXELFORMAT_RGBA16_UNORM },
	{ "rgba16f", PIXELFORMAT_RGBA16_FLOAT },
	{ "rgba32f", PIXELFORMAT_RGBA32_FLOAT },

	{ "rgba4",    PIXELFORMAT_RGBA4_UNORM    },
	{ "rgb5a1",   PIXELFORMAT_RGB5A1_UNORM   },
	{ "rgb565",   PIXELFORMAT_RGB565_UNORM   },
	{ "rgb10a2",  PIXELFORMAT_RGB10A2_UNORM  },
	{ "rg11b10f", PIXELFORMAT_RG11B10_FLOAT  },

	{ "stencil8",         PIXELFORMAT_STENCIL8               },
	{ "depth16",          PIXELFORMAT_DEPTH16_UNORM          },
	{ "depth24",          PIXELFORMAT_DEPTH24_UNORM          },
	{ "depth32f",         PIXELFORMAT_DEPTH32_FLOAT          },
	{ "depth24stencil8",  PIXELFORMAT_DEPTH24_UNORM_STENCIL8 },
	{ "depth32fstencil8", PIXELFORMAT_DEPTH32_FLOAT_STENCIL8 },
	
	{ "DXT1",      PIXELFORMAT_DXT1_UNORM       },
	{ "DXT3",      PIXELFORMAT_DXT3_UNORM       },
	{ "DXT5",      PIXELFORMAT_DXT5_UNORM       },
	{ "BC4",       PIXELFORMAT_BC4_UNORM        },
	{ "BC4s",      PIXELFORMAT_BC4_SNORM        },
	{ "BC5",       PIXELFORMAT_BC5_UNORM        },
	{ "BC5s",      PIXELFORMAT_BC5_SNORM        },
	{ "BC6h",      PIXELFORMAT_BC6H_UFLOAT      },
	{ "BC6hs",     PIXELFORMAT_BC6H_FLOAT       },
	{ "BC7",       PIXELFORMAT_BC7_UNORM        },
	{ "PVR1rgb2",  PIXELFORMAT_PVR1_RGB2_UNORM  },
	{ "PVR1rgb4",  PIXELFORMAT_PVR1_RGB4_UNORM  },
	{ "PVR1rgba2", PIXELFORMAT_PVR1_RGBA2_UNORM },
	{ "PVR1rgba4", PIXELFORMAT_PVR1_RGBA4_UNORM },
	{ "ETC1",      PIXELFORMAT_ETC1_UNORM       },
	{ "ETC2rgb",   PIXELFORMAT_ETC2_RGB_UNORM   },
	{ "ETC2rgba",  PIXELFORMAT_ETC2_RGBA_UNORM  },
	{ "ETC2rgba1", PIXELFORMAT_ETC2_RGBA1_UNORM },
	{ "EACr",      PIXELFORMAT_EAC_R_UNORM      },
	{ "EACrs",     PIXELFORMAT_EAC_R_SNORM      },
	{ "EACrg",     PIXELFORMAT_EAC_RG_UNORM     },
	{ "EACrgs",    PIXELFORMAT_EAC_RG_SNORM     },
	{ "ASTC4x4",   PIXELFORMAT_ASTC_4x4   },
	{ "ASTC5x4",   PIXELFORMAT_ASTC_5x4   },
	{ "ASTC5x5",   PIXELFORMAT_ASTC_5x5   },
	{ "ASTC6x5",   PIXELFORMAT_ASTC_6x5   },
	{ "ASTC6x6",   PIXELFORMAT_ASTC_6x6   },
	{ "ASTC8x5",   PIXELFORMAT_ASTC_8x5   },
	{ "ASTC8x6",   PIXELFORMAT_ASTC_8x6   },
	{ "ASTC8x8",   PIXELFORMAT_ASTC_8x8   },
	{ "ASTC10x5",  PIXELFORMAT_ASTC_10x5  },
	{ "ASTC10x6",  PIXELFORMAT_ASTC_10x6  },
	{ "ASTC10x8",  PIXELFORMAT_ASTC_10x8  },
	{ "ASTC10x10", PIXELFORMAT_ASTC_10x10 },
	{ "ASTC12x10", PIXELFORMAT_ASTC_12x10 },
	{ "ASTC12x12", PIXELFORMAT_ASTC_12x12 },
};

static_assert(sizeof(formatEntries) / sizeof(formatEntries[0]) == (size_t) PIXELFORMAT_MAX_ENUM, "pixel format string map is missing entries!");

static StringMap<PixelFormat, PIXELFORMAT_MAX_ENUM> formats(formatEntries, sizeof(formatEntries));

bool getConstant(const char *in, PixelFormat &out)
{
	return formats.find(in, out);
}

bool getConstant(PixelFormat in, const char *&out)
{
	return formats.find(in, out);
}

const PixelFormatInfo &getPixelFormatInfo(PixelFormat format)
{
	return formatInfo[format];
}

bool isPixelFormatCompressed(PixelFormat format)
{
	return formatInfo[format].compressed;
}

bool isPixelFormatDepthStencil(PixelFormat format)
{
	const PixelFormatInfo &info = formatInfo[format];
	return info.depth || info.stencil;
}

bool isPixelFormatDepth(PixelFormat format)
{
	return formatInfo[format].depth;
}

bool isPixelFormatStencil(PixelFormat format)
{
	return formatInfo[format].stencil;
}

PixelFormat getSRGBPixelFormat(PixelFormat format)
{
	if (format == PIXELFORMAT_RGBA8_UNORM)
		return PIXELFORMAT_sRGBA8_UNORM;
	return format;
}

PixelFormat getLinearPixelFormat(PixelFormat format)
{
	if (format == PIXELFORMAT_sRGBA8_UNORM)
		return PIXELFORMAT_RGBA8_UNORM;
	return format;
}

size_t getPixelFormatBlockSize(PixelFormat format)
{
	return formatInfo[format].blockSize;
}

size_t getPixelFormatUncompressedRowSize(PixelFormat format, int width)
{
	const PixelFormatInfo &info = formatInfo[format];
	if (info.compressed) return 0;
	return info.blockSize * width / info.blockWidth;
}

size_t getPixelFormatCompressedBlockRowSize(PixelFormat format, int width)
{
	const PixelFormatInfo &info = formatInfo[format];
	if (!info.compressed) return 0;
	return info.blockSize * ((width + info.blockWidth - 1) / info.blockWidth);
}

size_t getPixelFormatCompressedBlockRowCount(PixelFormat format, int height)
{
	const PixelFormatInfo &info = formatInfo[format];
	if (!info.compressed) return 0;
	return (height + info.blockHeight - 1) / info.blockHeight;
}

size_t getPixelFormatSliceSize(PixelFormat format, int width, int height)
{
	const PixelFormatInfo &info = formatInfo[format];
	size_t blockW = (width + info.blockWidth - 1) / info.blockWidth;
	size_t blockH = (height + info.blockHeight - 1) / info.blockHeight;
	return info.blockSize * blockW * blockH;
}

int getPixelFormatColorComponents(PixelFormat format)
{
	return formatInfo[format].components;
}

} // love
