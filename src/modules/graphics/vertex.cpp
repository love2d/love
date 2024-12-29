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

#include "vertex.h"
#include "common/StringMap.h"

namespace love
{
namespace graphics
{

static_assert(sizeof(VertexAttributeInfo) == 4, "Unexpected sizeof(VertexAttributeInfo)");

static_assert(sizeof(Color32) == 4, "sizeof(Color32) incorrect!");
static_assert(sizeof(STf_RGBAub) == sizeof(float)*2 + sizeof(Color32), "sizeof(STf_RGBAub) incorrect!");
static_assert(sizeof(STPf_RGBAub) == sizeof(float)*3 + sizeof(Color32), "sizeof(STPf_RGBAub) incorrect!");
static_assert(sizeof(XYf_STf) == sizeof(float)*2 + sizeof(float)*2, "sizeof(XYf_STf) incorrect!");
static_assert(sizeof(XYf_STPf) == sizeof(float)*2 + sizeof(float)*3, "sizeof(XYf_STPf) incorrect!");
static_assert(sizeof(XYf_STf_RGBAub) == sizeof(float)*2 + sizeof(float)*2 + sizeof(Color32), "sizeof(XYf_STf_RGBAub) incorrect!");
static_assert(sizeof(XYf_STus_RGBAub) == sizeof(float)*2 + sizeof(uint16)*2 + sizeof(Color32), "sizeof(XYf_STus_RGBAub) incorrect!");
static_assert(sizeof(XYf_STPf_RGBAub) == sizeof(float)*2 + sizeof(float)*3 + sizeof(Color32), "sizeof(XYf_STPf_RGBAub) incorrect!");

size_t getFormatStride(CommonFormat format)
{
	switch (format)
	{
		case CommonFormat::NONE: return 0;
		case CommonFormat::XYf: return sizeof(float) * 2;
		case CommonFormat::XYZf: return sizeof(float) * 3;
		case CommonFormat::RGBAub: return sizeof(uint8) * 4;
		case CommonFormat::STf_RGBAub: return sizeof(STf_RGBAub);
		case CommonFormat::STPf_RGBAub: return sizeof(STPf_RGBAub);
		case CommonFormat::XYf_STf: return sizeof(XYf_STf);
		case CommonFormat::XYf_STPf: return sizeof(XYf_STPf);
		case CommonFormat::XYf_STf_RGBAub: return sizeof(XYf_STf_RGBAub);
		case CommonFormat::XYf_STus_RGBAub: return sizeof(XYf_STus_RGBAub);
		case CommonFormat::XYf_STPf_RGBAub: return sizeof(XYf_STPf_RGBAub);
		case CommonFormat::COUNT: return 0;
	}
	return 0;
}

uint32 getFormatFlags(CommonFormat format)
{
	switch (format)
	{
	case CommonFormat::NONE:
		return 0;
	case CommonFormat::XYf:
	case CommonFormat::XYZf:
		return ATTRIBFLAG_POS;
	case CommonFormat::RGBAub:
		return ATTRIBFLAG_COLOR;
	case CommonFormat::STf_RGBAub:
	case CommonFormat::STPf_RGBAub:
		return ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR;
	case CommonFormat::XYf_STf:
	case CommonFormat::XYf_STPf:
		return ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD;
	case CommonFormat::XYf_STf_RGBAub:
	case CommonFormat::XYf_STus_RGBAub:
	case CommonFormat::XYf_STPf_RGBAub:
		return ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD | ATTRIBFLAG_COLOR;
	case CommonFormat::COUNT:
		return 0;
	}
	return 0;
}

int getFormatPositionComponents(CommonFormat format)
{
	switch (format)
	{
	case CommonFormat::NONE:
	case CommonFormat::RGBAub:
	case CommonFormat::STf_RGBAub:
	case CommonFormat::STPf_RGBAub:
		return 0;
	case CommonFormat::XYf:
	case CommonFormat::XYf_STf:
	case CommonFormat::XYf_STPf:
	case CommonFormat::XYf_STf_RGBAub:
	case CommonFormat::XYf_STus_RGBAub:
	case CommonFormat::XYf_STPf_RGBAub:
		return 2;
	case CommonFormat::XYZf:
		return 3;
	case CommonFormat::COUNT:
		return 0;
	}
	return 0;
}

// Order here relies on order of DataFormat enum.
static const DataFormatInfo dataFormatInfo[]
{
	// baseType, isMatrix, components, rows, columns, componentSize, size
	{ DATA_BASETYPE_FLOAT, false, 1, 0, 0, 4, 4  }, // DATAFORMAT_FLOAT
	{ DATA_BASETYPE_FLOAT, false, 2, 0, 0, 4, 8  }, // DATAFORMAT_FLOAT_VEC2
	{ DATA_BASETYPE_FLOAT, false, 3, 0, 0, 4, 12 }, // DATAFORMAT_FLOAT_VEC3
	{ DATA_BASETYPE_FLOAT, false, 4, 0, 0, 4, 16 }, // DATAFORMAT_FLOAT_VEC4

	{ DATA_BASETYPE_FLOAT, true, 4, 2, 2, 4, 16 }, // DATAFORMAT_FLOAT_MAT2X2
	{ DATA_BASETYPE_FLOAT, true, 6, 2, 3, 4, 24 }, // DATAFORMAT_FLOAT_MAT2X3
	{ DATA_BASETYPE_FLOAT, true, 8, 2, 4, 4, 32 }, // DATAFORMAT_FLOAT_MAT2X4

	{ DATA_BASETYPE_FLOAT, true, 6,  3, 2, 4, 24 }, // DATAFORMAT_FLOAT_MAT3X2
	{ DATA_BASETYPE_FLOAT, true, 9,  3, 3, 4, 36 }, // DATAFORMAT_FLOAT_MAT3X3
	{ DATA_BASETYPE_FLOAT, true, 12, 3, 4, 4, 48 }, // DATAFORMAT_FLOAT_MAT3X4

	{ DATA_BASETYPE_FLOAT, true, 8,  4, 2, 4, 32 }, // DATAFORMAT_FLOAT_MAT4X2
	{ DATA_BASETYPE_FLOAT, true, 12, 4, 3, 4, 48 }, // DATAFORMAT_FLOAT_MAT4X3
	{ DATA_BASETYPE_FLOAT, true, 16, 4, 4, 4, 64 }, // DATAFORMAT_FLOAT_MAT4X4

	{ DATA_BASETYPE_INT, false, 1, 0, 0, 4, 4  }, // DATAFORMAT_INT32
	{ DATA_BASETYPE_INT, false, 2, 0, 0, 4, 8  }, // DATAFORMAT_INT32_VEC2
	{ DATA_BASETYPE_INT, false, 3, 0, 0, 4, 12 }, // DATAFORMAT_INT32_VEC3
	{ DATA_BASETYPE_INT, false, 4, 0, 0, 4, 16 }, // DATAFORMAT_INT32_VEC4

	{ DATA_BASETYPE_UINT, false, 1, 0, 0, 4, 4  }, // DATAFORMAT_UINT32
	{ DATA_BASETYPE_UINT, false, 2, 0, 0, 4, 8  }, // DATAFORMAT_UINT32_VEC2
	{ DATA_BASETYPE_UINT, false, 3, 0, 0, 4, 12 }, // DATAFORMAT_UINT32_VEC3
	{ DATA_BASETYPE_UINT, false, 4, 0, 0, 4, 16 }, // DATAFORMAT_UINT32_VEC4

	{ DATA_BASETYPE_SNORM, false, 4, 0, 0, 1, 4 }, // DATAFORMAT_SNORM8_VEC4
	{ DATA_BASETYPE_UNORM, false, 4, 0, 0, 1, 4 }, // DATAFORMAT_UNORM8_VEC4
	{ DATA_BASETYPE_INT,   false, 4, 0, 0, 1, 4 }, // DATAFORMAT_INT8_VEC4
	{ DATA_BASETYPE_UINT,  false, 4, 0, 0, 1, 4 }, // DATAFORMAT_UINT8_VEC4

	{ DATA_BASETYPE_SNORM, false, 2, 0, 0, 2, 4 }, // DATAFORMAT_SNORM16_VEC2
	{ DATA_BASETYPE_SNORM, false, 4, 0, 0, 2, 8 }, // DATAFORMAT_SNORM16_VEC4

	{ DATA_BASETYPE_UNORM, false, 2, 0, 0, 2, 4 }, // DATAFORMAT_UNORM16_VEC2
	{ DATA_BASETYPE_UNORM, false, 4, 0, 0, 2, 8 }, // DATAFORMAT_UNORM16_VEC4

	{ DATA_BASETYPE_INT, false, 2, 0, 0, 2, 4 }, // DATAFORMAT_INT16_VEC2
	{ DATA_BASETYPE_INT, false, 4, 0, 0, 2, 8 }, // DATAFORMAT_INT16_VEC4

	{ DATA_BASETYPE_UINT, false, 1, 0, 0, 2, 2 }, // DATAFORMAT_UINT16
	{ DATA_BASETYPE_UINT, false, 2, 0, 0, 2, 4 }, // DATAFORMAT_UINT16_VEC2
	{ DATA_BASETYPE_UINT, false, 4, 0, 0, 2, 8 }, // DATAFORMAT_UINT16_VEC4

	{ DATA_BASETYPE_BOOL, false, 1, 0, 0, 4, 4  }, // DATAFORMAT_BOOL
	{ DATA_BASETYPE_BOOL, false, 2, 0, 0, 4, 8  }, // DATAFORMAT_BOOL_VEC2
	{ DATA_BASETYPE_BOOL, false, 3, 0, 0, 4, 12 }, // DATAFORMAT_BOOL_VEC3
	{ DATA_BASETYPE_BOOL, false, 4, 0, 0, 4, 16 }, // DATAFORMAT_BOOL_VEC4
};

static_assert((sizeof(dataFormatInfo) / sizeof(DataFormatInfo)) == DATAFORMAT_MAX_ENUM, "dataFormatInfo array size must match number of DataFormat enum values.");

const DataFormatInfo &getDataFormatInfo(DataFormat format)
{
	return dataFormatInfo[format];
}

size_t getIndexDataSize(IndexDataType type)
{
	switch (type)
	{
		case INDEX_UINT16: return sizeof(uint16);
		case INDEX_UINT32: return sizeof(uint32);
		default: return 0;
	}
}

IndexDataType getIndexDataTypeFromMax(size_t maxvalue)
{
	return maxvalue > LOVE_UINT16_MAX ? INDEX_UINT32 : INDEX_UINT16;
}

DataFormat getIndexDataFormat(IndexDataType type)
{
	return type == INDEX_UINT32 ? DATAFORMAT_UINT32 : DATAFORMAT_UINT16;
}

IndexDataType getIndexDataType(DataFormat format)
{
	switch (format)
	{
		case DATAFORMAT_UINT16: return INDEX_UINT16;
		case DATAFORMAT_UINT32: return INDEX_UINT32;
		default: return INDEX_MAX_ENUM;
	}
}

int getIndexCount(TriangleIndexMode mode, int vertexCount)
{
	switch (mode)
	{
	case TRIANGLEINDEX_NONE:
		return 0;
	case TRIANGLEINDEX_STRIP:
	case TRIANGLEINDEX_FAN:
		return 3 * (vertexCount - 2);
	case TRIANGLEINDEX_QUADS:
		return vertexCount * 6 / 4;
	}
	return 0;
}

template <typename T>
static void fillIndicesT(TriangleIndexMode mode, T vertexStart, T vertexCount, T *indices)
{
	switch (mode)
	{
	case TRIANGLEINDEX_NONE:
		break;
	case TRIANGLEINDEX_STRIP:
		{
			int i = 0;
			for (T index = 0; index < vertexCount - 2; index++)
			{
				indices[i++] = vertexStart + index;
				indices[i++] = vertexStart + index + 1 + (index & 1);
				indices[i++] = vertexStart + index + 2 - (index & 1);
			}
		}
		break;
	case TRIANGLEINDEX_FAN:
		{
			int i = 0;
			for (T index = 2; index < vertexCount; index++)
			{
				indices[i++] = vertexStart;
				indices[i++] = vertexStart + index - 1;
				indices[i++] = vertexStart + index;
			}
		}
		break;
	case TRIANGLEINDEX_QUADS:
		{
			// 0---2
			// | / |
			// 1---3
			int count = vertexCount / 4;
			for (int i = 0; i < count; i++)
			{
				int ii = i * 6;
				T vi = T(vertexStart + i * 4);

				indices[ii + 0] = vi + 0;
				indices[ii + 1] = vi + 1;
				indices[ii + 2] = vi + 2;

				indices[ii + 3] = vi + 2;
				indices[ii + 4] = vi + 1;
				indices[ii + 5] = vi + 3;
			}
		}
		break;
	}
}

void fillIndices(TriangleIndexMode mode, uint16 vertexStart, uint16 vertexCount, uint16 *indices)
{
	fillIndicesT(mode, vertexStart, vertexCount, indices);
}

void fillIndices(TriangleIndexMode mode, uint32 vertexStart, uint32 vertexCount, uint32 *indices)
{
	fillIndicesT(mode, vertexStart, vertexCount, indices);
}

void VertexAttributes::setCommonFormat(CommonFormat format, uint8 bufferindex)
{
	setBufferLayout(bufferindex, (uint16) getFormatStride(format));

	switch (format)
	{
	case CommonFormat::NONE:
		break;
	case CommonFormat::XYf:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		break;
	case CommonFormat::XYZf:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC3, 0, bufferindex);
		break;
	case CommonFormat::RGBAub:
		set(ATTRIB_COLOR, DATAFORMAT_UNORM8_VEC4, 0, bufferindex);
		break;
	case CommonFormat::STf_RGBAub:
		set(ATTRIB_TEXCOORD, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		set(ATTRIB_COLOR, DATAFORMAT_UNORM8_VEC4, uint16(sizeof(float) * 2), bufferindex);
		break;
	case CommonFormat::STPf_RGBAub:
		set(ATTRIB_TEXCOORD, DATAFORMAT_FLOAT_VEC3, 0, bufferindex);
		set(ATTRIB_COLOR, DATAFORMAT_UNORM8_VEC4, uint16(sizeof(float) * 3), bufferindex);
		break;
	case CommonFormat::XYf_STf:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATAFORMAT_FLOAT_VEC2, uint16(sizeof(float) * 2), bufferindex);
		break;
	case CommonFormat::XYf_STPf:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATAFORMAT_FLOAT_VEC3, uint16(sizeof(float) * 2), bufferindex);
		break;
	case CommonFormat::XYf_STf_RGBAub:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATAFORMAT_FLOAT_VEC2, uint16(sizeof(float) * 2), bufferindex);
		set(ATTRIB_COLOR, DATAFORMAT_UNORM8_VEC4, uint16(sizeof(float) * 4), bufferindex);
		break;
	case CommonFormat::XYf_STus_RGBAub:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATAFORMAT_UNORM16_VEC2, uint16(sizeof(float) * 2), bufferindex);
		set(ATTRIB_COLOR, DATAFORMAT_UNORM8_VEC4, uint16(sizeof(float) * 2 + sizeof(uint16) * 2), bufferindex);
		break;
	case CommonFormat::XYf_STPf_RGBAub:
		set(ATTRIB_POS, DATAFORMAT_FLOAT_VEC2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATAFORMAT_FLOAT_VEC3, uint16(sizeof(float) * 2), bufferindex);
		set(ATTRIB_COLOR, DATAFORMAT_UNORM8_VEC4, uint16(sizeof(float) * 5), bufferindex);
		break;
	case CommonFormat::COUNT:
		break;
	}
}

