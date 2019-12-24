/**
 * Copyright (c) 2006-2019 LOVE Development Team
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

static StringMap<PixelFormat, PIXELFORMAT_MAX_ENUM>::Entry formatEntries[] =
{
    { "unknown", PIXELFORMAT_UNKNOWN },

	{ "normal",  PIXELFORMAT_NORMAL  },
	{ "hdr",     PIXELFORMAT_HDR     },

	{ "r8",      PIXELFORMAT_R8_UNORM     },
	{ "rg8",     PIXELFORMAT_RG8_UNORM    },
	{ "rgba8",   PIXELFORMAT_RGBA8_UNORM  },
	{ "srgba8",  PIXELFORMAT_sRGBA8_UNORM },
	{ "r16",     PIXELFORMAT_R16_UNORM    },
	{ "rg16",    PIXELFORMAT_RG16_UNORM   },
	{ "rgba16",  PIXELFORMAT_RGBA16_UNORM },
	{ "r16f",    PIXELFORMAT_R16_FLOAT    },
	{ "rg16f",   PIXELFORMAT_RG16_FLOAT   },
	{ "rgba16f", PIXELFORMAT_RGBA16_FLOAT },
	{ "r32f",    PIXELFORMAT_R32_FLOAT    },
	{ "rg32f",   PIXELFORMAT_RG32_FLOAT   },
	{ "rgba32f", PIXELFORMAT_RGBA32_FLOAT },

	{ "la8",     PIXELFORMAT_LA8_UNORM    },

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

bool isPixelFormatCompressed(PixelFormat format)
{
	// I'm lazy
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_DXT1_UNORM && iformat < (int) PIXELFORMAT_MAX_ENUM;
}

bool isPixelFormatDepthStencil(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_STENCIL8 && iformat <= (int) PIXELFORMAT_DEPTH32_FLOAT_STENCIL8;
}

bool isPixelFormatDepth(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_DEPTH16_UNORM && iformat <= (int) PIXELFORMAT_DEPTH32_FLOAT_STENCIL8;
}

bool isPixelFormatStencil(PixelFormat format)
{
	return format == PIXELFORMAT_STENCIL8 || format == PIXELFORMAT_DEPTH24_UNORM_STENCIL8 || format == PIXELFORMAT_DEPTH32_FLOAT_STENCIL8;
}

size_t getPixelFormatSize(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_R8_UNORM:
	case PIXELFORMAT_STENCIL8:
		return 1;
	case PIXELFORMAT_RG8_UNORM:
	case PIXELFORMAT_R16_UNORM:
	case PIXELFORMAT_R16_FLOAT:
	case PIXELFORMAT_LA8_UNORM:
	case PIXELFORMAT_RGBA4_UNORM:
	case PIXELFORMAT_RGB5A1_UNORM:
	case PIXELFORMAT_RGB565_UNORM:
	case PIXELFORMAT_DEPTH16_UNORM:
		return 2;
	case PIXELFORMAT_RGBA8_UNORM:
	case PIXELFORMAT_sRGBA8_UNORM:
	case PIXELFORMAT_RG16_UNORM:
	case PIXELFORMAT_RG16_FLOAT:
	case PIXELFORMAT_R32_FLOAT:
	case PIXELFORMAT_RGB10A2_UNORM:
	case PIXELFORMAT_RG11B10_FLOAT:
	case PIXELFORMAT_DEPTH24_UNORM:
	case PIXELFORMAT_DEPTH32_FLOAT:
	case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
		return 4;
	case PIXELFORMAT_RGBA16_UNORM:
	case PIXELFORMAT_RGBA16_FLOAT:
	case PIXELFORMAT_RG32_FLOAT:
	case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		return 8;
	case PIXELFORMAT_RGBA32_FLOAT:
		return 16;
	default:
		// TODO: compressed formats
		return 0;
	}
}

int getPixelFormatColorComponents(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_R8_UNORM:
	case PIXELFORMAT_R16_UNORM:
	case PIXELFORMAT_R16_FLOAT:
	case PIXELFORMAT_R32_FLOAT:
		return 1;
	case PIXELFORMAT_RG8_UNORM:
	case PIXELFORMAT_RG16_UNORM:
	case PIXELFORMAT_RG16_FLOAT:
	case PIXELFORMAT_RG32_FLOAT:
	case PIXELFORMAT_LA8_UNORM:
		return 2;
	case PIXELFORMAT_RGB565_UNORM:
	case PIXELFORMAT_RG11B10_FLOAT:
		return 3;
	case PIXELFORMAT_RGBA8_UNORM:
	case PIXELFORMAT_sRGBA8_UNORM:
	case PIXELFORMAT_RGBA16_UNORM:
	case PIXELFORMAT_RGBA16_FLOAT:
	case PIXELFORMAT_RGBA32_FLOAT:
	case PIXELFORMAT_RGBA4_UNORM:
	case PIXELFORMAT_RGB5A1_UNORM:
	case PIXELFORMAT_RGB10A2_UNORM:
		return 4;
	default:
		return 0;
	}
}

} // love
