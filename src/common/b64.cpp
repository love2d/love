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

#include "b64.h"

namespace love
{

static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void b64_decode_block(char in[4], char out[3])
{
	out[0] = (char)(in[0] << 2 | in[1] >> 4);
	out[1] = (char)(in[1] << 4 | in[2] >> 2);
	out[2] = (char)(((in[2] << 6) & 0xc0) | in[3]);
}

char *b64_decode(const char *src, int slen, int &size)
{
	// Actual output may be smaller due to padding and/or whitespace in the
	// base64-encoded string.
	int max_size = (slen / 4) * 3;

	char *dst = new char[max_size];
	char *d = dst;

	char in[4] = {0}, out[3], v;
	int i, len, pos = 0;

	while (pos <= slen)
	{
		for (len = 0, i = 0; i < 4 && pos <= slen; i++)
		{
			v = 0;

			while (pos <= slen && v == 0)
			{
				v = src[pos++];
				v = (char)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
				if (v)
					v = (char)((v == '$') ? 0 : v - 61);
			}

			if (pos <= slen)
			{
				len++;
				if (v)
					in[i] = (char)(v - 1);
			}
			else
				in[i] = 0;
		}

		if (len)
		{
			b64_decode_block(in, out);
			for (i = 0; i < len - 1; i++)
				*(d++) = out[i];
		}
	}

	size = int(d - dst);

	return dst;
}

} // love
