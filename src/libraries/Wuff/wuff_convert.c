#include <string.h>

#include "wuff_config.h"
#include "wuff.h"
#include "wuff_convert.h"

/*
 * int8 functions.
 */

WUFF_CONV_FUNC(wuff_int8_to_int8)
{
	(void)offset;
	memcpy(dst, src, samples + head + tail);
}

WUFF_CONV_FUNC(wuff_int8_to_int16)
{
	wuff_sint16 i16;
	size_t i;

	if (head != 0)
	{
		i16 = (src[0] - 128) << 8;
		memcpy(dst, (wuff_uint8 *)&i16 + offset, head);
		src += 1;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		i16 = (src[i] - 128) << 8;
		memcpy(dst + i * 2, &i16, 2);
	}

	if (tail != 0)
	{
		i16 = (src[samples] - 128) << 8;
		memcpy(dst + samples * 2, &i16, tail);
	}
}

WUFF_CONV_FUNC(wuff_int8_to_int24)
{
	wuff_sint32 i24;
	size_t i;

	if (head != 0)
	{
		i24 = (src[0] - 128) << 24;
		memcpy(dst, (wuff_uint8 *)&i24 + 1 + offset, head);
		src += 1;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		i24 = (src[i] - 128) << 24;
		memcpy(dst + i * 3, (wuff_uint8 *)&i24 + 1, 3);
	}

	if (tail != 0)
	{
		i24 = (src[samples] - 128) << 24;
		memcpy(dst + samples * 3, (wuff_uint8 *)&i24 + 1, tail);
	}
}

