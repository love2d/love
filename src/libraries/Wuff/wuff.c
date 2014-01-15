#include <string.h>

#include "wuff_config.h"
#include "wuff.h"
#include "wuff_internal.h"


wuff_sint32 wuff_open(struct wuff_handle ** handle_pointer, struct wuff_callback * callback, void * userdata)
{
	struct wuff_handle * handle;
	wuff_sint32 wuff_status;

	if (handle_pointer == NULL || callback == NULL)
		return WUFF_INVALID_PARAM;

	handle = wuff_alloc(sizeof(struct wuff_handle));
	if (handle == NULL)
		return WUFF_MEMALLOC_ERROR;

	memset(handle, 0, sizeof(struct wuff_handle));
	handle->buffer.data = NULL;
	handle->callback = callback;
	handle->userdata = userdata;

	wuff_status = wuff_setup(handle);
	if (wuff_status < 0)
	{
		wuff_cleanup(handle);
		return wuff_status;
	}

	*handle_pointer = handle;

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_close(struct wuff_handle * handle)
{
	wuff_sint32 wuff_status;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	wuff_status = wuff_cleanup(handle);
	WUFF_STATUS_BAIL()

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_seek(struct wuff_handle * handle, wuff_uint64 offset)
{
	wuff_sint32 wuff_status;
	wuff_uint64 seek_offset;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	/* Clamp offset to stream length. */
	offset = offset <= handle->stream.length ? offset : handle->stream.length;
	seek_offset = offset * handle->stream.header.block_size;
	wuff_status = handle->callback->seek(handle->userdata, handle->stream.data.offset + seek_offset);
	WUFF_STATUS_BAIL()

	handle->stream.position = offset;
	handle->output.block_offset = 0;

	/* A new position requires an empty buffer. */
	wuff_status = wuff_buffer_clear(handle);
	WUFF_STATUS_BAIL()

	return	WUFF_SUCCESS;
}

wuff_sint32 wuff_tell(struct wuff_handle * handle, wuff_uint64 * offset)
{
	if (handle == NULL)
		return WUFF_INVALID_PARAM;

	*offset = handle->stream.position;

	return	WUFF_SUCCESS;
}

wuff_sint32 wuff_stream_info(struct wuff_handle * handle, struct wuff_info * info)
{
	if (handle == NULL || info == NULL)
		return WUFF_INVALID_PARAM;

	info->format = handle->stream.format;
	info->channels = handle->stream.header.channels;
	info->sample_rate = handle->stream.header.sample_rate;
	info->bits_per_sample = handle->stream.header.bits_per_sample;
	info->length = handle->stream.length;
	/* Think about adding channel mapping and perhaps other things. */

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_format(struct wuff_handle * handle, wuff_uint16 format)
{
	wuff_sint32 wuff_status;

	if (handle == NULL)
		return WUFF_INVALID_PARAM;
	else if (format >= WUFF_FORMAT_MAX)
		return WUFF_FORMAT_UNSUPPORTED;

	/* A format change resets the position to the start of the block. */
	wuff_status = wuff_seek(handle, handle->stream.position);
	WUFF_STATUS_BAIL()

	wuff_status = wuff_set_output_format(handle, format);
	WUFF_STATUS_BAIL()

	return WUFF_SUCCESS;
}

wuff_sint32 wuff_read(struct wuff_handle * handle, wuff_uint8 * out_buffer, size_t * out_size)
{
	size_t current_offset;
	size_t r_samples, num_samples;
	wuff_uint8 head_offset, head, tail, sample_size;
	wuff_uint8 * in_buffer;
	wuff_sint32 wuff_status;

	if (handle == NULL || out_buffer == NULL || out_size == NULL)
		return WUFF_INVALID_PARAM;

	if (*out_size == 0)
		return WUFF_SUCCESS;

	sample_size = (wuff_uint8)handle->output.bytes_per_sample;

	/* Calculating the number of samples that fit into the application buffer. */
	/* The first and last sample may be truncated. */
	current_offset = handle->output.block_offset;
	head_offset = current_offset % sample_size;
	head = head_offset == 0 ? 0 : sample_size - head_offset;
	num_samples = wuff_calculate_samples(*out_size, sample_size, &head, &tail);

	/* Requesting the number of samples from the buffer. */
	/* Calculate the new sample count if necessary and write the output. */
	r_samples = num_samples;
	wuff_status = wuff_buffer_request(handle, &in_buffer, &r_samples);
	WUFF_STATUS_BAIL()
	else if (r_samples == 0)
	{
		/* Possible EOF. */
		*out_size = 0;
	}
	else
	{
		if (r_samples == 1 && head != 0)
		{
			/* Only the first truncated sample fits. */
			/* I really hope nobody will use small buffers like this. */
			num_samples = 0;
			tail = 0;
		}
		else
		{
			/* At this point the first (possibly truncated) sample will be fully written. */
			/* Subtract the first and last sample from the count if they're truncated. */
			if (r_samples < num_samples)
				tail = 0;
			num_samples = r_samples - !!head - !!tail;
		}

		handle->output.function(out_buffer, in_buffer, num_samples, head_offset, head, tail);

		/* Report the number of bytes written. */
		*out_size = num_samples * sample_size + head + tail;

		/* Adjust the block offset and sample position. */
		current_offset += *out_size;
		if (current_offset >= handle->output.block_size)
		{
			handle->stream.position += current_offset / handle->output.block_size;
			handle->output.block_offset = current_offset % handle->output.block_size;
		}
		else
		{
			handle->output.block_offset = current_offset;
		}

		/* Release the fully processed samples from the buffer. */
		wuff_status = wuff_buffer_release(handle, head_offset + head == sample_size ? num_samples + 1 : num_samples);
		WUFF_STATUS_BAIL()
	}

	return WUFF_SUCCESS;
}

void wuff_version(struct wuff_version * version)
{
	if (version == NULL)
		return;

	version->major = WUFF_VERSION_MAJOR;
	version->minor = WUFF_VERSION_MINOR;
	version->build = WUFF_VERSION_BUILD;
	version->revision = WUFF_VERSION_REVISION;
}
