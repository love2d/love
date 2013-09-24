/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "Geometry.h"
#include "common/Exception.h"
#include "common/Vector.h"
#include "modules/math/MathModule.h"

using love::math::Math;

// STD
#include <limits>
#include <algorithm> // for std::swap()
#include <cstring> // For memcpy

namespace love
{
namespace graphics
{

Geometry::Geometry(const std::vector<Vertex> &polygon, const std::vector<uint16> &elements, Geometry::DrawMode mode)
	: vertexArray(NULL)
	, vertexCount(polygon.size())
	, elementArray(NULL)
	, elementCount(elements.size())
	, vertexColors(false)
	, drawMode(mode)
{
	if (polygon.size() < 3)
		throw love::Exception("At least 3 vertices are needed to create a Geometry.");

	for (size_t i = 0; i < elementCount; i++)
	{
		// All values in the element array need to be a valid vertex index.
		if (elements[i] >= vertexCount)
			throw love::Exception("Invalid vertex map value");
	}

	vertexArray = new Vertex[vertexCount];
	memcpy(vertexArray, &polygon[0], vertexCount * sizeof(Vertex));

	if (elementCount > 0)
	{
		elementArray = new uint16[elementCount];
		memcpy(elementArray, &elements[0], elementCount * sizeof(uint16));
	}
}

Geometry::Geometry(float x, float y, float w, float h, float sw, float sh)
	: vertexArray(NULL)
	, vertexCount(4)
	, elementArray(NULL)
	, elementCount(0)
	, vertexColors(false)
	, drawMode(DRAW_MODE_FAN)
{
	float s0 = x/sw, s1 = (x+w)/sw, t0 = y/sh, t1 = (y+h)/sh;

	Vertex verts[4] = {
		{0,0, s0,t0, 255, 255, 255, 255},
		{w,0, s1,t0, 255, 255, 255, 255},
		{w,h, s1,t1, 255, 255, 255, 255},
		{0,h, s0,t1, 255, 255, 255, 255}
	};

	vertexArray = new Vertex[4];

	for (int i = 0; i < 4; i++)
		vertexArray[i] = verts[i];
}

Geometry::Geometry(const Geometry &other)
	: vertexCount(other.vertexCount)
	, elementCount(other.elementCount)
	, vertexColors(other.vertexColors)
	, drawMode(other.drawMode)
{
	vertexArray = new Vertex[vertexCount];
	memcpy(vertexArray, other.vertexArray, vertexCount * sizeof(Vertex));

	if (elementCount > 0)
	{
		elementArray = new uint16[elementCount];
		memcpy(elementArray, other.elementArray, elementCount * sizeof(uint16));
	}
}

Geometry &Geometry::operator=(const Geometry &other)
{
	if (this != &other)
	{
		Geometry temp(other);
		std::swap(vertexArray, temp.vertexArray);
		std::swap(elementArray, temp.elementArray);

		vertexCount  = temp.vertexCount;
		elementCount = temp.elementCount;

		vertexColors = other.vertexColors;
		drawMode     = other.drawMode;
	}

	return *this;
}

Geometry::~Geometry()
{
	delete[] vertexArray;
	delete[] elementArray;
}

const Vertex &Geometry::getVertex(size_t i) const
{
	if (i >= vertexCount)
		throw Exception("Invalid vertex index");

	return vertexArray[i];
}

void Geometry::setVertex(size_t i, const Vertex &v)
{
	if (i >= vertexCount)
		throw Exception("Invalid vertex index");

	vertexArray[i] = v;
}


void Geometry::setElementArray(const uint16 *elements, size_t count)
{
	if (count == 0 || elements == NULL)
	{
		delete[] elementArray;
		elementArray = NULL;
		elementCount = 0;
		return;
	}

	for (size_t i = 0; i < count; i++)
	{
		if (elements[i] >= vertexCount)
			throw love::Exception("Invalid vertex map value");
	}

	if (count > elementCount)
	{
		delete[] elementArray;
		elementArray = new uint16[count];
	}

	elementCount = count;
	memcpy(elementArray, elements, elementCount * sizeof(uint16));
}

void Geometry::setVertexColors(bool on)
{
	vertexColors = on;
}

bool Geometry::getConstant(const char *in, Geometry::DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Geometry::getConstant(Geometry::DrawMode in, const char *&out)
{
	return drawModes.find(in, out);
}

StringMap<Geometry::DrawMode, Geometry::DRAW_MODE_MAX_ENUM>::Entry Geometry::drawModeEntries[] =
{
	{"fan", Geometry::DRAW_MODE_FAN},
	{"strip", Geometry::DRAW_MODE_STRIP},
	{"triangles", Geometry::DRAW_MODE_TRIANGLES}
};

StringMap<Geometry::DrawMode, Geometry::DRAW_MODE_MAX_ENUM> Geometry::drawModes(Geometry::drawModeEntries, sizeof(Geometry::drawModeEntries));

} // graphics
} // love
