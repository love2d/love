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

#include "vertex.h"
#include "common/StringMap.h"

namespace love
{
namespace graphics
{
namespace vertex
{

static_assert(sizeof(Color) == 4, "sizeof(Color) incorrect!");
static_assert(sizeof(Vertex) == sizeof(float)*2 + sizeof(float)*2 + sizeof(Color), "sizeof(Vertex) incorrect!");
static_assert(sizeof(XYf_STf) == sizeof(float)*2 + sizeof(float)*2, "sizeof(XYf_STf) incorrect!");
static_assert(sizeof(XYf_STf_RGBAub) == sizeof(float)*2 + sizeof(float)*2 + sizeof(Color), "sizeof(XYf_STf_RGBAub) incorrect!");
static_assert(sizeof(XYf_STus_RGBAub) == sizeof(float)*2 + sizeof(uint16)*2 + sizeof(Color), "sizeof(XYf_STus_RGBAub) incorrect!");

size_t getFormatStride(CommonFormat format)
{
	switch (format)
	{
	case CommonFormat::NONE:
		return 0;
	case CommonFormat::XYf:
		return sizeof(float) * 2;
	case CommonFormat::RGBAub:
		return sizeof(uint8) * 4;
	case CommonFormat::XYf_STf:
		return sizeof(XYf_STf);
	case CommonFormat::XYf_STf_RGBAub:
		return sizeof(XYf_STf_RGBAub);
	case CommonFormat::XYf_STus_RGBAub:
		return sizeof(XYf_STus_RGBAub);
	}
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

static StringMap<Usage, USAGE_MAX_ENUM>::Entry usageEntries[] =
{
	{ "stream",  USAGE_STREAM  },
	{ "dynamic", USAGE_DYNAMIC },
	{ "static",  USAGE_STATIC  },
};

static StringMap<Usage, USAGE_MAX_ENUM> usages(usageEntries, sizeof(usageEntries));

bool getConstant(const char *in, Usage &out)
{
	return usages.find(in, out);
}

bool getConstant(Usage in, const char *&out)
{
	return usages.find(in, out);
}

} // vertex
} // graphics
} // love
