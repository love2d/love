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

#include "HashFunction.h"

// FIXME: Probably trivial by having tole and tobe functions, which can be ifdeffed to being identity functions
#ifdef LOVE_BIG_ENDIAN
#	error Hashing not yet implemented for big endian
#endif

namespace love
{
namespace data
{

namespace
{
namespace impl
{

inline uint32 leftrot(uint32 x, uint8 amount)
{
	return (x << amount) | (x >> (32 - amount));
}

inline uint32 rightrot(uint32 x, uint8 amount)
{
	return (x >> amount) | (x << (32 - amount));
}

inline uint64 rightrot(uint64 x, uint8 amount)
{
	return (x >> amount) | (x << (64 - amount));
}

// Extend the value of `a` to make it a multiple of `n`.
inline uint64 extend_multiple(uint64 a, uint64 n)
{
	uint64 r = a % n;
	return r == 0 ? a : a + (n-r);
}

/**
 * The following implementation is based on the pseudocode provided by multiple
 * authors on wikipedia: https://en.wikipedia.org/wiki/MD5
 * The pseudocode is licensed under the CC-BY-SA license, but no authorship
 * information is present. I believe this note, and the zlib license of this
 * project satisfy the conditions of the license.
 **/
class MD5 : public HashFunction
{
private:
	static const uint8 shifts[64];
	static const uint32 constants[64];

public:
	bool isSupported(Function function) const override
	{
		return function == FUNCTION_MD5;
	}