WUFF_CONV_FUNC(wuff_int8_to_int32)
{
	wuff_sint32 i32;
	size_t i;

	if (head != 0)
	{
		i32 = (src[0] - 128) << 24;
		memcpy(dst, (wuff_uint8 *)&i32 + offset, head);
		src += 1;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		i32 = (src[i] - 128) << 24;
		memcpy(dst + i * 4, &i32, 4);
	}

	if (tail != 0)
	{
		i32 = (src[samples] - 128) << 24;
		memcpy(dst + samples * 4, &i32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int8_to_float32)
{
	float f32;
	size_t i;

	if (head != 0)
	{
		f32 = (float)(src[0] - 128) / 128.0f;
		memcpy(dst, (wuff_uint8 *)&f32 + offset, head);
		src += 1;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		f32 = (float)(src[i] - 128) / 128.0f;
		memcpy(dst + i * 4, &f32, 4);
	}

	if (tail != 0)
	{
		f32 = (float)(src[samples] - 128) / 128.0f;
		memcpy(dst + samples * 4, &f32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int8_to_float64)
{
	double f64;
	size_t i;

	if (head != 0)
	{
		f64 = (double)(src[0] - 128) / 128.0;
		memcpy(dst, (wuff_uint8 *)&f64 + offset, head);
		src += 1;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		f64 = (double)(src[i] - 128) / 128.0;
		memcpy(dst + i * 8, &f64, 8);
	}

	if (tail != 0)
	{
		f64 = (double)(src[samples] - 128) / 128.0;
		memcpy(dst + samples * 8, &f64, tail);
	}
}

/*
 * int16 functions.
 */

WUFF_CONV_FUNC(wuff_int16_to_int8)
{
	wuff_sint16 i16;
	size_t i;
	(void)offset; (void)head; (void)tail;

	for (i = 0; i < samples; i++)
	{
		memcpy(&i16, src + i * 2, 2);
		dst[i] = (i16 >> 8) + 128;
	}
}

WUFF_CONV_FUNC(wuff_int16_to_int16)
{
	memcpy(dst, src + offset, samples * 2 + head + tail);
}

WUFF_CONV_FUNC(wuff_int16_to_int24)
{
	wuff_sint16 i16;
	wuff_sint32 i24;
	size_t i;

	if (head != 0)
	{
		memcpy(&i16, src, 2);
		i24 = i16 << 16;
		memcpy(dst, (wuff_uint8 *)&i24 + 1 + offset, head);
		src += 2;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&i16, src + i * 2, 2);
		i24 = i16 << 16;
		memcpy(dst + i * 3, (wuff_uint8 *)&i24 + 1, 3);
	}

	if (tail != 0)
	{
		memcpy(&i16, src + samples * 2, 2);
		i24 = i16 << 16;
		memcpy(dst + samples * 3, (wuff_uint8 *)&i24 + 1, tail);
	}
}

WUFF_CONV_FUNC(wuff_int16_to_int32)
{
	wuff_sint16 i16;
	wuff_sint32 i32;
	size_t i;

	if (head != 0)
	{
		memcpy(&i16, src, 2);
		i32 = i16 << 16;
		memcpy(dst, (wuff_uint8 *)&i32 + offset, head);
		src += 2;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&i16, src + i * 2, 2);
		i32 = i16 << 16;
		memcpy(dst + i * 4, &i32, 4);
	}

	if (tail != 0)
	{
		memcpy(&i16, src + samples * 2, 2);
		i32 = i16 << 16;
		memcpy(dst + samples * 4, &i32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int16_to_float32)
{
	wuff_sint16 i16;
	float f32;
	size_t i;

	if (head != 0)
	{
		memcpy(&i16, src, 2);
		f32 = (float)i16 / 32768.0f;
		memcpy(dst, (wuff_uint8 *)&f32 + offset, head);
		src += 2;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&i16, src + i * 2, 2);
		f32 = (float)i16 / 32768.0f;
		memcpy(dst + i * 4, &f32, 4);
	}

	if (tail != 0)
	{
		memcpy(&i16, src + samples * 2, 2);
		f32 = (float)i16 / 32768.0f;
		memcpy(dst + samples * 4, &f32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int16_to_float64)
{
	wuff_sint16 i16;
	double f64;
	size_t i;

	if (head != 0)
	{
		memcpy(&i16, src, 2);
		f64 = (double)i16  / 32768.0;
		memcpy(dst, (wuff_uint8 *)&f64 + offset, head);
		src += 2;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&i16, src + i * 2, 2);
		f64 = (double)i16  / 32768.0;
		memcpy(dst + i * 8, &f64, 8);
	}

	if (tail != 0)
	{
		memcpy(&i16, src + samples * 2, 2);
		f64 = (double)i16 / 32768.0;
		memcpy(dst + samples * 8, &f64, tail);
	}
}

/*
 * int24 functions.
 */

WUFF_CONV_FUNC(wuff_int24_to_int8)
{
	wuff_sint32 i24 = 0;
	size_t i;
	(void)offset; (void)head; (void)tail;

	for (i = 0; i < samples; i++)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src + i * 3, 3);
		dst[i] = (wuff_uint8)((i24 >> 16) + 128);
	}
}

WUFF_CONV_FUNC(wuff_int24_to_int16)
{
	size_t i;

	if (head != 0)
	{
		memcpy(dst, src + 1 + offset, head);
		src += 3;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(dst + i * 2, src + 1 + i * 3, 2);
	}

	if (tail != 0)
	{
		memcpy(dst + samples * 2, src + 1 + samples * 3, tail);
	}
}

WUFF_CONV_FUNC(wuff_int24_to_int24)
{
	memcpy(dst, src + offset, samples * 3 + head + tail);
}

WUFF_CONV_FUNC(wuff_int24_to_int32)
{
	wuff_sint32 i32 = 0;
	size_t i;

	if (head != 0)
	{
		memcpy((wuff_uint8 *)&i32 + 1, src, 3);
		memcpy(dst, (wuff_uint8 *)&i32 + offset, head);
		src += 3;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy((wuff_uint8 *)&i32 + 1, src + i * 3, 3);
		memcpy(dst + i * 4, &i32, 4);
	}

	if (tail != 0)
	{
		memcpy((wuff_uint8 *)&i32 + 1, src + samples * 3, 3);
		memcpy(dst + samples * 4, &i32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int24_to_float32)
{
	wuff_sint32 i24 = 0;
	float f32;
	size_t i;

	if (head != 0)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src, 3);
		f32 = (float)((double)i24 / 2147483648.0);
		memcpy(dst, (wuff_uint8 *)&f32 + offset, head);
		src += 3;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src + i * 3, 3);
		f32 = (float)((double)i24 / 2147483648.0);
		memcpy(dst + i * 4, &f32, 4);
	}

	if (tail != 0)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src + samples * 3, 3);
		f32 = (float)((double)i24 / 2147483648.0);
		memcpy(dst + samples * 4, &f32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int24_to_float64)
{
	wuff_sint32 i24 = 0;
	double f64;
	size_t i;

	if (head != 0)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src, 3);
		f64 = (double)i24 / 2147483648.0;
		memcpy(dst, (wuff_uint8 *)&f64 + offset, head);
		src += 3;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src + i * 3, 3);
		f64 = (double)i24 / 2147483648.0;
		memcpy(dst + i * 8, &f64, 8);
	}

	if (tail != 0)
	{
		memcpy((wuff_uint8 *)&i24 + 1, src + samples * 3, 3);
		f64 = (double)i24 / 2147483648.0;
		memcpy(dst + samples * 8, &f64, tail);
	}
}

/*
 * int32 functions.
 */

WUFF_CONV_FUNC(wuff_int32_to_int8)
{
	wuff_sint32 i32 = 0;
	size_t i;
	(void)offset; (void)head; (void)tail;

	for (i = 0; i < samples; i++)
	{
		memcpy(&i32, src + i * 4, 4);
		dst[i] = (i32 >> 24) + 128;
	}
}

