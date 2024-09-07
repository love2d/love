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

#pragma once

// LOVE
#include "common/int.h"
#include "common/Color.h"
#include "common/StringMap.h"

// C
#include <stddef.h>
#include <string.h>
#include <vector>
#include <string>

namespace love
{
namespace graphics
{

class Resource;

// Vertex attribute indices used in shaders by LOVE. The values map to GPU
// generic vertex attribute indices.
enum BuiltinVertexAttribute
{
	ATTRIB_POS = 0,
	ATTRIB_TEXCOORD,
	ATTRIB_COLOR,
	ATTRIB_MAX_ENUM
};

enum BuiltinVertexAttributeFlags
{
	ATTRIBFLAG_POS = 1 << ATTRIB_POS,
	ATTRIBFLAG_TEXCOORD = 1 << ATTRIB_TEXCOORD,
	ATTRIBFLAG_COLOR = 1 << ATTRIB_COLOR,
};

enum BufferUsage
{
	BUFFERUSAGE_VERTEX = 0,
	BUFFERUSAGE_INDEX,
	BUFFERUSAGE_TEXEL,
	BUFFERUSAGE_UNIFORM,
	BUFFERUSAGE_SHADER_STORAGE,
	BUFFERUSAGE_INDIRECT_ARGUMENTS,
	BUFFERUSAGE_MAX_ENUM
};

enum BufferUsageFlags
{
	BUFFERUSAGEFLAG_NONE = 0,
	BUFFERUSAGEFLAG_VERTEX = 1 << BUFFERUSAGE_VERTEX,
	BUFFERUSAGEFLAG_INDEX = 1 << BUFFERUSAGE_INDEX,
	BUFFERUSAGEFLAG_TEXEL = 1 << BUFFERUSAGE_TEXEL,
	BUFFERUSAGEFLAG_SHADER_STORAGE = 1 << BUFFERUSAGE_SHADER_STORAGE,
	BUFFERUSAGEFLAG_INDIRECT_ARGUMENTS = 1 << BUFFERUSAGE_INDIRECT_ARGUMENTS,
};

enum IndexDataType
{
	INDEX_UINT16,
	INDEX_UINT32,
	INDEX_MAX_ENUM
};

// http://escience.anu.edu.au/lecture/cg/surfaceModeling/image/surfaceModeling015.png
enum PrimitiveType
{
	PRIMITIVE_TRIANGLES,
	PRIMITIVE_TRIANGLE_STRIP,
	PRIMITIVE_TRIANGLE_FAN,
	PRIMITIVE_POINTS,
	PRIMITIVE_MAX_ENUM
};

enum AttributeStep
{
	STEP_PER_VERTEX,
	STEP_PER_INSTANCE,
	STEP_MAX_ENUM
};

enum CullMode
{
	CULL_NONE,
	CULL_BACK,
	CULL_FRONT,
	CULL_MAX_ENUM
};

// The expected usage pattern of buffer data.
enum BufferDataUsage
{
	BUFFERDATAUSAGE_STREAM,
	BUFFERDATAUSAGE_DYNAMIC,
	BUFFERDATAUSAGE_STATIC,
	BUFFERDATAUSAGE_READBACK,
	BUFFERDATAUSAGE_MAX_ENUM
};

// Value types used when interfacing with the GPU (vertex and shader data).
// The order of this enum affects the dataFormatInfo array.
enum DataFormat
{
	DATAFORMAT_FLOAT,
	DATAFORMAT_FLOAT_VEC2,
	DATAFORMAT_FLOAT_VEC3,
	DATAFORMAT_FLOAT_VEC4,

	DATAFORMAT_FLOAT_MAT2X2,
	DATAFORMAT_FLOAT_MAT2X3,
	DATAFORMAT_FLOAT_MAT2X4,

	DATAFORMAT_FLOAT_MAT3X2,
	DATAFORMAT_FLOAT_MAT3X3,
	DATAFORMAT_FLOAT_MAT3X4,

