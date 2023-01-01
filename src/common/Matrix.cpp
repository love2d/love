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

#include "Matrix.h"
#include "common/config.h"

// STD
#include <cstring> // memcpy
#include <cmath>

#if defined(LOVE_SIMD_SSE)
#include <xmmintrin.h>
#endif

#if defined(LOVE_SIMD_NEON)
#include <arm_neon.h>
#endif

namespace love
{

//                 | e0 e4 e8  e12 |
//                 | e1 e5 e9  e13 |
//                 | e2 e6 e10 e14 |
//                 | e3 e7 e11 e15 |
// | e0 e4 e8  e12 |
// | e1 e5 e9  e13 |
// | e2 e6 e10 e14 |
// | e3 e7 e11 e15 |

void Matrix4::multiply(const Matrix4 &a, const Matrix4 &b, float t[16])
{
#if defined(LOVE_SIMD_SSE)

	// We can't guarantee 16-bit alignment (e.g. for heap-allocated Matrix4
	// objects) so we use unaligned loads and stores.
	__m128 col1 = _mm_loadu_ps(&a.e[0]);
	__m128 col2 = _mm_loadu_ps(&a.e[4]);
	__m128 col3 = _mm_loadu_ps(&a.e[8]);
	__m128 col4 = _mm_loadu_ps(&a.e[12]);

	for (int i = 0; i < 4; i++)
	{
		__m128 brod1 = _mm_set1_ps(b.e[4*i + 0]);
		__m128 brod2 = _mm_set1_ps(b.e[4*i + 1]);
		__m128 brod3 = _mm_set1_ps(b.e[4*i + 2]);
		__m128 brod4 = _mm_set1_ps(b.e[4*i + 3]);

		__m128 col = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(brod1, col1), _mm_mul_ps(brod2, col2)),
			_mm_add_ps(_mm_mul_ps(brod3, col3), _mm_mul_ps(brod4, col4))
		);

		_mm_storeu_ps(&t[4*i], col);
	}

#elif defined(LOVE_SIMD_NEON)

	float32x4_t cola1 = vld1q_f32(&a.e[0]);
	float32x4_t cola2 = vld1q_f32(&a.e[4]);
	float32x4_t cola3 = vld1q_f32(&a.e[8]);
	float32x4_t cola4 = vld1q_f32(&a.e[12]);

	float32x4_t col1 = vmulq_n_f32(cola1, b.e[0]);
	col1 = vmlaq_n_f32(col1, cola2, b.e[1]);
	col1 = vmlaq_n_f32(col1, cola3, b.e[2]);
	col1 = vmlaq_n_f32(col1, cola4, b.e[3]);

	float32x4_t col2 = vmulq_n_f32(cola1, b.e[4]);
	col2 = vmlaq_n_f32(col2, cola2, b.e[5]);
	col2 = vmlaq_n_f32(col2, cola3, b.e[6]);
	col2 = vmlaq_n_f32(col2, cola4, b.e[7]);

	float32x4_t col3 = vmulq_n_f32(cola1, b.e[8]);
	col3 = vmlaq_n_f32(col3, cola2, b.e[9]);
	col3 = vmlaq_n_f32(col3, cola3, b.e[10]);
	col3 = vmlaq_n_f32(col3, cola4, b.e[11]);

	float32x4_t col4 = vmulq_n_f32(cola1, b.e[12]);
	col4 = vmlaq_n_f32(col4, cola2, b.e[13]);
	col4 = vmlaq_n_f32(col4, cola3, b.e[14]);
	col4 = vmlaq_n_f32(col4, cola4, b.e[15]);

	vst1q_f32(&t[0], col1);
	vst1q_f32(&t[4], col2);
	vst1q_f32(&t[8], col3);
	vst1q_f32(&t[12], col4);