WUFF_CONV_FUNC(wuff_int32_to_int16)
{
	size_t i;

	if (head != 0)
	{
		memcpy(dst, src + 2 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(dst + i * 2, src + 2 + i * 4, 2);
	}

	if (tail != 0)
	{
		memcpy(dst + samples * 2, src + 2 + samples * 4, tail);
	}
}

WUFF_CONV_FUNC(wuff_int32_to_int24)
{
	size_t i;

	if (head != 0)
	{
		memcpy(dst, src + 1 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(dst + i * 3, src + 1 + i * 4, 3);
	}

	if (tail != 0)
	{
		memcpy(dst + samples * 3, src + 1 + samples * 4, tail);
	}
}

WUFF_CONV_FUNC(wuff_int32_to_int32)
{
	memcpy(dst, src + offset, samples * 4 + head + tail);
}

WUFF_CONV_FUNC(wuff_int32_to_float32)
{
	wuff_sint32 i32;
	float f32;
	size_t i;

	if (head != 0)
	{
		memcpy(&i32, src, 4);
		f32 = (float)((double)i32 / 2147483648.0);
		memcpy(dst, (wuff_uint8 *)&f32 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&i32, src + i * 4, 4);
		f32 = (float)((double)i32 / 2147483648.0);
		memcpy(dst + i * 4, &f32, 4);
	}

	if (tail != 0)
	{
		memcpy(&i32, src + samples * 4, 4);
		f32 = (float)((double)i32 / 2147483648.0);
		memcpy(dst + samples * 4, &f32, tail);
	}
}

WUFF_CONV_FUNC(wuff_int32_to_float64)
{
	wuff_sint32 i32;
	double f64;
	size_t i;

	if (head != 0)
	{
		memcpy(&i32, src, 4);
		f64 = (double)i32 / 2147483648.0;
		memcpy(dst, (wuff_uint8 *)&f64 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&i32, src + i * 4, 4);
		f64 = (double)i32 / 2147483648.0;
		memcpy(dst + i * 8, &f64, 8);
	}

	if (tail != 0)
	{
		memcpy(&i32, src + samples * 4, 4);
		f64 = (double)i32 / 2147483648.0;
		memcpy(dst + samples * 8, &f64, tail);
	}
}

/*
 * float32 functions.
 */

WUFF_CONV_FUNC(wuff_float32_to_int8)
{
	float f32;
	size_t i;
	(void)offset; (void)head; (void)tail;

	for (i = 0; i < samples; i++)
	{
		memcpy(&f32, src + i * 4, 4);
		dst[i] = (wuff_uint8)((f32 * 127.5f) + 128.0f);
	}
}

WUFF_CONV_FUNC(wuff_float32_to_int16)
{
	float f32;
	wuff_sint16 i16;
	size_t i;

	if (head != 0)
	{
		memcpy(&f32, src, 4);
		i16 = (wuff_sint16)(f32 * 32767.5f);
		memcpy(dst, (wuff_uint8 *)&i16 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f32, src + i * 4, 4);
		i16 = (wuff_sint16)(f32 * 32767.5f);
		memcpy(dst + i * 2, &i16, 2);
	}

	if (tail != 0)
	{
		memcpy(&f32, src + i * 4, 4);
		i16 = (wuff_sint16)(f32 * 32767.5f);
		memcpy(dst + i * 2, &i16, tail);
	}
}

WUFF_CONV_FUNC(wuff_float32_to_int24)
{
	float f32;
	wuff_sint32 i24;
	size_t i;

	if (head != 0)
	{
		memcpy(&f32, src, 4);
		i24 = (wuff_sint32)((double)f32 * 2147483647.5);
		memcpy(dst, (wuff_uint8 *)&i24 + 1 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f32, src + i * 4, 4);
		i24 = (wuff_sint32)((double)f32 * 2147483647.5);
		memcpy(dst + i * 3, (wuff_uint8 *)&i24 + 1, 3);
	}

	if (tail != 0)
	{
		memcpy(&f32, src + samples * 4, 4);
		i24 = (wuff_sint32)((double)f32 * 2147483647.5);
		memcpy(dst + samples * 3, (wuff_uint8 *)&i24 + 1, tail);
	}
}

WUFF_CONV_FUNC(wuff_float32_to_int32)
{
	float f32;
	wuff_sint32 i32;
	size_t i;

	if (head != 0)
	{
		memcpy(&f32, src, 4);
		i32 = (wuff_sint32)((double)f32 * 2147483647.5);
		memcpy(dst, (wuff_uint8 *)&i32 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f32, src + i * 4, 4);
		i32 = (wuff_sint32)((double)f32 * 2147483647.5);
		memcpy(dst + i * 4, &i32, 4);
	}

	if (tail != 0)
	{
		memcpy(&f32, src + samples * 4, 4);
		i32 = (wuff_sint32)((double)f32 * 2147483647.5);
		memcpy(dst + samples * 4, &i32, tail);
	}
}

WUFF_CONV_FUNC(wuff_float32_to_float32)
{
	memcpy(dst, src + offset, samples * 4 + head + tail);
}

WUFF_CONV_FUNC(wuff_float32_to_float64)
{
	float f32;
	double f64;
	size_t i;

	if (head != 0)
	{
		memcpy(&f32, src, 4);
		f64 = f32;
		memcpy(dst, (wuff_uint8 *)&f64 + offset, head);
		src += 4;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f32, src + i * 4, 4);
		f64 = f32;
		memcpy(dst + i * 8, &f64, 8);
	}

	if (tail != 0)
	{
		memcpy(&f32, src + samples * 4, 4);
		f64 = f32;
		memcpy(dst + samples * 8, &f64, tail);
	}
}

/*
 * float64 functions.
 */

WUFF_CONV_FUNC(wuff_float64_to_int8)
{
	double f64;
	size_t i;
	(void)offset; (void)head; (void)tail;

	for (i = 0; i < samples; i++)
	{
		memcpy(&f64, src + i * 8, 8);
		dst[i] = (wuff_uint8)((f64 * 127.5) + 128.0);
	}
}

WUFF_CONV_FUNC(wuff_float64_to_int16)
{
	double f64;
	wuff_sint16 i16;
	size_t i;

	if (head != 0)
	{
		memcpy(&f64, src, 8);
		i16 = (wuff_sint16)(f64 * 32767.5);
		memcpy(dst, (wuff_uint8 *)&i16 + offset, head);
		src += 8;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f64, src + i * 8, 8);
		i16 = (wuff_sint16)(f64 * 32767.5);
		memcpy(dst + i * 2, &i16, 2);
	}

	if (tail != 0)
	{
		memcpy(&f64, src + i * 8, 8);
		i16 = (wuff_sint16)(f64 * 32767.5);
		memcpy(dst + i * 2, &i16, tail);
	}
}

