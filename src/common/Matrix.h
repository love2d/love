/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_MATRIX_H
#define LOVE_MATRIX_H

// STD
#include <cmath>

// LOVE
#include "math.h"

namespace love
{

	// Forward declarations.
	class Vector;

	class Matrix
	{
		friend class Vector;

	public:

		// | e0 e4 e8  e12 |
		// | e1 e5 e9  e13 |
		// | e2 e6 e10 e14 |
		// | e3 e7 e11 e15 |
		float e[16];

		/**
		* Creates a new identity matrix.
		**/
		Matrix();

		/**
		* Destructor.
		**/
		~Matrix();

		Matrix operator * (const Matrix & m) const;
		void operator *= (const Matrix & m) const;

		const float * getElements() const;

		void setIdentity();
		void setTranslation(float x, float y);
		void setRotation(float rad);
		void setScale(float sx, float sy);

		/**
		* Creates a transformation with a certain position, orientation, scale
		* and offset.
		* 
		* @param x The translation along the x-axis.
		* @param y The translation along the y-axis.
		* @param angle The rotation (rad) around the center with offset (ox,oy).
		* @param sx Scale along x-axis.
		* @param sy Scale along y-axis.
		* @param ox The offset for rotation along the x-axis.
		* @param oy The offset for rotation along the y-axis.
		**/
		void setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy);

		void translate(float x, float y);
		void rotate(float rad);
		void scale(float sx, float sy);
		
		/**
		* Transforms an array of vertices.
		* @stride Stride in bytes.
		**/
		void transform(vertex * dst, const vertex * src, int size);

	};

} //love

#endif// LOVE_MATRIX_H
