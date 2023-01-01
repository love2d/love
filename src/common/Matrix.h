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

#ifndef LOVE_MATRIX_H
#define LOVE_MATRIX_H

// LOVE
#include "math.h"

namespace love
{

/**
 * This class is the basis for all transformations in LOVE. Although not really
 * needed for 2D, it contains 4x4 elements to be compatible with OpenGL without
 * conversions.
 **/
class Matrix4
{
private:

	static void multiply(const Matrix4 &a, const Matrix4 &b, float t[16]);

public:

	static void multiply(const Matrix4 &a, const Matrix4 &b, Matrix4 &result);

	/**
	 * Creates a new identity matrix.
	 **/
	Matrix4();
	
	/**
	 * Creates a new matrix with the transform values set.
	 **/
	Matrix4(float t00, float t10, float t01, float t11, float x, float y);

	/**
	 * Creates a new matrix from the specified elements. Be sure to pass
	 * exactly 16 elements in!
	 **/
	Matrix4(const float elements[16]);

	/**
	 * Creates a new matrix from the result of multiplying the two specified
	 * matrices.
	 **/
	Matrix4(const Matrix4 &a, const Matrix4 &b);

	/**
	 * Creates a new matrix set to a transformation.
	 **/
	Matrix4(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * Multiplies this Matrix with another Matrix, changing neither.
	 * @param m The Matrix to multiply with this Matrix.
	 * @return The combined matrix.
	 **/
	Matrix4 operator * (const Matrix4 &m) const;

	/**
	 * Multiplies a Matrix into this Matrix.
	 * @param m The Matrix to combine into this Matrix.
	 **/
	void operator *= (const Matrix4 &m);

	/**
	 * Gets a pointer to the 16 array elements.
	 * @return The array elements.
	 **/
	const float *getElements() const;

	/**
	 * Resets this Matrix to the identity matrix.
	 **/
	void setIdentity();

	/**
	 * Resets this Matrix to a translation.
	 * @param x Translation along x-axis.
	 * @param y Translation along y-axis.
	 **/
	void setTranslation(float x, float y);

	/**
	 * Resets this Matrix to a rotation.
	 * @param r The angle in radians.
	 **/
	void setRotation(float r);

	/**
	 * Resets this Matrix to a scale transformation.
	 * @param sx Scale factor along the x-axis.
	 * @param sy Scale factor along the y-axis.
	 **/
	void setScale(float sx, float sy);

	/**
	 * Resets this Matrix to a shear transformation.
	 * @param kx Shear along x-axis.
	 * @param ky Shear along y-axis.
	 **/
	void setShear(float kx, float ky);

	/**
	 * Calculates the scale factors for a 2D affine transform. The output values
	 * are absolute (not signed).
	 **/
	void getApproximateScale(float &sx, float &sy) const;
	
	/**
	 * Sets a transformation's values directly. Useful if you want to modify them inplace,
	 * or if you want to create a transformation that's not buildable with setTransformation()
	 * i.e. the inverse of setTransformation() is not easily built with another call
	 * to setTransformation() with tweaked values.
	 *
	 * @param t00 The sx*cos(angle) component of the transformation.
	 * @param t10 The sx*sin(angle) component of the transformation.
	 * @param t01 The sy*(-sin(angle)) component of the transformation.
	 * @param t11 The sy*cos(angle) component of the transformation.
	 * @param x The x translation component of the transformation.
	 * @param y The y translation component of the transformation.
	 **/
	void setRawTransformation(float t00, float t10, float t01, float t11, float x, float y);

	/**
	 * Creates a transformation with a certain position, orientation, scale
	 * and offset. Perfect for Drawables -- what a coincidence!
	 *
	 * @param x The translation along the x-axis.
	 * @param y The translation along the y-axis.
	 * @param angle The rotation (rad) around the center with offset (ox,oy).
	 * @param sx Scale along x-axis.
	 * @param sy Scale along y-axis.
	 * @param ox The offset for rotation along the x-axis.
	 * @param oy The offset for rotation along the y-axis.
	 * @param kx Shear along x-axis
	 * @param ky Shear along y-axis
	 **/
	void setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * Multiplies this Matrix with a translation.
	 * @param x Translation along x-axis.
	 * @param y Translation along y-axis.
	 **/
	void translate(float x, float y);

	/**
	 * Multiplies this Matrix with a rotation.
	 * @param r Angle in radians.
	 **/
	void rotate(float r);

	/**
	 * Multiplies this Matrix with a scale transformation.
	 * @param sx Scale factor along the x-axis.
	 * @param sy Scale factor along the y-axis.
	 **/
	void scale(float sx, float sy);

	/**
	 * Multiplies this Matrix with a shear transformation.
	 * @param kx Shear along the x-axis.
	 * @param ky Shear along the y-axis.
	 **/
	void shear(float kx, float ky);

	/**
	 * Transforms an array of 2-component vertices by this Matrix. The source
	 * and destination arrays may be the same.
	 **/
	template <typename Vdst, typename Vsrc>
	void transformXY(Vdst *dst, const Vsrc *src, int size) const;

	/**
	 * Transforms an array of 2-component vertices by this Matrix, and stores
	 * them in an array of 3-component vertices.
	 **/
	template <typename Vdst, typename Vsrc>
	void transformXY0(Vdst *dst, const Vsrc *src, int size) const;

	/**
	 * Transforms an array of 3-component vertices by this Matrix. The source
	 * and destination arrays may be the same.
	 **/
	template <typename Vdst, typename Vsrc>
	void transformXYZ(Vdst *dst, const Vsrc *src, int size) const;

	/**
	 * Gets whether this matrix is an affine 2D transform (if the only non-
	 * identity elements are the upper-left 2x2 and 2 translation values in the
	 * 4th column).
	 **/
	bool isAffine2DTransform() const;

	/**
	 * Computes and returns the inverse of the matrix.
	 **/
	Matrix4 inverse() const;

	/**
	 * Creates a new orthographic projection matrix.
	 **/
	static Matrix4 ortho(float left, float right, float bottom, float top, float near, float far);

private:

	/**
	 * | e0 e4 e8  e12 |
	 * | e1 e5 e9  e13 |
	 * | e2 e6 e10 e14 |
	 * | e3 e7 e11 e15 |
	 **/
	float e[16];

}; // Matrix4

class Matrix3
{
public:

