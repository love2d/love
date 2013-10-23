#ifndef WUFF_CONVERT_H
#define WUFF_CONVERT_H

#define WUFF_CONV_FUNC(name) WUFF_INTERN_API void name(wuff_uint8 * dst, wuff_uint8 * src, size_t samples, wuff_uint8 offset, wuff_uint8 head, wuff_uint8 tail)

WUFF_CONV_FUNC(wuff_int8_to_int8);
WUFF_CONV_FUNC(wuff_int8_to_int16);
WUFF_CONV_FUNC(wuff_int8_to_int24);
WUFF_CONV_FUNC(wuff_int8_to_int32);
WUFF_CONV_FUNC(wuff_int8_to_float32);
WUFF_CONV_FUNC(wuff_int8_to_float64);

WUFF_CONV_FUNC(wuff_int16_to_int8);
WUFF_CONV_FUNC(wuff_int16_to_int16);
WUFF_CONV_FUNC(wuff_int16_to_int24);
WUFF_CONV_FUNC(wuff_int16_to_int32);
WUFF_CONV_FUNC(wuff_int16_to_float32);
WUFF_CONV_FUNC(wuff_int16_to_float64);

WUFF_CONV_FUNC(wuff_int24_to_int8);
WUFF_CONV_FUNC(wuff_int24_to_int16);
WUFF_CONV_FUNC(wuff_int24_to_int24);
WUFF_CONV_FUNC(wuff_int24_to_int32);
WUFF_CONV_FUNC(wuff_int24_to_float32);
WUFF_CONV_FUNC(wuff_int24_to_float64);

WUFF_CONV_FUNC(wuff_int32_to_int8);
WUFF_CONV_FUNC(wuff_int32_to_int16);
WUFF_CONV_FUNC(wuff_int32_to_int24);
WUFF_CONV_FUNC(wuff_int32_to_int32);
WUFF_CONV_FUNC(wuff_int32_to_float32);
WUFF_CONV_FUNC(wuff_int32_to_float64);

WUFF_CONV_FUNC(wuff_float32_to_int8);
WUFF_CONV_FUNC(wuff_float32_to_int16);
WUFF_CONV_FUNC(wuff_float32_to_int24);
WUFF_CONV_FUNC(wuff_float32_to_int32);
WUFF_CONV_FUNC(wuff_float32_to_float32);
WUFF_CONV_FUNC(wuff_float32_to_float64);

WUFF_CONV_FUNC(wuff_float64_to_int8);
WUFF_CONV_FUNC(wuff_float64_to_int16);
WUFF_CONV_FUNC(wuff_float64_to_int24);
WUFF_CONV_FUNC(wuff_float64_to_int32);
WUFF_CONV_FUNC(wuff_float64_to_float32);
WUFF_CONV_FUNC(wuff_float64_to_float64);

#endif /* WUFF_CONVERT_H */
