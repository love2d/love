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

#ifndef LOVE_MATH_MODMATH_H
#define LOVE_MATH_MODMATH_H

#include "RandomGenerator.h"
#include "CompressedData.h"
#include "Compressor.h"

// LOVE
#include "common/Module.h"
#include "common/math.h"
#include "common/Vector.h"
#include "common/int.h"

// Noise
#include "libraries/noise1234/noise1234.h"
#include "libraries/noise1234/simplexnoise1234.h"

// STL
#include <vector>

namespace love
{
namespace math
{

class BezierCurve;

enum EncodeFormat
{
	ENCODE_BASE64,
	ENCODE_HEX,
	ENCODE_MAX_ENUM
};

/**
 * Triangulate a simple polygon.
 *
 * @param polygon Polygon to triangulate. Must not intersect itself.
 * @return List of triangles the polygon is composed of.
 **/
std::vector<Triangle> triangulate(const std::vector<Vertex> &polygon);

/**
 * Checks whether a polygon is convex.
 *
 * @param polygon Polygon to test.
 * @return True if the polygon is convex, false otherwise.
 **/
bool isConvex(const std::vector<Vertex> &polygon);

/**
 * Converts a value from the sRGB (gamma) colorspace to linear RGB.
 **/
float gammaToLinear(float c);

/**
 * Converts a value from linear RGB to the sRGB (gamma) colorspace.
 **/
float linearToGamma(float c);

/**
 * Calculate noise for the specified coordinate(s).
 *
 * @return Noise value in the range of [0, 1].
 **/
static float noise1(float x);
static float noise2(float x, float y);
static float noise3(float x, float y, float z);
static float noise4(float x, float y, float z, float w);

/**
 * Compresses a block of memory using the given compression format.
 *
 * @param format The compression format to use.
 * @param rawdata The data to compress.
 * @param level The amount of compression to apply (between 0 and 9.)
 *              A value of -1 indicates the default amount of compression.
 *              Specific formats may not use every level.
 * @return The newly compressed data.
 **/
CompressedData *compress(Compressor::Format format, Data *rawdata, int level = -1);
CompressedData *compress(Compressor::Format format, const char *rawbytes, size_t rawsize, int level = -1);

/**
 * Decompresses existing compressed data into raw bytes.
 *
 * @param[in] data The compressed data to decompress.
 * @param[out] decompressedsize The size in bytes of the decompressed data.
 * @return The newly decompressed data (allocated with new[]).
 **/
char *decompress(CompressedData *data, size_t &decompressedsize);

/**
 * Decompresses existing compressed data into raw bytes.
 *
 * @param[in] format The compression format the data is in.
 * @param[in] cbytes The compressed data to decompress.
 * @param[in] compressedsize The size in bytes of the compressed data.
 * @param[in,out] rawsize On input, the size in bytes of the original
 *               uncompressed data, or 0 if unknown. On return, the size in
 *               bytes of the newly decompressed data.
 * @return The newly decompressed data (allocated with new[]).
 **/
char *decompress(Compressor::Format format, const char *cbytes, size_t compressedsize, size_t &rawsize);

char *encode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen, size_t linelen = 0);
char *decode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen);

bool getConstant(const char *in, EncodeFormat &out);
bool getConstant(EncodeFormat in, const char *&out);


class Math : public Module
{
private:

	RandomGenerator rng;

public:

	virtual ~Math();

	RandomGenerator *getRandomGenerator()
	{
		return &rng;
	}

	/**
	 * Create a new random number generator.
	 **/
	RandomGenerator *newRandomGenerator();

	/**
	 * Creates a new bezier curve.
	 **/
	BezierCurve *newBezierCurve(const std::vector<Vector> &points);

	// Implements Module.
	virtual ModuleType getModuleType() const
	{
		return M_MATH;
	}

	virtual const char *getName() const
	{
		return "love.math";
	}

	static Math instance;

private:

	Math();

}; // Math


static inline float noise1(float x)
{
	return SimplexNoise1234::noise(x) * 0.5f + 0.5f;
}

static inline float noise2(float x, float y)
{
	return SimplexNoise1234::noise(x, y) * 0.5f + 0.5f;
}

// Perlin noise is used instead of Simplex noise in the 3D and 4D cases to avoid
// patent issues.

static inline float noise3(float x, float y, float z)
{
	return Noise1234::noise(x, y, z) * 0.5f + 0.5f;
}

static inline float noise4(float x, float y, float z, float w)
{
	return Noise1234::noise(x, y, z, w) * 0.5f + 0.5f;
}

} // math
} // love

#endif