#else

	t[0]  = (a.e[0]*b.e[0])  + (a.e[4]*b.e[1])  + (a.e[8]*b.e[2])  + (a.e[12]*b.e[3]);
	t[4]  = (a.e[0]*b.e[4])  + (a.e[4]*b.e[5])  + (a.e[8]*b.e[6])  + (a.e[12]*b.e[7]);
	t[8]  = (a.e[0]*b.e[8])  + (a.e[4]*b.e[9])  + (a.e[8]*b.e[10]) + (a.e[12]*b.e[11]);
	t[12] = (a.e[0]*b.e[12]) + (a.e[4]*b.e[13]) + (a.e[8]*b.e[14]) + (a.e[12]*b.e[15]);

	t[1]  = (a.e[1]*b.e[0])  + (a.e[5]*b.e[1])  + (a.e[9]*b.e[2])  + (a.e[13]*b.e[3]);
	t[5]  = (a.e[1]*b.e[4])  + (a.e[5]*b.e[5])  + (a.e[9]*b.e[6])  + (a.e[13]*b.e[7]);
	t[9]  = (a.e[1]*b.e[8])  + (a.e[5]*b.e[9])  + (a.e[9]*b.e[10]) + (a.e[13]*b.e[11]);
	t[13] = (a.e[1]*b.e[12]) + (a.e[5]*b.e[13]) + (a.e[9]*b.e[14]) + (a.e[13]*b.e[15]);

	t[2]  = (a.e[2]*b.e[0])  + (a.e[6]*b.e[1])  + (a.e[10]*b.e[2])  + (a.e[14]*b.e[3]);
	t[6]  = (a.e[2]*b.e[4])  + (a.e[6]*b.e[5])  + (a.e[10]*b.e[6])  + (a.e[14]*b.e[7]);
	t[10] = (a.e[2]*b.e[8])  + (a.e[6]*b.e[9])  + (a.e[10]*b.e[10]) + (a.e[14]*b.e[11]);
	t[14] = (a.e[2]*b.e[12]) + (a.e[6]*b.e[13]) + (a.e[10]*b.e[14]) + (a.e[14]*b.e[15]);

	t[3]  = (a.e[3]*b.e[0])  + (a.e[7]*b.e[1])  + (a.e[11]*b.e[2])  + (a.e[15]*b.e[3]);
	t[7]  = (a.e[3]*b.e[4])  + (a.e[7]*b.e[5])  + (a.e[11]*b.e[6])  + (a.e[15]*b.e[7]);
	t[11] = (a.e[3]*b.e[8])  + (a.e[7]*b.e[9])  + (a.e[11]*b.e[10]) + (a.e[15]*b.e[11]);
	t[15] = (a.e[3]*b.e[12]) + (a.e[7]*b.e[13]) + (a.e[11]*b.e[14]) + (a.e[15]*b.e[15]);

#endif
}

void Matrix4::multiply(const Matrix4 &a, const Matrix4 &b, Matrix4 &t)
{
	multiply(a, b, t.e);
}

// | e0 e4 e8  e12 |
// | e1 e5 e9  e13 |
// | e2 e6 e10 e14 |
// | e3 e7 e11 e15 |

Matrix4::Matrix4()
{
	setIdentity();
}


Matrix4::Matrix4(const float elements[16])
{
	memcpy(e, elements, sizeof(float) * 16);
}
	
Matrix4::Matrix4(float t00, float t10, float t01, float t11, float x, float y)
{
	setRawTransformation(t00, t10, t01, t11, x, y);
}

Matrix4::Matrix4(const Matrix4 &a, const Matrix4 &b)
{
	multiply(a, b, e);
}

Matrix4::Matrix4(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
}

Matrix4 Matrix4::operator * (const Matrix4 &m) const
{
	return Matrix4(*this, m);
}

void Matrix4::operator *= (const Matrix4 &m)
{
	float t[16];
	multiply(*this, m, t);
	memcpy(this->e, t, sizeof(float)*16);
}

const float *Matrix4::getElements() const
{
	return e;
}

void Matrix4::setIdentity()
{
	memset(e, 0, sizeof(float)*16);
	e[15] = e[10] = e[5] = e[0] = 1;
}

void Matrix4::setTranslation(float x, float y)
{
	setIdentity();
	e[12] = x;
	e[13] = y;
}

void Matrix4::setRotation(float rad)
{
	setIdentity();
	float c = cosf(rad), s = sinf(rad);
	e[0] = c;
	e[4] = -s;
	e[1] = s;
	e[5] = c;
}

void Matrix4::setScale(float sx, float sy)
{
	setIdentity();
	e[0] = sx;
	e[5] = sy;
}

