#ifndef WUFF_INTERNAL_H
#define WUFF_INTERNAL_H

#define WUFF_BUFFER_MIN_SIZE		4096
#define WUFF_BUFFER_MAX_SIZE		2097152
#define WUFF_STREAM_MIN_SIZE		36
#define WUFF_HEADER_MIN_SIZE		16
#define WUFF_HEADER_FETCH_SIZE		80

#define WUFF_FORMAT_PCM				1
#define WUFF_FORMAT_IEEE_FLOAT		3
#define WUFF_FORMAT_EXTENSIBLE		0xFFFE


#define WUFF_RIFF_CHUNK_ID   wuff_get_chunk_id("RIFF")
#define WUFF_WAVE_CHUNK_ID   wuff_get_chunk_id("WAVE")
#define WUFF_FORMAT_CHUNK_ID wuff_get_chunk_id("fmt ")
#define WUFF_DATA_CHUNK_ID   wuff_get_chunk_id("data")

#define WUFF_STATUS_BAIL() if (wuff_status < 0) return wuff_status;


static WUFF_INLINE wuff_uint32 wuff_get_uint32(wuff_uint8 * data)
{
	return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
}

static WUFF_INLINE wuff_uint16 wuff_get_uint16(wuff_uint8 * data)
{
	return data[0] + (data[1] << 8);
}

struct wuff_chunk_header
{
	wuff_uint32 id;
	wuff_uint32 size;
};

static WUFF_INLINE wuff_uint32 wuff_get_chunk_id(const char txt[5])
{
	const wuff_uint8 * id = (wuff_uint8*) txt;
	wuff_uint32 int_id;
	wuff_uint8 * id_bytes = (wuff_uint8 *)&int_id;
	id_bytes[0] = id[0];
	id_bytes[1] = id[1];
	id_bytes[2] = id[2];
	id_bytes[3] = id[3];

	return int_id;
}

static WUFF_INLINE void wuff_copy_chunk_header_data(struct wuff_chunk_header * chunk, wuff_uint8 * data)
{
	wuff_uint8 * id = (wuff_uint8 *)&chunk->id;
	id[0] = data[0];
	id[1] = data[1];
	id[2] = data[2];
	id[3] = data[3];

	chunk->size = wuff_get_uint32(data + 4);
}

struct wuff_stream_header
{
	wuff_uint64 size;
	wuff_uint64 offset;

	wuff_uint16 format;
	wuff_uint16 channels;
	wuff_uint32 sample_rate;
	wuff_uint16 bits_per_sample;
	wuff_uint16 bytes_per_sample;
	size_t block_size;
};

struct wuff_stream_data
{
	wuff_uint64 size;
	wuff_uint64 offset;
};

struct wuff_buffer
{
	wuff_uint8 * data;
	wuff_uint64 bytes_left;
	size_t size;
	size_t offset;
	size_t end;
};

struct wuff_output
{
	wuff_uint16 format;
	size_t bytes_per_sample;
	size_t block_size;
	size_t block_offset;
	void (* function)(wuff_uint8 *, wuff_uint8 *, size_t, wuff_uint8, wuff_uint8, wuff_uint8);
};

struct wuff_stream
{
	wuff_uint64 size;
	wuff_uint64 length;
	wuff_uint16 format;

	wuff_uint64 position;

	struct wuff_stream_header header;
	struct wuff_stream_data data;
};

struct wuff_handle
{
	struct wuff_stream stream;
	struct wuff_buffer buffer;
	struct wuff_output output;
	struct wuff_callback * callback;
	void * userdata;
};


/* Initializes the stream, allocates the buffer, and sets the output format. */
/* Expects a nulled wuff_handle and the callbacks set and ready. */
WUFF_INTERN_API wuff_sint32 wuff_setup(struct wuff_handle * handle);

/* Cleans the stream up, frees the buffer and the wuff_handle. */
WUFF_INTERN_API wuff_sint32 wuff_cleanup(struct wuff_handle * handle);

/* Called by wuff_setup. Initializes the stream by reading the data from the */
/* callbacks, searching for headers and stream information. */
WUFF_INTERN_API wuff_sint32 wuff_init_stream(struct wuff_handle * handle);

/* Searches for a specific chunk id and stops before another if it's not 0. */
/* If the id in wuff_chunk_header is not 0, it will be checked too and if */
/* they match, then the function will return immediately. */
/* Expects offset to point to the file position of a chunk and */
/* wuff_chunk_header to have the size of this chunk. */
WUFF_INTERN_API wuff_sint32 wuff_search_chunk(struct wuff_handle * handle, struct wuff_chunk_header * chunk, wuff_uint64 * offset, wuff_uint32 id, wuff_uint32 stop_id);

/* Sets the output struct of the stream to the new format. */
WUFF_INTERN_API wuff_sint32 wuff_set_output_format(struct wuff_handle * handle, wuff_uint16);

/* Checks if the number of bits per samples is supported and writes the */
/* output identifier to the 16-bit integer. */
WUFF_INTERN_API wuff_sint32 wuff_check_bits(wuff_uint16 bits, wuff_uint16 * format);

/* Calculates the number of samples that have to be requested from the buffer */
/* by also taking the truncated samples at the start and end into account. */
/* The return value is the number of samples needed. */
WUFF_INTERN_API size_t wuff_calculate_samples(size_t target_size, wuff_uint8 sample_size, wuff_uint8 * head, wuff_uint8 * tail);


/* Allocates the buffer for the input stream. */
/* Expects the stream to be initialized, as format information is needed. */
WUFF_INTERN_API wuff_sint32 wuff_buffer_alloc(struct wuff_handle * handle);

/* Fills the buffer with new data. */
WUFF_INTERN_API wuff_sint32 wuff_buffer_fill(struct wuff_handle * handle);

/* Marks all bytes in the buffer as free. */
WUFF_INTERN_API wuff_sint32 wuff_buffer_clear(struct wuff_handle * handle);

/* Requests samples and a pointer to them. */
/* The number of samples may be lower than requested. */
WUFF_INTERN_API wuff_sint32 wuff_buffer_request(struct wuff_handle * handle, wuff_uint8 ** buffer, size_t * samples);

/* Releases the number of samples from the buffer. */
WUFF_INTERN_API wuff_sint32 wuff_buffer_release(struct wuff_handle * handle, size_t samples);


#endif /* WUFF_INTERNAL_H */
