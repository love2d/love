/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

// LOVE
#include "Quad.h"

// C
#include <cstring> // For memcpy

namespace love
{
namespace graphics
{

Quad::Quad(const Quad::Viewport &v, double sw, double sh)
	: sw(sw)
	, sh(sh)
{
	memset(vertices, 255, sizeof(Vertex) * 4);
	refresh(v, sw, sh);
}

Quad::~Quad()
{
}

void Quad::refresh(const Quad::Viewport &v, double sw, double sh)
{
	viewport = v;

	// Vertices are ordered for use with triangle strips:
	// 0----2
	// |  / |
	// | /  |
	// 1----3
	vertices[0].x = 0.0f;
	vertices[0].y = 0.0f;
	vertices[1].x = 0.0f;
	vertices[1].y = (float) v.h;
	vertices[2].x = (float) v.w;
	vertices[2].y = 0.0f;
	vertices[3].x = (float) v.w;
	vertices[3].y = (float) v.h;

	vertices[0].s = (float) (v.x/sw);
	vertices[0].t = (float) (v.y/sh);
	vertices[1].s = (float) (v.x/sw);
	vertices[1].t = (float) ((v.y+v.h)/sh);
	vertices[2].s = (float) ((v.x+v.w)/sw);
	vertices[2].t = (float) (v.y/sh);
	vertices[3].s = (float) ((v.x+v.w)/sw);
	vertices[3].t = (float) ((v.y+v.h)/sh);
}

void Quad::setViewport(const Quad::Viewport &v)
{
	refresh(v, sw, sh);
}

Quad::Viewport Quad::getViewport() const
{
	return viewport;
}

const Vertex *Quad::getVertices() const
{
	return vertices;
}

} // graphics
} // love
