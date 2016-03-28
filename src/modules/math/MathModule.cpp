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

// LOVE
#include "MathModule.h"
#include "common/Vector.h"
#include "common/b64.h"
#include "common/int.h"
#include "common/StringMap.h"
#include "BezierCurve.h"

// STL
#include <cmath>
#include <list>
#include <iostream>

using std::list;
using std::vector;
using love::Vertex;

namespace
{

static const char hexchars[] = "0123456789abcdef";

char *bytesToHex(const love::uint8 *src, size_t srclen, size_t &dstlen)
{
	dstlen = srclen * 2;

	if (dstlen == 0)
		return nullptr;

	char *dst = nullptr;
	try
	{
		dst = new char[dstlen + 1];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}

	for (size_t i = 0; i < srclen; i++)
	{
		love::uint8 b = src[i];
		dst[i * 2 + 0] = hexchars[b >> 4];
		dst[i * 2 + 1] = hexchars[b & 0xF];
	}

	dst[dstlen] = '\0';
	return dst;
}

love::uint8 nibble(char c)
{
	if (c >= '0' && c <= '9')
		return (love::uint8) (c - '0');

	if (c >= 'A' && c <= 'F')
		return (love::uint8) (c - 'A' + 0x0a);

	if (c >= 'a' && c <= 'f')
		return (love::uint8) (c - 'a' + 0x0a);

	return 0;
}

love::uint8 *hexToBytes(const char *src, size_t srclen, size_t &dstlen)
{
	if (srclen >= 2 && src[0] == '0' && (src[1] == 'x' || src[1] == 'X'))
	{
		src += 2;
		srclen -= 2;
	}

	dstlen = (srclen + 1) / 2;

	if (dstlen == 0)
		return nullptr;

	love::uint8 *dst = nullptr;
	try
	{
		dst = new love::uint8[dstlen];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}

	for (size_t i = 0; i < dstlen; i++)
	{
		dst[i] = nibble(src[i * 2]) << 4;

		if (i * 2 + 1 < srclen)
			dst[i] |= nibble(src[i * 2 + 1]);
	}

	return dst;
}

// check if an angle is oriented counter clockwise
inline bool is_oriented_ccw(const Vertex &a, const Vertex &b, const Vertex &c)
{
	// return det(b-a, c-a) >= 0
	return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) >= 0;
}

// check if a and b are on the same side of the line c->d
bool on_same_side(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d)
{
	float px = d.x - c.x, py = d.y - c.y;
	// return det(p, a-c) * det(p, b-c) >= 0
	float l = px * (a.y - c.y) - py * (a.x - c.x);
	float m = px * (b.y - c.y) - py * (b.x - c.x);
	return l * m >= 0;
}

// checks is p is contained in the triangle abc
inline bool point_in_triangle(const Vertex &p, const Vertex &a, const Vertex &b, const Vertex &c)
{
	return on_same_side(p,a, b,c) && on_same_side(p,b, a,c) && on_same_side(p,c, a,b);
}

// checks if any vertex in `vertices' is in the triangle abc.
bool any_point_in_triangle(const list<const Vertex *> &vertices, const Vertex &a, const Vertex &b, const Vertex &c)
{
	list<const Vertex *>::const_iterator it, end = vertices.end();
	for (it = vertices.begin(); it != end; ++it)
	{
		const Vertex *p = *it;
		if ((p != &a) && (p != &b) && (p != &c) && point_in_triangle(*p, a,b,c)) // oh god...
			return true;
	}

	return false;
}

inline bool is_ear(const Vertex &a, const Vertex &b, const Vertex &c, const list<const Vertex *> &vertices)
{
	return is_oriented_ccw(a,b,c) && !any_point_in_triangle(vertices, a,b,c);
}

} // anonymous namespace

