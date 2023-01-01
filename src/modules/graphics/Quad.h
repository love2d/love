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

#ifndef LOVE_GRAPHICS_QUAD_H
#define LOVE_GRAPHICS_QUAD_H

// LOVE
#include "common/Object.h"
#include "common/math.h"
#include "common/Vector.h"

namespace love
{
namespace graphics
{

class Quad : public Object
{
public:

	static love::Type type;

	struct Viewport
	{
		double x, y;
		double w, h;
	};

	Quad(const Viewport &v, double sw, double sh);
	virtual ~Quad();

	void refresh(const Viewport &v, double sw, double sh);
	void setViewport(const Viewport &v);
	Viewport getViewport() const;

	double getTextureWidth() const;
	double getTextureHeight() const;

	const Vector2 *getVertexPositions() const { return vertexPositions; }
	const Vector2 *getVertexTexCoords() const { return vertexTexCoords; }

	void setLayer(int layer);
	int getLayer() const;

private:

	Vector2 vertexPositions[4];
	Vector2 vertexTexCoords[4];

	int arrayLayer;

	Viewport viewport;
	double sw;
	double sh;

}; // Quad

} // graphics
} // love

#endif // LOVE_GRAPHICS_QUAD_H
