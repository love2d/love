/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_MATH_H
#define B2_MATH_H

#include "b2Settings.h"
#include <cmath>
#include <cfloat>
#include <cstdlib>

#include <stdio.h>

#ifdef TARGET_FLOAT32_IS_FIXED

inline Fixed b2Min(const Fixed& a, const Fixed& b)
{
  return a < b ? a : b;
}

inline Fixed b2Max(const Fixed& a, const Fixed& b)
{
  return a > b ? a : b;
}

inline Fixed b2Clamp(Fixed a, Fixed low, Fixed high)
{
	return b2Max(low, b2Min(a, high));
}

inline bool b2IsValid(Fixed x)
{
	return true;
}

#define	b2Sqrt(x)	sqrt(x)
#define	b2Atan2(y, x)	atan2(y, x)

#else

/// This function is used to ensure that a floating point number is
/// not a NaN or infinity.
inline bool b2IsValid(float32 x)
{
#ifdef _MSC_VER
	return _finite(x) != 0;
#else
	return finite(x) != 0;
#endif
}

/// This is a approximate yet fast inverse square-root.
inline float32 b2InvSqrt(float32 x)
{
	union
	{
		float32 x;
		int32 i;
	} convert;

	convert.x = x;
	float32 xhalf = 0.5f * x;
	convert.i = 0x5f3759df - (convert.i >> 1);
	x = convert.x;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

#define	b2Sqrt(x)	sqrtf(x)
#define	b2Atan2(y, x)	atan2f(y, x)

#endif

inline float32 b2Abs(float32 a)
{
	return a > 0.0f ? a : -a;
}

/// A 2D column vector.

struct b2Vec2
{
	/// Default constructor does nothing (for performance).
	b2Vec2() {}

	/// Construct using coordinates.
	b2Vec2(float32 x, float32 y) : x(x), y(y) {}

	/// Set this vector to all zeros.
	void SetZero() { x = 0.0f; y = 0.0f; }

	/// Set this vector to some specified coordinates.
	void Set(float32 x_, float32 y_) { x = x_; y = y_; }

	/// Negate this vector.
	b2Vec2 operator -() const { b2Vec2 v; v.Set(-x, -y); return v; }
	
	/// Add a vector to this vector.
	void operator += (const b2Vec2& v)
	{
		x += v.x; y += v.y;
	}
	
	/// Subtract a vector from this vector.
	void operator -= (const b2Vec2& v)
	{
		x -= v.x; y -= v.y;
	}

	/// Multiply this vector by a scalar.
	void operator *= (float32 a)
	{
		x *= a; y *= a;
	}

	/// Get the length of this vector (the norm).
	float32 Length() const
	{
#ifdef TARGET_FLOAT32_IS_FIXED
		float est = b2Abs(x) + b2Abs(y);
		if(est == 0.0f) {
			return 0.0;
		} else if(est < 0.1) {
			return (1.0/256.0) * b2Vec2(x<<8, y<<8).Length();
		} else if(est < 180.0f) {
			return b2Sqrt(x * x + y * y);
		} else {
			return 256.0 * (b2Vec2(x>>8, y>>8).Length());
		}
#else
		return b2Sqrt(x * x + y * y);
#endif 
	}

	/// Get the length squared. For performance, use this instead of
	/// b2Vec2::Length (if possible).
	float32 LengthSquared() const
	{
		return x * x + y * y;
	}

	/// Convert this vector into a unit vector. Returns the length.
#ifdef TARGET_FLOAT32_IS_FIXED
	float32 Normalize()
	{
		float32 length = Length();
		if (length < B2_FLT_EPSILON)
		{
			return 0.0f;
		} 
#ifdef NORMALIZE_BY_INVERT_MULTIPLY
		if (length < (1.0/16.0)) {
			x = x << 4;
			y = y << 4;
			return (1.0/16.0)*Normalize();
		} else if(length > 16.0) {
			x = x >> 4;
			y = y >> 4;
			return 16.0*Normalize();
		}
		float32 invLength = 1.0f / length;
		x *= invLength;
		y *= invLength;
#else
		x /= length;
		y /= length;
#endif
		return length;
	}
#else
	float32 Normalize()
	{
		float32 length = Length();
		if (length < B2_FLT_EPSILON)
		{
			return 0.0f;
		}
		float32 invLength = 1.0f / length;
		x *= invLength;
		y *= invLength;

		return length;
	}
#endif

	/// Does this vector contain finite coordinates?
	bool IsValid() const
	{
		return b2IsValid(x) && b2IsValid(y);
	}

	float32 x, y;
};

/// A 2-by-2 matrix. Stored in column-major order.
struct b2Mat22
{
	/// The default constructor does nothing (for performance).
	b2Mat22() {}

	/// Construct this matrix using columns.
	b2Mat22(const b2Vec2& c1, const b2Vec2& c2)
	{
		col1 = c1;
		col2 = c2;
	}

	/// Construct this matrix using scalars.
	b2Mat22(float32 a11, float32 a12, float32 a21, float32 a22)
	{
		col1.x = a11; col1.y = a21;
		col2.x = a12; col2.y = a22;
	}

	/// Construct this matrix using an angle. This matrix becomes
	/// an orthonormal rotation matrix.
	explicit b2Mat22(float32 angle)
	{
		float32 c = cosf(angle), s = sinf(angle);
		col1.x = c; col2.x = -s;
		col1.y = s; col2.y = c;
	}

	/// Initialize this matrix using columns.
	void Set(const b2Vec2& c1, const b2Vec2& c2)
	{
		col1 = c1;
		col2 = c2;
	}

	/// Initialize this matrix using an angle. This matrix becomes
	/// an orthonormal rotation matrix.
	void Set(float32 angle)
	{
		float32 c = cosf(angle), s = sinf(angle);
		col1.x = c; col2.x = -s;
		col1.y = s; col2.y = c;
	}

	/// Set this to the identity matrix.
	void SetIdentity()
	{
		col1.x = 1.0f; col2.x = 0.0f;
		col1.y = 0.0f; col2.y = 1.0f;
	}

	/// Set this matrix to all zeros.
	void SetZero()
	{
		col1.x = 0.0f; col2.x = 0.0f;
		col1.y = 0.0f; col2.y = 0.0f;
	}

	/// Extract the angle from this matrix (assumed to be
	/// a rotation matrix).
	float32 GetAngle() const
	{
		return b2Atan2(col1.y, col1.x);
	}

#ifdef TARGET_FLOAT32_IS_FIXED

	/// Compute the inverse of this matrix, such that inv(A) * A = identity.
	b2Mat22 Invert() const
	{
		float32 a = col1.x, b = col2.x, c = col1.y, d = col2.y;
		float32 det = a * d - b * c;
		b2Mat22 B;
		int n = 0;

		if(b2Abs(det) <= (B2_FLT_EPSILON<<8))
		{
			n = 3;
			a = a<<n; b = b<<n; 
			c = c<<n; d = d<<n;
			det = a * d - b * c;
			b2Assert(det != 0.0f);
			det = float32(1) / det;
			B.col1.x = ( det * d) << n;	B.col2.x = (-det * b) << n;
			B.col1.y = (-det * c) << n;	B.col2.y = ( det * a) << n;
		} 
		else
		{
			n = (b2Abs(det) >= 16.0)? 4 : 0;
			b2Assert(det != 0.0f);
			det = float32(1<<n) / det;
			B.col1.x = ( det * d) >> n;	B.col2.x = (-det * b) >> n;
			B.col1.y = (-det * c) >> n;	B.col2.y = ( det * a) >> n;
		}
		
		return B;
	}

	// Solve A * x = b
	b2Vec2 Solve(const b2Vec2& b) const
	{
		float32 a11 = col1.x, a12 = col2.x, a21 = col1.y, a22 = col2.y;
		float32 det = a11 * a22 - a12 * a21;
		int n = 0;
		b2Vec2 x;

		
		if(b2Abs(det) <= (B2_FLT_EPSILON<<8))
		{
			n = 3;
			a11 = col1.x<<n; a12 = col2.x<<n;
			a21 = col1.y<<n; a22 = col2.y<<n;
			det = a11 * a22 - a12 * a21;
			b2Assert(det != 0.0f);
			det = float32(1) / det;
			x.x = (det * (a22 * b.x - a12 * b.y)) << n;
			x.y = (det * (a11 * b.y - a21 * b.x)) << n;
		} 
		else 
		{
			n = (b2Abs(det) >= 16.0) ? 4 : 0;
			b2Assert(det != 0.0f);
			det = float32(1<<n) / det;
			x.x = (det * (a22 * b.x - a12 * b.y)) >> n;
			x.y = (det * (a11 * b.y - a21 * b.x)) >> n;
		}

		return x;
	}

#else
	b2Mat22 Invert() const
	{
		float32 a = col1.x, b = col2.x, c = col1.y, d = col2.y;
		b2Mat22 B;
		float32 det = a * d - b * c;
		b2Assert(det != 0.0f);
		det = float32(1.0f) / det;
		B.col1.x =  det * d;	B.col2.x = -det * b;
		B.col1.y = -det * c;	B.col2.y =  det * a;
		return B;
	}

	/// Solve A * x = b, where b is a column vector. This is more efficient
	/// than computing the inverse in one-shot cases.
	b2Vec2 Solve(const b2Vec2& b) const
	{
		float32 a11 = col1.x, a12 = col2.x, a21 = col1.y, a22 = col2.y;
		float32 det = a11 * a22 - a12 * a21;
		b2Assert(det != 0.0f);
		det = 1.0f / det;
		b2Vec2 x;
		x.x = det * (a22 * b.x - a12 * b.y);
		x.y = det * (a11 * b.y - a21 * b.x);
		return x;
	}
#endif

	b2Vec2 col1, col2;
};

/// A transform contains translation and rotation. It is used to represent
/// the position and orientation of rigid frames.
struct b2XForm
{
	/// The default constructor does nothing (for performance).
	b2XForm() {}

	/// Initialize using a position vector and a rotation matrix.
	b2XForm(const b2Vec2& position, const b2Mat22& R) : position(position), R(R) {}

	/// Set this to the identity transform.
	void SetIdentity()
	{
		position.SetZero();
		R.SetIdentity();
	}

	b2Vec2 position;
	b2Mat22 R;
};

/// This describes the motion of a body/shape for TOI computation.
/// Shapes are defined with respect to the body origin, which may
/// no coincide with the center of mass. However, to support dynamics
/// we must interpolate the center of mass position.
struct b2Sweep
{
	/// Get the interpolated transform at a specific time.
	/// @param t the normalized time in [0,1].
	void GetXForm(b2XForm* xf, float32 t) const;

	/// Advance the sweep forward, yielding a new initial state.
	/// @param t the new initial time.
	void Advance(float32 t);

	b2Vec2 localCenter;	///< local center of mass position
	b2Vec2 c0, c;		///< center world positions
	float32 a0, a;		///< world angles
	float32 t0;			///< time interval = [t0,1], where t0 is in [0,1]
};


extern const b2Vec2 b2Vec2_zero;
extern const b2Mat22 b2Mat22_identity;
extern const b2XForm b2XForm_identity;

/// Peform the dot product on two vectors.
inline float32 b2Dot(const b2Vec2& a, const b2Vec2& b)
{
	return a.x * b.x + a.y * b.y;
}

/// Perform the cross product on two vectors. In 2D this produces a scalar.
inline float32 b2Cross(const b2Vec2& a, const b2Vec2& b)
{
	return a.x * b.y - a.y * b.x;
}

/// Perform the cross product on a vector and a scalar. In 2D this produces
/// a vector.
inline b2Vec2 b2Cross(const b2Vec2& a, float32 s)
{
	b2Vec2 v; v.Set(s * a.y, -s * a.x);
	return v;
}

/// Perform the cross product on a scalar and a vector. In 2D this produces
/// a vector.
inline b2Vec2 b2Cross(float32 s, const b2Vec2& a)
{
	b2Vec2 v; v.Set(-s * a.y, s * a.x);
	return v;
}

/// Multiply a matrix times a vector. If a rotation matrix is provided,
/// then this transforms the vector from one frame to another.
inline b2Vec2 b2Mul(const b2Mat22& A, const b2Vec2& v)
{
	b2Vec2 u;
	u.Set(A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y);
	return u;
}

/// Multiply a matrix transpose times a vector. If a rotation matrix is provided,
/// then this transforms the vector from one frame to another (inverse transform).
inline b2Vec2 b2MulT(const b2Mat22& A, const b2Vec2& v)
{
	b2Vec2 u;
	u.Set(b2Dot(v, A.col1), b2Dot(v, A.col2));
	return u;
}

/// Add two vectors component-wise.
inline b2Vec2 operator + (const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 v; v.Set(a.x + b.x, a.y + b.y);
	return v;
}

/// Subtract two vectors component-wise.
inline b2Vec2 operator - (const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 v; v.Set(a.x - b.x, a.y - b.y);
	return v;
}

inline b2Vec2 operator * (float32 s, const b2Vec2& a)
{
	b2Vec2 v; v.Set(s * a.x, s * a.y);
	return v;
}

inline bool operator == (const b2Vec2& a, const b2Vec2& b)
{
	return a.x == b.x && a.y == b.y;
}

inline float32 b2Distance(const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 c = a - b;
	return c.Length();
}

inline float32 b2DistanceSquared(const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 c = a - b;
	return b2Dot(c, c);
}

inline b2Mat22 operator + (const b2Mat22& A, const b2Mat22& B)
{
	b2Mat22 C;
	C.Set(A.col1 + B.col1, A.col2 + B.col2);
	return C;
}

// A * B
inline b2Mat22 b2Mul(const b2Mat22& A, const b2Mat22& B)
{
	b2Mat22 C;
	C.Set(b2Mul(A, B.col1), b2Mul(A, B.col2));
	return C;
}

// A^T * B
inline b2Mat22 b2MulT(const b2Mat22& A, const b2Mat22& B)
{
	b2Vec2 c1; c1.Set(b2Dot(A.col1, B.col1), b2Dot(A.col2, B.col1));
	b2Vec2 c2; c2.Set(b2Dot(A.col1, B.col2), b2Dot(A.col2, B.col2));
	b2Mat22 C;
	C.Set(c1, c2);
	return C;
}

inline b2Vec2 b2Mul(const b2XForm& T, const b2Vec2& v)
{
	return T.position + b2Mul(T.R, v);
}

inline b2Vec2 b2MulT(const b2XForm& T, const b2Vec2& v)
{
	return b2MulT(T.R, v - T.position);
}

inline b2Vec2 b2Abs(const b2Vec2& a)
{
	b2Vec2 b; b.Set(b2Abs(a.x), b2Abs(a.y));
	return b;
}

inline b2Mat22 b2Abs(const b2Mat22& A)
{
	b2Mat22 B;
	B.Set(b2Abs(A.col1), b2Abs(A.col2));
	return B;
}

template <typename T>
inline T b2Min(T a, T b)
{
	return a < b ? a : b;
}

inline b2Vec2 b2Min(const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 c;
	c.x = b2Min(a.x, b.x);
	c.y = b2Min(a.y, b.y);
	return c;
}

template <typename T>
inline T b2Max(T a, T b)
{
	return a > b ? a : b;
}

inline b2Vec2 b2Max(const b2Vec2& a, const b2Vec2& b)
{
	b2Vec2 c;
	c.x = b2Max(a.x, b.x);
	c.y = b2Max(a.y, b.y);
	return c;
}

template <typename T>
inline T b2Clamp(T a, T low, T high)
{
	return b2Max(low, b2Min(a, high));
}

inline b2Vec2 b2Clamp(const b2Vec2& a, const b2Vec2& low, const b2Vec2& high)
{
	return b2Max(low, b2Min(a, high));
}

template<typename T> inline void b2Swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

#define	RAND_LIMIT	32767

// Random number in range [-1,1]
inline float32 b2Random()
{
	float32 r = (float32)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = 2.0f * r - 1.0f;
	return r;
}

/// Random floating point number in range [lo, hi]
inline float32 b2Random(float32 lo, float32 hi)
{
	float32 r = (float32)(rand() & (RAND_LIMIT));
	r /= RAND_LIMIT;
	r = (hi - lo) * r + lo;
	return r;
}

/// "Next Largest Power of 2
/// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
/// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
/// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
/// largest power of 2. For a 32-bit value:"
inline uint32 b2NextPowerOfTwo(uint32 x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

inline bool b2IsPowerOfTwo(uint32 x)
{
	bool result = x > 0 && (x & (x - 1)) == 0;
	return result;
}

#endif
