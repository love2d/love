/**
 * Copyright (c) 2006-2018 LOVE Development Team
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

#ifndef LOVE_VECTOR_H
#define LOVE_VECTOR_H

// STD
#include <cmath>

namespace love
{

struct Vector2
{
	float x, y;

	Vector2()
		: x(0.0f), y(0.0f)
	{}

	Vector2(float x, float y)
		: x(x), y(y)
	{}

	Vector2(const Vector2 &v)
		: x(v.x), y(v.y)
	{}

	/**
	 * Gets the length of the Vector.
	 **/
	float getLength() const;
	float getLengthSquare() const;

	/**
	 * Normalizes the Vector.
	 * @param length Desired length of the vector.
	 * @return The old length of the Vector.
	 **/
	float normalize(float length = 1.0);

	/**
	 * Gets a vector perpendicular to the Vector.
	 * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
	 * @return A normal to the Vector.
	 **/
	Vector2 getNormal() const;

	/**
	 * Gets a vector perpendicular to the Vector.
	 * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
	 * @param scale factor to apply.
	 * @return A normal to the Vector.
	 **/
	Vector2 getNormal(float scale) const;

	static inline float dot(const Vector2 &a, const Vector2 &b);
	static inline float cross(const Vector2 &a, const Vector2 &b);

	/**
	 * Adds a Vector to this Vector.
	 **/
	Vector2 operator + (const Vector2 &v) const;

	/**
	 * Substracts a Vector from this Vector.
	 **/
	Vector2 operator - (const Vector2 &v) const;

	/**
	 * Component-wise multiplies the Vector by a scalar.
	 **/
	Vector2 operator * (float s) const;

	/**
	 * Component-wise divides the Vector by a scalar.
	 **/
	Vector2 operator / (float s) const;

	/**
	 * Component-wise negates the Vector.
	 **/
	Vector2 operator - () const;

	/**
	 * Adds a Vector to this Vector, and also saves changes in the first Vector.
	 **/
	void operator += (const Vector2 &v);

	/**
	 * Subtracts a Vector to this Vector, and also saves changes in the first Vector.
	 **/
	void operator -= (const Vector2 &v);

	/**
	 * Resizes the Vector, and also saves changes in the first Vector.
	 **/
	void operator *= (float s);

	/**
	 * Resizes the Vector, and also saves changes in the first Vector.
	 **/
	void operator /= (float s);

	bool operator == (const Vector2 &v) const;
	bool operator != (const Vector2 &v) const;

}; // Vector2


struct Vector3
{
	float x, y, z;

	Vector3()
		: x(0.0f), y(0.0f), z(0.0f)
	{}

	Vector3(float x, float y, float z)
		: x(x), y(y), z(z)
	{}

	Vector3(const Vector2 &v)
		: x(v.x), y(v.y), z(0.0f)
	{}

	/**
	 * Gets the length of the Vector.
	 **/
	float getLength() const;
	float getLengthSquare() const;

	/**
	 * Normalizes the Vector.
	 * @param length Desired length of the vector.
	 * @return The old length of the Vector.
	 **/
	float normalize(float length = 1.0);

	static inline float dot(const Vector3 &a, const Vector3 &b);
	static inline Vector3 cross(const Vector3 &a, const Vector3 &b);

	/**
	 * Adds a Vector to this Vector.
	 **/
	Vector3 operator + (const Vector3 &v) const;

	/**
	 * Substracts a Vector from this Vector.
	 **/
	Vector3 operator - (const Vector3 &v) const;

	/**
	 * Component-wise multiplies the Vector by a scalar.
	 **/
	Vector3 operator * (float s) const;

	/**
	 * Component-wise divides the Vector by a scalar.
	 **/
	Vector3 operator / (float s) const;

	/**
	 * Component-wise negates the Vector.
	 **/
	Vector3 operator - () const;

	/**
	 * Adds a Vector to this Vector, and also saves changes in the first Vector.
	 **/
	void operator += (const Vector3 &v);

	/**
	 * Subtracts a Vector to this Vector, and also saves changes in the first Vector.
	 **/
	void operator -= (const Vector3 &v);

