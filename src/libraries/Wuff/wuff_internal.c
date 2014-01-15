#include <stdlib.h>
#include <string.h>

#include "wuff_config.h"
#include "wuff.h"
#include "wuff_internal.h"
#include "wuff_convert.h"


wuff_sint32 wuff_setup(struct wuff_handle * handle)
{
	wuff_sint32 wuff_status;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	wuff_status = wuff_init_stream(handle);
	WUFF_STATUS_BAIL()

	/* Allocating the buffer for the handle requires information from the stream. */
	wuff_status = wuff_buffer_alloc(handle);
	WUFF_STATUS_BAIL()

	/* The output format defaults to the stream format. */
	wuff_status = wuff_format(handle, handle->stream.format);
	WUFF_STATUS_BAIL()

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_cleanup(struct wuff_handle * handle)
{
	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	if (handle->buffer.data != NULL)
		wuff_free(handle->buffer.data);
	wuff_free(handle);

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_set_output_format(struct wuff_handle * handle, wuff_uint16 format)
{
	wuff_uint16 bits;
	wuff_uint16 stream_format;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;
	else if (format >= WUFF_FORMAT_MAX)
		return WUFF_FORMAT_UNSUPPORTED;

	stream_format = handle->stream.format;

	switch (format)
	{
	case WUFF_FORMAT_PCM_U8:
		bits = 8;
		switch (stream_format)
		{
		case WUFF_FORMAT_PCM_U8:
			handle->output.function = wuff_int8_to_int8;
			break;
		case WUFF_FORMAT_PCM_S16:
			handle->output.function = wuff_int16_to_int8;
			break;
		case WUFF_FORMAT_PCM_S24:
			handle->output.function = wuff_int24_to_int8;
			break;
		case WUFF_FORMAT_PCM_S32:
			handle->output.function = wuff_int32_to_int8;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_32:
			handle->output.function = wuff_float32_to_int8;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_64:
			handle->output.function = wuff_float64_to_int8;
			break;
		}
		break;
	case WUFF_FORMAT_PCM_S16:
		bits = 16;
		switch (stream_format)
		{
		case WUFF_FORMAT_PCM_U8:
			handle->output.function = wuff_int8_to_int16;
			break;
		case WUFF_FORMAT_PCM_S16:
			handle->output.function = wuff_int16_to_int16;
			break;
		case WUFF_FORMAT_PCM_S24:
			handle->output.function = wuff_int24_to_int16;
			break;
		case WUFF_FORMAT_PCM_S32:
			handle->output.function = wuff_int32_to_int16;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_32:
			handle->output.function = wuff_float32_to_int16;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_64:
			handle->output.function = wuff_float64_to_int16;
			break;
		}
		break;
	case WUFF_FORMAT_PCM_S24:
		bits = 24;
		switch (stream_format)
		{
		case WUFF_FORMAT_PCM_U8:
			handle->output.function = wuff_int8_to_int24;
			break;
		case WUFF_FORMAT_PCM_S16:
			handle->output.function = wuff_int16_to_int24;
			break;
		case WUFF_FORMAT_PCM_S24:
			handle->output.function = wuff_int24_to_int24;
			break;
		case WUFF_FORMAT_PCM_S32:
			handle->output.function = wuff_int32_to_int24;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_32:
			handle->output.function = wuff_float32_to_int24;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_64:
			handle->output.function = wuff_float64_to_int24;
			break;
		}
		break;
	case WUFF_FORMAT_PCM_S32:
		bits = 32;
		switch (stream_format)
		{
		case WUFF_FORMAT_PCM_U8:
			handle->output.function = wuff_int8_to_int32;
			break;
		case WUFF_FORMAT_PCM_S16:
			handle->output.function = wuff_int16_to_int32;
			break;
		case WUFF_FORMAT_PCM_S24:
			handle->output.function = wuff_int24_to_int32;
			break;
		case WUFF_FORMAT_PCM_S32:
			handle->output.function = wuff_int32_to_int32;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_32:
			handle->output.function = wuff_float32_to_int32;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_64:
			handle->output.function = wuff_float64_to_int32;
			break;
		}
		break;
	case WUFF_FORMAT_IEEE_FLOAT_32:
		bits = 32;
		switch (stream_format)
		{
		case WUFF_FORMAT_PCM_U8:
			handle->output.function = wuff_int8_to_float32;
			break;
		case WUFF_FORMAT_PCM_S16:
			handle->output.function = wuff_int16_to_float32;
			break;
		case WUFF_FORMAT_PCM_S24:
			handle->output.function = wuff_int24_to_float32;
			break;
		case WUFF_FORMAT_PCM_S32:
			handle->output.function = wuff_int32_to_float32;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_32:
			handle->output.function = wuff_float32_to_float32;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_64:
			handle->output.function = wuff_float64_to_float32;
			break;
		}
		break;
	case WUFF_FORMAT_IEEE_FLOAT_64:
		bits = 64;
		switch (stream_format)
		{
		case WUFF_FORMAT_PCM_U8:
			handle->output.function = wuff_int8_to_float64;
			break;
		case WUFF_FORMAT_PCM_S16:
			handle->output.function = wuff_int16_to_float64;
			break;
		case WUFF_FORMAT_PCM_S24:
			handle->output.function = wuff_int24_to_float64;
			break;
		case WUFF_FORMAT_PCM_S32:
			handle->output.function = wuff_int32_to_float64;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_32:
			handle->output.function = wuff_float32_to_float64;
			break;
		case WUFF_FORMAT_IEEE_FLOAT_64:
			handle->output.function = wuff_float64_to_float64;
			break;
		}
		break;
	default:
		return WUFF_FORMAT_UNSUPPORTED;
	}

	handle->output.format = format;
	handle->output.bytes_per_sample = bits / 8;
	handle->output.block_size = handle->stream.header.channels * (bits / 8);

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_check_bits(wuff_uint16 bits, wuff_uint16 * format)
{
	if (*format == WUFF_FORMAT_PCM)
	{
		switch (bits)
		{
		case 8:
			*format = WUFF_FORMAT_PCM_U8;
			break;
		case 16:
			*format = WUFF_FORMAT_PCM_S16;
			break;
		case 24:
			*format = WUFF_FORMAT_PCM_S24;
			break;
		case 32:
			*format = WUFF_FORMAT_PCM_S32;
			break;
		default:
			return WUFF_FORMAT_UNSUPPORTED;
		}
	}
	else if (*format == WUFF_FORMAT_IEEE_FLOAT)
	{
		switch (bits)
		{
		case 32:
			*format = WUFF_FORMAT_IEEE_FLOAT_32;
			break;
		case 64:
			*format = WUFF_FORMAT_IEEE_FLOAT_64;
			break;
		default:
			return WUFF_FORMAT_UNSUPPORTED;
		}
	}
	else
	{
		return WUFF_FORMAT_UNSUPPORTED;
	}

	return WUFF_SUCCESS;
}

size_t wuff_calculate_samples(size_t target_size, wuff_uint8 sample_size, wuff_uint8 * head, wuff_uint8 * tail)
{
	size_t samples = 0;

	if (*head != 0)
	{
		if (target_size <= *head)
		{
			*head = (wuff_uint8)target_size;
			*tail = 0;
			return 1;
		}
		target_size -= *head;
		++samples;
	}

	samples = target_size / sample_size;
	*tail = target_size % sample_size;
	if (*tail != 0)
		++samples;
	return samples;
}

wuff_sint32 wuff_init_stream(struct wuff_handle * handle)
{
	/* Allocate some space on the stack. */
	/* No need to do dynamic allocation for simple header probing. */
	wuff_uint8 buffer[WUFF_HEADER_FETCH_SIZE];
	size_t buffer_size = WUFF_HEADER_FETCH_SIZE;
	wuff_uint64 search_offset;
	struct wuff_chunk_header chunk;
	wuff_sint32 wuff_status;

	wuff_status = handle->callback->read(handle->userdata, buffer, &buffer_size);
	WUFF_STATUS_BAIL()
	else if (buffer_size < WUFF_STREAM_MIN_SIZE)
		return WUFF_STREAM_NOT_RIFF;

	/* Check for RIFF signature. */
	wuff_copy_chunk_header_data(&chunk, buffer);
	if (chunk.id != WUFF_RIFF_CHUNK_ID)
		return WUFF_STREAM_NOT_RIFF;
	handle->stream.size = chunk.size;

	/* Check for WAVE format. */
	wuff_copy_chunk_header_data(&chunk, buffer + 8);
	if (chunk.id != WUFF_WAVE_CHUNK_ID)
		return WUFF_STREAM_NOT_WAVE;

	/* Search fmt chunk. */
	wuff_copy_chunk_header_data(&chunk, buffer + 12);
	search_offset = 12;
	if (chunk.id != WUFF_FORMAT_CHUNK_ID)
	{
		chunk.id = 0;
		/* The fmt chunk must appear before the data chunk. */
		wuff_status = wuff_search_chunk(handle, &chunk, &search_offset, WUFF_FORMAT_CHUNK_ID, WUFF_DATA_CHUNK_ID);
		if (wuff_status == WUFF_STREAM_CHUNK_NOT_FOUND)
			return WUFF_STREAM_FORMAT_CHUNK_MISSING;
		else WUFF_STATUS_BAIL()

		/* In case the fmt chunk is not the first chunk, align it on the stack buffer as if it were. */
		buffer_size = WUFF_HEADER_FETCH_SIZE - 20;
		wuff_status = handle->callback->read(handle->userdata, buffer + 20, &buffer_size);
		WUFF_STATUS_BAIL()
		/* EOF bail. */
		else if (buffer_size < WUFF_HEADER_MIN_SIZE)
			return WUFF_STREAM_INVALID;
	}

	/* Extract header information. */
	handle->stream.header.size = chunk.size;
	handle->stream.header.offset = search_offset + 8;
	handle->stream.header.format = wuff_get_uint16(buffer + 20);
	handle->stream.header.channels = wuff_get_uint16(buffer + 22);
	handle->stream.header.sample_rate = wuff_get_uint32(buffer + 24);
	handle->stream.header.bits_per_sample = wuff_get_uint16(buffer + 34);
	handle->stream.header.bytes_per_sample = handle->stream.header.bits_per_sample / 8;
	handle->stream.header.block_size = handle->stream.header.channels * handle->stream.header.bytes_per_sample;

	/* Bail on invalid streams. */
	if (handle->stream.header.channels == 0)
		return WUFF_STREAM_ZERO_CHANNELS;
	else if (handle->stream.header.sample_rate == 0)
		return WUFF_STREAM_ZERO_SAMPLE_RATE;
	else if (handle->stream.header.bits_per_sample == 0)
		return WUFF_STREAM_ZERO_BITS_PER_SAMPLE;

	/* Grab the format from the extended header. */
	if (handle->stream.header.size > WUFF_HEADER_MIN_SIZE && wuff_get_uint16(buffer + 36) == 22)
	{
		if (handle->stream.header.format == WUFF_FORMAT_EXTENSIBLE)
			handle->stream.header.format = wuff_get_uint16(buffer + 44);
	}

	/* The check if this format is actually supported. */
	handle->stream.format = handle->stream.header.format;
	wuff_status = wuff_check_bits(handle->stream.header.bits_per_sample, &handle->stream.format);
	WUFF_STATUS_BAIL()

	/* The search for the data chunk begins. */
	wuff_copy_chunk_header_data(&chunk, buffer + 20 + handle->stream.header.size);
	search_offset = handle->stream.header.offset + handle->stream.header.size;
	wuff_status = wuff_search_chunk(handle, &chunk, &search_offset, WUFF_DATA_CHUNK_ID, 0);
	if (wuff_status == WUFF_STREAM_CHUNK_NOT_FOUND)
		return WUFF_STREAM_DATA_CHUNK_MISSING;
	else WUFF_STATUS_BAIL()

	handle->stream.data.size = chunk.size;
	handle->stream.data.offset = search_offset + 8;
	handle->stream.length = handle->stream.data.size / handle->stream.header.channels / handle->stream.header.bytes_per_sample;
	handle->stream.position = 0;

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_search_chunk(struct wuff_handle * handle, struct wuff_chunk_header * chunk, wuff_uint64 * offset, wuff_uint32 id, wuff_uint32 stop_id)
{
	wuff_uint8 buffer[8];
	wuff_uint64 search_offset;
	size_t buffer_size;
	wuff_sint32 wuff_status = 0;

	if (chunk->id != 0 && chunk->id == id)
		return WUFF_SUCCESS;

	/* Copy the current file position. */
	search_offset = *offset;

	while (wuff_status >= 0)
	{
		search_offset += 8 + chunk->size;
		/* FIXME: Non-compliant RIFFs may not pad to WORD alignment. What now? */
		if (search_offset & 1)
			search_offset++;

		wuff_status = handle->callback->seek(handle->userdata, search_offset);
		WUFF_STATUS_BAIL()
		/*else if (wuff_status == WUFF_CALLBACK_EOF)
			return WUFF_STREAM_CHUNK_NOT_FOUND;*/

		buffer_size = 8;
		wuff_status = handle->callback->read(handle->userdata, buffer, &buffer_size);
		WUFF_STATUS_BAIL()

		wuff_copy_chunk_header_data(chunk, buffer);
		/* Bail if we're at the EOF or the stop id. */
		if (buffer_size < 8 || (stop_id != 0 && chunk->id == stop_id))
			return WUFF_STREAM_CHUNK_NOT_FOUND;
		else if (chunk->id == id)
			break;
	}

	/* Report chunk offset. */
	*offset = search_offset;

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_buffer_alloc(struct wuff_handle * handle)
{
	wuff_sint32 wuff_status;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	/* Try to allocate a buffer for 0.25 seconds, but clamp at some minimum and maximum value. */
	handle->buffer.size = handle->stream.header.sample_rate * handle->stream.header.block_size / 4;
	if (handle->buffer.size < WUFF_BUFFER_MIN_SIZE)
		handle->buffer.size = WUFF_BUFFER_MIN_SIZE;
	else if (handle->buffer.size > WUFF_BUFFER_MAX_SIZE)
		handle->buffer.size = WUFF_BUFFER_MAX_SIZE;

	handle->buffer.data = wuff_alloc(handle->buffer.size);
	if (handle->buffer.data == NULL)
		return WUFF_MEMALLOC_ERROR;

	/* Just in case, let's null the offsets. */
	wuff_status = wuff_buffer_clear(handle);
	WUFF_STATUS_BAIL()

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_buffer_clear(struct wuff_handle * handle)
{
	wuff_uint64 position;
	wuff_sint32 wuff_status;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	wuff_status = handle->callback->tell(handle->userdata, &position);
	WUFF_STATUS_BAIL()

	if (position < handle->stream.data.offset || position > handle->stream.data.offset + handle->stream.data.size)
		return WUFF_BUFFER_INVALID_STREAM_POSITION;

	handle->buffer.bytes_left = handle->stream.data.size - (position - handle->stream.data.offset);
	handle->buffer.offset = 0;
	handle->buffer.end = 0;

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_buffer_fill(struct wuff_handle * handle)
{
	size_t bytes_in_buffer;
	size_t bytes_to_read;
	wuff_sint32 wuff_status;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	/* Check if there are bytes in the buffer and move them to the start of the buffer. */
	/* Probably not the most efficient way. Think on it some more! */
	bytes_in_buffer = handle->buffer.end - handle->buffer.offset;

	if (bytes_in_buffer == handle->buffer.size)
		return WUFF_SUCCESS;
	else if (bytes_in_buffer > 0)
		memmove(handle->buffer.data, handle->buffer.data + handle->buffer.offset, bytes_in_buffer);

	bytes_to_read = handle->buffer.size - bytes_in_buffer;
	if (bytes_to_read > handle->buffer.bytes_left)
		bytes_to_read = (size_t)handle->buffer.bytes_left;

	wuff_status = handle->callback->read(handle->userdata, handle->buffer.data + bytes_in_buffer, &bytes_to_read);
	WUFF_STATUS_BAIL()

	handle->buffer.offset = 0;
	handle->buffer.end = bytes_in_buffer + bytes_to_read;
	handle->buffer.bytes_left -= bytes_to_read;

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_buffer_release(struct wuff_handle * handle, size_t samples)
{
	size_t size;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	size = samples * handle->stream.header.bytes_per_sample;

	/* Check for an attempt to release more samples than the buffer could hold. */
	/* "This should never happen." Let's throw an error anyway in case.*/
	if (size > handle->buffer.end - handle->buffer.offset)
		return WUFF_BUFFER_INVALID_SIZE;

	handle->buffer.offset += size;

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_buffer_request(struct wuff_handle * handle, wuff_uint8 ** buffer, size_t * samples)
{
	size_t request_samples = *samples;
	size_t buffer_samples, size;
	size_t bps = handle->stream.header.bytes_per_sample;
	wuff_sint32 wuff_status;

	if (handle == NULL || buffer == NULL || samples == NULL)
		return WUFF_INVALID_PARAM;

	/* Fill the buffer some more if the requested size is bigger than the current data in the buffer. */
	size = request_samples * bps;
	if (size > handle->buffer.end - handle->buffer.offset)
	{
		wuff_status = wuff_buffer_fill(handle);
		WUFF_STATUS_BAIL()
	}

	buffer_samples = (handle->buffer.end - handle->buffer.offset) / bps;

	/* Report sample count change. */
	if (buffer_samples < request_samples)
		*samples = buffer_samples;

	/* Report sample buffer start. */
	*buffer = handle->buffer.data + handle->buffer.offset;

	return WUFF_SUCCESS;
}
