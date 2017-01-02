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
#include "Color.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

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

enum class PrimitiveMode
{
	TRIANGLES,
	POINTS,
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
	RGBAub,
	XYf_STf,
	XYf_STf_RGBAub,
	XYf_STus_RGBAub,
};

struct XYf_STf
{
	float x, y;
	float s, t;
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

size_t getFormatStride(CommonFormat format);

int getIndexCount(TriangleIndexMode mode, int vertexCount);

void fillIndices(TriangleIndexMode mode, uint16 vertexStart, uint16 vertexCount, uint16 *indices);
void fillIndices(TriangleIndexMode mode, uint32 vertexStart, uint32 vertexCount, uint32 *indices);

bool getConstant(const char *in, Usage &out);
bool getConstant(Usage in, const char *&out);

} // vertex

typedef vertex::XYf_STf_RGBAub Vertex;

} // graphics
} // love
