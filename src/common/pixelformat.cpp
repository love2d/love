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

#include "pixelformat.h"
#include "StringMap.h"

namespace love
{

static PixelFormatInfo formatInfo[] =
{
	// components, blockW, blockH, blockSize, color, depth, stencil, compressed, sRGB, dataType
    { 0, 1, 1, 0, false, false, false, false, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_UNKNOWN

	{ 0, 1, 1, 0, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_NORMAL
	{ 0, 1, 1, 0, true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_HDR

	{ 1, 1, 1, 1, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_R8_UNORM
	{ 1, 1, 1, 1, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_R8_INT
	{ 1, 1, 1, 1, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_R8_UINT
	{ 1, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_R16_UNORM
	{ 1, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_R16_FLOAT
	{ 1, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_R16_INT
	{ 1, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_R16_UINT
	{ 1, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_R32_FLOAT
	{ 1, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_R32_INT
	{ 1, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_R32_UINT

	{ 2, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RG8_UNORM
	{ 2, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_RG8_INT
	{ 2, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_RG8_UINT
	{ 2, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_LA8_UNORM
	{ 2, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RG16_UNORM
	{ 2, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_RG16_FLOAT
	{ 2, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_RG16_INT
	{ 2, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_RG16_UINT
	{ 2, 1, 1, 8, true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_RG32_FLOAT
	{ 2, 1, 1, 8, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_RG32_INT
	{ 2, 1, 1, 8, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_RG32_UINT

	{ 4, 1, 1, 4,  true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGBA8_UNORM
	{ 4, 1, 1, 4,  true, false, false, false, true,  PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGBA8_sRGB
	{ 4, 1, 1, 4,  true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_BGRA8_UNORM
	{ 4, 1, 1, 4,  true, false, false, false, true,  PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_BGRA8_sRGB
	{ 4, 1, 1, 4,  true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_RGBA8_INT
	{ 4, 1, 1, 4,  true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_RGBA8_UINT
	{ 4, 1, 1, 8,  true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGBA16_UNORM
	{ 4, 1, 1, 8,  true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_RGBA16_FLOAT
	{ 4, 1, 1, 8,  true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_RGBA16_INT
	{ 4, 1, 1, 8,  true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_RGBA16_UINT
	{ 4, 1, 1, 16, true, false, false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_RGBA32_FLOAT
	{ 4, 1, 1, 16, true, false, false, false, false, PIXELFORMATTYPE_SINT   }, // PIXELFORMAT_RGBA32_INT
	{ 4, 1, 1, 16, true, false, false, false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_RGBA32_UINT

	{ 4, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGBA4_UNORM
	{ 4, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGB5A1_UNORM
	{ 3, 1, 1, 2, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGB565_UNORM
	{ 4, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_RGB10A2_UNORM
	{ 3, 1, 1, 4, true, false, false, false, false, PIXELFORMATTYPE_UFLOAT }, // PIXELFORMAT_RG11B10_FLOAT

	{ 1, 1, 1, 1, false, false, true , false, false, PIXELFORMATTYPE_UINT   }, // PIXELFORMAT_STENCIL8
	{ 1, 1, 1, 2, false, true,  false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DEPTH16_UNORM
	{ 1, 1, 1, 3, false, true,  false, false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DEPTH24_UNORM
	{ 1, 1, 1, 4, false, true,  false, false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_DEPTH32_FLOAT
	{ 2, 1, 1, 4, false, true,  true , false, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DEPTH24_UNORM_STENCIL8
	{ 2, 1, 1, 5, false, true,  true , false, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_DEPTH32_FLOAT_STENCIL8

	{ 3, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DXT1_UNORM
	{ 3, 4, 4, 8,  true, false, false, true, true,  PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DXT1_sRGB
	{ 4, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DXT3_UNORM
	{ 4, 4, 4, 16, true, false, false, true, true,  PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DXT3_sRGB
	{ 4, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DXT5_UNORM
	{ 4, 4, 4, 16, true, false, false, true, true,  PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_DXT5_sRGB
	{ 1, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_BC4_UNORM
	{ 1, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_SNORM  }, // PIXELFORMAT_BC4_SNORM
	{ 2, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_BC5_UNORM
	{ 2, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_SNORM  }, // PIXELFORMAT_BC5_SNORM
	{ 3, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UFLOAT }, // PIXELFORMAT_BC6H_UFLOAT
	{ 3, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_SFLOAT }, // PIXELFORMAT_BC6H_FLOAT
	{ 4, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_BC7_UNORM
	{ 4, 4, 4, 16, true, false, false, true, true,  PIXELFORMATTYPE_UNORM  }, // PIXELFORMAT_BC7_sRGB

	{ 3, 16, 8, 32, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGB2_UNORM
	{ 3, 16, 8, 32, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGB2_sRGB
	{ 3, 8,  8, 32, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGB4_UNORM
	{ 3, 8,  8, 32, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGB4_sRGB
	{ 4, 16, 8, 32, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGBA2_UNORM
	{ 4, 16, 8, 32, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGBA2_sRGB
	{ 4, 8,  8, 32, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGBA4_UNORM
	{ 4, 8,  8, 32, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_PVR1_RGBA4_sRGB

	{ 3, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC1_UNORM
	{ 3, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC2_RGB_UNORM
	{ 3, 4, 4, 8,  true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC2_RGB_sRGB
	{ 4, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC2_RGBA_UNORM
	{ 4, 4, 4, 16, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC2_RGBA_sRGB
	{ 4, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC2_RGBA1_UNORM
	{ 4, 4, 4, 8,  true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ETC2_RGBA1_sRGB
	{ 1, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_EAC_R_UNORM
	{ 1, 4, 4, 8,  true, false, false, true, false, PIXELFORMATTYPE_SNORM }, // PIXELFORMAT_EAC_R_SNORM
	{ 2, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_EAC_RG_UNORM
	{ 2, 4, 4, 16, true, false, false, true, false, PIXELFORMATTYPE_SNORM }, // PIXELFORMAT_EAC_RG_SNORM

	{ 4, 4,  4,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_4x4_UNORM
	{ 4, 5,  4,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_5x4_UNORM
	{ 4, 5,  5,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_5x5_UNORM
	{ 4, 6,  5,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_6x5_UNORM
	{ 4, 6,  6,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_6x6_UNORM
	{ 4, 8,  5,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_8x5_UNORM
	{ 4, 8,  6,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_8x6_UNORM
	{ 4, 8,  8,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_8x8_UNORM
	{ 4, 8,  5,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x5_UNORM
	{ 4, 10, 6,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x6_UNORM
	{ 4, 10, 8,  1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x8_UNORM
	{ 4, 10, 10, 1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x10_UNORM
	{ 4, 12, 10, 1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_12x10_UNORM
	{ 4, 12, 12, 1, true, false, false, true, false, PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_12x12_UNORM
	{ 4, 4,  4,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_4x4_sRGB
	{ 4, 5,  4,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_5x4_sRGB
	{ 4, 5,  5,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_5x5_sRGB
	{ 4, 6,  5,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_6x5_sRGB
	{ 4, 6,  6,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_6x6_sRGB
	{ 4, 8,  5,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_8x5_sRGB
	{ 4, 8,  6,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_8x6_sRGB
	{ 4, 8,  8,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_8x8_sRGB
	{ 4, 8,  5,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x5_sRGB
	{ 4, 10, 6,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x6_sRGB
	{ 4, 10, 8,  1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x8_sRGB
	{ 4, 10, 10, 1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_10x10_sRGB
	{ 4, 12, 10, 1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_12x10_sRGB
	{ 4, 12, 12, 1, true, false, false, true, true,  PIXELFORMATTYPE_UNORM }, // PIXELFORMAT_ASTC_12x12_sRGB
};

static_assert(sizeof(formatInfo) / sizeof(PixelFormatInfo) == PIXELFORMAT_MAX_ENUM, "Update the formatInfo array when adding or removing a PixelFormat");

static StringMap<PixelFormat, PIXELFORMAT_MAX_ENUM>::Entry formatEntries[] =
{
    { "unknown", PIXELFORMAT_UNKNOWN },

	{ "normal",  PIXELFORMAT_NORMAL  },
	{ "hdr",     PIXELFORMAT_HDR     },

	{ "r8",    PIXELFORMAT_R8_UNORM  },
	{ "r8i",   PIXELFORMAT_R8_INT    },
	{ "r8ui",  PIXELFORMAT_R8_UINT   },
	{ "r16",   PIXELFORMAT_R16_UNORM },
	{ "r16f",  PIXELFORMAT_R16_FLOAT },
	{ "r16i",  PIXELFORMAT_R16_INT   },
	{ "r16ui", PIXELFORMAT_R16_UINT  },
	{ "r32f",  PIXELFORMAT_R32_FLOAT },
	{ "r32i",  PIXELFORMAT_R32_INT   },
	{ "r32ui", PIXELFORMAT_R32_UINT  },

	{ "rg8",    PIXELFORMAT_RG8_UNORM  },
	{ "rg8i",   PIXELFORMAT_RG8_INT    },
	{ "rg8ui",  PIXELFORMAT_RG8_UINT   },
	{ "la8",    PIXELFORMAT_LA8_UNORM  },
	{ "rg16",   PIXELFORMAT_RG16_UNORM },
	{ "rg16f",  PIXELFORMAT_RG16_FLOAT },
	{ "rg16i",  PIXELFORMAT_RG16_INT   },
	{ "rg16ui", PIXELFORMAT_RG16_UINT  },
	{ "rg32f",  PIXELFORMAT_RG32_FLOAT },
	{ "rg32i",  PIXELFORMAT_RG32_INT   },
	{ "rg32ui", PIXELFORMAT_RG32_UINT  },

	{ "rgba8",     PIXELFORMAT_RGBA8_UNORM  },
	{ "srgba8",    PIXELFORMAT_RGBA8_sRGB   },
	{ "bgra8",     PIXELFORMAT_BGRA8_UNORM  },
	{ "bgra8srgb", PIXELFORMAT_BGRA8_sRGB   },
	{ "rgba8i",    PIXELFORMAT_RGBA8_INT    },
	{ "rgba8ui",   PIXELFORMAT_RGBA8_UINT   },
	{ "rgba16",    PIXELFORMAT_RGBA16_UNORM },
	{ "rgba16f",   PIXELFORMAT_RGBA16_FLOAT },
	{ "rgba16i",   PIXELFORMAT_RGBA16_INT   },
	{ "rgba16ui",  PIXELFORMAT_RGBA16_UINT  },
	{ "rgba32f",   PIXELFORMAT_RGBA32_FLOAT },
	{ "rgba32i",   PIXELFORMAT_RGBA32_INT   },
	{ "rgba32ui",  PIXELFORMAT_RGBA32_UINT  },

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
	
	{ "DXT1",     PIXELFORMAT_DXT1_UNORM  },
	{ "DXT1srgb", PIXELFORMAT_DXT1_sRGB   },
	{ "DXT3",     PIXELFORMAT_DXT3_UNORM  },
	{ "DXT3srgb", PIXELFORMAT_DXT3_sRGB   },
	{ "DXT5",     PIXELFORMAT_DXT5_UNORM  },
	{ "DXT5srgb", PIXELFORMAT_DXT5_sRGB   },
	{ "BC4",      PIXELFORMAT_BC4_UNORM   },
	{ "BC4s",     PIXELFORMAT_BC4_SNORM   },
	{ "BC5",      PIXELFORMAT_BC5_UNORM   },
	{ "BC5s",     PIXELFORMAT_BC5_SNORM   },
	{ "BC6h",     PIXELFORMAT_BC6H_UFLOAT },
	{ "BC6hs",    PIXELFORMAT_BC6H_FLOAT  },
	{ "BC7",      PIXELFORMAT_BC7_UNORM   },
	{ "BC7srgb",  PIXELFORMAT_BC7_sRGB    },

	{ "PVR1rgb2",      PIXELFORMAT_PVR1_RGB2_UNORM  },
	{ "PVR1rgb2srgb",  PIXELFORMAT_PVR1_RGB2_sRGB   },
	{ "PVR1rgb4",      PIXELFORMAT_PVR1_RGB4_UNORM  },
	{ "PVR1rgb4srgb",  PIXELFORMAT_PVR1_RGB4_sRGB   },
	{ "PVR1rgba2",     PIXELFORMAT_PVR1_RGBA2_UNORM },
	{ "PVR1rgba2srgb", PIXELFORMAT_PVR1_RGBA2_sRGB  },
	{ "PVR1rgba4",     PIXELFORMAT_PVR1_RGBA4_UNORM },
	{ "PVR1rgba4srgb", PIXELFORMAT_PVR1_RGBA4_sRGB  },

	{ "ETC1",       PIXELFORMAT_ETC1_UNORM       },
	{ "ETC2rgb",    PIXELFORMAT_ETC2_RGB_UNORM   },
	{ "ETC2srgb",   PIXELFORMAT_ETC2_RGB_sRGB    },
	{ "ETC2rgba",   PIXELFORMAT_ETC2_RGBA_UNORM  },
	{ "ETC2srgba",  PIXELFORMAT_ETC2_RGBA_sRGB   },
	{ "ETC2rgba1",  PIXELFORMAT_ETC2_RGBA1_UNORM },
	{ "ETC2srgba1", PIXELFORMAT_ETC2_RGBA1_sRGB  },
	{ "EACr",       PIXELFORMAT_EAC_R_UNORM      },
	{ "EACrs",      PIXELFORMAT_EAC_R_SNORM      },
	{ "EACrg",      PIXELFORMAT_EAC_RG_UNORM     },
	{ "EACrgs",     PIXELFORMAT_EAC_RG_SNORM     },

	{ "ASTC4x4",       PIXELFORMAT_ASTC_4x4_UNORM   },
	{ "ASTC5x4",       PIXELFORMAT_ASTC_5x4_UNORM   },
	{ "ASTC5x5",       PIXELFORMAT_ASTC_5x5_UNORM   },
	{ "ASTC6x5",       PIXELFORMAT_ASTC_6x5_UNORM   },
	{ "ASTC6x6",       PIXELFORMAT_ASTC_6x6_UNORM   },
	{ "ASTC8x5",       PIXELFORMAT_ASTC_8x5_UNORM   },
	{ "ASTC8x6",       PIXELFORMAT_ASTC_8x6_UNORM   },
	{ "ASTC8x8",       PIXELFORMAT_ASTC_8x8_UNORM   },
	{ "ASTC10x5",      PIXELFORMAT_ASTC_10x5_UNORM  },
	{ "ASTC10x6",      PIXELFORMAT_ASTC_10x6_UNORM  },
	{ "ASTC10x8",      PIXELFORMAT_ASTC_10x8_UNORM  },
	{ "ASTC10x10",     PIXELFORMAT_ASTC_10x10_UNORM },
	{ "ASTC12x10",     PIXELFORMAT_ASTC_12x10_UNORM },
	{ "ASTC12x12",     PIXELFORMAT_ASTC_12x12_UNORM },
	{ "ASTC4x4srgb",   PIXELFORMAT_ASTC_4x4_sRGB    },
	{ "ASTC5x4srgb",   PIXELFORMAT_ASTC_5x4_sRGB    },
	{ "ASTC5x5srgb",   PIXELFORMAT_ASTC_5x5_sRGB    },
	{ "ASTC6x5srgb",   PIXELFORMAT_ASTC_6x5_sRGB    },
	{ "ASTC6x6srgb",   PIXELFORMAT_ASTC_6x6_sRGB    },
	{ "ASTC8x5srgb",   PIXELFORMAT_ASTC_8x5_sRGB    },
	{ "ASTC8x6srgb",   PIXELFORMAT_ASTC_8x6_sRGB    },
	{ "ASTC8x8srgb",   PIXELFORMAT_ASTC_8x8_sRGB    },
	{ "ASTC10x5srgb",  PIXELFORMAT_ASTC_10x5_sRGB   },
	{ "ASTC10x6srgb",  PIXELFORMAT_ASTC_10x6_sRGB   },
	{ "ASTC10x8srgb",  PIXELFORMAT_ASTC_10x8_sRGB   },
	{ "ASTC10x10srgb", PIXELFORMAT_ASTC_10x10_sRGB  },
	{ "ASTC12x10srgb", PIXELFORMAT_ASTC_12x10_sRGB  },
	{ "ASTC12x12srgb", PIXELFORMAT_ASTC_12x12_sRGB  },
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

const char *getPixelFormatName(PixelFormat format)
{
	const char *name = "unknown";
	getConstant(format, name);
	return name;
}

bool isPixelFormatCompressed(PixelFormat format)
{
	return formatInfo[format].compressed;
}

bool isPixelFormatColor(PixelFormat format)
{
	return formatInfo[format].color;
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

bool isPixelFormatSRGB(PixelFormat format)
{
	return formatInfo[format].sRGB;
}

bool isPixelFormatInteger(PixelFormat format)
{
	auto type = formatInfo[format].dataType;
	return type == PIXELFORMATTYPE_SINT || type == PIXELFORMATTYPE_UINT;
}

PixelFormat getSRGBPixelFormat(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_RGBA8_UNORM: return PIXELFORMAT_RGBA8_sRGB;
	case PIXELFORMAT_BGRA8_UNORM: return PIXELFORMAT_BGRA8_sRGB;
	case PIXELFORMAT_DXT1_UNORM: return PIXELFORMAT_DXT1_sRGB;
	case PIXELFORMAT_DXT3_UNORM: return PIXELFORMAT_DXT3_sRGB;
	case PIXELFORMAT_DXT5_UNORM: return PIXELFORMAT_DXT5_sRGB;
	case PIXELFORMAT_BC7_UNORM: return PIXELFORMAT_BC7_sRGB;
	case PIXELFORMAT_PVR1_RGB2_UNORM: return PIXELFORMAT_PVR1_RGB2_sRGB;
	case PIXELFORMAT_PVR1_RGB4_UNORM: return PIXELFORMAT_PVR1_RGB4_sRGB;
	case PIXELFORMAT_PVR1_RGBA2_UNORM: return PIXELFORMAT_PVR1_RGBA2_sRGB;
	case PIXELFORMAT_PVR1_RGBA4_UNORM: return PIXELFORMAT_PVR1_RGBA4_sRGB;
	case PIXELFORMAT_ETC1_UNORM: return PIXELFORMAT_ETC2_RGB_sRGB; // ETC2 can load ETC1 data.
	case PIXELFORMAT_ETC2_RGB_UNORM: return PIXELFORMAT_ETC2_RGB_sRGB;
	case PIXELFORMAT_ETC2_RGBA_UNORM: return PIXELFORMAT_ETC2_RGBA_sRGB;
	case PIXELFORMAT_ETC2_RGBA1_UNORM: return PIXELFORMAT_ETC2_RGBA1_sRGB;
	case PIXELFORMAT_ASTC_4x4_UNORM: return PIXELFORMAT_ASTC_4x4_sRGB;
	case PIXELFORMAT_ASTC_5x4_UNORM: return PIXELFORMAT_ASTC_5x4_sRGB;
	case PIXELFORMAT_ASTC_5x5_UNORM: return PIXELFORMAT_ASTC_5x5_sRGB;
	case PIXELFORMAT_ASTC_6x5_UNORM: return PIXELFORMAT_ASTC_6x5_sRGB;
	case PIXELFORMAT_ASTC_6x6_UNORM: return PIXELFORMAT_ASTC_6x6_sRGB;
	case PIXELFORMAT_ASTC_8x5_UNORM: return PIXELFORMAT_ASTC_8x5_sRGB;
	case PIXELFORMAT_ASTC_8x6_UNORM: return PIXELFORMAT_ASTC_8x6_sRGB;
	case PIXELFORMAT_ASTC_8x8_UNORM: return PIXELFORMAT_ASTC_8x8_sRGB;
	case PIXELFORMAT_ASTC_10x5_UNORM: return PIXELFORMAT_ASTC_10x5_sRGB;
	case PIXELFORMAT_ASTC_10x6_UNORM: return PIXELFORMAT_ASTC_10x6_sRGB;
	case PIXELFORMAT_ASTC_10x8_UNORM: return PIXELFORMAT_ASTC_10x8_sRGB;
	case PIXELFORMAT_ASTC_10x10_UNORM: return PIXELFORMAT_ASTC_10x10_sRGB;
	case PIXELFORMAT_ASTC_12x10_UNORM: return PIXELFORMAT_ASTC_12x10_sRGB;
	case PIXELFORMAT_ASTC_12x12_UNORM: return PIXELFORMAT_ASTC_12x12_sRGB;
	default: return format;
	}
}

PixelFormat getLinearPixelFormat(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_RGBA8_sRGB: return PIXELFORMAT_RGBA8_UNORM;
	case PIXELFORMAT_BGRA8_sRGB: return PIXELFORMAT_BGRA8_UNORM;
	case PIXELFORMAT_DXT1_sRGB: return PIXELFORMAT_DXT1_UNORM;
	case PIXELFORMAT_DXT3_sRGB: return PIXELFORMAT_DXT3_UNORM;
	case PIXELFORMAT_DXT5_sRGB: return PIXELFORMAT_DXT5_UNORM;
	case PIXELFORMAT_BC7_sRGB: return PIXELFORMAT_BC7_UNORM;
	case PIXELFORMAT_PVR1_RGB2_sRGB: return PIXELFORMAT_PVR1_RGB2_UNORM;
	case PIXELFORMAT_PVR1_RGB4_sRGB: return PIXELFORMAT_PVR1_RGB4_UNORM;
	case PIXELFORMAT_PVR1_RGBA2_sRGB: return PIXELFORMAT_PVR1_RGBA2_UNORM;
	case PIXELFORMAT_PVR1_RGBA4_sRGB: return PIXELFORMAT_PVR1_RGBA4_UNORM;
	case PIXELFORMAT_ETC2_RGB_sRGB: return PIXELFORMAT_ETC2_RGB_UNORM;
	case PIXELFORMAT_ETC2_RGBA_sRGB: return PIXELFORMAT_ETC2_RGBA_UNORM;
	case PIXELFORMAT_ETC2_RGBA1_sRGB: return PIXELFORMAT_ETC2_RGBA1_UNORM;
	case PIXELFORMAT_ASTC_4x4_sRGB: return PIXELFORMAT_ASTC_4x4_UNORM;
	case PIXELFORMAT_ASTC_5x4_sRGB: return PIXELFORMAT_ASTC_5x4_UNORM;
	case PIXELFORMAT_ASTC_5x5_sRGB: return PIXELFORMAT_ASTC_5x5_UNORM;
	case PIXELFORMAT_ASTC_6x5_sRGB: return PIXELFORMAT_ASTC_6x5_UNORM;
	case PIXELFORMAT_ASTC_6x6_sRGB: return PIXELFORMAT_ASTC_6x6_UNORM;
	case PIXELFORMAT_ASTC_8x5_sRGB: return PIXELFORMAT_ASTC_8x5_UNORM;
	case PIXELFORMAT_ASTC_8x6_sRGB: return PIXELFORMAT_ASTC_8x6_UNORM;
	case PIXELFORMAT_ASTC_8x8_sRGB: return PIXELFORMAT_ASTC_8x8_UNORM;
	case PIXELFORMAT_ASTC_10x5_sRGB: return PIXELFORMAT_ASTC_10x5_UNORM;
	case PIXELFORMAT_ASTC_10x6_sRGB: return PIXELFORMAT_ASTC_10x6_UNORM;
	case PIXELFORMAT_ASTC_10x8_sRGB: return PIXELFORMAT_ASTC_10x8_UNORM;
	case PIXELFORMAT_ASTC_10x10_sRGB: return PIXELFORMAT_ASTC_10x10_UNORM;
	case PIXELFORMAT_ASTC_12x10_sRGB: return PIXELFORMAT_ASTC_12x10_UNORM;
	case PIXELFORMAT_ASTC_12x12_sRGB: return PIXELFORMAT_ASTC_12x12_UNORM;
	default: return format;
	}
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
