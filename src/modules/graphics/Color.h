/**
 * Copyright (c) 2006-2011 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_COLOR_H
#define LOVE_GRAPHICS_COLOR_H

namespace love
{
namespace graphics
{

template <typename T>
struct ColorT
{
	T r;
	T g;
	T b;
	T a;

	ColorT() : r(0), g(0), b(0), a(0) {}
	ColorT(T r_, T g_, T b_, T a_) : r(r_), g(g_), b(b_), a(a_) {}
	void set(T r_, T g_, T b_, T a_)
	{
		r = r_;
		g = g_;
		b = b_;
		a = a_;
	}

	bool operator==(const ColorT<T> &other) const;
	bool operator!=(const ColorT<T> &other) const;

	ColorT<T> operator+=(const ColorT<T> &other);
	ColorT<T> operator*=(T s);
	ColorT<T> operator/=(T s);
};

template <typename T>
bool ColorT<T>::operator==(const ColorT<T> &other) const
{
	return r == other.r && g == other.g && b == other.b && a == other.a;
}

template <typename T>
bool ColorT<T>::operator!=(const ColorT<T> &other) const
{
	return !(operator==(other));
}

template <typename T>
ColorT<T> ColorT<T>::operator+=(const ColorT<T> &other)
{
	r += other.r;
	g += other.g;
	b += other.b;
	a += other.a;
	return *this;
}

template <typename T>
ColorT<T> ColorT<T>::operator*=(T s)
{
	r *= s;
	g *= s;
	b *= s;
	a *= s;
	return *this;
}

template <typename T>
ColorT<T> ColorT<T>::operator/=(T s)
{
	r /= s;
	g /= s;
	b /= s;
	a /= s;
	return *this;
}

template <typename T>
ColorT<T> operator+(const ColorT<T> &a, const ColorT<T> &b)
{
	ColorT<T> tmp(a);
	return tmp += b;
}

template <typename T>
ColorT<T> operator*(const ColorT<T> &a, T s)
{
	ColorT<T> tmp(a);
	return tmp *= s;
}

template <typename T>
ColorT<T> operator/(const ColorT<T> &a, T s)
{
	ColorT<T> tmp(a);
	return tmp /= s;
}

typedef ColorT<unsigned char> Color;
typedef ColorT<float> Colorf;

} // graphics
} // love

#endif // LOVE_GRAPHICS_COLOR_H