	/**
	 * Resizes the Vector, and also saves changes in the first Vector.
	 **/
	void operator *= (float s);

	/**
	 * Resizes the Vector, and also saves changes in the first Vector.
	 **/
	void operator /= (float s);

	bool operator == (const Vector3 &v) const;
	bool operator != (const Vector3 &v) const;

}; // Vector3


inline float Vector2::getLength() const
{
	return sqrtf(x*x + y*y);
}

inline float Vector2::getLengthSquare() const
{
	return x*x + y*y;
}

inline Vector2 Vector2::getNormal() const
{
	return Vector2(-y, x);
}

inline Vector2 Vector2::getNormal(float scale) const
{
	return Vector2(-y * scale, x * scale);
}

inline float Vector2::dot(const Vector2 &a, const Vector2 &b)
{
	return a.x * b.x + a.y * b.y;
}

inline float Vector2::cross(const Vector2 &a, const Vector2 &b)
{
	return a.x * b.y - a.y * b.x;
}

inline float Vector2::normalize(float length)
{
	float length_current = getLength();

	if (length_current > 0)
		(*this) *= length / length_current;

	return length_current;
}

inline Vector2 Vector2::operator + (const Vector2 &v) const
{
	return Vector2(x + v.x, y + v.y);
}

inline Vector2 Vector2::operator - (const Vector2 &v) const
{
	return Vector2(x - v.x, y - v.y);
}

inline Vector2 Vector2::operator * (float s) const
{
	return Vector2(x*s, y*s);
}

inline Vector2 Vector2::operator / (float s) const
{
	float invs = 1.0f / s;
	return Vector2(x*invs, y*invs);
}

inline Vector2 Vector2::operator - () const
{
	return Vector2(-x, -y);
}

inline void Vector2::operator += (const Vector2 &v)
{
	x += v.x;
	y += v.y;
}

inline void Vector2::operator -= (const Vector2 &v)
{
	x -= v.x;
	y -= v.y;
}

inline void Vector2::operator *= (float s)
{
	x *= s;
	y *= s;
}

inline void Vector2::operator /= (float s)
{
	float invs = 1.0f / s;
	x *= invs;
	y *= invs;
}

inline bool Vector2::operator == (const Vector2 &v) const
{
	return x == v.x && y == v.y;
}

inline bool Vector2::operator != (const Vector2 &v) const
{
	return x != v.x || y != v.y;
}


inline float Vector3::getLength() const
{
	return sqrtf(x*x + y*y + z*z);
}

inline float Vector3::getLengthSquare() const
{
	return x*x + y*y + z*z;
}

inline float Vector3::dot(const Vector3 &a, const Vector3 &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 Vector3::cross(const Vector3 &a, const Vector3 &b)
{
	return Vector3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

inline float Vector3::normalize(float length)
{
	float length_current = getLength();

	if (length_current > 0)
		(*this) *= length / length_current;

	return length_current;
}

inline Vector3 Vector3::operator + (const Vector3 &v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

inline Vector3 Vector3::operator - (const Vector3 &v) const
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

inline Vector3 Vector3::operator * (float s) const
{
	return Vector3(x*s, y*s, z*s);
}

inline Vector3 Vector3::operator / (float s) const
{
	float invs = 1.0f / s;
	return Vector3(x*invs, y*invs, z*invs);
}

inline Vector3 Vector3::operator - () const
{
	return Vector3(-x, -y, -z);
}

inline void Vector3::operator += (const Vector3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

inline void Vector3::operator -= (const Vector3 &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

inline void Vector3::operator *= (float s)
{
	x *= s;
	y *= s;
	z *= s;
}

inline void Vector3::operator /= (float s)
{
	float invs = 1.0f / s;
	x *= invs;
	y *= invs;
	z *= invs;
}

inline bool Vector3::operator == (const Vector3 &v) const
{
	return x == v.x && y == v.y && z == v.z;
}

inline bool Vector3::operator != (const Vector3 &v) const
{
	return x != v.x || y != v.y || z != v.z;
}

} //love

#endif// LOVE_VECTOR_H
