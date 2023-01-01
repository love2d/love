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

#pragma once

// LOVE
#include "common/int.h"
#include "common/Color.h"

// C
#include <stddef.h>
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
	ATTRIB_CONSTANTCOLOR,
	ATTRIB_MAX_ENUM
};

enum BuiltinVertexAttributeFlag
{
	ATTRIBFLAG_POS = 1 << ATTRIB_POS,
	ATTRIBFLAG_TEXCOORD = 1 << ATTRIB_TEXCOORD,
	ATTRIBFLAG_COLOR = 1 << ATTRIB_COLOR,
	ATTRIBFLAG_CONSTANTCOLOR = 1 << ATTRIB_CONSTANTCOLOR
};

enum BufferType
{
	BUFFER_VERTEX = 0,
	BUFFER_INDEX,
	BUFFER_MAX_ENUM
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

namespace vertex
{

// The expected usage pattern of vertex data.
enum Usage
{
	USAGE_STREAM,
	USAGE_DYNAMIC,
	USAGE_STATIC,
	USAGE_MAX_ENUM
};

enum DataType
{
	DATA_UNORM8,
	DATA_UNORM16,
	DATA_FLOAT,
	DATA_MAX_ENUM
};

enum Winding
{
	WINDING_CW,
	WINDING_CCW,
	WINDING_MAX_ENUM
};

enum class TriangleIndexMode
{
	NONE,
	STRIP,
	FAN,
	QUADS,
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

struct AttributeInfo
{
	uint8 bufferIndex;
	DataType type : 4;
	uint8 components : 4;
	uint16 offsetFromVertex;
};

struct BufferLayout
{
	uint16 stride;
};

struct Attributes
{
	static const uint32 MAX = 32;

	uint32 enableBits = 0; // indexed by attribute
	uint32 instanceBits = 0; // indexed by buffer

	AttributeInfo attribs[MAX];
	BufferLayout bufferLayouts[BufferBindings::MAX];

	Attributes() {}
	Attributes(CommonFormat format, uint8 bufferindex)
	{
		setCommonFormat(format, bufferindex);
	}

	void set(uint32 index, DataType type, uint8 components, uint16 offsetfromvertex, uint8 bufferindex)
	{
		enableBits |= (1u << index);

		attribs[index].bufferIndex = bufferindex;
		attribs[index].type = type;
		attribs[index].components = components;
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
};

size_t getFormatStride(CommonFormat format);

uint32 getFormatFlags(CommonFormat format);

int getFormatPositionComponents(CommonFormat format);

inline CommonFormat getSinglePositionFormat(bool is2D)
{
	return is2D ? CommonFormat::XYf : CommonFormat::XYZf;
}

size_t getIndexDataSize(IndexDataType type);
size_t getDataTypeSize(DataType datatype);

IndexDataType getIndexDataTypeFromMax(size_t maxvalue);

int getIndexCount(TriangleIndexMode mode, int vertexCount);

void fillIndices(TriangleIndexMode mode, uint16 vertexStart, uint16 vertexCount, uint16 *indices);
void fillIndices(TriangleIndexMode mode, uint32 vertexStart, uint32 vertexCount, uint32 *indices);

bool getConstant(const char *in, BuiltinVertexAttribute &out);
bool getConstant(BuiltinVertexAttribute in, const char *&out);

bool getConstant(const char *in, IndexDataType &out);
bool getConstant(IndexDataType in, const char *&out);
std::vector<std::string> getConstants(IndexDataType);

bool getConstant(const char *in, Usage &out);
bool getConstant(Usage in, const char *&out);
std::vector<std::string> getConstants(Usage);

bool getConstant(const char *in, PrimitiveType &out);
bool getConstant(PrimitiveType in, const char *&out);
std::vector<std::string> getConstants(PrimitiveType);

bool getConstant(const char *in, AttributeStep &out);
bool getConstant(AttributeStep in, const char *&out);
std::vector<std::string> getConstants(AttributeStep);

bool getConstant(const char *in, DataType &out);
bool getConstant(DataType in, const char *&out);
std::vector<std::string> getConstants(DataType);

bool getConstant(const char *in, CullMode &out);
bool getConstant(CullMode in, const char *&out);
std::vector<std::string> getConstants(CullMode);

bool getConstant(const char *in, Winding &out);
bool getConstant(Winding in, const char *&out);
std::vector<std::string> getConstants(Winding);

} // vertex

typedef vertex::XYf_STf_RGBAub Vertex;

} // graphics
} // love
