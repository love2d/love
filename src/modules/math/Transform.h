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
#include "common/Object.h"
#include "common/Matrix.h"
#include "common/Vector.h"

namespace love
{
namespace math
{

class Transform : public Object
{
public:

	static love::Type type;

	Transform();
	Transform(const Matrix4 &m);
	Transform(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky);

	virtual ~Transform();

	Transform *clone();
	Transform *inverse();

	void apply(Transform *other);

	void translate(float x, float y);
	void rotate(float angle);
	void scale(float x, float y);
	void shear(float x, float y);

	void reset();
	void setTransformation(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky);

	love::Vector transformPoint(love::Vector p) const;
	love::Vector inverseTransformPoint(love::Vector p);

	const Matrix4 &getMatrix() const;
	void setMatrix(const Matrix4 &m);

private:

	inline const Matrix4 &getInverseMatrix()
	{
		if (inverseDirty)
		{
			inverseDirty = false;
			inverseMatrix = matrix.inverse();
		}

		return inverseMatrix;
	}
	
	Matrix4 matrix;
	bool inverseDirty;
	Matrix4 inverseMatrix;

}; // Transform


} // math
} // love
