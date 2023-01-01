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

#ifndef LOVE_VECTOR_H
#define LOVE_VECTOR_H

// STD
#include <cmath>

namespace love
{

// All math operators are component-wise.
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

	float getLength() const;
	float getLengthSquare() const;

	/**
	 * Normalizes the Vector.
	 * @param length Desired length of the vector.
	 **/
	void normalize(float length = 1.0f);

	/**
	 * Gets a vector perpendicular to the Vector.
	 * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
	 **/
	Vector2 getNormal() const;

	/**
	 * Gets a vector perpendicular to the Vector.
	 * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
	 **/
	Vector2 getNormal(float scale) const;

	static inline float dot(const Vector2 &a, const Vector2 &b);
	static inline float cross(const Vector2 &a, const Vector2 &b);

	Vector2 operator + (const Vector2 &v) const;
	Vector2 operator - (const Vector2 &v) const;

	Vector2 operator * (float s) const;
	Vector2 operator / (float s) const;

	Vector2 operator - () const;

	void operator += (const Vector2 &v);
	void operator -= (const Vector2 &v);

	void operator *= (float s);
	void operator /= (float s);

	bool operator == (const Vector2 &v) const;
	bool operator != (const Vector2 &v) const;

}; // Vector2


// All math operators are component-wise.
struct Vector3
{
	float x, y, z;

	Vector3()
		: x(0.0f), y(0.0f), z(0.0f)
	{}

	Vector3(float x, float y, float z)
		: x(x), y(y), z(z)
	{}

	Vector3(const Vector2 &v, float z = 0.0f)
		: x(v.x), y(v.y), z(z)
	{}

	float getLength() const;
	float getLengthSquare() const;

	/**
	 * Normalizes the Vector.
	 * @param length Desired length of the vector.
	 **/
	void normalize(float length = 1.0);

	static inline float dot(const Vector3 &a, const Vector3 &b);
	static inline Vector3 cross(const Vector3 &a, const Vector3 &b);

	Vector3 operator + (const Vector3 &v) const;
	Vector3 operator - (const Vector3 &v) const;

	Vector3 operator * (float s) const;
	Vector3 operator / (float s) const;

	Vector3 operator - () const;

	void operator += (const Vector3 &v);
	void operator -= (const Vector3 &v);

	void operator *= (float s);
	void operator /= (float s);

	bool operator == (const Vector3 &v) const;
	bool operator != (const Vector3 &v) const;

}; // Vector3


// All math operators are component-wise.
struct Vector4
{
	float x, y, z, w;

	Vector4()
		: x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{}

	Vector4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)
	{}

	Vector4(const Vector2 &v, float z = 0.0f, float w = 0.0f)
		: x(v.x), y(v.y), z(z), w(w)
	{}

	Vector4(const Vector3 &v, float w = 0.0f)
		: x(v.x), y(v.y), z(v.z), w(w)
	{}

	float getLength() const;
	float getLengthSquare() const;

	/**
	 * Normalizes the Vector.
	 * @param length Desired length of the vector.
	 **/
	void normalize(float length = 1.0);

	static inline float dot(const Vector4 &a, const Vector4 &b);

	Vector4 operator + (const Vector4 &v) const;
	Vector4 operator - (const Vector4 &v) const;

	Vector4 operator * (float s) const;
	Vector4 operator / (float s) const;

	Vector4 operator - () const;

	void operator += (const Vector4 &v);
	void operator -= (const Vector4 &v);

	void operator *= (float s);
	void operator /= (float s);

	bool operator == (const Vector4 &v) const;
	bool operator != (const Vector4 &v) const;

}; // Vector4


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

inline void Vector2::normalize(float length)
{
	float length_current = getLength();
	if (length_current > 0)
	{
		float m = length / length_current;
		x *= m;
		y *= m;
	}
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

inline void Vector3::normalize(float length)
{
	float length_current = getLength();
	if (length_current > 0)
	{
		float m = length / length_current;
		x *= m;
		y *= m;
		z *= m;
	}
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


inline float Vector4::getLength() const
{
	return sqrtf(x*x + y*y + z*z + w*w);
}

inline float Vector4::getLengthSquare() const
{
	return x*x + y*y + z*z + w*w;
}

inline float Vector4::dot(const Vector4 &a, const Vector4 &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline void Vector4::normalize(float length)
{
	float length_current = getLength();
	if (length_current > 0)
	{
		float m = length / length_current;
		x *= m;
		y *= m;
		z *= m;
		w *= m;
	}
}

inline Vector4 Vector4::operator + (const Vector4 &v) const
{
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

inline Vector4 Vector4::operator - (const Vector4 &v) const
{
	return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

inline Vector4 Vector4::operator * (float s) const
{
	return Vector4(x*s, y*s, z*s, w*s);
}

inline Vector4 Vector4::operator / (float s) const
{
	float invs = 1.0f / s;
	return Vector4(x*invs, y*invs, z*invs, w*invs);
}

inline Vector4 Vector4::operator - () const
{
	return Vector4(-x, -y, -z, -w);
}

inline void Vector4::operator += (const Vector4 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

inline void Vector4::operator -= (const Vector4 &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

inline void Vector4::operator *= (float s)
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
}

inline void Vector4::operator /= (float s)
{
	float invs = 1.0f / s;
	x *= invs;
	y *= invs;
	z *= invs;
	w *= invs;
}

inline bool Vector4::operator == (const Vector4 &v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

inline bool Vector4::operator != (const Vector4 &v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}

} //love

#endif// LOVE_VECTOR_H