void Matrix4::setShear(float kx, float ky)
{
	setIdentity();
	e[1] = ky;
	e[4] = kx;
}

void Matrix4::getApproximateScale(float &sx, float &sy) const
{
	sx = sqrtf(e[0] * e[0] + e[4] * e[4]);
	sy = sqrtf(e[1] * e[1] + e[5] * e[5]);
}
	
void Matrix4::setRawTransformation(float t00, float t10, float t01, float t11, float x, float y)
{
	memset(e, 0, sizeof(float)*16); // zero out matrix
	e[10] = e[15] = 1.0f;
	e[0] = t00;
	e[1] = t10;
	e[4] = t01;
	e[5] = t11;
	e[12] = x;
	e[13] = y;
}

void Matrix4::setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	memset(e, 0, sizeof(float)*16); // zero out matrix
	float c = cosf(angle), s = sinf(angle);
	// matrix multiplication carried out on paper:
	// |1     x| |c -s    | |sx       | | 1 ky    | |1     -ox|
	// |  1   y| |s  c    | |   sy    | |kx  1    | |  1   -oy|
	// |    1  | |     1  | |      1  | |      1  | |    1    |
	// |      1| |       1| |        1| |        1| |       1 |
	//   move      rotate      scale       skew       origin
	e[10] = e[15] = 1.0f;
	e[0]  = c * sx - ky * s * sy; // = a
	e[1]  = s * sx + ky * c * sy; // = b
	e[4]  = kx * c * sx - s * sy; // = c
	e[5]  = kx * s * sx + c * sy; // = d
	e[12] = x - ox * e[0] - oy * e[4];
	e[13] = y - ox * e[1] - oy * e[5];
}

void Matrix4::translate(float x, float y)
{
	Matrix4 t;
	t.setTranslation(x, y);
	this->operator *=(t);
}

void Matrix4::rotate(float rad)
{
	Matrix4 t;
	t.setRotation(rad);
	this->operator *=(t);
}

void Matrix4::scale(float sx, float sy)
{
	Matrix4 t;
	t.setScale(sx, sy);
	this->operator *=(t);
}

void Matrix4::shear(float kx, float ky)
{
	Matrix4 t;
	t.setShear(kx,ky);
	this->operator *=(t);
}

bool Matrix4::isAffine2DTransform() const
{
	return fabsf(e[2] + e[3] + e[6] + e[7] + e[8] + e[9] + e[11] + e[14]) < 0.00001f
		&& fabsf(e[10] + e[15] - 2.0f) < 0.00001f;
}