bool VertexAttributes::operator == (const VertexAttributes &other) const
{
	if (enableBits != other.enableBits || instanceBits != other.instanceBits)
		return false;

	uint32 allbits = enableBits;
	uint32 i = 0;

	while (allbits)
	{
		if (isEnabled(i))
		{
			const auto &a = attribs[i];
			const auto &b = other.attribs[i];
			if (a.bufferIndex != b.bufferIndex || a.packedFormat != b.packedFormat || a.offsetFromVertex != b.offsetFromVertex)
				return false;

			if (bufferLayouts[a.bufferIndex].stride != other.bufferLayouts[a.bufferIndex].stride)
				return false;
		}

		i++;
		allbits >>= 1;
	}

	return true;
}

STRINGMAP_BEGIN(BuiltinVertexAttribute, ATTRIB_MAX_ENUM, attribName)
{
	{ "VertexPosition", ATTRIB_POS           },
	{ "VertexTexCoord", ATTRIB_TEXCOORD      },
	{ "VertexColor",    ATTRIB_COLOR         },
}
STRINGMAP_END(BuiltinVertexAttribute, ATTRIB_MAX_ENUM, attribName)

const char *getConstant(BuiltinVertexAttribute attrib)
{
	const char *name = nullptr;
	getConstant(attrib, name);
	return name;
}

