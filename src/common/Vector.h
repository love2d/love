/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

// LOVE
#include "Matrix.h"

namespace love
{

/**
 * 2D Vector class.
 *
 * @author Anders Ruud
 * @date 2006-05-13
 **/
class Vector
{
public:

	// The components.
	float x, y;

	/**
	 * Creates a new (1,1) Vector.
	 **/
	Vector();

	/**
	 * Creates a new Vector.
	 * @param x The x position/dimension.
	 * @param y The y position/dimension.
	 **/
	Vector(float x, float y);

	/**
	 * Gets the length of the Vector.
	 * @return The length of the Vector.
	 *
	 * This method requires sqrtf() and should be used
	 * carefully.
	 **/
	float getLength() const;

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
	Vector getNormal() const;

	/**
	 * Gets a vector perpendicular to the Vector.
	 * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
	 * @param scale factor to apply.
	 * @return A normal to the Vector.
	 **/
	Vector getNormal(float scale) const;

	/**
	 * Adds a Vector to this Vector.
	 * @param v The Vector we want to add to this Vector.
	 * @return The resulting Vector.
	 **/
	Vector operator + (const Vector &v) const;

	/**
	 * Substracts a Vector to this Vector.
	 * @param v The Vector we want to subtract to this Vector.
	 * @return The resulting Vector.
	 **/
	Vector operator - (const Vector &v) const;

	/**
	 * Resizes a Vector by a scalar.
	 * @param s The scalar with which to resize the Vector.
	 * @return The resulting Vector.
	 **/
	Vector operator * (float s) const;

	/**
	 * Resizes a Vector by a scalar.
	 * @param s The scalar with which to resize the Vector.
	 * @return The resulting Vector.
	 **/
	Vector operator / (float s) const;

	/**
	 * Reverses the Vector.
	 * @return The reversed Vector.
	 **/
	Vector operator - () const;

	/**
	 * Adds a Vector to this Vector, and also saves changes in the first Vector.
	 * @param v The Vector we want to add to this Vector.
	 **/
	void operator += (const Vector &v);

	/**
	 * Subtracts a Vector to this Vector, and also saves changes in the first Vector.
	 * @param v The Vector we want to subtract to this Vector.
	 **/
	void operator -= (const Vector &v);

	/**
	 * Resizes the Vector, and also saves changes in the first Vector.
	 * @param s The scalar by which we want to resize the Vector.
	 **/
	void operator *= (float s);

	/**
	 * Resizes the Vector, and also saves changes in the first Vector.
	 * @param s The scalar by which we want to resize the Vector.
	 **/
	void operator /= (float s);

	/**
	 * Calculates the dot product of two Vectors.
	 * @return The dot product of the two Vectors.
	 **/
	float operator * (const Vector &v) const;

	/**
	 * Calculates the cross product of two Vectors.
	 * @return The cross product of the two Vectors.
	 **/
	float operator ^ (const Vector &v) const;

	bool operator == (const Vector &v) const;

	bool operator < (const Vector &v) const;
	/**
	 * Gets the x value of the Vector.
	 * @return The x value of the Vector.
	 **/
	float getX() const;

	/**
	 * Gets the x value of the Vector.
	 * @return The x value of the Vector.
	 **/
	float getY() const;

	/**
	 * Sets the x value of the Vector.
	 * @param x The x value of the Vector.
	 **/
	void setX(float x);

	/**
	 * Sets the x value of the Vector.
	 * @param y The x value of the Vector.
	 **/
	void setY(float y);

};

inline float Vector::getLength() const
{
	return sqrtf(x*x + y*y);
}

inline Vector Vector::getNormal() const
{
	return Vector(-y, x);
}

inline Vector Vector::getNormal(float scale) const
{
	return Vector(-y * scale, x * scale);
}

inline float Vector::normalize(float length)
{

	float length_current = getLength();

	if (length_current > 0)
		(*this) *= length / length_current;

	return length_current;
}

/**
 * Inline methods must have body in header.
 **/

inline Vector::Vector()
	: x(0.0f)
	, y(0.0f)
{
}

inline Vector::Vector(float x, float y)
	: x(x)
	, y(y)
{
}

inline Vector Vector::operator + (const Vector &v) const
{
	return Vector(x + v.x, y + v.y);
}

inline Vector Vector::operator - (const Vector &v) const
{
	return Vector(x - v.getX(), y - v.getY());
}

inline Vector Vector::operator * (float s) const
{
	return Vector(x*s, y*s);
}

inline Vector Vector::operator / (float s) const
{
	return Vector(x/s, y/s);
}

inline Vector Vector::operator - () const
{
	return Vector(-x, -y);
}

inline void Vector::operator += (const Vector &v)
{
	x += v.getX();
	y += v.getY();
}

inline void Vector::operator -= (const Vector &v)
{
	x -= v.getX();
	y -= v.getY();
}

inline void Vector::operator *= (float s)
{
	x *= s;
	y *= s;
}

inline void Vector::operator /= (float s)
{
	x /= s;
	y /= s;
}

inline float Vector::operator * (const Vector &v) const
{
	return x * v.getX() + y * v.getY();
}

inline float Vector::operator ^ (const Vector &v) const
{
	return x * v.getY() - y * v.getX();
}

inline bool Vector::operator == (const Vector &v) const
{
	return getLength() == v.getLength();
}

inline bool Vector::operator < (const Vector &v) const
{
	return getLength() < v.getLength();
}

/**
 * Accessor methods
 **/

inline float Vector::getX() const
{
	return x;
}

inline float Vector::getY() const
{
	return y;
}

inline void Vector::setX(float x)
{
	this->x = x;
}

inline void Vector::setY(float y)
{
	this->y = y;
}

} //love

#endif// LOVE_VECTOR_H
