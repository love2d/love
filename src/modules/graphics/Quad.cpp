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

// LOVE
#include "Quad.h"

// C
#include <cstring> // For memcpy

namespace love
{
namespace graphics
{

love::Type Quad::type("Quad", &Object::type);

Quad::Quad(const Quad::Viewport &v, double sw, double sh)
	: sw(sw)
	, sh(sh)
{
	arrayLayer = 0;
	refresh(v, sw, sh);
}

Quad::~Quad()
{
}

void Quad::refresh(const Quad::Viewport &v, double sw, double sh)
{
	this->viewport = v;
	this->sw = sw;
	this->sh = sh;

	// Vertices are ordered for use with triangle strips:
	// 0---2
	// | / |
	// 1---3
	vertexPositions[0] = Vector2(0.0f, 0.0f);
	vertexPositions[1] = Vector2(0.0f, (float) v.h);
	vertexPositions[2] = Vector2((float) v.w, 0.0f);
	vertexPositions[3] = Vector2((float) v.w, (float) v.h);

	vertexTexCoords[0] = Vector2((float) (v.x / sw), (float) (v.y / sh));
	vertexTexCoords[1] = Vector2((float) (v.x / sw), (float) ((v.y + v.h) / sh));
	vertexTexCoords[2] = Vector2((float) ((v.x + v.w) / sw), (float) (v.y / sh));
	vertexTexCoords[3] = Vector2((float) ((v.x + v.w) / sw), (float) ((v.y + v.h) / sh));
}

void Quad::setViewport(const Quad::Viewport &v)
{
	refresh(v, sw, sh);
}

Quad::Viewport Quad::getViewport() const
{
	return viewport;
}

double Quad::getTextureWidth() const
{
	return sw;
}

double Quad::getTextureHeight() const
{
	return sh;
}

void Quad::setLayer(int layer)
{
	arrayLayer = layer;
}

int Quad::getLayer() const
{
	return arrayLayer;
}

} // graphics
} // love