WUFF_CONV_FUNC(wuff_float64_to_int24)
{
	double f64;
	wuff_sint32 i24;
	size_t i;

	if (head != 0)
	{
		memcpy(&f64, src, 8);
		i24 = (wuff_sint32)(f64 * 2147483647.5);
		memcpy(dst, (wuff_uint8 *)&i24 + 1 + offset, head);
		src += 8;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f64, src + i * 8, 8);
		i24 = (wuff_sint32)(f64 * 2147483647.5);
		memcpy(dst + i * 3, (wuff_uint8 *)&i24 + 1, 3);
	}

	if (tail != 0)
	{
		memcpy(&f64, src + samples * 8, 8);
		i24 = (wuff_sint32)(f64 * 2147483647.5);
		memcpy(dst + samples * 3, (wuff_uint8 *)&i24 + 1, tail);
	}
}

WUFF_CONV_FUNC(wuff_float64_to_int32)
{
	double f64;
	wuff_sint32 i32;
	size_t i;

	if (head != 0)
	{
		memcpy(&f64, src, 8);
		i32 = (wuff_sint32)(f64 * 2147483647.5);
		memcpy(dst, (wuff_uint8 *)&i32 + offset, head);
		src += 8;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f64, src + i * 8, 8);
		i32 = (wuff_sint32)(f64 * 2147483647.5);
		memcpy(dst + i * 4, &i32, 4);
	}

	if (tail != 0)
	{
		memcpy(&f64, src + samples * 8, 8);
		i32 = (wuff_sint32)(f64 * 2147483647.5);
		memcpy(dst + samples * 4, &i32, tail);
	}
}

WUFF_CONV_FUNC(wuff_float64_to_float32)
{
	double f64;
	float f32;
	size_t i;

	if (head != 0)
	{
		memcpy(&f64, src, 8);
		f32 = (float)f64;
		memcpy(dst, (wuff_uint8 *)&f32 + offset, head);
		src += 8;
		dst += head;
	}

	for (i = 0; i < samples; i++)
	{
		memcpy(&f64, src + i * 8, 8);
		f32 = (float)f64;
		memcpy(dst + i * 4, &f32, 4);
	}

	if (tail != 0)
	{
		memcpy(&f64, src + samples * 8, 8);
		f32 = (float)f64;
		memcpy(dst + samples * 4, &f32, tail);
	}
}

WUFF_CONV_FUNC(wuff_float64_to_float64)
{
	memcpy(dst, src + offset, samples * 8 + head + tail);
}