STRINGMAP_BEGIN(BufferUsage, BUFFERUSAGE_MAX_ENUM, bufferUsageName)
{
	{ "vertex",            BUFFERUSAGE_VERTEX             },
	{ "index",             BUFFERUSAGE_INDEX              },
	{ "texel",             BUFFERUSAGE_TEXEL              },
	{ "shaderstorage",     BUFFERUSAGE_SHADER_STORAGE     },
	{ "indirectarguments", BUFFERUSAGE_INDIRECT_ARGUMENTS },
}
STRINGMAP_END(BufferUsage, BUFFERUSAGE_MAX_ENUM, bufferUsageName)

STRINGMAP_BEGIN(IndexDataType, INDEX_MAX_ENUM, indexType)
{
	{ "uint16", INDEX_UINT16 },
	{ "uint32", INDEX_UINT32 },
}
STRINGMAP_END(IndexDataType, INDEX_MAX_ENUM, indexType)

STRINGMAP_BEGIN(BufferDataUsage, BUFFERDATAUSAGE_MAX_ENUM, bufferDataUsage)
{
	{ "stream",   BUFFERDATAUSAGE_STREAM   },
	{ "dynamic",  BUFFERDATAUSAGE_DYNAMIC  },
	{ "static",   BUFFERDATAUSAGE_STATIC   },
	{ "readback", BUFFERDATAUSAGE_READBACK },
}
STRINGMAP_END(BufferDataUsage, BUFFERDATAUSAGE_MAX_ENUM, bufferDataUsage)

