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

#include "vertex.h"
#include "common/StringMap.h"

namespace love
{
namespace graphics
{
namespace vertex
{

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
	case CommonFormat::NONE:
		return 0;
	case CommonFormat::XYf:
		return sizeof(float) * 2;
	case CommonFormat::XYZf:
		return sizeof(float) * 3;
	case CommonFormat::RGBAub:
		return sizeof(uint8) * 4;
	case CommonFormat::STf_RGBAub:
		return sizeof(STf_RGBAub);
	case CommonFormat::STPf_RGBAub:
		return sizeof(STPf_RGBAub);
	case CommonFormat::XYf_STf:
		return sizeof(XYf_STf);
	case CommonFormat::XYf_STPf:
		return sizeof(XYf_STPf);
	case CommonFormat::XYf_STf_RGBAub:
		return sizeof(XYf_STf_RGBAub);
	case CommonFormat::XYf_STus_RGBAub:
		return sizeof(XYf_STus_RGBAub);
	case CommonFormat::XYf_STPf_RGBAub:
		return sizeof(XYf_STPf_RGBAub);
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
	}
	return 0;
}

size_t getIndexDataSize(IndexDataType type)
{
	switch (type)
	{
	case INDEX_UINT16:
		return sizeof(uint16);
	case INDEX_UINT32:
		return sizeof(uint32);
	default:
		return 0;
	}
}

size_t getDataTypeSize(DataType datatype)
{
	switch (datatype)
	{
	case DATA_UNORM8:
		return sizeof(uint8);
	case DATA_UNORM16:
		return sizeof(uint16);
	case DATA_FLOAT:
		return sizeof(float);
	default:
		return 0;
	}
}

IndexDataType getIndexDataTypeFromMax(size_t maxvalue)
{
	IndexDataType types[] = {INDEX_UINT16, INDEX_UINT32};
	return types[maxvalue > LOVE_UINT16_MAX ? 1 : 0];
}

int getIndexCount(TriangleIndexMode mode, int vertexCount)
{
	switch (mode)
	{
	case TriangleIndexMode::NONE:
		return 0;
	case TriangleIndexMode::STRIP:
	case TriangleIndexMode::FAN:
		return 3 * (vertexCount - 2);
	case TriangleIndexMode::QUADS:
		return vertexCount * 6 / 4;
	}
	return 0;
}

template <typename T>
static void fillIndicesT(TriangleIndexMode mode, T vertexStart, T vertexCount, T *indices)
{
	switch (mode)
	{
	case TriangleIndexMode::NONE:
		break;
	case TriangleIndexMode::STRIP:
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
	case TriangleIndexMode::FAN:
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
	case TriangleIndexMode::QUADS:
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

void Attributes::setCommonFormat(CommonFormat format, uint8 bufferindex)
{
	setBufferLayout(bufferindex, (uint16) getFormatStride(format));

	switch (format)
	{
	case CommonFormat::NONE:
		break;
	case CommonFormat::XYf:
		set(ATTRIB_POS, DATA_FLOAT, 2, 0, bufferindex);
		break;
	case CommonFormat::XYZf:
		set(ATTRIB_POS, DATA_FLOAT, 3, 0, bufferindex);
		break;
	case CommonFormat::RGBAub:
		set(ATTRIB_COLOR, DATA_UNORM8, 4, 0, bufferindex);
		break;
	case CommonFormat::STf_RGBAub:
		set(ATTRIB_TEXCOORD, DATA_FLOAT, 2, 0, bufferindex);
		set(ATTRIB_COLOR, DATA_UNORM8, 4, uint16(sizeof(float) * 2), bufferindex);
		break;
	case CommonFormat::STPf_RGBAub:
		set(ATTRIB_TEXCOORD, DATA_FLOAT, 3, 0, bufferindex);
		set(ATTRIB_COLOR, DATA_UNORM8, 4, uint16(sizeof(float) * 3), bufferindex);
		break;
	case CommonFormat::XYf_STf:
		set(ATTRIB_POS, DATA_FLOAT, 2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATA_FLOAT, 2, uint16(sizeof(float) * 2), bufferindex);
		break;
	case CommonFormat::XYf_STPf:
		set(ATTRIB_POS, DATA_FLOAT, 2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATA_FLOAT, 3, uint16(sizeof(float) * 2), bufferindex);
		break;
	case CommonFormat::XYf_STf_RGBAub:
		set(ATTRIB_POS, DATA_FLOAT, 2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATA_FLOAT, 2, uint16(sizeof(float) * 2), bufferindex);
		set(ATTRIB_COLOR, DATA_UNORM8, 4, uint16(sizeof(float) * 4), bufferindex);
		break;
	case CommonFormat::XYf_STus_RGBAub:
		set(ATTRIB_POS, DATA_FLOAT, 2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATA_UNORM16, 2, uint16(sizeof(float) * 2), bufferindex);
		set(ATTRIB_COLOR, DATA_UNORM8, 4, uint16(sizeof(float) * 2 + sizeof(uint16) * 2), bufferindex);
		break;
	case CommonFormat::XYf_STPf_RGBAub:
		set(ATTRIB_POS, DATA_FLOAT, 2, 0, bufferindex);
		set(ATTRIB_TEXCOORD, DATA_FLOAT, 3, uint16(sizeof(float) * 2), bufferindex);
		set(ATTRIB_COLOR, DATA_UNORM8, 4, uint16(sizeof(float) * 5), bufferindex);
		break;
	}
}

static StringMap<BuiltinVertexAttribute, ATTRIB_MAX_ENUM>::Entry attribNameEntries[] =
{
	{ "VertexPosition", ATTRIB_POS           },
	{ "VertexTexCoord", ATTRIB_TEXCOORD      },
	{ "VertexColor",    ATTRIB_COLOR         },
	{ "ConstantColor",  ATTRIB_CONSTANTCOLOR },
};

static StringMap<BuiltinVertexAttribute, ATTRIB_MAX_ENUM> attribNames(attribNameEntries, sizeof(attribNameEntries));

static StringMap<IndexDataType, INDEX_MAX_ENUM>::Entry indexTypeEntries[] =
{
	{ "uint16", INDEX_UINT16 },
	{ "uint32", INDEX_UINT32 },
};

static StringMap<IndexDataType, INDEX_MAX_ENUM> indexTypes(indexTypeEntries, sizeof(indexTypeEntries));

static StringMap<Usage, USAGE_MAX_ENUM>::Entry usageEntries[] =
{
	{ "stream",  USAGE_STREAM  },
	{ "dynamic", USAGE_DYNAMIC },
	{ "static",  USAGE_STATIC  },
};

static StringMap<Usage, USAGE_MAX_ENUM> usages(usageEntries, sizeof(usageEntries));

static StringMap<PrimitiveType, PRIMITIVE_MAX_ENUM>::Entry primitiveTypeEntries[] =
{
	{ "fan",       PRIMITIVE_TRIANGLE_FAN   },
	{ "strip",     PRIMITIVE_TRIANGLE_STRIP },
	{ "triangles", PRIMITIVE_TRIANGLES      },
	{ "points",    PRIMITIVE_POINTS         },
};

static StringMap<PrimitiveType, PRIMITIVE_MAX_ENUM> primitiveTypes(primitiveTypeEntries, sizeof(primitiveTypeEntries));

static StringMap<AttributeStep, STEP_MAX_ENUM>::Entry attributeStepEntries[] =
{
	{ "pervertex",   STEP_PER_VERTEX   },
	{ "perinstance", STEP_PER_INSTANCE },
};

static StringMap<AttributeStep, STEP_MAX_ENUM> attributeSteps(attributeStepEntries, sizeof(attributeStepEntries));

static StringMap<DataType, DATA_MAX_ENUM>::Entry dataTypeEntries[] =
{
	{ "byte",    DATA_UNORM8  }, // Legacy / more user-friendly name...
	{ "unorm16", DATA_UNORM16 },
	{ "float",   DATA_FLOAT   },
};

static StringMap<DataType, DATA_MAX_ENUM> dataTypes(dataTypeEntries, sizeof(dataTypeEntries));

static StringMap<CullMode, CULL_MAX_ENUM>::Entry cullModeEntries[] =
{
	{ "none",  CULL_NONE  },
	{ "back",  CULL_BACK  },
	{ "front", CULL_FRONT },
};

static StringMap<CullMode, CULL_MAX_ENUM> cullModes(cullModeEntries, sizeof(cullModeEntries));

static StringMap<Winding, WINDING_MAX_ENUM>::Entry windingEntries[] =
{
	{ "cw",  WINDING_CW  },
	{ "ccw", WINDING_CCW },
};

static StringMap<Winding, WINDING_MAX_ENUM> windings(windingEntries, sizeof(windingEntries));

bool getConstant(const char *in, BuiltinVertexAttribute &out)
{
	return attribNames.find(in, out);
}

bool getConstant(BuiltinVertexAttribute in, const char *&out)
{
	return attribNames.find(in, out);
}

bool getConstant(const char *in, IndexDataType &out)
{
	return indexTypes.find(in, out);
}

bool getConstant(IndexDataType in, const char *&out)
{
	return indexTypes.find(in, out);
}

std::vector<std::string> getConstants(IndexDataType)
{
	return indexTypes.getNames();
}

bool getConstant(const char *in, Usage &out)
{
	return usages.find(in, out);
}

bool getConstant(Usage in, const char *&out)
{
	return usages.find(in, out);
}

std::vector<std::string> getConstants(Usage)
{
	return usages.getNames();
}

bool getConstant(const char *in, PrimitiveType &out)
{
	return primitiveTypes.find(in, out);
}

bool getConstant(PrimitiveType in, const char *&out)
{
	return primitiveTypes.find(in, out);
}

std::vector<std::string> getConstants(PrimitiveType)
{
	return primitiveTypes.getNames();
}

bool getConstant(const char *in, AttributeStep &out)
{
	return attributeSteps.find(in, out);
}

bool getConstant(AttributeStep in, const char *&out)
{
	return attributeSteps.find(in, out);
}

std::vector<std::string> getConstants(AttributeStep)
{
	return attributeSteps.getNames();
}

bool getConstant(const char *in, DataType &out)
{
	return dataTypes.find(in, out);
}

bool getConstant(DataType in, const char *&out)
{
	return dataTypes.find(in, out);
}

std::vector<std::string> getConstants(DataType)
{
	return dataTypes.getNames();
}

bool getConstant(const char *in, CullMode &out)
{
	return cullModes.find(in, out);
}

bool getConstant(CullMode in, const char *&out)
{
	return cullModes.find(in, out);
}

std::vector<std::string> getConstants(CullMode)
{
	return cullModes.getNames();
}

bool getConstant(const char *in, Winding &out)
{
	return windings.find(in, out);
}

bool getConstant(Winding in, const char *&out)
{
	return windings.find(in, out);
}

std::vector<std::string> getConstants(Winding)
{
	return windings.getNames();
}

} // vertex
} // graphics
} // love
