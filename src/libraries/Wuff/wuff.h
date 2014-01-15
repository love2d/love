/*
 *	Wuff - A very basic WAVE reader
 */

#ifndef WUFF_H
#define WUFF_H

#ifdef __cplusplus
extern "C" {
#endif


#define WUFF_VERSION_MAJOR         0
#define WUFF_VERSION_MINOR         0
#define WUFF_VERSION_BUILD         0
#define WUFF_VERSION_REVISION      2


#ifndef WUFF_API_OVERRIDE
	#if defined(_WIN32) && defined(WUFF_DYNAMIC_LIB)
		#define WUFF_EXPORT __declspec(dllexport)
		#define WUFF_IMPORT __declspec(dllimport)
	#else
		#define WUFF_EXPORT
		#define WUFF_IMPORT
	#endif
#endif

#ifdef WUFF_BUILDING_CORE
	#define WUFF_API WUFF_EXPORT
#else
	#define WUFF_API WUFF_IMPORT
#endif


#ifdef WUFF_FORCE_STDINT_H
	#include <stdint.h>
typedef uint8_t             wuff_uint8;
typedef int8_t              wuff_sint8;
typedef uint16_t            wuff_uint16;
typedef int16_t             wuff_sint16;
typedef uint32_t            wuff_uint32;
typedef int32_t             wuff_sint32;
	#ifdef WUFF_NO_64BIT_TYPE
typedef uint32_t            wuff_uint64;
typedef int32_t             wuff_sint64;
	#else
typedef uint64_t            wuff_uint64;
typedef int64_t             wuff_sint64;
	#endif
#elif _MSC_VER
typedef unsigned __int8     wuff_uint8;
typedef signed __int8       wuff_sint8;
typedef unsigned __int16    wuff_uint16;
typedef signed __int16      wuff_sint16;
typedef unsigned __int32    wuff_uint32;
typedef signed __int32      wuff_sint32;
typedef unsigned __int64    wuff_uint64;
typedef signed __int64      wuff_sint64;
#else
typedef unsigned char       wuff_uint8;
typedef signed char         wuff_sint8;
typedef unsigned short      wuff_uint16;
typedef signed short        wuff_sint16;
typedef unsigned int        wuff_uint32;
typedef signed int          wuff_sint32;
	#ifdef WUFF_NO_64BIT_TYPE
typedef unsigned long       wuff_uint64;
typedef signed long         wuff_sint64;
	#else
typedef unsigned long long  wuff_uint64;
typedef signed long long    wuff_sint64;
	#endif
#endif

/** @file */
/**
 * Opaque structure used to identify the open Wuff streams.
 */
struct wuff_handle;

/**
 * Callbacks that control the delivery of the data of the WAVE file.
 *
 * The return values of the functions indicate their status. A zero or positive
 * value means success and a negative value failure. The macros WUFF_SUCCESS and
 * WUFF_ERROR, or a value equal or below WUFF_USER_ERROR can be used. The error
 * value will be returned by the function called by the application.
 */
struct wuff_callback
{
	/**
	 * The read callback requests the linking application to write at least
	 * 'size' bytes into the memory where 'buffer' is pointing to. The value
	 * pointed to by 'size' must be update to the actual number of bytes
	 * written. Zero will be interepreted as the end-of-file.
	 *
	 * @param userdata The userdata set with wuff_open.
	 * @param buffer A pointer to the memory where the data can be written to.
	 * @param size A pointer to the size of the buffer and the bytes written.
	 */
	wuff_sint32 (* read)(void * userdata, wuff_uint8 * buffer, size_t * size);

	/**
	 * The seek callback requests the linking application to seek to a new byte
	 * offset in the WAVE data. The next call to the read callback must then
	 * write data starting from this position. The offset is always relative
	 * to the beginning of the WAVE data.
	 *
	 * @param userdata The userdata set with wuff_open.
	 * @param offset The new offset.
	 */
	wuff_sint32 (* seek)(void * userdata, wuff_uint64 offset);

	/**
	 * The tell callback requests the linking application to write the current
	 * byte position to the integer pointed to by 'offset'.
	 *
	 * @param userdata The userdata set with wuff_open.
	 * @param offset A pointer to an integer where the current position can be written to.
	 */
	wuff_sint32 (* tell)(void * userdata, wuff_uint64 * offset);
};


/**
 * Stream information structure.
 */
struct wuff_info
{
	wuff_uint16 format;           /**< The format of the stream.
	                               *   See "Wuff raw sample formats" below. */
	wuff_uint16 channels;         /**< The number of channels in the stream. */
	wuff_uint32 sample_rate;      /**< The sample rate in hertz. */
	wuff_uint16 bits_per_sample;  /**< The number of bits per sample. */
	wuff_uint64 length;           /**< The length of the stream in samples. */
};


/**
 * Version information structure.
 */
struct wuff_version
{
	wuff_uint16 major;
	wuff_uint16 minor;
	wuff_uint16 build;
	wuff_uint16 revision;
};


/**
 * Opens a new Wuff stream. This will read from the callbacks immediately, make
 * sure they're ready. It will check if the WAVE file is supported.
 *
 * @param handle A pointer to pointer of a wuff_handle that will be
 * initialized if the function succeeds.
 * @param callback The callbacks for the data of the WAVE file.
 * @param userdata A void pointer that will be passed to the callbacks.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_open(struct wuff_handle ** handle, struct wuff_callback * callback, void * userdata);

/**
 * Closes a Wuff stream.
 *
 * @param handle The Wuff stream handle.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_close(struct wuff_handle * handle);

/**
 * Fills the wuff_info struct with information about the stream.
 *
 * @param handle The Wuff stream handle.
 * @param info A pointer to a wuff_info struct.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_stream_info(struct wuff_handle * handle, struct wuff_info * info);

/**
 * Sets the output format of the decoder. A new format resets the decoder output
 * to the beginning of the current block (the sample of the first channel).
 *
 * @param handle The Wuff stream handle.
 * @param format The new output format.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_format(struct wuff_handle * handle, wuff_uint16 format);

/**
 * Decodes samples to the passed memory location. The size_t pointer points to
 * the maximum number of bytes that can be written to the buffer. This count
 * will be adjusted to the number of bytes written to the buffer.
 *
 * @param handle The Wuff stream handle.
 * @param buffer The buffer to write to.
 * @param size The maximum number of bytes to write to the buffer.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_read(struct wuff_handle * handle, wuff_uint8 * buffer, size_t * size);

/**
 * Seeks to a sample location.
 * The next call to wuff_read will return samples starting from this position.
 *
 * @param handle The Wuff stream handle.
 * @param offset The sample offset to seek to.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_seek(struct wuff_handle * handle, wuff_uint64 offset);

/**
 * Sets the current position.
 *
 * @param handle The Wuff stream handle.
 * @param offset A pointer to a integer that will receive the sample offset.
 * @return Returns a negative value if an error occured.
 */
WUFF_API wuff_sint32 wuff_tell(struct wuff_handle * handle, wuff_uint64 * offset);

/**
 * Copies the Wuff version of the binary into the struct.
 * For compile-time version information use the WUFF_VERSION_MAJOR,
 * WUFF_VERSION_MINOR, WUFF_VERSION_BUILD, and WUFF_VERSION_REVISION macros.
 *
 * @param version A pointer to a wuff_version struct that will receive the
 * version information.
 */
WUFF_API void wuff_version(struct wuff_version * version);


/* Wuff raw sample formats. */
#define WUFF_FORMAT_PCM_U8                             0
#define WUFF_FORMAT_PCM_S16                            1
#define WUFF_FORMAT_PCM_S24                            2
#define WUFF_FORMAT_PCM_S32                            3
#define WUFF_FORMAT_IEEE_FLOAT_32                      4
#define WUFF_FORMAT_IEEE_FLOAT_64                      5
#define WUFF_FORMAT_MAX                                6


/* Success and error return values for all functions. */
#define WUFF_STREAM_EOF                              100

#define WUFF_SUCCESS                                   0

#define WUFF_ERROR                                    -1
#define WUFF_INVALID_PARAM                            -2
#define WUFF_MEMALLOC_ERROR                           -3

#define WUFF_STREAM_NOT_RIFF                        -100
#define WUFF_STREAM_NOT_WAVE                        -101
#define WUFF_STREAM_INVALID                         -102
#define WUFF_STREAM_ZERO_CHANNELS                   -103
#define WUFF_STREAM_ZERO_SAMPLE_RATE                -104
#define WUFF_STREAM_ZERO_BITS_PER_SAMPLE            -105
#define WUFF_STREAM_FORMAT_CHUNK_MISSING            -106
#define WUFF_STREAM_DATA_CHUNK_MISSING              -107
#define WUFF_STREAM_CHUNK_NOT_FOUND                 -108

#define WUFF_FORMAT_UNSUPPORTED                     -200

#define WUFF_BUFFER_INVALID_SIZE                    -300
#define WUFF_BUFFER_INVALID_STREAM_POSITION         -301

#define WUFF_USER_ERROR                           -10000


#ifdef __cplusplus
}
#endif

#endif /* WUFF_H */
