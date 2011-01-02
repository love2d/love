/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#include "Matrix.h"

// STD
#include <cstring> // memcpy
#include <cmath>

namespace love
{

	// | e0 e4 e8  e12 |
	// | e1 e5 e9  e13 |
	// | e2 e6 e10 e14 |
	// | e3 e7 e11 e15 |

	Matrix::Matrix()
	{
		setIdentity();
	}

	Matrix::~Matrix()
	{
	}

	//                 | e0 e4 e8  e12 |
	//                 | e1 e5 e9  e13 |
	//                 | e2 e6 e10 e14 |
	//                 | e3 e7 e11 e15 |
	// | e0 e4 e8  e12 |
	// | e1 e5 e9  e13 |
	// | e2 e6 e10 e14 |
	// | e3 e7 e11 e15 |

	Matrix Matrix::operator * (const Matrix & m) const
	{
		Matrix t;
		
		t.e[0] = (e[0]*m.e[0]) + (e[4]*m.e[1]) + (e[8]*m.e[2]) + (e[12]*m.e[3]);
		t.e[4] = (e[0]*m.e[4]) + (e[4]*m.e[5]) + (e[8]*m.e[6]) + (e[12]*m.e[7]);
		t.e[8] = (e[0]*m.e[8]) + (e[4]*m.e[9]) + (e[8]*m.e[10]) + (e[12]*m.e[11]);
		t.e[12] = (e[0]*m.e[12]) + (e[4]*m.e[13]) + (e[8]*m.e[14]) + (e[12]*m.e[15]);

		t.e[1] = (e[1]*m.e[0]) + (e[5]*m.e[1]) + (e[9]*m.e[2]) + (e[13]*m.e[3]);
		t.e[5] = (e[1]*m.e[4]) + (e[5]*m.e[5]) + (e[9]*m.e[6]) + (e[13]*m.e[7]);
		t.e[9] = (e[1]*m.e[8]) + (e[5]*m.e[9]) + (e[9]*m.e[10]) + (e[13]*m.e[11]);
		t.e[13] = (e[1]*m.e[12]) + (e[5]*m.e[13]) + (e[9]*m.e[14]) + (e[13]*m.e[15]);

		t.e[2] = (e[2]*m.e[0]) + (e[6]*m.e[1]) + (e[10]*m.e[2]) + (e[14]*m.e[3]);
		t.e[6] = (e[2]*m.e[4]) + (e[6]*m.e[5]) + (e[10]*m.e[6]) + (e[14]*m.e[7]);
		t.e[10] = (e[2]*m.e[8]) + (e[6]*m.e[9]) + (e[10]*m.e[10]) + (e[14]*m.e[11]);
		t.e[14] = (e[2]*m.e[12]) + (e[6]*m.e[13]) + (e[10]*m.e[14]) + (e[14]*m.e[15]);

		t.e[3] = (e[3]*m.e[0]) + (e[7]*m.e[1]) + (e[11]*m.e[2]) + (e[15]*m.e[3]);
		t.e[7] = (e[3]*m.e[4]) + (e[7]*m.e[5]) + (e[11]*m.e[6]) + (e[15]*m.e[7]);
		t.e[11] = (e[3]*m.e[8]) + (e[7]*m.e[9]) + (e[11]*m.e[10]) + (e[15]*m.e[11]);
		t.e[15] = (e[3]*m.e[12]) + (e[7]*m.e[13]) + (e[11]*m.e[14]) + (e[15]*m.e[15]);

		return t;
	}

	void Matrix::operator *= (const Matrix & m)
	{
		Matrix t = (*this) * m;
		memcpy((void*)this->e, (void*)t.e, sizeof(float)*16);
	}

	const float * Matrix::getElements() const
	{
		return e;
	}

	void Matrix::setIdentity()
	{
		memset(e, 0, sizeof(float)*16);
		e[0] = e[5] = e[10] = e[15] = 1;
	}

	void Matrix::setTranslation(float x, float y)
	{
		setIdentity();
		e[12] = x;
		e[13] = y;
	}

	void Matrix::setRotation(float rad)
	{
		setIdentity();
		float c = cos(rad), s = sin(rad);
		e[0] = c; e[4] = -s;
		e[1] = s; e[5] = c;
	}

	void Matrix::setScale(float sx, float sy)
	{
		setIdentity();
		e[0] = sx;
		e[5] = sy;
	}

	void Matrix::setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy)
	{
		memset(e, 0, sizeof(float)*16); // zero out matrix
		float c = cos(angle), s = sin(angle);
		// matrix multiplication carried out on paper:
		// |1     x| |c -s    | |sx       | |1     -ox|
		// |  1   y| |s  c    | |   sy    | |  1   -oy|
		// |    1  | |     1  | |      1  | |    1    |
		// |      1| |       1| |        1| |       1 |
		//   move      rotate      scale       origin
		e[10] = e[15] = 1.0f;
		e[0] = sx * c;
		e[1] = sx * s;
		e[4] = -sy * s;
		e[5] = sy * c;
		e[12] = -ox * e[0] - oy * e[4] + x;
		e[13] = -ox * e[1] - oy * e[5] + y;
	}

	void Matrix::translate(float x, float y)
	{
		Matrix t;
		t.setTranslation(x, y);
		this->operator *=(t);
	}

	void Matrix::rotate(float rad)
	{
		Matrix t;
		t.setRotation(rad);
		this->operator *=(t);
	}

	void Matrix::scale(float sx, float sy)
	{
		Matrix t;
		t.setScale(sx, sy);
		this->operator *=(t);
	}

	//                 | x |
	//                 | y |
	//                 | 0 |
	//                 | 1 |
	// | e0 e4 e8  e12 |
	// | e1 e5 e9  e13 |
	// | e2 e6 e10 e14 |
	// | e3 e7 e11 e15 |

	void Matrix::transform(vertex * dst, const vertex * src, int size) const
	{
		for(int i = 0;i<size;i++)
		{
			// Store in temp variables in case src = dst
			float x = (e[0]*src[i].x) + (e[4]*src[i].y) + (0) + (e[12]);
			float y = (e[1]*src[i].x) + (e[5]*src[i].y) + (0) + (e[13]);

			dst[i].x = x;
			dst[i].y = y;
		}
	}


} // love
