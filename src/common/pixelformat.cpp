/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

	{ "r8",      PIXELFORMAT_R8      },
	{ "rg8",     PIXELFORMAT_RG8     },
	{ "rgba8",   PIXELFORMAT_RGBA8   },
	{ "srgba8",  PIXELFORMAT_sRGBA8  },
	{ "r16",     PIXELFORMAT_R16     },
	{ "rg16",    PIXELFORMAT_RG16    },
	{ "rgba16",  PIXELFORMAT_RGBA16  },
	{ "r16f",    PIXELFORMAT_R16F    },
	{ "rg16f",   PIXELFORMAT_RG16F   },
	{ "rgba16f", PIXELFORMAT_RGBA16F },
	{ "r32f",    PIXELFORMAT_R32F    },
	{ "rg32f",   PIXELFORMAT_RG32F   },
	{ "rgba32f", PIXELFORMAT_RGBA32F },

	{ "la8",     PIXELFORMAT_LA8     },

	{ "rgba4",    PIXELFORMAT_RGBA4    },
	{ "rgb5a1",   PIXELFORMAT_RGB5A1   },
	{ "rgb565",   PIXELFORMAT_RGB565   },
	{ "rgb10a2",  PIXELFORMAT_RGB10A2  },
	{ "rg11b10f", PIXELFORMAT_RG11B10F },

	{ "stencil8",         PIXELFORMAT_STENCIL8          },
	{ "depth16",          PIXELFORMAT_DEPTH16           },
	{ "depth24",          PIXELFORMAT_DEPTH24           },
	{ "depth32f",         PIXELFORMAT_DEPTH32F          },
	{ "depth24stencil8",  PIXELFORMAT_DEPTH24_STENCIL8  },
	{ "depth32fstencil8", PIXELFORMAT_DEPTH32F_STENCIL8 },
	
	{ "DXT1",      PIXELFORMAT_DXT1       },
	{ "DXT3",      PIXELFORMAT_DXT3       },
	{ "DXT5",      PIXELFORMAT_DXT5       },
	{ "BC4",       PIXELFORMAT_BC4        },
	{ "BC4s",      PIXELFORMAT_BC4s       },
	{ "BC5",       PIXELFORMAT_BC5        },
	{ "BC5s",      PIXELFORMAT_BC5s       },
	{ "BC6h",      PIXELFORMAT_BC6H       },
	{ "BC6hs",     PIXELFORMAT_BC6Hs      },
	{ "BC7",       PIXELFORMAT_BC7        },
	{ "PVR1rgb2",  PIXELFORMAT_PVR1_RGB2  },
	{ "PVR1rgb4",  PIXELFORMAT_PVR1_RGB4  },
	{ "PVR1rgba2", PIXELFORMAT_PVR1_RGBA2 },
	{ "PVR1rgba4", PIXELFORMAT_PVR1_RGBA4 },
	{ "ETC1",      PIXELFORMAT_ETC1       },
	{ "ETC2rgb",   PIXELFORMAT_ETC2_RGB   },
	{ "ETC2rgba",  PIXELFORMAT_ETC2_RGBA  },
	{ "ETC2rgba1", PIXELFORMAT_ETC2_RGBA1 },
	{ "EACr",      PIXELFORMAT_EAC_R      },
	{ "EACrs",     PIXELFORMAT_EAC_Rs     },
	{ "EACrg",     PIXELFORMAT_EAC_RG     },
	{ "EACrgs",    PIXELFORMAT_EAC_RGs    },
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
	return iformat >= (int) PIXELFORMAT_DXT1 && iformat < (int) PIXELFORMAT_MAX_ENUM;
}

bool isPixelFormatDepthStencil(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_STENCIL8 && iformat <= (int) PIXELFORMAT_DEPTH32F_STENCIL8;
}

bool isPixelFormatDepth(PixelFormat format)
{
	int iformat = (int) format;
	return iformat >= (int) PIXELFORMAT_DEPTH16 && iformat <= (int) PIXELFORMAT_DEPTH32F_STENCIL8;
}

bool isPixelFormatStencil(PixelFormat format)
{
	return format == PIXELFORMAT_STENCIL8 || format == PIXELFORMAT_DEPTH24_STENCIL8 || format == PIXELFORMAT_DEPTH32F_STENCIL8;
}

size_t getPixelFormatSize(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_R8:
	case PIXELFORMAT_STENCIL8:
		return 1;
	case PIXELFORMAT_RG8:
	case PIXELFORMAT_R16:
	case PIXELFORMAT_R16F:
	case PIXELFORMAT_LA8:
	case PIXELFORMAT_RGBA4:
	case PIXELFORMAT_RGB5A1:
	case PIXELFORMAT_RGB565:
	case PIXELFORMAT_DEPTH16:
		return 2;
	case PIXELFORMAT_RGBA8:
	case PIXELFORMAT_sRGBA8:
	case PIXELFORMAT_RG16:
	case PIXELFORMAT_RG16F:
	case PIXELFORMAT_R32F:
	case PIXELFORMAT_RGB10A2:
	case PIXELFORMAT_RG11B10F:
	case PIXELFORMAT_DEPTH24:
	case PIXELFORMAT_DEPTH32F:
	case PIXELFORMAT_DEPTH24_STENCIL8:
		return 4;
	case PIXELFORMAT_RGBA16:
	case PIXELFORMAT_RGBA16F:
	case PIXELFORMAT_RG32F:
	case PIXELFORMAT_DEPTH32F_STENCIL8:
		return 8;
	case PIXELFORMAT_RGBA32F:
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
	case PIXELFORMAT_R8:
	case PIXELFORMAT_R16:
	case PIXELFORMAT_R16F:
	case PIXELFORMAT_R32F:
		return 1;
	case PIXELFORMAT_RG8:
	case PIXELFORMAT_RG16:
	case PIXELFORMAT_RG16F:
	case PIXELFORMAT_RG32F:
	case PIXELFORMAT_LA8:
		return 2;
	case PIXELFORMAT_RGB565:
	case PIXELFORMAT_RG11B10F:
		return 3;
	case PIXELFORMAT_RGBA8:
	case PIXELFORMAT_sRGBA8:
	case PIXELFORMAT_RGBA16:
	case PIXELFORMAT_RGBA16F:
	case PIXELFORMAT_RGBA32F:
	case PIXELFORMAT_RGBA4:
	case PIXELFORMAT_RGB5A1:
	case PIXELFORMAT_RGB10A2:
		return 4;
	default:
		return 0;
	}
}

} // love