	DATAFORMAT_FLOAT_MAT4X2,
	DATAFORMAT_FLOAT_MAT4X3,
	DATAFORMAT_FLOAT_MAT4X4,

	DATAFORMAT_INT32,
	DATAFORMAT_INT32_VEC2,
	DATAFORMAT_INT32_VEC3,
	DATAFORMAT_INT32_VEC4,

	DATAFORMAT_UINT32,
	DATAFORMAT_UINT32_VEC2,
	DATAFORMAT_UINT32_VEC3,
	DATAFORMAT_UINT32_VEC4,

	DATAFORMAT_SNORM8_VEC4,
	DATAFORMAT_UNORM8_VEC4,
	DATAFORMAT_INT8_VEC4,
	DATAFORMAT_UINT8_VEC4,

	DATAFORMAT_SNORM16_VEC2,
	DATAFORMAT_SNORM16_VEC4,

	DATAFORMAT_UNORM16_VEC2,
	DATAFORMAT_UNORM16_VEC4,

	DATAFORMAT_INT16_VEC2,
	DATAFORMAT_INT16_VEC4,

	DATAFORMAT_UINT16,
	DATAFORMAT_UINT16_VEC2,
	DATAFORMAT_UINT16_VEC4,

	DATAFORMAT_BOOL,
	DATAFORMAT_BOOL_VEC2,
	DATAFORMAT_BOOL_VEC3,
	DATAFORMAT_BOOL_VEC4,

	DATAFORMAT_MAX_ENUM
};

enum DataBaseType
{
	DATA_BASETYPE_FLOAT,
	DATA_BASETYPE_INT,
	DATA_BASETYPE_UINT,
	DATA_BASETYPE_SNORM,
	DATA_BASETYPE_UNORM,
	DATA_BASETYPE_BOOL,
	DATA_BASETYPE_MAX_ENUM
};

enum Winding
{
	WINDING_CW,
	WINDING_CCW,
	WINDING_MAX_ENUM
};

enum TriangleIndexMode
{
	TRIANGLEINDEX_NONE,
	TRIANGLEINDEX_STRIP,
	TRIANGLEINDEX_FAN,
	TRIANGLEINDEX_QUADS,
};

enum class CommonFormat
{
	NONE,
	XYf,
	XYZf,
	RGBAub,
	STf_RGBAub,
	STPf_RGBAub,
	XYf_STf,
	XYf_STPf,
	XYf_STf_RGBAub,
	XYf_STus_RGBAub,
	XYf_STPf_RGBAub,
	COUNT,
};

struct DataFormatInfo
{
	DataBaseType baseType;
	bool isMatrix;
	int components;
	int matrixRows;
	int matrixColumns;
	size_t componentSize;
	size_t size;
};

struct STf_RGBAub
{
	float s, t;
	Color32 color;
};

struct STPf_RGBAub
{
	float s, t, p;
	Color32 color;
};

struct XYf_STf
{
	float x, y;
	float s, t;
};

struct XYf_STPf
{
	float x, y;
	float s, t, p;
};

struct XYf_STf_RGBAub
{
	float x, y;
	float s, t;
	Color32 color;
};

typedef XYf_STf_RGBAub Vertex;

struct XYf_STus_RGBAub
{
	float  x, y;
	uint16 s, t;
	Color32 color;
};

struct XYf_STPf_RGBAub
{
	float x, y;
	float s, t, p;
	Color32 color;
};

struct BufferBindings
{
	static const uint32 MAX = 32;

	uint32 useBits = 0;

	struct
	{
		Resource *buffer;
		size_t offset;
	} info[MAX];

	void set(uint32 index, Resource *r, size_t offset)
	{
		useBits |= (1u << index);
		info[index] = {r, offset};
	}

	void disable(uint32 index) { useBits &= (1u << index); }
	void clear() { useBits = 0; }
};

struct VertexAttributeInfo
{
	uint16 offsetFromVertex;
	uint8 packedFormat;
	uint8 bufferIndex;