	void hash(Function function, const char *input, uint64 length, Value &output) const override
	{
		if (function != FUNCTION_MD5)
			throw love::Exception("Hash function not supported by MD5 implementation");

		uint32 a0 = 0x67452301;
		uint32 b0 = 0xefcdab89;
		uint32 c0 = 0x98badcfe;
		uint32 d0 = 0x10325476;

		// Compute final padded length, accounting for the appended bit (byte) and size
		uint64 paddedLength = extend_multiple(length + 1 + 8, 64);

		uint32 *padded = new uint32[paddedLength / 4];
		memcpy(padded, input, length);
		memset(((uint8*)padded) + length, 0, paddedLength - length);
		*(((uint8*)padded) + length) = 0x80; // append bit

		// Append length in bits
		uint64 bit_length = length * 8;
		memcpy(((uint8*)padded) + paddedLength - 8, &bit_length, 8);

		// Process chunks
		for (uint64 i = 0; i < paddedLength/4; i += 16)
		{
			uint32 *chunk = &padded[i];

			uint32 A = a0;
			uint32 B = b0;
			uint32 C = c0;
			uint32 D = d0;
			uint32 F;
			uint32 g;

			for (int j = 0; j < 64; j++)
			{
				if (j < 16)
				{
					F = (B & C) | (~B & D);
					g = j;
				}
				else if (j < 32)
				{
					F = (D & B) | (~D & C);
					g = (5*j + 1) % 16;
				}
				else if (j < 48)
				{
					F = B ^ C ^ D;
					g = (3*j + 5) % 16;
				}
				else
				{
					F = C ^ (B | ~D);
					g = (7*j) % 16;
				}

				uint32 temp = D;
				D = C;
				C = B;
				B += leftrot(A + F + constants[j] + chunk[g], shifts[j]);
				A = temp;
			}

			a0 += A;
			b0 += B;
			c0 += C;
			d0 += D;
		}

		delete[] padded;

		memcpy(&output.data[ 0], &a0, 4);
		memcpy(&output.data[ 4], &b0, 4);
		memcpy(&output.data[ 8], &c0, 4);
		memcpy(&output.data[12], &d0, 4);
		output.size = 16;
	}
} md5;

const uint8 MD5::shifts[64] = {
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

const uint32 MD5::constants[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

/**
 * The following implementation was based on the text, not the code listings,
 * in RFC3174. I believe this means no copyright other than that of the L�VE
 * Development Team applies.
 **/
class SHA1 : public HashFunction
{
public:
	bool isSupported(Function function) const override
	{
		return function == FUNCTION_SHA1;
	}

	void hash(Function function, const char *input, uint64 length, Value &output) const override
	{
		if (function != FUNCTION_SHA1)
			throw love::Exception("Hash function not supported by SHA1 implementation");

		uint32 intermediate[5] = {
			0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
		};

		// Compute final padded length, accounting for the appended bit (byte) and size
		uint64 paddedLength = extend_multiple(length + 1 + 8, 64);

		uint32 *padded = new uint32[paddedLength / 4];
		memcpy(padded, input, length);
		memset(((uint8*)padded) + length, 0, paddedLength - length);
		*(((uint8*)padded) + length) = 0x80; // append bit

		// Append length in bits (big endian)
		uint64 bit_length = length * 8;
		for (int i = 0; i < 8; ++i)
			*(((uint8*)padded) + (paddedLength - 8 + i)) = (bit_length >> (56 - i * 8)) & 0xFF;

		// Allocate our extended words
		uint32 words[80];

		for (uint64 i = 0; i < paddedLength/4; i += 16)
		{
			uint32 *chunk = &padded[i];
			for (int j = 0; j < 16; j++)
			{
				char *c = (char*) &words[j];
				c[0] = (chunk[j] >> 24) & 0xFF;
				c[1] = (chunk[j] >> 16) & 0xFF;
				c[2] = (chunk[j] >>  8) & 0xFF;
				c[3] = (chunk[j] >>  0) & 0xFF;
			}
			for (int j = 16; j < 80; j++)
				words[j] = leftrot(words[j-3] ^ words[j-8] ^ words[j-14] ^ words[j-16], 1);

			uint32 A = intermediate[0];
			uint32 B = intermediate[1];
			uint32 C = intermediate[2];
			uint32 D = intermediate[3];
			uint32 E = intermediate[4];

			for (int j = 0; j < 80; j++)
			{
				uint32 temp = leftrot(A, 5) + E + words[j];

				if (j < 20)
					temp += 0x5A827999 + ((B & C) | (~B & D));
				else if (j < 40)
					temp += 0x6ED9EBA1 + (B ^ C ^ D);
				else if (j < 60)
					temp += 0x8F1BBCDC + ((B & C) | (B & D) | (C & D));
				else
					temp += 0xCA62C1D6 + (B ^ C ^ D);

				E = D;
				D = C;
				C = leftrot(B, 30);
				B = A;
				A = temp;
			}

			intermediate[0] += A;
			intermediate[1] += B;
			intermediate[2] += C;
			intermediate[3] += D;
			intermediate[4] += E;
		}

		delete[] padded;

		for (int i = 0; i < 20; i += 4)
		{
			output.data[i+0] = (intermediate[i/4] >> 24) & 0xFF;
			output.data[i+1] = (intermediate[i/4] >> 16) & 0xFF;
			output.data[i+2] = (intermediate[i/4] >>  8) & 0xFF;
			output.data[i+3] = (intermediate[i/4] >>  0) & 0xFF;
		}

		output.size = 20;
	}
} sha1;

/**
 * This implementation was based on the description in RFC-6234.
 **/
// SHA-2: SHA-224 and SHA-256
class SHA256 : public HashFunction
{
private:
	static const uint32 initial224[8];
	static const uint32 initial256[8];
	static const uint32 constants[64];

public:
	bool isSupported(Function function) const override
	{
		return function == FUNCTION_SHA224 || function == FUNCTION_SHA256;
	}

	void hash(Function function, const char *input, uint64 length, Value &output) const override
	{
		if (!isSupported(function))
			throw love::Exception("Hash function not supported by SHA-224/SHA-256 implementation");

		// Compute final padded length, accounting for the appended bit (byte) and size
		uint64 paddedLength = extend_multiple(length + 1 + 8, 64);

		uint32 *padded = new uint32[paddedLength / 4];
		memcpy(padded, input, length);
		memset(((uint8*)padded) + length, 0, paddedLength - length);
		*(((uint8*)padded) + length) = 0x80; // append bit

		// Append length in bits (big endian)
		uint64 bit_length = length * 8;
		for (int i = 0; i < 8; ++i)
			*(((uint8*)padded) + (paddedLength - 8 + i)) = (bit_length >> (56 - i * 8)) & 0xFF;

		uint32 intermediate[8];
		if (function == FUNCTION_SHA224)
			memcpy(intermediate, initial224, sizeof(intermediate));
		else
			memcpy(intermediate, initial256, sizeof(intermediate));

		// Allocate our extended words
		uint32 words[64];

		for (uint64 i = 0; i < paddedLength/4; i += 16)
		{
			uint32 *chunk = &padded[i];
			for (int j = 0; j < 16; j++)
			{
				char *c = (char*) &words[j];
				c[0] = (chunk[j] >> 24) & 0xFF;
				c[1] = (chunk[j] >> 16) & 0xFF;
				c[2] = (chunk[j] >>  8) & 0xFF;
				c[3] = (chunk[j] >>  0) & 0xFF;
			}
			for (int j = 16; j < 64; j++)
			{
				words[j] = rightrot(words[j-2], 17) ^ rightrot(words[j-2], 19) ^ (words[j-2] >> 10);
				words[j] += rightrot(words[j-15], 7) ^ rightrot(words[j-15], 18) ^ (words[j-15] >> 3);
				words[j] += words[j-7] + words[j-16];
			}

			uint32 A = intermediate[0];
			uint32 B = intermediate[1];
			uint32 C = intermediate[2];
			uint32 D = intermediate[3];
			uint32 E = intermediate[4];
			uint32 F = intermediate[5];
			uint32 G = intermediate[6];
			uint32 H = intermediate[7];

			for (int j = 0; j < 64; j++)
			{
				uint32 temp1 = H + constants[j] + words[j];
				temp1 += rightrot(E, 6) ^ rightrot(E, 11) ^ rightrot(E, 25);
				temp1 += (E & F) ^ (~E & G);
				uint32 temp2 = rightrot(A, 2) ^ rightrot(A, 13) ^ rightrot(A, 22);
				temp2 += (A & B) ^ (A & C) ^ (B & C);

				H = G;
				G = F;
				F = E;
				E = D + temp1;
				D = C;
				C = B;
				B = A;
				A = temp1 + temp2;
			}

			intermediate[0] += A;
			intermediate[1] += B;
			intermediate[2] += C;
			intermediate[3] += D;
			intermediate[4] += E;
			intermediate[5] += F;
			intermediate[6] += G;
			intermediate[7] += H;
		}

		delete[] padded;

		int hashlength = 32;
		if (function == FUNCTION_SHA224)
			hashlength = 28;

		for (int i = 0; i < hashlength; i += 4)
		{
			output.data[i+0] = (intermediate[i/4] >> 24) & 0xFF;
			output.data[i+1] = (intermediate[i/4] >> 16) & 0xFF;
			output.data[i+2] = (intermediate[i/4] >>  8) & 0xFF;
			output.data[i+3] = (intermediate[i/4] >>  0) & 0xFF;
		}

		output.size = hashlength;
	}
} sha256;

const uint32 SHA256::initial224[8] = {
	0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
	0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4,
};

const uint32 SHA256::initial256[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};

const uint32 SHA256::constants[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

/**
 * This implementation was based on the description in RFC-6234.
 **/
// SHA-2: SHA-384 and SHA-512
class SHA512 : public HashFunction
{
private:
	static const uint64 initial384[8];
	static const uint64 initial512[8];
	static const uint64 constants[80];

public:
	bool isSupported(Function function) const override
	{
		return function == FUNCTION_SHA384 || function == FUNCTION_SHA512;
	}

	void hash(Function function, const char *input, uint64 length, Value &output) const override
	{
		if (!isSupported(function))
			throw love::Exception("Hash function not supported by SHA-384/SHA-512 implementation");

		uint64 intermediates[8];
		if (function == FUNCTION_SHA384)
			memcpy(intermediates, initial384, sizeof(intermediates));
		else
			memcpy(intermediates, initial512, sizeof(intermediates));

		// Compute final padded length, accounting for the appended bit (byte) and size
		uint64 paddedLength = extend_multiple(length + 1 + 16, 128);

		uint64 *padded = new uint64[paddedLength / 8];
		memcpy(padded, input, length);
		memset(((uint8*)padded) + length, 0, paddedLength - length);
		*(((uint8*)padded) + length) = 0x80; // append bit

		// Append length in bits (big endian), note we only write a 64-bit int, so
		// we have filled the first 8 bytes with zeroes
		uint64 bit_length = length * 8;
		for (int i = 0; i < 8; ++i)
			*(((uint8*)padded) + (paddedLength - 8 + i)) = (bit_length >> (56 - i * 8)) & 0xFF;

		// Allocate our extended words
		uint64 words[80];

		for (uint64 i = 0; i < paddedLength/8; i += 16)
		{
			uint64 *chunk = &padded[i];
			for (int j = 0; j < 16; ++j)
			{
				char *c = (char*) &words[j];
				c[0] = (chunk[j] >> 56) & 0xFF;
				c[1] = (chunk[j] >> 48) & 0xFF;
				c[2] = (chunk[j] >> 40) & 0xFF;
				c[3] = (chunk[j] >> 32) & 0xFF;
				c[4] = (chunk[j] >> 24) & 0xFF;
				c[5] = (chunk[j] >> 16) & 0xFF;
				c[6] = (chunk[j] >>  8) & 0xFF;
				c[7] = (chunk[j] >>  0) & 0xFF;
			}
			for (int j = 16; j < 80; ++j)
			{
				words[j] = words[j-7] + words[j-16];
				words[j] += rightrot(words[j-2], 19) ^ rightrot(words[j-2], 61) ^ (words[j-2] >> 6);
				words[j] += rightrot(words[j-15], 1) ^ rightrot(words[j-15], 8) ^ (words[j-15] >> 7);
			}

			uint64 A = intermediates[0];
			uint64 B = intermediates[1];
			uint64 C = intermediates[2];
			uint64 D = intermediates[3];
			uint64 E = intermediates[4];
			uint64 F = intermediates[5];
			uint64 G = intermediates[6];
			uint64 H = intermediates[7];

			for (int j = 0; j < 80; ++j)
			{
				uint64 temp1 = H + constants[j] + words[j];
				temp1 += rightrot(E, 14) ^ rightrot(E, 18) ^ rightrot(E, 41);
				temp1 += (E & F) ^ (~E & G);
				uint64 temp2 = rightrot(A, 28) ^ rightrot(A, 34) ^ rightrot(A, 39);
				temp2 += (A & B) ^ (A & C) ^ (B & C);
				H = G;
				G = F;
				F = E;
				E = D + temp1;
				D = C;
				C = B;
				B = A;
				A = temp1 + temp2;
			}

			intermediates[0] += A;
			intermediates[1] += B;
			intermediates[2] += C;
			intermediates[3] += D;
			intermediates[4] += E;
			intermediates[5] += F;
			intermediates[6] += G;
			intermediates[7] += H;
		}

		delete[] padded;

		int hashlength = 64;
		if (function == FUNCTION_SHA384)
			hashlength = 48;

		for (int i = 0; i < hashlength; i += 8)
		{
			output.data[i+0] = (intermediates[i/8] >> 56) & 0xFF;
			output.data[i+1] = (intermediates[i/8] >> 48) & 0xFF;
			output.data[i+2] = (intermediates[i/8] >> 40) & 0xFF;
			output.data[i+3] = (intermediates[i/8] >> 32) & 0xFF;
			output.data[i+4] = (intermediates[i/8] >> 24) & 0xFF;
			output.data[i+5] = (intermediates[i/8] >> 16) & 0xFF;
			output.data[i+6] = (intermediates[i/8] >>  8) & 0xFF;
			output.data[i+7] = (intermediates[i/8] >>  0) & 0xFF;
		}

		output.size = hashlength;
	}
} sha512;

const uint64 SHA512::initial384[8] = {
	0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939,
	0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4,
};

const uint64 SHA512::initial512[8] = {
	0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
	0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179,
};

const uint64 SHA512::constants[80] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817,
};

} // impl
}

HashFunction *HashFunction::getHashFunction(Function function)
{
	switch(function)
	{
	case FUNCTION_MD5:
		return &impl::md5;
	case FUNCTION_SHA1:
		return &impl::sha1;
	case FUNCTION_SHA224:
	case FUNCTION_SHA256:
		return &impl::sha256;
	case FUNCTION_SHA384:
	case FUNCTION_SHA512:
		return &impl::sha512;
	case FUNCTION_MAX_ENUM:
		return nullptr;
	// No default for compiler warnings
	}
    return nullptr;
}

bool HashFunction::getConstant(const char *in, Function &out)
{
	return functionNames.find(in, out);
}

bool HashFunction::getConstant(const Function &in, const char *&out)
{
	return functionNames.find(in, out);
}
std::vector<std::string> HashFunction::getConstants(Function)
{
	return functionNames.getNames();
}

StringMap<HashFunction::Function, HashFunction::FUNCTION_MAX_ENUM>::Entry HashFunction::functionEntries[] =
{
	{"md5",  FUNCTION_MD5},
	{"sha1", FUNCTION_SHA1},
	{"sha224", FUNCTION_SHA224},
	{"sha256", FUNCTION_SHA256},
	{"sha384", FUNCTION_SHA384},
	{"sha512", FUNCTION_SHA512},
};

StringMap<HashFunction::Function, HashFunction::FUNCTION_MAX_ENUM> HashFunction::functionNames(HashFunction::functionEntries, sizeof(HashFunction::functionEntries));

} // data
} // love