	Matrix3();

	/**
	 * Constructs a 3x3 matrix from the upper left section of a 4x4 matrix.
	 **/
	Matrix3(const Matrix4 &mat4);

	/**
	 * Creates a new matrix set to a transformation.
	 **/
	Matrix3(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	~Matrix3();

	/**
	 * Resets this matrix to the identity matrix.
	 **/
	void setIdentity();

	Matrix3 operator * (const Matrix3 &m) const;
	void operator *= (const Matrix3 &m);

	/**
	 * Gets a pointer to the 9 array elements.
	 **/
	const float *getElements() const;

	/**
	 * Calculates the inverse of the transpose of this matrix.
	 **/
	Matrix3 transposedInverse() const;

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
	 * @param kx Shear along x-axis
	 * @param ky Shear along y-axis
	 **/
	void setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * Transforms an array of vertices by this matrix.
	 **/
	template <typename Vdst, typename Vsrc>
	void transformXY(Vdst *dst, const Vsrc *src, int size) const;

private:

	/**
	 * | e0 e3 e6
	 * | e1 e4 e7
	 * | e2 e5 e8
	 **/
	float e[9];

}; // Matrix3

//                 | x |
//                 | y |
//                 | 0 |
//                 | 1 |
// | e0 e4 e8  e12 |
// | e1 e5 e9  e13 |
// | e2 e6 e10 e14 |
// | e3 e7 e11 e15 |

template <typename Vdst, typename Vsrc>
void Matrix4::transformXY(Vdst *dst, const Vsrc *src, int size) const
{
	for (int i = 0; i < size; i++)
	{
		// Store in temp variables in case src = dst
		float x = (e[0]*src[i].x) + (e[4]*src[i].y) + (0) + (e[12]);
		float y = (e[1]*src[i].x) + (e[5]*src[i].y) + (0) + (e[13]);

		dst[i].x = x;
		dst[i].y = y;
	}
}

template <typename Vdst, typename Vsrc>
void Matrix4::transformXY0(Vdst *dst, const Vsrc *src, int size) const
{
	for (int i = 0; i < size; i++)
	{
		// Store in temp variables in case src = dst
		float x = (e[0]*src[i].x) + (e[4]*src[i].y) + (0) + (e[12]);
		float y = (e[1]*src[i].x) + (e[5]*src[i].y) + (0) + (e[13]);
		float z = (e[2]*src[i].x) + (e[6]*src[i].y) + (0) + (e[14]);

		dst[i].x = x;
		dst[i].y = y;
		dst[i].z = z;
	}
}

//                 | x |
//                 | y |
//                 | z |
//                 | 1 |
// | e0 e4 e8  e12 |
// | e1 e5 e9  e13 |
// | e2 e6 e10 e14 |
// | e3 e7 e11 e15 |

template <typename Vdst, typename Vsrc>
void Matrix4::transformXYZ(Vdst *dst, const Vsrc *src, int size) const
{
	for (int i = 0; i < size; i++)
	{
		// Store in temp variables in case src = dst
		float x = (e[0]*src[i].x) + (e[4]*src[i].y) + (e[ 8]*src[i].z) + (e[12]);
		float y = (e[1]*src[i].x) + (e[5]*src[i].y) + (e[ 9]*src[i].z) + (e[13]);
		float z = (e[2]*src[i].x) + (e[6]*src[i].y) + (e[10]*src[i].z) + (e[14]);

		dst[i].x = x;
		dst[i].y = y;
		dst[i].z = z;
	}
}

//            | x |
//            | y |
//            | 1 |
// | e0 e3 e6 |
// | e1 e4 e7 |
// | e2 e5 e8 |
template <typename Vdst, typename Vsrc>
void Matrix3::transformXY(Vdst *dst, const Vsrc *src, int size) const
{
	for (int i = 0; i < size; i++)
	{
		float x = (e[0]*src[i].x) + (e[3]*src[i].y) + (e[6]);
		float y = (e[1]*src[i].x) + (e[4]*src[i].y) + (e[7]);

		dst[i].x = x;
		dst[i].y = y;
	}
}

} //love

#endif// LOVE_MATRIX_H