	void setFormat(DataFormat format) { packedFormat = (uint8)format; }
	DataFormat getFormat() const { return (DataFormat)packedFormat; }
};

struct VertexBufferLayout
{
	// Attribute step rate is stored outside this struct as a bitmask.
	uint16 stride;
};

struct VertexAttributes
{
	static const uint32 MAX = 30;

	uint32 enableBits = 0; // indexed by attribute
	uint32 instanceBits = 0; // indexed by buffer

	VertexAttributeInfo attribs[MAX];
	VertexBufferLayout bufferLayouts[BufferBindings::MAX];

	VertexAttributes()
	{
		memset(this, 0, sizeof(VertexAttributes));
	}

	VertexAttributes(CommonFormat format, uint8 bufferindex)
	{
		memset(this, 0, sizeof(VertexAttributes));
		setCommonFormat(format, bufferindex);
	}

	void set(uint32 index, DataFormat format, uint16 offsetfromvertex, uint8 bufferindex)
	{
		enableBits |= (1u << index);

		attribs[index].bufferIndex = bufferindex;
		attribs[index].setFormat(format);
		attribs[index].offsetFromVertex = offsetfromvertex;
	}

	void setBufferLayout(uint32 bufferindex, uint16 stride, AttributeStep step = STEP_PER_VERTEX)
	{
		uint32 bufferbit = (1u << bufferindex);

		if (step == STEP_PER_INSTANCE)
			instanceBits |= bufferbit;
		else
			instanceBits &= ~bufferbit;

		bufferLayouts[bufferindex].stride = stride;
	}

	void disable(uint32 index)
	{
		enableBits &= ~(1u << index);
	}

	void clear()
	{
		enableBits = 0;
	}

	bool isEnabled(uint32 index) const
	{
		return (enableBits & (1u << index)) != 0;
	}

	AttributeStep getBufferStep(uint32 index) const
	{
		return (instanceBits & (1u << index)) != 0 ? STEP_PER_INSTANCE : STEP_PER_VERTEX;
	}

	void setCommonFormat(CommonFormat format, uint8 bufferindex);

	bool operator == (const VertexAttributes &other) const;
};

struct VertexAttributesID
{
	int id = 0;

	bool isValid() const { return id > 0; }
	void invalidate() { id = 0; }

	bool operator == (VertexAttributesID other) const { return other.id == id; }
	bool operator != (VertexAttributesID other) const { return other.id != id; }
};

size_t getFormatStride(CommonFormat format);

uint32 getFormatFlags(CommonFormat format);

int getFormatPositionComponents(CommonFormat format);

inline CommonFormat getSinglePositionFormat(bool is2D)
{
	return is2D ? CommonFormat::XYf : CommonFormat::XYZf;
}

const DataFormatInfo &getDataFormatInfo(DataFormat format);

size_t getIndexDataSize(IndexDataType type);
IndexDataType getIndexDataTypeFromMax(size_t maxvalue);
DataFormat getIndexDataFormat(IndexDataType type);
IndexDataType getIndexDataType(DataFormat format);

int getIndexCount(TriangleIndexMode mode, int vertexCount);

void fillIndices(TriangleIndexMode mode, uint16 vertexStart, uint16 vertexCount, uint16 *indices);
void fillIndices(TriangleIndexMode mode, uint32 vertexStart, uint32 vertexCount, uint32 *indices);

STRINGMAP_DECLARE(BuiltinVertexAttribute);
STRINGMAP_DECLARE(BufferUsage);
STRINGMAP_DECLARE(IndexDataType);
STRINGMAP_DECLARE(BufferDataUsage);
STRINGMAP_DECLARE(PrimitiveType);
STRINGMAP_DECLARE(AttributeStep);
STRINGMAP_DECLARE(DataFormat);
STRINGMAP_DECLARE(DataBaseType);
STRINGMAP_DECLARE(CullMode);
STRINGMAP_DECLARE(Winding);

const char *getConstant(BuiltinVertexAttribute attrib);

} // graphics
} // love