namespace love
{
namespace math
{

vector<Triangle> triangulate(const vector<Vertex> &polygon)
{
	if (polygon.size() < 3)
		throw love::Exception("Not a polygon");
	else if (polygon.size() == 3)
		return vector<Triangle>(1, Triangle(polygon[0], polygon[1], polygon[2]));

	// collect list of connections and record leftmost item to check if the polygon
	// has the expected winding
	vector<size_t> next_idx(polygon.size()), prev_idx(polygon.size());
	size_t idx_lm = 0;
	for (size_t i = 0; i < polygon.size(); ++i)
	{
		const Vertex &lm = polygon[idx_lm], &p = polygon[i];
		if (p.x < lm.x || (p.x == lm.x && p.y < lm.y))
			idx_lm = i;
		next_idx[i] = i+1;
		prev_idx[i] = i-1;
	}
	next_idx[next_idx.size()-1] = 0;
	prev_idx[0] = prev_idx.size()-1;

	// check if the polygon has the expected winding and reverse polygon if needed
	if (!is_oriented_ccw(polygon[prev_idx[idx_lm]], polygon[idx_lm], polygon[next_idx[idx_lm]]))
		next_idx.swap(prev_idx);

	// collect list of concave polygons
	list<const Vertex *> concave_vertices;
	for (size_t i = 0; i < polygon.size(); ++i)
	{
		if (!is_oriented_ccw(polygon[prev_idx[i]], polygon[i], polygon[next_idx[i]]))
			concave_vertices.push_back(&polygon[i]);
	}

	// triangulation according to kong
	vector<Triangle> triangles;
	size_t n_vertices = polygon.size();
	size_t current = 1, skipped = 0, next, prev;
	while (n_vertices > 3)
	{
		next = next_idx[current];
		prev = prev_idx[current];
		const Vertex &a = polygon[prev], &b = polygon[current], &c = polygon[next];
		if (is_ear(a,b,c, concave_vertices))
		{
			triangles.push_back(Triangle(a,b,c));
			next_idx[prev] = next;
			prev_idx[next] = prev;
			concave_vertices.remove(&b);
			--n_vertices;
			skipped = 0;
		}
		else if (++skipped > n_vertices)
		{
			throw love::Exception("Cannot triangulate polygon.");
		}
		current = next;
	}
	next = next_idx[current];
	prev = prev_idx[current];
	triangles.push_back(Triangle(polygon[prev], polygon[current], polygon[next]));

	return triangles;
}

bool isConvex(const std::vector<Vertex> &polygon)
{
	if (polygon.size() < 3)
		return false;

	// a polygon is convex if all corners turn in the same direction
	// turning direction can be determined using the cross-product of
	// the forward difference vectors
	size_t i = polygon.size() - 2, j = polygon.size() - 1, k = 0;
	Vector p(polygon[j].x - polygon[i].x, polygon[j].y - polygon[i].y);
	Vector q(polygon[k].x - polygon[j].x, polygon[k].y - polygon[j].y);
	float winding = p ^ q;

	while (k+1 < polygon.size())
	{
		i = j; j = k; k++;
		p.x = polygon[j].x - polygon[i].x;
		p.y = polygon[j].y - polygon[i].y;
		q.x = polygon[k].x - polygon[j].x;
		q.y = polygon[k].y - polygon[j].y;

		if ((p^q) * winding < 0)
			return false;
	}
	return true;
}

/**
 * http://en.wikipedia.org/wiki/SRGB#The_reverse_transformation
 **/
float gammaToLinear(float c)
{
	if (c <= 0.04045f)
		return c / 12.92f;
	else
		return powf((c + 0.055f) / 1.055f, 2.4f);
}

/**
 * http://en.wikipedia.org/wiki/SRGB#The_forward_transformation_.28CIE_xyY_or_CIE_XYZ_to_sRGB.29
 **/
float linearToGamma(float c)
{
	if (c <= 0.0031308f)
		return c * 12.92f;
	else
		return 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
}

CompressedData *compress(Compressor::Format format, love::Data *rawdata, int level)
{
	return compress(format, (const char *) rawdata->getData(), rawdata->getSize(), level);
}

CompressedData *compress(Compressor::Format format, const char *rawbytes, size_t rawsize, int level)
{
	Compressor *compressor = Compressor::getCompressor(format);

	if (compressor == nullptr)
		throw love::Exception("Invalid compression format.");

	size_t compressedsize = 0;
	char *cbytes = compressor->compress(format, rawbytes, rawsize, level, compressedsize);

	CompressedData *data = nullptr;

	try
	{
		data = new CompressedData(format, cbytes, compressedsize, rawsize, true);
	}
	catch (love::Exception &)
	{
		delete[] cbytes;
		throw;
	}

	return data;
}

char *decompress(CompressedData *data, size_t &decompressedsize)
{
	size_t rawsize = data->getDecompressedSize();

	char *rawbytes = decompress(data->getFormat(), (const char *) data->getData(),
	                            data->getSize(), rawsize);

	decompressedsize = rawsize;
	return rawbytes;
}

char *decompress(Compressor::Format format, const char *cbytes, size_t compressedsize, size_t &rawsize)
{
	Compressor *compressor = Compressor::getCompressor(format);

	if (compressor == nullptr)
		throw love::Exception("Invalid compression format.");

	return compressor->decompress(format, cbytes, compressedsize, rawsize);
}

char *encode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen, size_t linelen)
{
	switch (format)
	{
	case ENCODE_BASE64:
	default:
		return b64_encode(src, srclen, linelen, dstlen);
	case ENCODE_HEX:
		return bytesToHex((const uint8 *) src, srclen, dstlen);
	}
}

char *decode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen)
{
	switch (format)
	{
	case ENCODE_BASE64:
	default:
		return b64_decode(src, srclen, dstlen);
	case ENCODE_HEX:
		return (char *) hexToBytes(src, srclen, dstlen);
	}
}

Math Math::instance;

Math::Math()
	: rng()
{
	// prevent the runtime from free()-ing this
	retain();
}

Math::~Math()
{
}

RandomGenerator *Math::newRandomGenerator()
{
	return new RandomGenerator();
}

BezierCurve *Math::newBezierCurve(const vector<Vector> &points)
{
	return new BezierCurve(points);
}

static StringMap<EncodeFormat, ENCODE_MAX_ENUM>::Entry encoderEntries[] =
{
	{ "base64", ENCODE_BASE64 },
	{ "hex",    ENCODE_HEX },
};

static StringMap<EncodeFormat, ENCODE_MAX_ENUM> encoders(encoderEntries, sizeof(encoderEntries));

bool getConstant(const char *in, EncodeFormat &out)
{
	return encoders.find(in, out);
}

bool getConstant(EncodeFormat in, const char *&out)
{
	return encoders.find(in, out);
}

} // math
} // love