STRINGMAP_BEGIN(PrimitiveType, PRIMITIVE_MAX_ENUM, primitiveType)
{
	{ "fan",       PRIMITIVE_TRIANGLE_FAN   },
	{ "strip",     PRIMITIVE_TRIANGLE_STRIP },
	{ "triangles", PRIMITIVE_TRIANGLES      },
	{ "points",    PRIMITIVE_POINTS         },
}
STRINGMAP_END(PrimitiveType, PRIMITIVE_MAX_ENUM, primitiveType)

STRINGMAP_BEGIN(AttributeStep, STEP_MAX_ENUM, attributeStep)
{
	{ "pervertex",   STEP_PER_VERTEX   },
	{ "perinstance", STEP_PER_INSTANCE },
}
STRINGMAP_END(AttributeStep, STEP_MAX_ENUM, attributeStep)

STRINGMAP_BEGIN(DataFormat, DATAFORMAT_MAX_ENUM, dataFormat)
{
	{ "float",     DATAFORMAT_FLOAT      },
	{ "floatvec2", DATAFORMAT_FLOAT_VEC2 },
	{ "floatvec3", DATAFORMAT_FLOAT_VEC3 },
	{ "floatvec4", DATAFORMAT_FLOAT_VEC4 },

	{ "floatmat2x2", DATAFORMAT_FLOAT_MAT2X2 },
	{ "floatmat2x3", DATAFORMAT_FLOAT_MAT2X3 },
	{ "floatmat2x4", DATAFORMAT_FLOAT_MAT2X4 },

	{ "floatmat3x2", DATAFORMAT_FLOAT_MAT3X2 },
	{ "floatmat3x3", DATAFORMAT_FLOAT_MAT3X3 },
	{ "floatmat3x4", DATAFORMAT_FLOAT_MAT3X4 },

	{ "floatmat4x2", DATAFORMAT_FLOAT_MAT4X2 },
	{ "floatmat4x3", DATAFORMAT_FLOAT_MAT4X3 },
	{ "floatmat4x4", DATAFORMAT_FLOAT_MAT4X4 },

	{ "int32",     DATAFORMAT_INT32      },
	{ "int32vec2", DATAFORMAT_INT32_VEC2 },
	{ "int32vec3", DATAFORMAT_INT32_VEC3 },
	{ "int32vec4", DATAFORMAT_INT32_VEC4 },

	{ "uint32",     DATAFORMAT_UINT32      },
	{ "uint32vec2", DATAFORMAT_UINT32_VEC2 },
	{ "uint32vec3", DATAFORMAT_UINT32_VEC3 },
	{ "uint32vec4", DATAFORMAT_UINT32_VEC4 },

	{ "snorm8vec4", DATAFORMAT_SNORM8_VEC4 },
	{ "unorm8vec4", DATAFORMAT_UNORM8_VEC4 },
	{ "int8vec4",   DATAFORMAT_INT8_VEC4   },
	{ "uint8vec4",  DATAFORMAT_UINT8_VEC4  },

	{ "snorm16vec2", DATAFORMAT_SNORM16_VEC2 },
	{ "snorm16vec4", DATAFORMAT_SNORM16_VEC4 },

	{ "unorm16vec2", DATAFORMAT_UNORM16_VEC2 },
	{ "unorm16vec4", DATAFORMAT_UNORM16_VEC4 },

	{ "int16vec2", DATAFORMAT_INT16_VEC2 },
	{ "int16vec4", DATAFORMAT_INT16_VEC4 },

	{ "uint16",     DATAFORMAT_UINT16      },
	{ "uint16vec2", DATAFORMAT_UINT16_VEC2 },
	{ "uint16vec4", DATAFORMAT_UINT16_VEC4 },

	{ "bool",     DATAFORMAT_BOOL      },
	{ "boolvec2", DATAFORMAT_BOOL_VEC2 },
	{ "boolvec3", DATAFORMAT_BOOL_VEC3 },
	{ "boolvec4", DATAFORMAT_BOOL_VEC4 },
}
STRINGMAP_END(DataFormat, DATAFORMAT_MAX_ENUM, dataFormat)

STRINGMAP_BEGIN(DataBaseType, DATA_BASETYPE_MAX_ENUM, dataBaseType)
{
	{ "float", DATA_BASETYPE_FLOAT },
	{ "int",   DATA_BASETYPE_INT   },
	{ "uint",  DATA_BASETYPE_UINT  },
	{ "snorm", DATA_BASETYPE_SNORM },
	{ "unorm", DATA_BASETYPE_UNORM },
	{ "bool",  DATA_BASETYPE_BOOL  },
}
STRINGMAP_END(DataBaseType, DATA_BASETYPE_MAX_ENUM, dataBaseType)

STRINGMAP_BEGIN(CullMode, CULL_MAX_ENUM, cullMode)
{
	{ "none",  CULL_NONE  },
	{ "back",  CULL_BACK  },
	{ "front", CULL_FRONT },
}
STRINGMAP_END(CullMode, CULL_MAX_ENUM, cullMode)

STRINGMAP_BEGIN(Winding, WINDING_MAX_ENUM, winding)
{
	{ "cw",  WINDING_CW  },
	{ "ccw", WINDING_CCW },
}
STRINGMAP_END(Winding, WINDING_MAX_ENUM, winding)

} // graphics
} // love
