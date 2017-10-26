/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

// Vertex attribute indices used in shaders by LOVE. The values map to OpenGL
// generic vertex attribute indices.
enum VertexAttribID
{
	ATTRIB_POS = 0,
	ATTRIB_TEXCOORD,
	ATTRIB_COLOR,
	ATTRIB_CONSTANTCOLOR,
	ATTRIB_MAX_ENUM
};

enum VertexAttribFlags
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
	Color color;
};

struct STPf_RGBAub
{
	float s, t, p;
	Color color;
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
	Color color;
};

struct XYf_STus_RGBAub
{
	float  x, y;
	uint16 s, t;
	Color  color;
};

struct XYf_STPf_RGBAub
{
	float x, y;
	float s, t, p;
	Color color;
};

struct Buffers
{
	static const unsigned int MAX = 32;

	uint32 usebits = 0;

	struct
	{
		Resource *buffer;
		size_t offset;
	} info[MAX];

	void set(unsigned int index, Resource *r, size_t offset)
	{
		usebits |= (1u << index);
		info[index] = {r, offset};
	}

	void disable(unsigned int index) { usebits &= (1u << index); }
	void clear() { usebits = 0; }
};

struct AttributeInfo
{
	uint8 bufferindex;
	DataType type : 8;
	uint16 components;
	uint16 offsetfromvertex;
	uint16 stride;
};

struct Attributes
{
	static const unsigned int MAX = 32;

	uint32 enablebits = 0;
	uint32 instancebits = 0;

	union
	{
		AttributeInfo attribs[MAX];
		uint64 data[MAX];
	};

	Attributes() {}
	Attributes(CommonFormat format, uint8 bufferindex)
	{
		setCommonFormat(format, bufferindex);
	}

	void set(unsigned int index, DataType type, uint16 components, uint16 offsetfromvertex, uint16 stride, uint8 bufferindex, AttributeStep step = STEP_PER_VERTEX)
	{
		uint32 bit = (1u << index);

		enablebits |= bit;

		if (step == STEP_PER_INSTANCE)
			instancebits |= bit;
		else
			instancebits &= ~bit;

		attribs[index].bufferindex = bufferindex;
		attribs[index].type = type;
		attribs[index].components = components;
		attribs[index].offsetfromvertex = offsetfromvertex;
		attribs[index].stride = stride;
	}

	void disable(unsigned int index)
	{
		enablebits &= ~(1u << index);
	}

	void clear()
	{
		enablebits = 0;
	}

	bool isEnabled(unsigned int index) const
	{
		return (enablebits & (1u << index)) != 0;
	}

	AttributeStep getStep(unsigned int index) const
	{
		return (instancebits & (1u << index)) != 0 ? STEP_PER_INSTANCE : STEP_PER_VERTEX;
	}

	void setCommonFormat(CommonFormat format, uint8 bufferindex);

	bool operator == (const Attributes &other) const
	{
		if (enablebits != other.enablebits)
			return false;

		uint32 instancediff = instancebits ^ other.instancebits;

		for (unsigned int i = 0; i < MAX; i++)
		{
			if (isEnabled(i))
			{
				if (instancediff & (1u << i))
					return false;

				if (data[i] != other.data[i])
					return false;
			}
		}

		return true;
	}
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

bool getConstant(const char *in, VertexAttribID &out);
bool getConstant(VertexAttribID in, const char *&out);

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

} // vertex

typedef vertex::XYf_STf_RGBAub Vertex;

} // graphics
} // love