Matrix4 Matrix4::inverse() const
{
	Matrix4 inv;

	inv.e[0] = e[5]  * e[10] * e[15] -
	           e[5]  * e[11] * e[14] -
	           e[9]  * e[6]  * e[15] +
	           e[9]  * e[7]  * e[14] +
	           e[13] * e[6]  * e[11] -
	           e[13] * e[7]  * e[10];

	inv.e[4] = -e[4]  * e[10] * e[15] +
	            e[4]  * e[11] * e[14] +
	            e[8]  * e[6]  * e[15] -
	            e[8]  * e[7]  * e[14] -
	            e[12] * e[6]  * e[11] +
	            e[12] * e[7]  * e[10];

	inv.e[8] = e[4]  * e[9]  * e[15] -
	           e[4]  * e[11] * e[13] -
	           e[8]  * e[5]  * e[15] +
	           e[8]  * e[7]  * e[13] +
	           e[12] * e[5]  * e[11] -
	           e[12] * e[7]  * e[9];

	inv.e[12] = -e[4]  * e[9]  * e[14] +
	             e[4]  * e[10] * e[13] +
	             e[8]  * e[5]  * e[14] -
	             e[8]  * e[6]  * e[13] -
	             e[12] * e[5]  * e[10] +
	             e[12] * e[6]  * e[9];

	inv.e[1] = -e[1]  * e[10] * e[15] +
	            e[1]  * e[11] * e[14] +
	            e[9]  * e[2]  * e[15] -
	            e[9]  * e[3]  * e[14] -
	            e[13] * e[2]  * e[11] +
	            e[13] * e[3]  * e[10];

	inv.e[5] = e[0]  * e[10] * e[15] -
	           e[0]  * e[11] * e[14] -
	           e[8]  * e[2]  * e[15] +
	           e[8]  * e[3]  * e[14] +
	           e[12] * e[2]  * e[11] -
	           e[12] * e[3]  * e[10];

	inv.e[9] = -e[0]  * e[9]  * e[15] +
	            e[0]  * e[11] * e[13] +
	            e[8]  * e[1]  * e[15] -
	            e[8]  * e[3]  * e[13] -
	            e[12] * e[1]  * e[11] +
	            e[12] * e[3]  * e[9];

	inv.e[13] = e[0]  * e[9]  * e[14] -
	            e[0]  * e[10] * e[13] -
	            e[8]  * e[1]  * e[14] +
	            e[8]  * e[2]  * e[13] +
	            e[12] * e[1]  * e[10] -
	            e[12] * e[2]  * e[9];

	inv.e[2] = e[1]  * e[6] * e[15] -
	           e[1]  * e[7] * e[14] -
	           e[5]  * e[2] * e[15] +
	           e[5]  * e[3] * e[14] +
	           e[13] * e[2] * e[7] -
	           e[13] * e[3] * e[6];

	inv.e[6] = -e[0]  * e[6] * e[15] +
	            e[0]  * e[7] * e[14] +
	            e[4]  * e[2] * e[15] -
	            e[4]  * e[3] * e[14] -
	            e[12] * e[2] * e[7] +
	            e[12] * e[3] * e[6];

	inv.e[10] = e[0]  * e[5] * e[15] -
	            e[0]  * e[7] * e[13] -
	            e[4]  * e[1] * e[15] +
	            e[4]  * e[3] * e[13] +
	            e[12] * e[1] * e[7] -
	            e[12] * e[3] * e[5];

	inv.e[14] = -e[0]  * e[5] * e[14] +
	             e[0]  * e[6] * e[13] +
	             e[4]  * e[1] * e[14] -
	             e[4]  * e[2] * e[13] -
	             e[12] * e[1] * e[6] +
	             e[12] * e[2] * e[5];

	inv.e[3] = -e[1] * e[6] * e[11] +
	            e[1] * e[7] * e[10] +
	            e[5] * e[2] * e[11] -
	            e[5] * e[3] * e[10] -
	            e[9] * e[2] * e[7] +
	            e[9] * e[3] * e[6];

	inv.e[7] = e[0] * e[6] * e[11] -
	           e[0] * e[7] * e[10] -
	           e[4] * e[2] * e[11] +
	           e[4] * e[3] * e[10] +
	           e[8] * e[2] * e[7] -
	           e[8] * e[3] * e[6];

	inv.e[11] = -e[0] * e[5] * e[11] +
	             e[0] * e[7] * e[9] +
	             e[4] * e[1] * e[11] -
	             e[4] * e[3] * e[9] -
	             e[8] * e[1] * e[7] +
	             e[8] * e[3] * e[5];

	inv.e[15] = e[0] * e[5] * e[10] -
	            e[0] * e[6] * e[9] -
	            e[4] * e[1] * e[10] +
	            e[4] * e[2] * e[9] +
	            e[8] * e[1] * e[6] -
	            e[8] * e[2] * e[5];

	float det = e[0] * inv.e[0] + e[1] * inv.e[4] + e[2] * inv.e[8] + e[3] * inv.e[12];

	float invdet = 1.0f / det;

	for (int i = 0; i < 16; i++)
		inv.e[i] *= invdet;

	return inv;
}

Matrix4 Matrix4::ortho(float left, float right, float bottom, float top, float near, float far)
{
	Matrix4 m;

	m.e[0] = 2.0f / (right - left);
	m.e[5] = 2.0f / (top - bottom);
	m.e[10] = -2.0f / (far - near);

	m.e[12] = -(right + left) / (right - left);
	m.e[13] = -(top + bottom) / (top - bottom);
	m.e[14] = -(far + near) / (far - near);

	return m;
}

/**
 * | e0 e3 e6 |
 * | e1 e4 e7 |
 * | e2 e5 e8 |
 **/
Matrix3::Matrix3()
{
	setIdentity();
}

Matrix3::Matrix3(const Matrix4 &mat4)
{
	const float *mat4elems = mat4.getElements();

	// Column 0.
	e[0] = mat4elems[0];
	e[1] = mat4elems[1];
	e[2] = mat4elems[2];

	// Column 1.
	e[3] = mat4elems[4];
	e[4] = mat4elems[5];
	e[5] = mat4elems[6];

	// Column 2.
	e[6] = mat4elems[8];
	e[7] = mat4elems[9];
	e[8] = mat4elems[10];
}

Matrix3::Matrix3(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
}

Matrix3::~Matrix3()
{
}

void Matrix3::setIdentity()
{
	memset(e, 0, sizeof(float) * 9);
	e[8] = e[4] = e[0] = 1.0f;
}

Matrix3 Matrix3::operator * (const love::Matrix3 &m) const
{
	Matrix3 t;

	t.e[0] = (e[0]*m.e[0]) + (e[3]*m.e[1]) + (e[6]*m.e[2]);
	t.e[3] = (e[0]*m.e[3]) + (e[3]*m.e[4]) + (e[6]*m.e[5]);
	t.e[6] = (e[0]*m.e[6]) + (e[3]*m.e[7]) + (e[6]*m.e[8]);

	t.e[1] = (e[1]*m.e[0]) + (e[4]*m.e[1]) + (e[7]*m.e[2]);
	t.e[4] = (e[1]*m.e[3]) + (e[4]*m.e[4]) + (e[7]*m.e[5]);
	t.e[7] = (e[1]*m.e[6]) + (e[4]*m.e[7]) + (e[7]*m.e[8]);

	t.e[2] = (e[2]*m.e[0]) + (e[5]*m.e[1]) + (e[8]*m.e[2]);
	t.e[5] = (e[2]*m.e[3]) + (e[5]*m.e[4]) + (e[8]*m.e[5]);
	t.e[8] = (e[2]*m.e[6]) + (e[5]*m.e[7]) + (e[8]*m.e[8]);

	return t;
}

void Matrix3::operator *= (const Matrix3 &m)
{
	Matrix3 t = (*this) * m;
	memcpy(e, t.e, sizeof(float) * 9);
}

const float *Matrix3::getElements() const
{
	return e;
}

Matrix3 Matrix3::transposedInverse() const
{
	// e0 e3 e6
	// e1 e4 e7
	// e2 e5 e8

	float det = e[0] * (e[4]*e[8] - e[7]*e[5])
	          - e[1] * (e[3]*e[8] - e[5]*e[6])
	          + e[2] * (e[3]*e[7] - e[4]*e[6]);

	float invdet = 1.0f / det;

	Matrix3 m;

	m.e[0] =  invdet * (e[4]*e[8] - e[7]*e[5]);
	m.e[3] = -invdet * (e[1]*e[8] - e[2]*e[7]);
	m.e[6] =  invdet * (e[1]*e[5] - e[2]*e[4]);
	m.e[1] = -invdet * (e[3]*e[8] - e[5]*e[6]);
	m.e[4] =  invdet * (e[0]*e[8] - e[2]*e[6]);
	m.e[7] = -invdet * (e[0]*e[5] - e[3]*e[2]);
	m.e[2] =  invdet * (e[3]*e[7] - e[6]*e[4]);
	m.e[5] = -invdet * (e[0]*e[7] - e[6]*e[1]);
	m.e[8] =  invdet * (e[0]*e[4] - e[3]*e[1]);

	return m;
}

void Matrix3::setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	float c = cosf(angle), s = sinf(angle);
	// matrix multiplication carried out on paper:
	// |1    x| |c -s  | |sx     | | 1 ky  | |1   -ox|
	// |  1  y| |s  c  | |   sy  | |kx  1  | |  1 -oy|
	// |     1| |     1| |      1| |      1| |     1 |
	//   move    rotate    scale     skew      origin
	e[0] = c * sx - ky * s * sy; // = a
	e[1] = s * sx + ky * c * sy; // = b
	e[3] = kx * c * sx - s * sy; // = c
	e[4] = kx * s * sx + c * sy; // = d
	e[6] = x - ox * e[0] - oy * e[3];
	e[7] = y - ox * e[1] - oy * e[4];

	e[2] = e[5] = 0.0f;
	e[8] = 1.0f;
}

} // love
