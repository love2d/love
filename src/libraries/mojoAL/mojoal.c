/**
 * MojoAL; a simple drop-in OpenAL implementation.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#include <float.h>   // needed for FLT_MAX

// Unless compiling statically into another app, we want the public API
//   to export on Windows. Define these before including al.h, so we override
//   its attempt to mark these as `dllimport`.
#if defined(_WIN32) && !defined(AL_LIBTYPE_STATIC)
  #define AL_API __declspec(dllexport)
  #define ALC_API __declspec(dllexport)
#endif

// This is for debugging and/or pulling the fire alarm.
#ifndef MOJOAL_FORCE_SCALAR_FALLBACK
#  define MOJOAL_FORCE_SCALAR_FALLBACK 0
#endif
#if MOJOAL_FORCE_SCALAR_FALLBACK
#  define SDL_DISABLE_SSE
#  define SDL_DISABLE_NEON
#endif

#include "AL/al.h"
#include "AL/alc.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_intrin.h>

#if defined(SDL_SSE_INTRINSICS)  // if you are on x86 or x86-64, we assume you have SSE1 by now.
#define NEED_SCALAR_FALLBACK 0
#elif defined(SDL_NEON_INTRINSICS) && (defined(__ARM_ARCH) && (__ARM_ARCH >= 8))  // ARMv8 always has NEON.
#define NEED_SCALAR_FALLBACK 0
#elif defined(SDL_NEON_INTRINSICS) && (defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7))   // All ARMv7 chips from Apple have NEON.
#define NEED_SCALAR_FALLBACK 0
#elif defined(SDL_NEON_INTRINSICS) && (defined(__WINDOWS__) || defined(__WINRT__)) && defined(_M_ARM)  // all WinRT-level Microsoft devices have NEON
#define NEED_SCALAR_FALLBACK 0
#else
#define NEED_SCALAR_FALLBACK 1
#endif

#define OPENAL_VERSION_MAJOR 1
#define OPENAL_VERSION_MINOR 1
#define OPENAL_VERSION_STRING2_FINAL(major, minor) #major "." #minor
#define OPENAL_VERSION_STRING2(major, minor) OPENAL_VERSION_STRING2_FINAL(major, minor)
#define OPENAL_VERSION_STRING3_FINAL(major, minor, micro) #major "." #minor "." #micro
#define OPENAL_VERSION_STRING3(major, minor, micro) OPENAL_VERSION_STRING3_FINAL(major, minor, micro)

#define MOJOAL_VERSION_MAJOR 2
#define MOJOAL_VERSION_MINOR 9
#define MOJOAL_VERSION_MICRO 9
#define OPENAL_VERSION_STRING OPENAL_VERSION_STRING2(OPENAL_VERSION_MAJOR, OPENAL_VERSION_MINOR) " MOJOAL " OPENAL_VERSION_STRING3(MOJOAL_VERSION_MAJOR, MOJOAL_VERSION_MINOR, MOJOAL_VERSION_MICRO)
#define OPENAL_VENDOR_STRING "Ryan C. Gordon"
#define OPENAL_RENDERER_STRING "MojoAL"

#define DEFAULT_PLAYBACK_DEVICE "Default OpenAL playback device"
#define DEFAULT_CAPTURE_DEVICE "Default OpenAL capture device"

// Number of buffers to allocate at once when we need a new block during alGenBuffers().
#ifndef OPENAL_BUFFER_BLOCK_SIZE
#define OPENAL_BUFFER_BLOCK_SIZE 256
#endif

// Number of sources to allocate at once when we need a new block during alGenSources().
#ifndef OPENAL_SOURCE_BLOCK_SIZE
#define OPENAL_SOURCE_BLOCK_SIZE 64
#endif

// AL_EXT_FLOAT32 support...
#ifndef AL_FORMAT_MONO_FLOAT32
#define AL_FORMAT_MONO_FLOAT32 0x10010
#endif

#ifndef AL_FORMAT_STEREO_FLOAT32
#define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif

// AL_EXT_32bit_formats support...
// (this doesn't cover the AL_EXT_MCFORMATS items at the moment, per spec).
#ifndef AL_FORMAT_MONO_I32
#define AL_FORMAT_MONO_I32 0x19DB
#endif

#ifndef AL_FORMAT_STEREO_I32
#define AL_FORMAT_STEREO_I32 0x19DC
#endif

// ALC_EXT_DISCONNECTED support...
#ifndef ALC_CONNECTED
#define ALC_CONNECTED 0x313
#endif

// AL_EXT_source_distance_model support...
#ifndef AL_SOURCE_DISTANCE_MODEL
#define AL_SOURCE_DISTANCE_MODEL 0x200
#endif

/*
The locking strategy for this OpenAL implementation:

- The initial work on this implementation attempted to be completely
  lock free, and it lead to fragile, overly-clever, and complicated code.
  Attempt #2 is making more reasonable tradeoffs.

- All API entry points are protected by a global mutex, which means that
  calls into the API are serialized, but we expect this to not be a
  serious problem; most AL calls are likely to come from a single thread
  and uncontested mutexes generally aren't very expensive. This mutex
  is not shared with the mixer thread, so there is never a point where
  an innocent "fast" call into the AL will block because of the bad luck
  of a high mixing load and the wrong moment.

- As of the migration to SDL3, each OpenAL context is backed by an
  SDL_AudioStream bound to the audio device, so the "mixer lock" is no longer
  global lock that blocks all contexts during mixing, but rather blocks a
  single context at a time as it mixes. For most purposes, this doesn't
  change much, as most apps probably only have a single context. References
  to the "mixer thread" mean the thread that mixes all contexts, but each
  context is locked separately as this thread progresses.

- In rare cases we'll lock the mixer thread for a brief time; when a playing
  source is accessible to the mixer, it is flagged as such. The mixer locks
  each source separately as it mixes it (each source has an SDL_AudioStream
  and the mixer holds its lock), and if we need to touch a source
  that is flagged as accessible, we'll grab that lock to make sure there isn't
  a conflict. Not all source changes need to do this. The likelihood of
  hitting this case is extremely small, and the lock hold time is pretty
  short. Things that might do this, only on currently-playing sources:
  alDeleteSources, alSourceStop, alSourceRewind. alSourcePlay and
  alSourcePause never need to lock.

- Devices are expected to live for the entire life of your OpenAL
  experience, so closing one while another thread is using it is your own
  fault. Don't do that. Devices are allocated pointers, and the AL doesn't
  know if you've deleted it, making the pointer invalid. Device open and
  close are not meant to be "fast" calls.

- Generating an object (source, buffer, etc) might need to allocate
  memory, which can always take longer than you would expect. We allocate in
  blocks, so not every call will allocate more memory. Generating an object
  does not lock the mixer thread.

- Deleting a buffer does not lock the mixer thread (in-use buffers can
  not be deleted per API spec). Deleting a source will lock the mixer briefly
  if the source is still visible to the mixer. We don't believe this will be
  a serious issue in normal use cases. Deleted objects' memory is marked for
  reuse, but no memory is free'd by deleting sources or buffers until the
  context or device, respectively, are destroyed. A deleted source that's
  still visible to the mixer will not be available for reallocation until
  the mixer runs another iteration, where it will mark it as no longer
  visible. If you call alGenSources() during this time, a different source
  will be allocated.

- alBufferData needs to allocate memory to copy new audio data. Often,
  you can avoid doing these things in time-critical code. You can't set
  a buffer's data when it's attached to a source (either with AL_BUFFER
  or buffer queueing), so there's never a chance of contention with the
  mixer thread here.

- Buffers and sources are allocated in blocks of OPENAL_BUFFER_BLOCK_SIZE
  (or OPENAL_SOURCE_BLOCK_SIZE). These blocks are never deallocated as long
  as the device (for buffers) or context (for sources) lives, so they don't
  need a lock to access as the pointers are immutable once they're wired in.
  We don't keep a ALuint name index array, but rather an array of block
  pointers, which lets us find the right offset in the correct block without
  iteration. The mixer thread never references the blocks directly, as they
  get buffer and source pointers to objects within those blocks. Sources keep
  a pointer to their specifically-bound buffer, and the mixer keeps a list of
  pointers to playing sources. Since the API is serialized and the mixer
  doesn't touch them, we don't need to tapdance to add new blocks.

- Buffer data is owned by the AL, and it's illegal to delete a buffer or
  alBufferData() its contents while attached to a source with either
  AL_BUFFER or alSourceQueueBuffers(). We keep an atomic refcount for each
  buffer, and you can't change its state or delete it when its refcount is
  > 0, so there isn't a race with the mixer. Refcounts only change when
  changing a source's AL_BUFFER or altering its buffer queue, both of which
  are protected by the api lock. The mixer thread doesn't touch the
  refcount, as a buffer moving from AL_PENDING to AL_PROCESSED is still
  attached to a source.

- alSource(Stop|Pause|Rewind)v with > 1 source used will always lock the
  mixer thread to guarantee that all sources change in sync (!!! FIXME?).
  The non-v version of these functions do not lock the mixer thread.
  alSourcePlayv never locks the mixer thread (it atomically appends to a
  linked list of sources to be played, which the mixer will pick up all
  at once).

- alSourceQueueBuffers will build a linked list of buffers, then atomically
  move this list into position for the mixer to obtain it. The mixer will
  process this list without the need to be atomic (as it owns it once it
  atomically claims it from from the just_queued field where
  alSourceQueueBuffers staged it). As buffers are processed, the mixer moves
  them atomically to a linked list that other threads can pick up for
  alSourceUnqueueBuffers.

- Setting a source's offset (AL_SEC_OFFSET, AL_SAMPLE_OFFSET, AL_BYTE_OFFSET)
  will lock the source to adjust several things.

- Capture just locks the SDL audio device for everything, since it's a very
  lightweight load and a much simplified API; good enough. The capture device
  thread just dumps excessive audio from the audiostream if too much is
  queueing up, so this never takes long, and is good enough.

- Probably other things. These notes might get updates later.
*/

#if 1
#define FIXME(x)
#else
#define FIXME(x) { \
    static ALboolean seen = AL_FALSE; \
    if (!seen) { \
        seen = AL_TRUE; \
        SDL_Log("MOJOAL FIXME: %s (%s@%s:%d)", x, __FUNCTION__, __FILE__, __LINE__); \
    } \
}
#endif

#if defined(SDL_SSE_INTRINSICS)   // we assume you always have this on x86/x86-64 chips. SSE1 is 20+ years old!
#define has_sse AL_TRUE
#endif

#if defined(SDL_NEON_INTRINSICS)
#if NEED_SCALAR_FALLBACK
static ALboolean has_neon = AL_FALSE;
#else
#define has_neon AL_TRUE
#endif
#endif

#define IS_SIMD_ALIGNED(x) ( (((size_t) (x)) % 16) == 0 )

static SDL_Mutex *api_lock = NULL;

static int init_api_lock(void)
{
    if (!api_lock) {
        api_lock = SDL_CreateMutex();
        if (!api_lock) {
            return 0;
        }
    }
    return 1;
}

static void grab_api_lock(void)
{
    if (!api_lock) {
        if (!init_api_lock()) {
            return;
        }
    }
    SDL_LockMutex(api_lock);
}

static void ungrab_api_lock(void)
{
    if (!api_lock) {
        init_api_lock();
        return;
    }

    SDL_UnlockMutex(api_lock);
}

#define ENTRYPOINT(rettype,fn,params,args) \
    rettype fn params { rettype retval; grab_api_lock(); retval = _##fn args ; ungrab_api_lock(); return retval; }

#define ENTRYPOINTVOID(fn,params,args) \
    void fn params { grab_api_lock(); _##fn args ; ungrab_api_lock(); }


static size_t simd_alignment = 0;

static void *malloc_simd_aligned(const size_t len)
{
    SDL_assert(simd_alignment > 0);
    return SDL_aligned_alloc(simd_alignment, len);
}

static void free_simd_aligned(void *ptr)
{
    SDL_aligned_free(ptr);
}

// VBAP code originally from https://github.com/drbafflegab/vbap/ ... CC0 license (public domain).

#define VBAP2D_MAX_RESOLUTION 3600
#define VBAP2D_MAX_SPEAKER_COUNT 8   // original code had 64, assumed you'd use less, but we're hardcoding our current maximum.
#define VBAP2D_RESOLUTION 36   // 10 degrees per division

static SDL_INLINE float VBAP2D_division_to_angle(int const division)
{
    return (float)division * (2.0f * SDL_PI_F) / (float)VBAP2D_RESOLUTION;
}

static SDL_INLINE int VBAP2D_angle_to_span(float const angle)
{
    return (int)SDL_floorf(angle * (float)VBAP2D_RESOLUTION / (2.0f * SDL_PI_F));
}

static SDL_INLINE bool VBAP2D_contains(int division, int last_division, int next_division)
{
    if (last_division < next_division) {
        return last_division <= division && division < next_division;
    } else {
        const bool cond_a = 0 <= division && division < next_division;
        const bool cond_b = last_division <= division && division < VBAP2D_RESOLUTION;
        return cond_a || cond_b;
    }
}

static SDL_INLINE void VBAP2D_unpack_speaker_pair(int speaker_pair, int speaker_count, int *speakers)
{
    speakers[0] = (speaker_pair == 0 ? speaker_count : speaker_pair) - 1;
    speakers[1] = speaker_pair;
}

typedef struct VBAP2D_SpeakerPosition
{
    const Uint8 division;  // this is in degrees--positive to the left--divided by the resolution. RESOLUTION MUST BE < 256 TO FIT IN UINT8!
    const Uint8 sdl_channel;  // the channel in SDL's layout (in stereo: {left=0, right=1}...etc).
} VBAP2D_SpeakerPosition;

typedef struct VBAP2D_SpeakerLayout
{
    const VBAP2D_SpeakerPosition *positions;
    const int lfe_channel;
} VBAP2D_SpeakerLayout;

// these have to go from smallest to largest angle, I think...
#define P(angle) ( (Uint8) ((angle / 360.0) * VBAP2D_RESOLUTION ) )
static const VBAP2D_SpeakerPosition VBAP2D_SpeakerPositions_quad[] = { { P(45), 1 }, { P(135), 0 }, { P(225), 2 }, { P(315), 3 } };
static const VBAP2D_SpeakerPosition VBAP2D_SpeakerPositions_4_1[] = { { P(45), 1 }, { P(135), 0 }, { P(225), 3 }, { P(315), 4 } };
static const VBAP2D_SpeakerPosition VBAP2D_SpeakerPositions_5_1[] = { { P(60), 1 }, { P(90), 2 }, { P(120), 0 }, { P(240), 4 }, { P(300), 5 } };
static const VBAP2D_SpeakerPosition VBAP2D_SpeakerPositions_6_1[] = { { P(60), 1 }, { P(90), 2 }, { P(120), 0 }, { P(190), 5 }, { P(270), 4 }, { P(350), 6 } };
static const VBAP2D_SpeakerPosition VBAP2D_SpeakerPositions_7_1[] = { { P(0), 7 }, { P(60), 1 }, { P(90), 2 }, { P(120), 0 }, { P(200), 6 }, { P(240), 4 }, { P(300), 5 } };
static const VBAP2D_SpeakerLayout VBAP2D_SpeakerLayouts[VBAP2D_MAX_SPEAKER_COUNT-3] = {  // -3 to skip mono/stereo/2.1
    { VBAP2D_SpeakerPositions_quad, -1 },
    { VBAP2D_SpeakerPositions_4_1, 2 },
    { VBAP2D_SpeakerPositions_5_1, 3 },
    { VBAP2D_SpeakerPositions_6_1, 3 },
    { VBAP2D_SpeakerPositions_7_1, 3 }
};
#undef P

typedef struct VBAP2D_Bucket { Uint8 speaker_pair; } VBAP2D_Bucket;
typedef struct VBAP2D_Matrix { float a00, a01, a10, a11; } VBAP2D_Matrix;

typedef struct VBAP2D
{
    int speaker_count;
    VBAP2D_Bucket buckets[VBAP2D_RESOLUTION];
    VBAP2D_Matrix matrices[VBAP2D_MAX_SPEAKER_COUNT-1];   // the upper ones all have an LFE channel, which we don't track here, so minus one.
} VBAP2D;

static void VBAP2D_Init(VBAP2D *vbap2d, int speaker_count)
{
    SDL_assert(speaker_count > 0);
    SDL_assert(speaker_count <= VBAP2D_MAX_SPEAKER_COUNT);
    SDL_assert(VBAP2D_RESOLUTION <= VBAP2D_MAX_RESOLUTION);

    if (speaker_count < 4) {
        return;  // no VBAP for mono, stereo, or 2.1.
    }

    const VBAP2D_SpeakerLayout *speaker_layout = &VBAP2D_SpeakerLayouts[speaker_count - 4];  // offset to zero, skip mono/stereo/2.1
    const VBAP2D_SpeakerPosition *speaker_positions = speaker_layout->positions;

    vbap2d->speaker_count = speaker_count;

    if (speaker_layout->lfe_channel >= 0) {
        speaker_count--;  // for our purposes, collapse out the subwoofer channel
    }

    VBAP2D_Bucket *buckets = vbap2d->buckets;
    for (int division = 0, speaker_pair = 0; division < VBAP2D_RESOLUTION; division++) {
        int speakers[2];
        VBAP2D_unpack_speaker_pair(speaker_pair, speaker_count, speakers);
        const int last_division = speaker_positions[speakers[0]].division;
        const int next_division = speaker_positions[speakers[1]].division;

        if (!VBAP2D_contains(division, last_division, next_division)) {
            speaker_pair = (speaker_pair + 1) % speaker_count;
        }

        buckets[division].speaker_pair = speaker_pair;
    }

    VBAP2D_Matrix *matrices = vbap2d->matrices;
    for (int speaker_pair = 0; speaker_pair < speaker_count; speaker_pair++) {
        int speakers[2];
        VBAP2D_unpack_speaker_pair(speaker_pair, speaker_count, speakers);
        const int last_division = speaker_positions[speakers[0]].division;
        const int next_division = speaker_positions[speakers[1]].division;
        const float last_angle = VBAP2D_division_to_angle(last_division);
        const float next_angle = VBAP2D_division_to_angle(next_division);
        const float a00 = SDL_cosf(last_angle), a01 = SDL_cosf(next_angle);
        const float a10 = SDL_sinf(last_angle), a11 = SDL_sinf(next_angle);
        const float det = 1.0f / (a00 * a11 - a01 * a10);

        matrices[speaker_pair].a00 = +a11 * det;
        matrices[speaker_pair].a01 = -a01 * det;
        matrices[speaker_pair].a10 = -a10 * det;
        matrices[speaker_pair].a11 = +a00 * det;
    }
}

static void VBAP2D_CalculateGains(const VBAP2D *vbap2d, float source_angle, float *gains, int *speakers)
{
    int speaker_count = vbap2d->speaker_count;
    SDL_assert(speaker_count >= 4);

    const VBAP2D_SpeakerLayout *speaker_layout = &VBAP2D_SpeakerLayouts[speaker_count - 4];  // offset to zero, skip mono/stereo/2.1

    if (speaker_layout->lfe_channel >= 0) {
        speaker_count--;  // for our purposes, collapse out the subwoofer channel
    }

    // shift so angle 0 is due east instead of due north, and normalize it to the 0 to 2pi range.
    source_angle += SDL_PI_F / 2.0f;

    while (source_angle < 0.0f) {
        source_angle += 2.0f * SDL_PI_F;
    }
    while (source_angle > (2.0f * SDL_PI_F)) {
        source_angle -= 2.0f * SDL_PI_F;
    }

    const float source_x = SDL_cosf(source_angle);
    const float source_y = SDL_sinf(source_angle);
    const int span = VBAP2D_angle_to_span(source_angle);
    const int speaker_pair = vbap2d->buckets[span].speaker_pair;
    int vbap_speakers[2];

    VBAP2D_unpack_speaker_pair(speaker_pair, speaker_count, vbap_speakers);

    const VBAP2D_Matrix *matrix = &vbap2d->matrices[speaker_pair];
    const float gain_a = source_x * matrix->a00 + source_y * matrix->a01;
    const float gain_b = source_x * matrix->a10 + source_y * matrix->a11;

    const float scale = 1.0f / SDL_sqrtf(gain_a * gain_a + gain_b * gain_b);

    const float gain_a_normalized = gain_a * scale;
    const float gain_b_normalized = gain_b * scale;

    speakers[0] = speaker_layout->positions[vbap_speakers[0]].sdl_channel;
    speakers[1] = speaker_layout->positions[vbap_speakers[1]].sdl_channel;
    gains[0] = gain_a_normalized;
    gains[1] = gain_b_normalized;
}

// end VBAP code.



typedef struct ALbuffer
{
    ALboolean allocated;
    ALuint name;
    SDL_AudioSpec spec;
    ALsizei len;   // length of data in bytes.
    const void *data;
    SDL_AtomicInt refcount;  // if zero, can be deleted or alBufferData'd
} ALbuffer;

// !!! FIXME: buffers and sources use almost identical code for blocks
typedef struct BufferBlock
{
    ALbuffer buffers[OPENAL_BUFFER_BLOCK_SIZE];  // allocate these in blocks so we can step through faster.
    ALuint used;
    ALuint tmp;  // only touch under api_lock, assume it'll be gone later.
} BufferBlock;

typedef struct BufferQueueItem
{
    ALbuffer *buffer;
    void *next;  // void* because we'll atomicgetptr it.
} BufferQueueItem;

typedef struct BufferQueue
{
    void *just_queued;  // void* because we'll atomicgetptr it.
    BufferQueueItem *head;
    BufferQueueItem *tail;
    SDL_AtomicInt num_items;  // counts just_queued+head/tail
} BufferQueue;


typedef struct ALsource ALsource;

struct SDL_ALIGNED(16) ALsource  // aligned to 16 bytes for SIMD support
{
    // keep these first to help guarantee that its elements are aligned for SIMD
    ALfloat position[4];
    ALfloat velocity[4];
    ALfloat direction[4];
    int speakers[2];  // speakers we will play through (at max, currently, it's never more than 2 of them).
    ALfloat panning[2];
    SDL_AtomicInt mixer_accessible;
    SDL_AtomicInt state;  // initial, playing, paused, stopped
    ALuint name;
    ALboolean allocated;
    ALenum type;  // undetermined, static, streaming
    ALboolean recalc;
    ALboolean source_relative;
    ALboolean looping;
    ALfloat gain;
    ALfloat min_gain;
    ALfloat max_gain;
    ALfloat reference_distance;
    ALfloat max_distance;
    ALfloat rolloff_factor;
    ALfloat pitch;
    ALfloat cone_inner_angle;
    ALfloat cone_outer_angle;
    ALfloat cone_outer_gain;
    ALenum distance_model;
    ALbuffer *buffer;
    SDL_AudioStream *stream;  // for conversion, resampling, pitch, etc. ALL DATA GOES THROUGH HERE NOW.
    SDL_AtomicInt total_queued_buffers;   // everything queued, playing and processed. AL_BUFFERS_QUEUED value.
    BufferQueue buffer_queue;
    BufferQueue buffer_queue_processed;
    ALsizei offset;  // offset in bytes for converted stream!
    ALboolean offset_latched;  // AL_SEC_OFFSET, etc, say set values apply to next alSourcePlay if not currently playing!
    ALint queue_channels;
    ALsizei queue_frequency;
    ALsource *playlist_next;  // linked list that contains currently-playing sources! Only touched by mixer thread!
};

// !!! FIXME: buffers and sources use almost identical code for blocks
typedef struct SourceBlock
{
    ALsource sources[OPENAL_SOURCE_BLOCK_SIZE];  // allocate these in blocks so we can step through faster.
    ALuint used;
    ALuint tmp;  // only touch under api_lock, assume it'll be gone later.
} SourceBlock;


typedef struct SourcePlayTodo
{
    ALsource *source;
    struct SourcePlayTodo *next;
} SourcePlayTodo;

struct ALCdevice_struct
{
    SDL_AudioDeviceID device_id;  // physical device ID (or default).
    char *name;
    ALCenum error;
    SDL_AtomicInt connected;
    ALCboolean iscapture;
    float *mix_buffer;
    ALCsizei mix_buffer_len;
    float *get_buffer;
    ALCsizei get_buffer_len;

    union {
        struct {
            ALCcontext *contexts;
            BufferBlock **buffer_blocks;  // buffers are shared between contexts on the same device.
            ALCsizei num_buffer_blocks;
            BufferQueueItem *buffer_queue_pool;  // mixer thread doesn't touch this.
            void *source_todo_pool;  // void* because we'll atomicgetptr it.
        } playback;
        struct {
            SDL_AudioStream *stream;
            SDL_AudioSpec spec;
            ALCsizei framesize;
            ALCsizei max_samples;
        } capture;
    };
};

struct ALCcontext_struct
{
    // keep these first to help guarantee that its elements are aligned for SIMD
    struct SDL_ALIGNED(16) {
        ALfloat position[4];
        ALfloat velocity[4];
        ALfloat orientation[8];
        ALfloat gain;
    } listener;

    SourceBlock **source_blocks;
    ALsizei num_source_blocks;

    ALCdevice *device;
    SDL_AudioStream *stream;
    SDL_AudioDeviceID device_id;  // logical device id.
    VBAP2D vbap2d;

    SDL_AudioSpec spec;
    ALCsizei framesize;

    SDL_AtomicInt processing;
    ALenum error;
    ALCint *attributes;
    ALCsizei attributes_count;

    ALCboolean recalc;
    ALCboolean source_distance_model;
    ALenum distance_model;
    ALfloat doppler_factor;
    ALfloat doppler_velocity;
    ALfloat speed_of_sound;

    void *playlist_todo;  // void* so we can AtomicCASPtr it. Transmits new play commands from api thread to mixer thread
    ALsource *playlist;  // linked list of currently-playing sources. Mixer thread only!
    ALsource *playlist_tail;  // end of playlist so we know if last item is being readded. Mixer thread only!

    ALCcontext *prev;  // contexts are in a double-linked list
    ALCcontext *next;
};

// forward declarations
static float source_get_offset(ALsource *src, ALenum param);
static void source_set_offset(ALsource *src, ALenum param, ALfloat value);

static void lock_source(ALsource *src)
{
    SDL_LockAudioStream(src->stream);
}

static void unlock_source(ALsource *src)
{
    SDL_UnlockAudioStream(src->stream);
}

// the just_queued list is backwards. Add it to the queue in the correct order.
static void queue_new_buffer_items_recursive(BufferQueue *queue, BufferQueueItem *items)
{
    if (items == NULL) {
        return;
    }

    queue_new_buffer_items_recursive(queue, (BufferQueueItem*)items->next);
    items->next = NULL;
    if (queue->tail) {
        queue->tail->next = items;
    } else {
        queue->head = items;
    }
    queue->tail = items;
}

static void obtain_newly_queued_buffers(BufferQueue *queue)
{
    BufferQueueItem *items;
    do {
        items = (BufferQueueItem *) SDL_GetAtomicPointer(&queue->just_queued);
    } while (!SDL_CompareAndSwapAtomicPointer(&queue->just_queued, items, NULL));

    // Now that we own this pointer, we can just do whatever we want with it.
    //  Nothing touches the head/tail fields other than the mixer thread, so we
    //  move it there. Not even atomically!  :)
    SDL_assert((queue->tail != NULL) == (queue->head != NULL));

    queue_new_buffer_items_recursive(queue, items);
}

// You probably need to hold a lock before you call this (currently).
static void source_mark_all_buffers_processed(ALsource *src)
{
    obtain_newly_queued_buffers(&src->buffer_queue);
    while (src->buffer_queue.head) {
        void *ptr;
        BufferQueueItem *item = src->buffer_queue.head;
        src->buffer_queue.head = (BufferQueueItem*)item->next;
        SDL_AddAtomicInt(&src->buffer_queue.num_items, -1);

        // Move it to the processed queue for alSourceUnqueueBuffers() to pick up.
        do {
            ptr = SDL_GetAtomicPointer(&src->buffer_queue_processed.just_queued);
            SDL_SetAtomicPointer(&item->next, ptr);
        } while (!SDL_CompareAndSwapAtomicPointer(&src->buffer_queue_processed.just_queued, ptr, item));

        SDL_AddAtomicInt(&src->buffer_queue_processed.num_items, 1);
    }
    src->buffer_queue.tail = NULL;
}

/* several things might be touching an ALbuffer's data: source streams that NoCopy'd it, alBufferData() replacing it, etc.
   we overallocate the data, and stick a refcount before the aligned data, one SIMD alignment before it. When everything
   touching it drops its reference, it's safe to free it. */
static void SDLCALL albuffer_in_audiostream_complete(void *userdata, const void *buf, int buflen)
{
    SDL_AtomicInt *refcount = (SDL_AtomicInt *) userdata;
    if (SDL_AtomicDecRef(refcount)) {
        SDL_aligned_free((void *) refcount);
    }
}

static void put_albuffer_to_audiostream(ALCcontext *ctx, ALbuffer *buffer, int offset, SDL_AudioStream *stream)
{
    if (buffer) {
        SDL_AtomicInt *refcount = (SDL_AtomicInt *) (((Uint8 *) buffer->data) - simd_alignment);
        const SDL_AudioSpec output_spec = { SDL_AUDIO_F32, buffer->spec.channels, ctx->spec.freq };
        SDL_AtomicIncRef(refcount);
        SDL_SetAudioStreamFormat(stream, &buffer->spec, &output_spec);
        SDL_PutAudioStreamDataNoCopy(stream, ((const Uint8 *) buffer->data) + offset, buffer->len - offset, albuffer_in_audiostream_complete, refcount);
    }
}

static void source_release_buffer_queue(ALCcontext *ctx, ALsource *src)
{
    // move any buffer queue items to the device's available pool for reuse.
    obtain_newly_queued_buffers(&src->buffer_queue);
    if (src->buffer_queue.tail != NULL) {
        BufferQueueItem *i;
        for (i = src->buffer_queue.head; i; i = (BufferQueueItem*)i->next) {
            (void) SDL_AtomicDecRef(&i->buffer->refcount);
        }
        src->buffer_queue.tail->next = ctx->device->playback.buffer_queue_pool;
        ctx->device->playback.buffer_queue_pool = src->buffer_queue.head;
    }
    src->buffer_queue.head = src->buffer_queue.tail = NULL;
    SDL_SetAtomicInt(&src->buffer_queue.num_items, 0);

    obtain_newly_queued_buffers(&src->buffer_queue_processed);
    if (src->buffer_queue_processed.tail != NULL) {
        BufferQueueItem *i;
        for (i = src->buffer_queue_processed.head; i; i = (BufferQueueItem*)i->next) {
            (void) SDL_AtomicDecRef(&i->buffer->refcount);
        }
        src->buffer_queue_processed.tail->next = ctx->device->playback.buffer_queue_pool;
        ctx->device->playback.buffer_queue_pool = src->buffer_queue_processed.head;
    }
    src->buffer_queue_processed.head = src->buffer_queue_processed.tail = NULL;
    SDL_SetAtomicInt(&src->buffer_queue_processed.num_items, 0);
}




// Spatialization ...

// All the 3D math here is way overcommented because I HAVE NO IDEA WHAT I'M
// DOING and had to research the hell out of what are probably pretty simple
// concepts. Pay attention in math class, kids.
//
// The scalar versions have explanitory comments and links. The SIMD versions don't.
//
// The work starts in calculate_channel_gains(). Everything between here and there
// is math support code.


/* Get the sin(angle) and cos(angle) at the same time. Ideally, with one
   instruction, like what is offered on the x86.
   angle is in radians, not degrees. */
static void calculate_sincos(const ALfloat angle, ALfloat *_sin, ALfloat *_cos)
{
    *_sin = SDL_sinf(angle);
    *_cos = SDL_cosf(angle);
}

static ALfloat calculate_distance_attenuation(const ALCcontext *ctx, const ALsource *src, ALfloat distance)
{
    // AL SPEC: "With all the distance models, if the formula can not be
    // evaluated then the source will not be attenuated. For example, if a
    // linear model is being used with AL_REFERENCE_DISTANCE equal to
    // AL_MAX_DISTANCE, then the gain equation will have a divide-by-zero
    // error in it. In this case, there is no attenuation for that source."
    FIXME("check divisions by zero");

    const ALenum distance_model = ctx->source_distance_model ? src->distance_model : ctx->distance_model;
    switch (ctx->distance_model) {
        case AL_INVERSE_DISTANCE_CLAMPED:
            distance = SDL_min(SDL_max(distance, src->reference_distance), src->max_distance);
            SDL_FALLTHROUGH;
        case AL_INVERSE_DISTANCE:
            // AL SPEC: "gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance - AL_REFERENCE_DISTANCE))"
            return src->reference_distance / (src->reference_distance + src->rolloff_factor * (distance - src->reference_distance));

        case AL_LINEAR_DISTANCE_CLAMPED:
            distance = SDL_max(distance, src->reference_distance);
            SDL_FALLTHROUGH;
        case AL_LINEAR_DISTANCE:
            // AL SPEC: "distance = min(distance, AL_MAX_DISTANCE) // avoid negative gain
            //           gain = (1 - AL_ROLLOFF_FACTOR * (distance - AL_REFERENCE_DISTANCE) / (AL_MAX_DISTANCE - AL_REFERENCE_DISTANCE))"
            return 1.0f - src->rolloff_factor * (SDL_min(distance, src->max_distance) - src->reference_distance) / (src->max_distance - src->reference_distance);

        case AL_EXPONENT_DISTANCE_CLAMPED:
            distance = SDL_min(SDL_max(distance, src->reference_distance), src->max_distance);
            SDL_FALLTHROUGH;
        case AL_EXPONENT_DISTANCE:
            // AL SPEC: "gain = (distance / AL_REFERENCE_DISTANCE) ^ (- AL_ROLLOFF_FACTOR)"
            return SDL_powf(distance / src->reference_distance, -src->rolloff_factor);

        default: break;
    }

    SDL_assert(!"Unexpected distance model");
    return 1.0f;
}

#if NEED_SCALAR_FALLBACK
// XYZZY!! https://en.wikipedia.org/wiki/Cross_product#Mnemonic
//
// Calculates cross product. https://en.wikipedia.org/wiki/Cross_product
// Basically takes two vectors and gives you a vector that's perpendicular
// to both.
static void xyzzy_scalar(ALfloat *v, const ALfloat *a, const ALfloat *b)
{
    v[0] = (a[1] * b[2]) - (a[2] * b[1]);
    v[1] = (a[2] * b[0]) - (a[0] * b[2]);
    v[2] = (a[0] * b[1]) - (a[1] * b[0]);
}

// calculate dot product (multiply each element of two vectors, sum them)
static ALfloat dotproduct_scalar(const ALfloat *a, const ALfloat *b)
{
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

// calculate distance ("magnitude") in 3D space:
//    https://math.stackexchange.com/questions/42640/calculate-distance-in-3d-space
// assumes vector starts at (0,0,0).
static ALfloat magnitude_scalar(const ALfloat *v)
{
    // technically, the inital part on this is just a dot product of itself.
    return SDL_sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

static void calculate_distance_attenuation_and_angle_scalar(const ALCcontext *ctx, const ALsource *src, ALfloat *_gain, ALfloat *_radians)
{
    const ALfloat *at = &ctx->listener.orientation[0];
    const ALfloat *up = &ctx->listener.orientation[4];

    // If !source_relative, position is in world space, otherwise, it's in relation to the Listener's position.
    //   So a source-relative source that's at ( 0, 0, 0 ) will be treated as being on top of the listener,
    //   no matter where the listener moves. If not source relative, it'll get quieter as the listener moves away.
    ALfloat position[3];
    if (!src->source_relative) {
        position[0] = src->position[0] - ctx->listener.position[0];
        position[1] = src->position[1] - ctx->listener.position[1];
        position[2] = src->position[2] - ctx->listener.position[2];
    } else {
        position[0] = src->position[0];
        position[1] = src->position[1];
        position[2] = src->position[2];
    }

    // Remove upwards component so it lies completely within the horizontal plane.
    const ALfloat a = dotproduct_scalar(position, up);

    ALfloat V[3];
    V[0] = position[0] - (a * up[0]);
    V[1] = position[1] - (a * up[1]);
    V[2] = position[2] - (a * up[2]);

    // Calculate angle
    const ALfloat mags = magnitude_scalar(at) * magnitude_scalar(V);
    ALfloat radians;
    if (mags == 0.0f) {
        radians = 0.0f;
    } else {
        ALfloat cosangle = dotproduct_scalar(at, V) / mags;
        cosangle = SDL_clamp(cosangle, -1.0f, 1.0f);
        radians = SDL_acosf(cosangle);
    }

    ALfloat R[3];
    xyzzy_scalar(R, at, up);   // Get "right" vector

    *_radians = (dotproduct_scalar(R, V) < 0.0f) ? -radians : radians;   // make it negative to the left, positive to the right.
    *_gain = calculate_distance_attenuation(ctx, src, magnitude_scalar(position));
}
#endif

#if defined(SDL_SSE_INTRINSICS)
static __m128 SDL_TARGETING("sse") xyzzy_sse(const __m128 a, const __m128 b)
{
    // http://fastcpp.blogspot.com/2011/04/vector-cross-product-using-sse-code.html
    //  this is the "three shuffle" version in the comments, plus the variables swapped around for handedness in the later comment.
    const __m128 v = _mm_sub_ps(
        _mm_mul_ps(a, _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1))),
        _mm_mul_ps(b, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)))
    );
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1));
}

static ALfloat SDL_TARGETING("sse") dotproduct_sse(const __m128 a, const __m128 b)
{
    const __m128 prod = _mm_mul_ps(a, b);
    const __m128 sum1 = _mm_add_ps(prod, _mm_shuffle_ps(prod, prod, _MM_SHUFFLE(1, 0, 3, 2)));
    const __m128 sum2 = _mm_add_ps(sum1, _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(2, 2, 0, 0)));
    FIXME("this can use _mm_hadd_ps in SSE3, or _mm_dp_ps in SSE4.1");
    return _mm_cvtss_f32(_mm_shuffle_ps(sum2, sum2, _MM_SHUFFLE(3, 3, 3, 3)));
}

static ALfloat SDL_TARGETING("sse") magnitude_sse(const __m128 v)
{
    return SDL_sqrtf(dotproduct_sse(v, v));
}

static void SDL_TARGETING("sse") calculate_distance_attenuation_and_angle_sse(const ALCcontext *ctx, const ALsource *src, ALfloat *_gain, ALfloat *_radians)
{
    // (the math is explained in the scalar version.)
    const __m128 position_sse = !src->source_relative ? _mm_sub_ps(_mm_load_ps(src->position), _mm_load_ps(ctx->listener.position)) : _mm_load_ps(src->position);
    const __m128 at_sse = _mm_load_ps(&ctx->listener.orientation[0]);
    const __m128 up_sse = _mm_load_ps(&ctx->listener.orientation[4]);
    const ALfloat a = dotproduct_sse(position_sse, up_sse);
    const __m128 V_sse = _mm_sub_ps(position_sse, _mm_mul_ps(_mm_set1_ps(a), up_sse));
    const ALfloat mags = magnitude_sse(at_sse) * magnitude_sse(V_sse);
    ALfloat radians;

    if (mags == 0.0f) {
        radians = 0.0f;
    } else {
        ALfloat cosangle = dotproduct_sse(at_sse, V_sse) / mags;
        cosangle = SDL_clamp(cosangle, -1.0f, 1.0f);
        radians = SDL_acosf(cosangle);
    }

    const __m128 R_sse = xyzzy_sse(at_sse, up_sse);
    *_radians = (dotproduct_sse(R_sse, V_sse) < 0.0f) ? -radians : radians;
    *_gain = calculate_distance_attenuation(ctx, src, magnitude_sse(position_sse));
}
#endif

#if defined(SDL_NEON_INTRINSICS)
static float32x4_t SDL_TARGETING("neon") xyzzy_neon(const float32x4_t a, const float32x4_t b)
{
    const float32x4_t shuf_a = { a[1], a[2], a[0], a[3] };
    const float32x4_t shuf_b = { b[1], b[2], b[0], b[3] };
    const float32x4_t v = vsubq_f32(vmulq_f32(a, shuf_b), vmulq_f32(b, shuf_a));
    const float32x4_t retval = { v[1], v[2], v[0], v[3] };
    FIXME("need a better permute");
    return retval;
}

static ALfloat SDL_TARGETING("neon") dotproduct_neon(const float32x4_t a, const float32x4_t b)
{
    const float32x4_t prod = vmulq_f32(a, b);
    const float32x4_t sum1 = vaddq_f32(prod, vrev64q_f32(prod));
    const float32x4_t sum2 = vaddq_f32(sum1, vcombine_f32(vget_high_f32(sum1), vget_low_f32(sum1)));
    return sum2[3];
}

static ALfloat SDL_TARGETING("neon") magnitude_neon(const float32x4_t v)
{
    return SDL_sqrtf(dotproduct_neon(v, v));
}

static void SDL_TARGETING("neon") calculate_distance_attenuation_and_angle_neon(const ALCcontext *ctx, const ALsource *src, ALfloat *_gain, ALfloat *_radians)
{
    // (the math is explained in the scalar version.)
    const float32x4_t position_neon = !src->source_relative ? vsubq_f32(vld1q_f32(src->position), vld1q_f32(ctx->listener.position)) : vld1q_f32(src->position);
    const float32x4_t at_neon = vld1q_f32(&ctx->listener.orientation[0]);
    const float32x4_t up_neon = vld1q_f32(&ctx->listener.orientation[4]);
    const ALfloat a = dotproduct_neon(position_neon, up_neon);
    const float32x4_t V_neon = vsubq_f32(position_neon, vmulq_f32(vdupq_n_f32(a), up_neon));
    const ALfloat mags = magnitude_neon(at_neon) * magnitude_neon(V_neon);
    ALfloat radians;

    if (mags == 0.0f) {
        radians = 0.0f;
    } else {
        ALfloat cosangle = dotproduct_neon(at_neon, V_neon) / mags;
        cosangle = SDL_clamp(cosangle, -1.0f, 1.0f);
        radians = SDL_acosf(cosangle);
    }

    const float32x4_t R_neon = xyzzy_neon(at_neon, up_neon);
    *_radians = (dotproduct_neon(R_neon, V_neon) < 0.0f) ? -radians : radians;
    *_gain = calculate_distance_attenuation(ctx, src, magnitude_neon(position_neon));
}
#endif

static void calculate_distance_attenuation_and_angle(const ALCcontext *ctx, const ALsource *src, float *_gain, float *_radians)
{
    SDL_assert( IS_SIMD_ALIGNED(&src->position[0]) );  // source position must be aligned for SIMD access.
    SDL_assert( IS_SIMD_ALIGNED(&ctx->listener.position[0]) );  // listener position must be aligned for SIMD access.
    SDL_assert( IS_SIMD_ALIGNED(&ctx->listener.orientation[0]) );  // listener "at" must be aligned for SIMD access.
    SDL_assert( IS_SIMD_ALIGNED(&ctx->listener.orientation[4]) );  // listener "up" must be aligned for SIMD access.

    // this goes through most of the steps the AL spec dictates for gain and distance attenuation...
    #if defined(SDL_SSE_INTRINSICS)
    if (has_sse) { calculate_distance_attenuation_and_angle_sse(ctx, src, _gain, _radians); return; }
    #elif defined(SDL_NEON_INTRINSICS)
    if (has_neon) { calculate_distance_attenuation_and_angle_neon(ctx, src, _gain, _radians); return; }
    #endif

    #if SDL_MIXER_NEED_SCALAR_FALLBACK
    calculate_distance_attenuation_and_angle_scalar(ctx, src, _gain, _radians);
    #else
    SDL_assert(!"uhoh, we didn't compile in scalar fallback!");
    #endif
}

static void calculate_channel_gains(const ALCcontext *ctx, ALsource *src)
{
    // rolloff==0.0f makes all distance models result in 1.0f, and we never spatialize non-mono sources, per the AL spec.
    const ALenum distance_model = ctx->source_distance_model ? src->distance_model : ctx->distance_model;
    const ALboolean spatialize = (distance_model != AL_NONE) &&
                                 (src->queue_channels == 1) &&
                                 (src->rolloff_factor != 0.0f);

    // this goes through the steps the AL spec dictates for gain and distance attenuation...

    if (!spatialize) {
        // simpler path through the same AL spec details if not spatializing.
        const ALfloat gain = SDL_min(SDL_max(src->gain, src->min_gain), src->max_gain) * ctx->listener.gain;
        src->panning[0] = src->panning[1] = gain;  // no spatialization, but AL_GAIN (etc) is still applied.
        src->speakers[0] = 0;
        src->speakers[1] = 1;
        return;
    }

    // AL SPEC: ""1. Distance attenuation is calculated first, including
    //  minimum (AL_REFERENCE_DISTANCE) and maximum (AL_MAX_DISTANCE)
    //  thresholds."
    ALfloat gain, radians;
    calculate_distance_attenuation_and_angle(ctx, src, &gain, &radians);

    // AL SPEC: "2. The result is then multiplied by source gain (AL_GAIN)."
    gain *= src->gain;

    // AL SPEC: "3. If the source is directional (AL_CONE_INNER_ANGLE less
    // than AL_CONE_OUTER_ANGLE), an angle-dependent attenuation is calculated
    // depending on AL_CONE_OUTER_GAIN, and multiplied with the distance
    // dependent attenuation. The resulting attenuation factor for the given
    // angle and distance between listener and source is multiplied with
    // source AL_GAIN."
    if (src->cone_inner_angle < src->cone_outer_angle) {
        FIXME("directional sources");
    }

    // AL SPEC: "4. The effective gain computed this way is compared against
    // AL_MIN_GAIN and AL_MAX_GAIN thresholds."
    gain = SDL_min(SDL_max(gain, src->min_gain), src->max_gain);

    // AL SPEC: "5. The result is guaranteed to be clamped to [AL_MIN_GAIN,
    // AL_MAX_GAIN], and subsequently multiplied by listener gain which serves
    // as an overall volume control. The implementation is free to clamp
    // listener gain if necessary due to hardware or implementation
    // constraints."
    gain *= ctx->listener.gain;

    // now figure out positioning.
    const int output_channels = ctx->spec.channels;
    if (output_channels == 1) {  // no positioning for mono output, just distance attenuation.
        src->speakers[0] = src->speakers[1] = 0;
        src->panning[0] = src->panning[1] = gain;
    } else if (output_channels < 4) {
        // If we're aiming for stereo (or 2.1), we just need a simple panning effect.
        // We're going to do what's called "constant power panning," as explained...
        // https://dsp.stackexchange.com/questions/21691/algorithm-to-pan-audio

        src->speakers[0] = 0;
        src->speakers[1] = 1;

        // here comes the Constant Power Panning magic...
        #define SQRT2_DIV2 0.7071067812f  // sqrt(2.0) / 2.0 ...

        // This might be a terrible idea, which is totally my own doing here,
        // but here you go: Constant Power Panning only works from -45 to 45
        // degrees in front of the listener. So we split this into 4 quadrants.
        //
        // - from -45 to 45: standard panning.
        // - from 45 to 135: pan full right.
        // - from 135 to 225: flip angle so it works like standard panning.
        // - from 225 to -45: pan full left.

        #define RADIANS_45_DEGREES 0.7853981634f
        #define RADIANS_135_DEGREES 2.3561944902f
        if ((radians >= -RADIANS_45_DEGREES) && (radians <= RADIANS_45_DEGREES)) {
            ALfloat sine, cosine;
            calculate_sincos(radians, &sine, &cosine);
            src->panning[0] = (SQRT2_DIV2 * (cosine - sine));
            src->panning[1] = (SQRT2_DIV2 * (cosine + sine));
        } else if ((radians >= RADIANS_45_DEGREES) && (radians <= RADIANS_135_DEGREES)) {
            src->panning[0] = 0.0f;
            src->panning[1] = 1.0f;
        } else if ((radians >= -RADIANS_135_DEGREES) && (radians <= -RADIANS_45_DEGREES)) {
            src->panning[0] = 1.0f;
            src->panning[1] = 0.0f;
        } else if (radians < 0.0f) {  // back left
            ALfloat sine, cosine;
            calculate_sincos(-(radians + SDL_PI_F), &sine, &cosine);
            src->panning[0] = (SQRT2_DIV2 * (cosine - sine));
            src->panning[1] = (SQRT2_DIV2 * (cosine + sine));
        } else { // back right
            ALfloat sine, cosine;
            calculate_sincos(-(radians - SDL_PI_F), &sine, &cosine);
            src->panning[0] = (SQRT2_DIV2 * (cosine - sine));
            src->panning[1] = (SQRT2_DIV2 * (cosine + sine));
        }

        // apply distance attenuation and gain to positioning.
        src->panning[0] *= gain;
        src->panning[1] *= gain;
    } else {
        // If we're aiming for surround sound, we use Vector Based Amplitude Panning,
        //  which picks the two speakers best positioned to play the sounds and assigns
        //  a gain value to each.

        // we're going negative to the _right_ here, at the moment, so negative radians.
        VBAP2D_CalculateGains(&ctx->vbap2d, -radians, src->panning, src->speakers);

        // apply distance attenuation and gain to positioning.
        src->panning[0] *= gain;
        src->panning[1] *= gain;
    }
}




// Mixers...

static void mix_float32_mono_to_mono_scalar(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat adjust = panning[0];
    const ALsizei unrolled = mixframes / 4;
    const ALsizei leftover = mixframes % 4;

    if (adjust == 1.0f) {
        for (ALsizei i = 0; i < unrolled; i++, stream += 4, data += 4) {
            stream[0] += data[0];
            stream[1] += data[1];
            stream[2] += data[2];
            stream[3] += data[3];
        }
        for (ALsizei i = 0; i < leftover; i++, stream++, data++) {
            *stream += data[0];
        }
    } else {
        for (ALsizei i = 0; i < unrolled; i++, stream += 4, data += 4) {
            stream[0] += data[0] * adjust;
            stream[1] += data[1] * adjust;
            stream[2] += data[2] * adjust;
            stream[3] += data[3] * adjust;
        }
        for (ALsizei i = 0; i < leftover; i++, stream++, data++) {
            *stream += data[0] * adjust;
        }
    }
}

static void mix_float32_stereo_to_mono_scalar(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat adjust = panning[0];
    const ALsizei unrolled = mixframes / 4;
    const ALsizei leftover = mixframes % 4;

    if (adjust == 1.0f) {
        for (ALsizei i = 0; i < unrolled; i++, stream += 4, data += 8) {
            stream[0] += (data[0] + data[1]) * 0.5f;
            stream[1] += (data[2] + data[3]) * 0.5f;
            stream[2] += (data[4] + data[5]) * 0.5f;
            stream[3] += (data[6] + data[7]) * 0.5f;
        }
        for (ALsizei i = 0; i < leftover; i++, stream++, data += 2) {
            *stream += (data[0] + data[1]) * 0.5f;
        }
    } else {
        for (ALsizei i = 0; i < unrolled; i++, stream += 4, data += 8) {
            stream[0] += ((data[0] + data[1]) * 0.5f) * adjust;
            stream[1] += ((data[2] + data[3]) * 0.5f) * adjust;
            stream[2] += ((data[4] + data[5]) * 0.5f) * adjust;
            stream[3] += ((data[6] + data[7]) * 0.5f) * adjust;
        }
        for (ALsizei i = 0; i < leftover; i++, stream++, data += 2) {
            *stream += ((data[0] + data[1]) * 0.5f) * adjust;
        }
    }
}

static void mix_float32_mono_to_stereo_scalar(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const ALsizei unrolled = mixframes / 4;
    const ALsizei leftover = mixframes % 4;

    if ((left == 1.0f) && (right == 1.0f)) {
        for (ALsizei i = 0; i < unrolled; i++, data += 4, stream += 8) {
            const float samp0 = data[0];
            const float samp1 = data[1];
            const float samp2 = data[2];
            const float samp3 = data[3];
            stream[0] += samp0;
            stream[1] += samp0;
            stream[2] += samp1;
            stream[3] += samp1;
            stream[4] += samp2;
            stream[5] += samp2;
            stream[6] += samp3;
            stream[7] += samp3;
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp;
            stream[1] += samp;
        }
    } else {
        for (ALsizei i = 0; i < unrolled; i++, data += 4, stream += 8) {
            const float samp0 = data[0];
            const float samp1 = data[1];
            const float samp2 = data[2];
            const float samp3 = data[3];
            stream[0] += samp0 * left;
            stream[1] += samp0 * right;
            stream[2] += samp1 * left;
            stream[3] += samp1 * right;
            stream[4] += samp2 * left;
            stream[5] += samp2 * right;
            stream[6] += samp3 * left;
            stream[7] += samp3 * right;
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp * left;
            stream[1] += samp * right;
        }
    }
}

static void mix_float32_stereo_to_stereo_scalar(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const ALsizei unrolled = mixframes / 4;
    const ALsizei leftover = mixframes % 4;

    if ((left == 1.0f) && (right == 1.0f)) {
        for (ALsizei i = 0; i < unrolled; i++, stream += 8, data += 8) {
            stream[0] += data[0];
            stream[1] += data[1];
            stream[2] += data[2];
            stream[3] += data[3];
            stream[4] += data[4];
            stream[5] += data[5];
            stream[6] += data[6];
            stream[7] += data[7];
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0];
            stream[1] += data[1];
        }
    } else {
        for (ALsizei i = 0; i < unrolled; i++, stream += 8, data += 8) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
            stream[2] += data[2] * left;
            stream[3] += data[3] * right;
            stream[4] += data[4] * left;
            stream[5] += data[5] * right;
            stream[6] += data[6] * left;
            stream[7] += data[7] * right;
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
        }
    }
}

// so the idea is that this mixer is used when rendering for a system with >= 3 speakers. Since this is either an unspatialized
// stereo source or using VBAP2D, we always mix to exactly 2 speakers, although they might be any 2 arbitary channels.
static void mix_float32_mono_to_surround_scalar(const ALfloat * SDL_RESTRICT panning, const int output_channels, const int * SDL_RESTRICT speakers, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat panning0 = panning[0];
    const ALfloat panning1 = panning[1];
    const int speaker0 = speakers[0];
    const int speaker1 = speakers[1];

    FIXME("unroll");
    FIXME("call mono_to_stereo for output to stereo to get SIMD mixing");
    for (ALsizei i = 0; i < mixframes; i++, stream += output_channels, data++) {
        const float sample = *data;
        stream[speaker0] += sample * panning0;
        stream[speaker1] += sample * panning1;
    }
}

static void mix_float32_stereo_to_surround_scalar(const ALfloat * SDL_RESTRICT panning, const int output_channels, const int * SDL_RESTRICT speakers, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat panning0 = panning[0];
    const ALfloat panning1 = panning[1];
    const int speaker0 = speakers[0];
    const int speaker1 = speakers[1];

    FIXME("unroll");
    FIXME("call stereo_to_stereo for output to stereo to get SIMD mixing");
    for (ALsizei i = 0; i < mixframes; i++, stream += output_channels, data += 2) {
        stream[speaker0] += data[0] * panning0;
        stream[speaker1] += data[1] * panning1;
    }
}

#if defined(SDL_SSE_INTRINSICS)
static void SDL_TARGETING("sse") mix_float32_mono_to_stereo_sse(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 8;
    const int leftover = mixframes % 8;

    // We can align this to 16 in one special case.
    if ( ((((size_t)data) % 16) == 8) && ((((size_t)stream) % 16) == 0) && (mixframes >= 2) ) {
        stream[0] += data[0] * left;
        stream[1] += data[0] * right;
        stream[2] += data[1] * left;
        stream[3] += data[1] * right;
        mix_float32_mono_to_stereo_sse(panning, data + 2, stream + 4, mixframes - 2);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        // unaligned, do scalar version.
        mix_float32_mono_to_stereo_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (ALsizei i = 0; i < unrolled; i++, data += 8, stream += 16) {
            // We have 8 SSE registers, load 6 of them, have two for math (unrolled once).
            {
                const __m128 vdataload1 = _mm_load_ps(data);
                const __m128 vdataload2 = _mm_load_ps(data+4);
                const __m128 vstream1 = _mm_load_ps(stream);
                const __m128 vstream2 = _mm_load_ps(stream+4);
                const __m128 vstream3 = _mm_load_ps(stream+8);
                const __m128 vstream4 = _mm_load_ps(stream+12);
                _mm_store_ps(stream, _mm_add_ps(vstream1, _mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(0, 0, 1, 1))));
                _mm_store_ps(stream+4, _mm_add_ps(vstream2, _mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(2, 2, 3, 3))));
                _mm_store_ps(stream+8, _mm_add_ps(vstream3, _mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(0, 0, 1, 1))));
                _mm_store_ps(stream+12, _mm_add_ps(vstream4, _mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(2, 2, 3, 3))));
            }
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp;
            stream[1] += samp;
        }
    } else {
        const __m128 vleftright = { left, right, left, right };
        for (ALsizei i = 0; i < unrolled; i++, data += 8, stream += 16) {
            // We have 8 SSE registers, load 6 of them, have two for math (unrolled once).
            const __m128 vdataload1 = _mm_load_ps(data);
            const __m128 vdataload2 = _mm_load_ps(data+4);
            const __m128 vstream1 = _mm_load_ps(stream);
            const __m128 vstream2 = _mm_load_ps(stream+4);
            const __m128 vstream3 = _mm_load_ps(stream+8);
            const __m128 vstream4 = _mm_load_ps(stream+12);
            _mm_store_ps(stream, _mm_add_ps(vstream1, _mm_mul_ps(_mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(0, 0, 1, 1)), vleftright)));
            _mm_store_ps(stream+4, _mm_add_ps(vstream2, _mm_mul_ps(_mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(2, 2, 3, 3)), vleftright)));
            _mm_store_ps(stream+8, _mm_add_ps(vstream3, _mm_mul_ps(_mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(0, 0, 1, 1)), vleftright)));
            _mm_store_ps(stream+12, _mm_add_ps(vstream4, _mm_mul_ps(_mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(2, 2, 3, 3)), vleftright)));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp * left;
            stream[1] += samp * right;
        }
    }
}

static void SDL_TARGETING("sse") mix_float32_stereo_to_stereo_sse(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const ALsizei unrolled = mixframes / 4;
    const ALsizei leftover = mixframes % 4;

    // We can align this to 16 in one special case.
    if ( ((((size_t)stream) % 16) == 8) && ((((size_t)data) % 16) == 8) && mixframes ) {
        stream[0] += data[0] * left;
        stream[1] += data[1] * right;
        mix_float32_stereo_to_stereo_sse(panning, data + 2, stream + 2, mixframes - 1);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        // unaligned, do scalar version.
        mix_float32_stereo_to_stereo_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (ALsizei i = 0; i < unrolled; i++, data += 8, stream += 8) {
            const __m128 vdata1 = _mm_load_ps(data);
            const __m128 vdata2 = _mm_load_ps(data+4);
            const __m128 vstream1 = _mm_load_ps(stream);
            const __m128 vstream2 = _mm_load_ps(stream+4);
            _mm_store_ps(stream, _mm_add_ps(vstream1, vdata1));
            _mm_store_ps(stream+4, _mm_add_ps(vstream2, vdata2));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0];
            stream[1] += data[1];
        }
    } else {
        const __m128 vleftright = { left, right, left, right };
        for (ALsizei i = 0; i < unrolled; i++, data += 8, stream += 8) {
            const __m128 vdata1 = _mm_load_ps(data);
            const __m128 vdata2 = _mm_load_ps(data+4);
            const __m128 vstream1 = _mm_load_ps(stream);
            const __m128 vstream2 = _mm_load_ps(stream+4);
            _mm_store_ps(stream, _mm_add_ps(vstream1, _mm_mul_ps(vdata1, vleftright)));
            _mm_store_ps(stream+4, _mm_add_ps(vstream2, _mm_mul_ps(vdata2, vleftright)));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
        }
    }
}
#endif

#if defined(SDL_NEON_INTRINSICS)
static void SDL_TARGETING("neon") mix_float32_mono_to_stereo_neon(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const ALsizei unrolled = mixframes / 8;
    const ALsizei leftover = mixframes % 8;

    // We can align this to 16 in one special case.
    if ( ((((size_t)data) % 16) == 8) && ((((size_t)stream) % 16) == 0) && (mixframes >= 2) ) {
        stream[0] += data[0] * left;
        stream[1] += data[0] * right;
        stream[2] += data[1] * left;
        stream[3] += data[1] * right;
        mix_float32_mono_to_stereo_neon(panning, data + 2, stream + 4, mixframes - 2);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        // unaligned, do scalar version.
        mix_float32_mono_to_stereo_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (ALsizei i = 0; i < unrolled; i++, data += 8, stream += 16) {
            const float32x4_t vdataload1 = vld1q_f32(data);
            const float32x4_t vdataload2 = vld1q_f32(data+4);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            const float32x4x2_t vzipped1 = vzipq_f32(vdataload1, vdataload1);
            const float32x4x2_t vzipped2 = vzipq_f32(vdataload2, vdataload2);
            vst1q_f32(stream, vaddq_f32(vstream1, vzipped1.val[0]));
            vst1q_f32(stream+4, vaddq_f32(vstream2, vzipped1.val[1]));
            vst1q_f32(stream+8, vaddq_f32(vstream3, vzipped2.val[0]));
            vst1q_f32(stream+12, vaddq_f32(vstream4, vzipped2.val[1]));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp;
            stream[1] += samp;
        }
    } else {
        const float32x4_t vleftright = { left, right, left, right };
        for (ALsizei i = 0; i < unrolled; i++, data += 8, stream += 16) {
            const float32x4_t vdataload1 = vld1q_f32(data);
            const float32x4_t vdataload2 = vld1q_f32(data+4);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            const float32x4x2_t vzipped1 = vzipq_f32(vdataload1, vdataload1);
            const float32x4x2_t vzipped2 = vzipq_f32(vdataload2, vdataload2);
            vst1q_f32(stream, vmlaq_f32(vstream1, vzipped1.val[0], vleftright));
            vst1q_f32(stream+4, vmlaq_f32(vstream2, vzipped1.val[1], vleftright));
            vst1q_f32(stream+8, vmlaq_f32(vstream3, vzipped2.val[0], vleftright));
            vst1q_f32(stream+12, vmlaq_f32(vstream4, vzipped2.val[1], vleftright));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp * left;
            stream[1] += samp * right;
        }
    }
}

static void SDL_TARGETING("neon") mix_float32_stereo_to_stereo_neon(const ALfloat * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const ALsizei unrolled = mixframes / 8;
    const ALsizei leftover = mixframes % 8;

    // We can align this to 16 in one special case.
    if ( ((((size_t)stream) % 16) == 8) && ((((size_t)data) % 16) == 8) && mixframes ) {
        stream[0] += data[0] * left;
        stream[1] += data[1] * right;
        mix_float32_stereo_to_stereo_neon(panning, data + 2, stream + 2, mixframes - 1);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        // unaligned, do scalar version.
        mix_float32_stereo_to_stereo_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (ALsizei i = 0; i < unrolled; i++, data += 16, stream += 16) {
            const float32x4_t vdata1 = vld1q_f32(data);
            const float32x4_t vdata2 = vld1q_f32(data+4);
            const float32x4_t vdata3 = vld1q_f32(data+8);
            const float32x4_t vdata4 = vld1q_f32(data+12);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            vst1q_f32(stream, vaddq_f32(vstream1, vdata1));
            vst1q_f32(stream+4, vaddq_f32(vstream2, vdata2));
            vst1q_f32(stream+8, vaddq_f32(vstream3, vdata3));
            vst1q_f32(stream+12, vaddq_f32(vstream4, vdata4));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0];
            stream[1] += data[1];
        }
    } else {
        const float32x4_t vleftright = { left, right, left, right };
        for (ALsizei i = 0; i < unrolled; i++, data += 16, stream += 16) {
            const float32x4_t vdata1 = vld1q_f32(data);
            const float32x4_t vdata2 = vld1q_f32(data+4);
            const float32x4_t vdata3 = vld1q_f32(data+8);
            const float32x4_t vdata4 = vld1q_f32(data+12);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            vst1q_f32(stream, vmlaq_f32(vstream1, vdata1, vleftright));
            vst1q_f32(stream+4, vmlaq_f32(vstream2, vdata2, vleftright));
            vst1q_f32(stream+8, vmlaq_f32(vstream3, vdata3, vleftright));
            vst1q_f32(stream+12, vmlaq_f32(vstream4, vdata4, vleftright));
        }
        for (ALsizei i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
        }
    }
}
#endif

static void mix_float32_mono_to_stereo(const float * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    #if defined(SDL_SSE_INTRINSICS)
    if (has_sse) { mix_float32_mono_to_stereo_sse(panning, data, stream, mixframes); return; }
    #elif defined(SDL_NEON_INTRINSICS)
    if (has_neon) { mix_float32_mono_to_stereo_neon(panning, data, stream, mixframes); return; }
    #endif

    #if NEED_SCALAR_FALLBACK
    mix_float32_mono_to_stereo_scalar(panning, data, stream, mixframes);
    #else
    SDL_assert(!"uhoh, we didn't compile in scalar fallback!");
    #endif
}

static void mix_float32_stereo_to_stereo(const float * SDL_RESTRICT panning, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    #if defined(SDL_SSE_INTRINSICS)
    if (has_sse) { mix_float32_stereo_to_stereo_sse(panning, data, stream, mixframes); return; }
    #elif defined(SDL_NEON_INTRINSICS)
    if (has_neon) { mix_float32_stereo_to_stereo_neon(panning, data, stream, mixframes); return; }
    #endif

    #if NEED_SCALAR_FALLBACK
    mix_float32_stereo_to_stereo_scalar(panning, data, stream, mixframes);
    #else
    SDL_assert(!"uhoh, we didn't compile in scalar fallback!");
    #endif
}



// Context mixing ...

static void mix_buffer(ALsource *src, const ALbuffer *buffer, const int output_channels, const float * SDL_RESTRICT data, float * SDL_RESTRICT stream, const ALsizei mixframes)
{
    // Currently buffers can only be mono or stereo. These assertions will fail if we add extensions for buffers with more channels.
    SDL_assert(buffer->spec.channels > 0);
    SDL_assert(buffer->spec.channels <= 2);

    const ALfloat *panning = src->panning;
    if ((panning[0] == 0.0f) && (panning[1] == 0.0f)) {  // don't bother mixing in silence.
        return;
    } else if (output_channels == 1) {
        FIXME("SIMD");  // not actually sure it's worth the time and code bulk to SIMD this, though.
        if (buffer->spec.channels == 1) {
            mix_float32_mono_to_mono_scalar(panning, data, stream, mixframes);
        } else if (buffer->spec.channels == 2) {
            mix_float32_stereo_to_mono_scalar(panning, data, stream, mixframes);
        }
    } else if (output_channels == 2) {
        if (buffer->spec.channels == 1) {
            mix_float32_mono_to_stereo(panning, data, stream, mixframes);
        } else if (buffer->spec.channels == 2) {
            mix_float32_stereo_to_stereo(panning, data, stream, mixframes);
        }
    } else {
        FIXME("SIMD");
        if (buffer->spec.channels == 1) {
            mix_float32_mono_to_surround_scalar(panning, output_channels, src->speakers, data, stream, mixframes);
        } else if (buffer->spec.channels == 2) {
            mix_float32_stereo_to_surround_scalar(panning, output_channels, src->speakers, data, stream, mixframes);
        }
    }
}

static ALboolean mix_source_buffer(ALCcontext *ctx, ALsource *src, BufferQueueItem *queue, float **output, int *len)
{
    const ALbuffer *buffer = queue ? queue->buffer : NULL;
    ALboolean processed = AL_TRUE;

    SDL_assert(*len > 0);

    // you can legally queue or set a NULL buffer.
    if (buffer && buffer->data && (buffer->len > 0)) {
        // The stream will output data in `*output`'s format, except for the channel count, which matches the buffer, so we can mix it properly.
        // So we need to figure out the difference between what we need and what we're getting.
        const int bufferframesize = (int) (sizeof (float) * buffer->spec.channels);
        const int needed_frames = *len / (int) ctx->framesize;
        const int get_bytes = needed_frames * bufferframesize;
        SDL_AudioStream *stream = src->stream;

        if (ctx->device->get_buffer_len < get_bytes) {
            void *ptr = malloc_simd_aligned(get_bytes);
            if (!ptr) {
                SDL_ClearAudioStream(stream);
                return AL_TRUE;  // oh well, we're in trouble.
            }
            free_simd_aligned(ctx->device->get_buffer);
            ctx->device->get_buffer = (float *) ptr;
            ctx->device->get_buffer_len = get_bytes;
        }

        float *get_buffer = ctx->device->get_buffer;

        // if we still have data queued in src->stream, we assume it's `buffer` still mixing.
        if (SDL_GetAudioStreamAvailable(stream) <= get_bytes) {
            SDL_assert(queue != NULL);
            if (queue->next == NULL) {  // we're either AL_STATIC or at the end of the queued buffers. Time to flush. If the app queues more, too late, there's a gap.
                SDL_FlushAudioStream(stream);  // make sure we get the last few bytes when resampling
            }
        }

        const int original_queued = SDL_GetAudioStreamQueued(stream);
        const int br = SDL_GetAudioStreamData(stream, get_buffer, get_bytes);
        if (br > 0) {
            const int now_queued = SDL_GetAudioStreamQueued(stream);
            SDL_assert(now_queued <= original_queued);
            const int got_frames = br / bufferframesize;
            mix_buffer(src, buffer, ctx->spec.channels, get_buffer, *output, got_frames);
            *len -= got_frames * ctx->framesize;
            *output += got_frames * ctx->spec.channels;
            src->offset += original_queued - now_queued;
            if (src->offset > buffer->len) {
                src->offset = buffer->len;   // this might overflow if we were holding back data for resampling and flushed on the last buffer. Just clamp.
            }
        }

        processed = ((br <= 0) || (SDL_GetAudioStreamAvailable(stream) == 0));
        if (processed) {
            FIXME("does the offset have to represent the whole queue or just the current buffer?");
            src->offset = 0;
        }
    }

    return processed;
}

static ALCboolean mix_source_buffer_queue(ALCcontext *ctx, ALsource *src, BufferQueueItem *queue, float *stream, int len)
{
    ALCboolean keep = ALC_TRUE;

    while ((len > 0) && mix_source_buffer(ctx, src, queue, &stream, &len)) {
        // Finished this buffer!
        BufferQueueItem *item = queue;
        BufferQueueItem *next = queue ? (BufferQueueItem*)queue->next : NULL;

        SDL_assert((src->type == AL_STATIC) || (src->type == AL_STREAMING));

        if (src->looping) {
            if (src->type == AL_STATIC) {
                FIXME("looping is supposed to move to AL_INITIAL then immediately to AL_PLAYING, but I'm not sure what side effect this is meant to trigger");
                put_albuffer_to_audiostream(ctx, src->buffer, 0, src->stream);   // put it back in the stream for another round
                src->offset = 0;
                continue;
            } else if (src->type == AL_STREAMING) {
                FIXME("what does looping do with the AL_STREAMING state?");
            }
        }

        if (queue) {
            queue->next = NULL;
            queue = next;
        }

        if (src->type == AL_STREAMING) {  // mark buffer processed.
            SDL_assert(item == src->buffer_queue.head);
            FIXME("bubble out all these NULL checks");  // these are only here because we check for looping/stopping in this loop, but we really shouldn't enter this loop at all if queue==NULL.
            if (item != NULL) {
                src->buffer_queue.head = next;
                if (!next) {
                    src->buffer_queue.tail = NULL;
                }
                SDL_AddAtomicInt(&src->buffer_queue.num_items, -1);

                // Move it to the processed queue for alSourceUnqueueBuffers() to pick up.
                void *ptr;
                do {
                    ptr = SDL_GetAtomicPointer(&src->buffer_queue_processed.just_queued);
                    SDL_SetAtomicPointer(&item->next, ptr);
                } while (!SDL_CompareAndSwapAtomicPointer(&src->buffer_queue_processed.just_queued, ptr, item));

                SDL_AddAtomicInt(&src->buffer_queue_processed.num_items, 1);

                if (next && next->buffer) {
                    put_albuffer_to_audiostream(ctx, next->buffer, 0, src->stream);
                }
            }
        }

        if (queue == NULL) {  // nothing else to play?
            SDL_SetAtomicInt(&src->state, AL_STOPPED);
            keep = ALC_FALSE;
            break;  // nothing else to mix here, so stop.
        }
    }

    return keep;
}

static ALCboolean mix_source(ALCcontext *ctx, ALsource *src, float *stream, int len, const ALboolean force_recalc)
{
    ALCboolean keep = (SDL_GetAtomicInt(&src->state) == AL_PLAYING);
    if (keep) {
        SDL_assert(src->allocated);
        if (src->recalc || force_recalc) {
            SDL_MemoryBarrierAcquire();
            src->recalc = AL_FALSE;
            calculate_channel_gains(ctx, src);
        }
        if (src->type == AL_STATIC) {
            BufferQueueItem fakequeue = { src->buffer, NULL };
            keep = mix_source_buffer_queue(ctx, src, &fakequeue, stream, len);
        } else if (src->type == AL_STREAMING) {
            obtain_newly_queued_buffers(&src->buffer_queue);
            keep = mix_source_buffer_queue(ctx, src, src->buffer_queue.head, stream, len);
        } else if (src->type == AL_UNDETERMINED) {
            keep = ALC_FALSE;  // this has AL_BUFFER set to 0; just dump it.
        } else {
            SDL_assert(!"unknown source type");
        }
    }

    return keep;
}

// move new play requests over to the mixer thread.
static void migrate_playlist_requests(ALCcontext *ctx)
{
    SourcePlayTodo *todo;
    do {  // take the todo list atomically, now we own it.
        todo = (SourcePlayTodo *) ctx->playlist_todo;
    } while (!SDL_CompareAndSwapAtomicPointer(&ctx->playlist_todo, todo, NULL));

    if (!todo) {
        return;  // nothing new.
    }

    SourcePlayTodo *todoend = todo;

    // ctx->playlist and ALsource->playlist_next are only ever touched by the mixer thread, and source pointers live until context destruction.
    SourcePlayTodo *i;
    for (i = todo; i != NULL; i = i->next) {
        ALsource *src = i->source;
        todoend = i;
        if ((src != ctx->playlist_tail) && (!src->playlist_next)) {
            SDL_SetAtomicInt(&src->mixer_accessible, 1);  // just in case

            // get the initial buffer loaded into the stream...
            SDL_ClearAudioStream(src->stream);
            ALbuffer *buffer = NULL;
            if (src->type == AL_STATIC) {
                buffer = src->buffer;
            } else if (src->type == AL_STREAMING) {
                obtain_newly_queued_buffers(&src->buffer_queue);
                if (src->buffer_queue.head) {
                    buffer = src->buffer_queue.head->buffer;
                }
            }
            if (buffer) {
                put_albuffer_to_audiostream(ctx, buffer, src->offset, src->stream);
            }

            src->playlist_next = ctx->playlist;
            if (!ctx->playlist) {
                ctx->playlist_tail = src;
            }
            ctx->playlist = src;
        }
    }

    // put these objects back in the pool for reuse
    do {
        todoend->next = i = (SourcePlayTodo *) ctx->device->playback.source_todo_pool;
    } while (!SDL_CompareAndSwapAtomicPointer(&ctx->device->playback.source_todo_pool, i, todo));
}

static void mix_context(ALCcontext *ctx, SDL_AudioStream *stream, int len)
{
    if (ctx->device->mix_buffer_len < len) {
        void *ptr = malloc_simd_aligned(len);
        if (!ptr) {
            return;  // oh well, we're in trouble.
        }
        free_simd_aligned(ctx->device->mix_buffer);
        ctx->device->mix_buffer = (float *) ptr;
        ctx->device->mix_buffer_len = len;
    }

    float *mix_buffer = ctx->device->mix_buffer;

    SDL_memset(mix_buffer, '\0', len);

    const ALboolean force_recalc = ctx->recalc;
    if (force_recalc) {
        SDL_MemoryBarrierAcquire();
        ctx->recalc = AL_FALSE;
    }

    migrate_playlist_requests(ctx);

    ALsource *next = NULL;
    ALsource *prev = NULL;
    for (ALsource *i = ctx->playlist; i != NULL; i = next) {
        lock_source(i);
        next = i->playlist_next;  // save this to a local in case we leave the list.

        if (!mix_source(ctx, i, mix_buffer, len, force_recalc)) {
            // take it out of the playlist. It wasn't actually playing or it just finished.
            i->playlist_next = NULL;
            if (next == NULL) {
                SDL_assert(i == ctx->playlist_tail);
                ctx->playlist_tail = prev;
            }
            if (prev) {
                prev->playlist_next = next;
            } else {
                SDL_assert(i == ctx->playlist);
                ctx->playlist = next;
            }
            SDL_SetAtomicInt(&i->mixer_accessible, 0);
            SDL_ClearAudioStream(i->stream);  // just in case.
        } else {
            prev = i;
        }
        unlock_source(i);
    }

    FIXME("figure out the largest amount of source generated, and Put less here if possible.");
    SDL_PutAudioStreamData(stream, mix_buffer, len);
}

// Disconnected devices move all PLAYING sources to STOPPED, making their buffer queues processed.
static void mix_disconnected_context(ALCcontext *ctx)
{
    migrate_playlist_requests(ctx);

    ALsource *next = NULL;
    for (ALsource *i = ctx->playlist; i != NULL; i = next) {
        next = i->playlist_next;

        lock_source(i);
        // remove from playlist; all playing things got stopped, paused/initial/stopped shouldn't be listed.
        if (SDL_GetAtomicInt(&i->state) == AL_PLAYING) {
            SDL_assert(i->allocated);
            SDL_SetAtomicInt(&i->state, AL_STOPPED);
            source_mark_all_buffers_processed(i);
        }

        i->playlist_next = NULL;
        SDL_ClearAudioStream(i->stream);  // just in case.
        SDL_SetAtomicInt(&i->mixer_accessible, 0);
        unlock_source(i);
    }
    ctx->playlist = NULL;
    ctx->playlist_tail = NULL;
}

// We process a single ALC context during this call, mixing their
//  output and putting it to `stream`. SDL then mixes all the contexts' streams
//  together for playback on the hardware.
static void SDLCALL context_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    ALCcontext *ctx = (ALCcontext *) userdata;
    if (SDL_GetAtomicInt(&ctx->processing)) {
        if (SDL_GetAtomicInt(&ctx->device->connected)) {
            mix_context(ctx, stream, additional_amount);
        } else {
            mix_disconnected_context(ctx);
        }
    }
}




// ALC implementation...

static void *current_context = NULL;
static ALCenum null_device_error = ALC_NO_ERROR;

// we don't have any device-specific extensions.
#define ALC_EXTENSION_ITEMS \
    ALC_EXTENSION_ITEM(ALC_ENUMERATION_EXT) \
    ALC_EXTENSION_ITEM(ALC_EXT_CAPTURE) \
    ALC_EXTENSION_ITEM(ALC_EXT_DISCONNECT) \
    ALC_EXTENSION_ITEM(ALC_EXT_thread_local_context)

#define AL_EXTENSION_ITEMS \
    AL_EXTENSION_ITEM(AL_EXT_FLOAT32) \
    AL_EXTENSION_ITEM(AL_EXT_32bit_formats) \
    AL_EXTENSION_ITEM(AL_EXT_source_distance_model)


static void set_alc_error(ALCdevice *device, const ALCenum error)
{
    ALCenum *perr = device ? &device->error : &null_device_error;
    // can't set a new error when the previous hasn't been cleared yet.
    if (*perr == ALC_NO_ERROR) {
        *perr = error;
    }
}

// all data written before the release barrier must be available before the recalc flag changes.
#define context_needs_recalc(ctx) SDL_MemoryBarrierRelease(); ctx->recalc = AL_TRUE;
#define source_needs_recalc(src) SDL_MemoryBarrierRelease(); src->recalc = AL_TRUE;

// catch events to see if a device has disconnected.
static bool SDLCALL DeviceDisconnectedEventWatcher(void *userdata, SDL_Event *event)
{
    ALCdevice *device = (ALCdevice *) userdata;
    if (event->type == SDL_EVENT_AUDIO_DEVICE_REMOVED) {
        if (device->device_id == event->adevice.which) {
            //SDL_Log("MojoAL device=%p device_id=%u is DISCONNECTED", device, (unsigned int) device->device_id);
            if (device->iscapture) {
                SDL_PauseAudioStreamDevice(device->capture.stream);  // the app can still read existing data, but don't pull anymore (silence) from the device.
                SDL_FlushAudioStream(device->capture.stream);
            }
            SDL_SetAtomicInt(&device->connected, (int) ALC_FALSE);
        }
    }
    return true;
}

static ALCdevice *prep_alc_device(const char *devicename, const ALCboolean iscapture)
{
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        return NULL;
    }

    #if defined(SDL_SSE_INTRINSICS)
    if (!SDL_HasSSE()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;  // whoa! Better order a new Pentium III from Gateway 2000!
    }
    #endif

    #if defined(SDL_NEON_INTRINSICS) && !NEED_SCALAR_FALLBACK
    if (!SDL_HasNEON()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;  // :(
    }
    #elif defined(SDL_NEON_INTRINSICS) && NEED_SCALAR_FALLBACK
    has_neon = SDL_HasNEON() ? AL_TRUE : AL_FALSE;
    #endif

    if (!simd_alignment) {
        simd_alignment = SDL_GetSIMDAlignment();
        if (!simd_alignment) {
            simd_alignment = 16;  // oh well.
        }
    }

    SDL_AudioDeviceID devid = 0;
    if (SDL_strcmp(devicename, iscapture ? DEFAULT_CAPTURE_DEVICE : DEFAULT_PLAYBACK_DEVICE) == 0) {
        devid = iscapture ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
    } else {
        int total = 0;
        SDL_AudioDeviceID *devices = iscapture ? SDL_GetAudioRecordingDevices(&total) : SDL_GetAudioPlaybackDevices(&total);
        int i;

        for (i = 0; i < total; i++) {
            const SDL_AudioDeviceID thisdevid = devices[i];
            const char *thisdevname = SDL_GetAudioDeviceName(thisdevid);
            if (devicename && (SDL_strcmp(devicename, thisdevname) == 0)) {
                devid = thisdevid;
                break;
            }
        }
        SDL_free(devices);

        if (devid == 0) {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            return NULL;
        }
    }

    if (!init_api_lock()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    ALCdevice *device = (ALCdevice *) SDL_calloc(1, sizeof (*device));
    if (!device) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    device->device_id = devid;

    device->name = SDL_strdup(devicename);
    if (!device->name) {
        SDL_free(device);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    SDL_SetAtomicInt(&device->connected, (int) ALC_TRUE);
    device->iscapture = iscapture;

    SDL_AddEventWatch(DeviceDisconnectedEventWatcher, device);

    return device;
}

// no api lock; this creates it and otherwise doesn't have any state that can race
ALCdevice *alcOpenDevice(const ALCchar *devicename)
{
    if (!devicename) {
        devicename = DEFAULT_PLAYBACK_DEVICE;  // so ALC_DEVICE_SPECIFIER is meaningful
    }
    return prep_alc_device(devicename, ALC_FALSE);

    // we don't actually open an SDL audio device until the first context is created, so we can attempt to match audio formats.
}

// no api lock; this requires you to not destroy a device that's still in use
ALCboolean alcCloseDevice(ALCdevice *device)
{
    if (!device || device->iscapture) {
        return ALC_FALSE;
    }

    // spec: "Failure will occur if all the device's contexts and buffers have not been destroyed."
    if (device->playback.contexts) {
        return ALC_FALSE;
    }

    for (ALCsizei i = 0; i <device->playback.num_buffer_blocks; i++) {
        if (device->playback.buffer_blocks[i]->used > 0) {
            return ALC_FALSE;  // still buffers allocated.
        }
    }

    SDL_RemoveEventWatch(DeviceDisconnectedEventWatcher, device);

    for (ALCsizei i = 0; i < device->playback.num_buffer_blocks; i++) {
        SDL_free(device->playback.buffer_blocks[i]);
    }
    SDL_free(device->playback.buffer_blocks);

    BufferQueueItem *item = device->playback.buffer_queue_pool;
    while (item) {
        BufferQueueItem *next = (BufferQueueItem*)item->next;
        SDL_free(item);
        item = next;
    }

    SourcePlayTodo *todo = (SourcePlayTodo *) device->playback.source_todo_pool;
    while (todo) {
        SourcePlayTodo *next = todo->next;
        SDL_free(todo);
        todo = next;
    }

    free_simd_aligned(device->get_buffer);
    free_simd_aligned(device->mix_buffer);
    SDL_free(device->name);
    SDL_free(device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return ALC_TRUE;
}


static ALCboolean alcfmt_to_sdlfmt(const ALCenum alfmt, SDL_AudioFormat *sdlfmt, int *channels, ALCsizei *framesize)
{
    switch (alfmt) {
        case AL_FORMAT_MONO8:
            *sdlfmt = SDL_AUDIO_U8;
            *channels = 1;
            *framesize = 1;
            break;
        case AL_FORMAT_MONO16:
            *sdlfmt = SDL_AUDIO_S16;
            *channels = 1;
            *framesize = 2;
            break;
        case AL_FORMAT_STEREO8:
            *sdlfmt = SDL_AUDIO_U8;
            *channels = 2;
            *framesize = 2;
            break;
        case AL_FORMAT_STEREO16:
            *sdlfmt = SDL_AUDIO_S16;
            *channels = 2;
            *framesize = 4;
            break;
        case AL_FORMAT_MONO_FLOAT32:
            *sdlfmt = SDL_AUDIO_F32;
            *channels = 1;
            *framesize = 4;
            break;
        case AL_FORMAT_STEREO_FLOAT32:
            *sdlfmt = SDL_AUDIO_F32;
            *channels = 2;
            *framesize = 8;
            break;
        case AL_FORMAT_MONO_I32:
            *sdlfmt = SDL_AUDIO_S32;
            *channels = 1;
            *framesize = 4;
            break;
        case AL_FORMAT_STEREO_I32:
            *sdlfmt = SDL_AUDIO_S32;
            *channels = 2;
            *framesize = 8;
            break;
        default:
            return ALC_FALSE;
    }

    return ALC_TRUE;
}

// catch events to see if output device format has changed. This can let us move to/from surround sound support on the fly, not to mention spend less time doing unnecessary conversions.
static bool SDLCALL ContextDeviceChangeEventWatcher(void *userdata, SDL_Event *event)
{
    if (event->type == SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED) {
        ALCcontext *ctx = (ALCcontext *) userdata;
        if (ctx->device_id == event->adevice.which) {
            // stop the whole world while we update everything.
            grab_api_lock();
            SDL_LockAudioStream(ctx->stream);

            SDL_AudioSpec spec;
            SDL_GetAudioDeviceFormat(ctx->device_id, &spec, NULL);
            spec.format = SDL_AUDIO_F32;

            //SDL_Log("Changing MojoAL context format for ctx=%p on device_id=%u", ctx, (unsigned int) ctx->device_id);
            //SDL_Log("ctx=%p was { fmt=%s, channels=%d, freq=%d }", ctx, SDL_GetAudioFormatName(ctx->spec.format), ctx->spec.channels, ctx->spec.freq);
            //SDL_Log("ctx=%p now { fmt=%s, channels=%d, freq=%d }", ctx, SDL_GetAudioFormatName(spec.format), spec.channels, spec.freq);

            SDL_SetAudioStreamFormat(ctx->stream, &spec, NULL);  // the output end is connected to the logical device; it would have been updated by SDL.

            if (ctx->spec.channels != spec.channels) {
                VBAP2D_Init(&ctx->vbap2d, spec.channels);  // make sure we have the right speaker layout.
            }

            for (ALCsizei blocki = 0; blocki < ctx->num_source_blocks; blocki++) {
                SourceBlock *sb = ctx->source_blocks[blocki];
                if (sb->used > 0) {
                    for (ALsizei i = 0; i < SDL_arraysize(sb->sources); i++) {
                        ALsource *src = &sb->sources[i];
                        if (src->allocated) {
                            SDL_AudioSpec outspec;
                            SDL_GetAudioStreamFormat(src->stream, &outspec, NULL);
                            outspec.format = SDL_AUDIO_F32;
                            outspec.freq = spec.freq;
                            SDL_SetAudioStreamFormat(src->stream, NULL, &outspec);
                        }
                    }
                }
            }

            SDL_copyp(&ctx->spec, &spec);

            SDL_UnlockAudioStream(ctx->stream);
            ungrab_api_lock();
        }
    }

    return true;
}


static ALCcontext *_alcCreateContext(ALCdevice *device, const ALCint* attrlist)
{
    // we don't care about ALC_MONO_SOURCES or ALC_STEREO_SOURCES as we have no hardware limitation.

    if (!device) {
        set_alc_error(NULL, ALC_INVALID_DEVICE);
        return NULL;
    } else if (!SDL_GetAtomicInt(&device->connected)) {
        set_alc_error(device, ALC_INVALID_DEVICE);
        return NULL;
    }

    ALCsizei attrcount = 0;
    ALCint freq = 48000;
    ALCboolean freq_set = false;
    ALCboolean sync = ALC_FALSE;
    ALCint refresh = 100;

    if (attrlist != NULL) {
        ALCint attr;
        while ((attr = attrlist[attrcount++]) != 0) {
            switch (attr) {
                case ALC_FREQUENCY: freq = attrlist[attrcount++]; freq_set = true; break;
                case ALC_REFRESH: refresh = attrlist[attrcount++]; break;
                case ALC_SYNC: sync = (attrlist[attrcount++] ? ALC_TRUE : ALC_FALSE); break;
                default: FIXME("fail for unknown attributes?"); break;
            }
        }
    }

    FIXME("use these variables at some point"); (void) refresh; (void) sync;

    ALCcontext *ctx = (ALCcontext *) malloc_simd_aligned(sizeof (ALCcontext));
    if (!ctx) {
        set_alc_error(device, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    SDL_zerop(ctx);

    // Make sure everything that wants to use SIMD is aligned for it.
    SDL_assert( (((size_t) &ctx->listener.position[0]) % 16) == 0 );
    SDL_assert( (((size_t) &ctx->listener.orientation[0]) % 16) == 0 );
    SDL_assert( (((size_t) &ctx->listener.velocity[0]) % 16) == 0 );

    ctx->attributes = (ALCint *) SDL_malloc(attrcount * sizeof (ALCint));
    if (!ctx->attributes) {
        set_alc_error(device, ALC_OUT_OF_MEMORY);
        free_simd_aligned(ctx);
        return NULL;
    }
    SDL_memcpy(ctx->attributes, attrlist, attrcount * sizeof (ALCint));
    ctx->attributes_count = attrcount;

    SDL_AudioSpec spec;
    SDL_GetAudioDeviceFormat(device->device_id, &spec, NULL);
    if (freq_set) {
        spec.freq = freq;   // Only force a frequency if the context requested one. Otherwise just take the device-preferred format.
    }

    // always take device channels, spatialize to whatever they offer.
    //spec.channels = 2;

    // we always want to work in float32, to keep our work simple and let us use SIMD. If the device
    //  will take it, great, otherwise we'll let SDL convert when feeding the device.
    spec.format = SDL_AUDIO_F32;

    // each context gets an SDL_AudioStream, bound to the audio device.
    // Open a logical device for each context; first one might get can get the exact format we want from the hardware.
    ctx->stream = SDL_OpenAudioDeviceStream(device->device_id, &spec, context_callback, ctx);
    if (!ctx->stream) {
        SDL_free(ctx->attributes);
        free_simd_aligned(ctx);
        set_alc_error(device, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    ctx->device_id = SDL_GetAudioStreamDevice(ctx->stream);
    SDL_copyp(&ctx->spec, &spec);
    ctx->framesize = SDL_AUDIO_FRAMESIZE(spec);
    ctx->distance_model = AL_INVERSE_DISTANCE_CLAMPED;
    ctx->doppler_factor = 1.0f;
    ctx->doppler_velocity = 1.0f;
    ctx->speed_of_sound = 343.3f;
    ctx->listener.gain = 1.0f;
    ctx->listener.orientation[2] = -1.0f;
    ctx->listener.orientation[5] = 1.0f;
    ctx->device = device;
    context_needs_recalc(ctx);
    SDL_SetAtomicInt(&ctx->processing, 1);  // contexts default to processing

    // these are protected by the api lock; the mixer thread no longer looks at device->playback.contexts as of the migration to SDL3.
    if (device->playback.contexts != NULL) {
        SDL_assert(device->playback.contexts->prev == NULL);
        device->playback.contexts->prev = ctx;
    }
    ctx->next = device->playback.contexts;
    device->playback.contexts = ctx;

    VBAP2D_Init(&ctx->vbap2d, spec.channels);

    SDL_AddEventWatch(ContextDeviceChangeEventWatcher, ctx);

    SDL_ResumeAudioStreamDevice(ctx->stream);

    return ctx;
}
ENTRYPOINT(ALCcontext *,alcCreateContext,(ALCdevice *device, const ALCint* attrlist),(device,attrlist))

static SDL_AtomicInt tls_ctx_used;
static SDL_TLSID tlsid_current_ctx;
static SDL_INLINE ALCcontext *get_current_context(void)
{
    ALCcontext *tlsctx = (ALCcontext *) SDL_GetTLS(&tlsid_current_ctx);
    return tlsctx ? tlsctx : (ALCcontext *) SDL_GetAtomicPointer(&current_context);
}

// no api lock; it's thread-local.
ALCboolean alcSetThreadContext(ALCcontext *context)
{
    SDL_SetAtomicInt(&tls_ctx_used, 1);
    return SDL_SetTLS(&tlsid_current_ctx, context, NULL);
}

// no api lock; it's thread-local.
ALCcontext *alcGetThreadContext(void)
{
    return (ALCcontext *) SDL_GetTLS(&tlsid_current_ctx);
}

// no api lock; it just sets an atomic pointer at the moment
ALCboolean alcMakeContextCurrent(ALCcontext *ctx)
{
    FIXME("maybe just use the api lock");
    SDL_SetAtomicPointer(&current_context, ctx);
    FIXME("any reason this might return ALC_FALSE?");  // technically this could return false for an invalid non-NULL context but we don't keep a list of created contexts.
    if (SDL_GetAtomicInt(&tls_ctx_used)) {  // we don't want to create all the TLS infrastructure until someone has actually used ALC_EXT_thread_local_context, so query first.
        SDL_SetTLS(&tlsid_current_ctx, ctx, NULL);
    }
    return ALC_TRUE;
}

static void _alcProcessContext(ALCcontext *ctx)
{
    if (!ctx) {
        set_alc_error(NULL, ALC_INVALID_CONTEXT);
        return;
    }

    SDL_assert(!ctx->device->iscapture);
    SDL_SetAtomicInt(&ctx->processing, 1);
}
ENTRYPOINTVOID(alcProcessContext,(ALCcontext *ctx),(ctx))

static void _alcSuspendContext(ALCcontext *ctx)
{
    if (!ctx) {
        set_alc_error(NULL, ALC_INVALID_CONTEXT);
    } else {
        SDL_assert(!ctx->device->iscapture);
        SDL_SetAtomicInt(&ctx->processing, 0);
    }
}
ENTRYPOINTVOID(alcSuspendContext,(ALCcontext *ctx),(ctx))

static void _alcDestroyContext(ALCcontext *ctx)
{
    FIXME("Should NULL context be an error?");
    if (!ctx) {
        return;
    }

    // The spec says it's illegal to delete the current context.
    if (get_current_context() == ctx) {
        set_alc_error(ctx->device, ALC_INVALID_CONTEXT);
        return;
    }

    // do this first in case the mixer is running _right now_.
    SDL_SetAtomicInt(&ctx->processing, 0);

    SDL_RemoveEventWatch(ContextDeviceChangeEventWatcher, ctx);
    SDL_DestroyAudioStream(ctx->stream);  // will unbind from the audio device, mixer thread will no longer touch.

    // these are protected by the api lock; the mixer thread no longer looks at device->playback.contexts as of the migration to SDL3.
    if (ctx->prev) {
        ctx->prev->next = ctx->next;
    } else {
        SDL_assert(ctx == ctx->device->playback.contexts);
        ctx->device->playback.contexts = ctx->next;
    }
    if (ctx->next) {
        ctx->next->prev = ctx->prev;
    }

    for (ALCsizei blocki = 0; blocki < ctx->num_source_blocks; blocki++) {
        SourceBlock *sb = ctx->source_blocks[blocki];
        if (sb->used > 0) {
            for (ALsizei i = 0; i < SDL_arraysize(sb->sources); i++) {
                ALsource *src = &sb->sources[i];
                if (!src->allocated) {
                    continue;
                }

                SDL_DestroyAudioStream(src->stream);
                source_release_buffer_queue(ctx, src);
                if (--sb->used == 0) {
                    break;
                }
            }
        }
        free_simd_aligned(sb);
    }

    SDL_free(ctx->source_blocks);
    SDL_free(ctx->attributes);
    free_simd_aligned(ctx);
}
ENTRYPOINTVOID(alcDestroyContext,(ALCcontext *ctx),(ctx))

// no api lock; atomic (if not thread-local).
ALCcontext *alcGetCurrentContext(void)
{
    return get_current_context();
}

// no api lock; immutable.
ALCdevice *alcGetContextsDevice(ALCcontext *context)
{
    return context ? context->device : NULL;
}

static ALCenum _alcGetError(ALCdevice *device)
{
    ALCenum *perr = device ? &device->error : &null_device_error;
    const ALCenum retval = *perr;
    *perr = ALC_NO_ERROR;
    return retval;
}
ENTRYPOINT(ALCenum,alcGetError,(ALCdevice *device),(device))

// no api lock; immutable
ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname)
{
    #define ALC_EXTENSION_ITEM(ext) if (SDL_strcasecmp(extname, #ext) == 0) { return ALC_TRUE; }
    ALC_EXTENSION_ITEMS
    #undef ALC_EXTENSION_ITEM
    return ALC_FALSE;
}

// no api lock; immutable
void *alcGetProcAddress(ALCdevice *device, const ALCchar *funcname)
{
    if (!funcname) {
        set_alc_error(device, ALC_INVALID_VALUE);
        return NULL;
    }

    #define FN_TEST(fn) if (SDL_strcmp(funcname, #fn) == 0) return (void *) fn
    FN_TEST(alcCreateContext);
    FN_TEST(alcMakeContextCurrent);
    FN_TEST(alcProcessContext);
    FN_TEST(alcSuspendContext);
    FN_TEST(alcDestroyContext);
    FN_TEST(alcGetCurrentContext);
    FN_TEST(alcGetContextsDevice);
    FN_TEST(alcOpenDevice);
    FN_TEST(alcCloseDevice);
    FN_TEST(alcGetError);
    FN_TEST(alcIsExtensionPresent);
    FN_TEST(alcGetProcAddress);
    FN_TEST(alcGetEnumValue);
    FN_TEST(alcGetString);
    FN_TEST(alcGetIntegerv);
    FN_TEST(alcCaptureOpenDevice);
    FN_TEST(alcCaptureCloseDevice);
    FN_TEST(alcCaptureStart);
    FN_TEST(alcCaptureStop);
    FN_TEST(alcCaptureSamples);
    FN_TEST(alcSetThreadContext);
    FN_TEST(alcGetThreadContext);
    #undef FN_TEST

    set_alc_error(device, ALC_INVALID_VALUE);
    return NULL;
}

// no api lock; immutable
ALCenum alcGetEnumValue(ALCdevice *device, const ALCchar *enumname)
{
    if (!enumname) {
        set_alc_error(device, ALC_INVALID_VALUE);
        return (ALCenum) AL_NONE;
    }

    #define ENUM_TEST(en) if (SDL_strcmp(enumname, #en) == 0) return en
    ENUM_TEST(ALC_FALSE);
    ENUM_TEST(ALC_TRUE);
    ENUM_TEST(ALC_FREQUENCY);
    ENUM_TEST(ALC_REFRESH);
    ENUM_TEST(ALC_SYNC);
    ENUM_TEST(ALC_MONO_SOURCES);
    ENUM_TEST(ALC_STEREO_SOURCES);
    ENUM_TEST(ALC_NO_ERROR);
    ENUM_TEST(ALC_INVALID_DEVICE);
    ENUM_TEST(ALC_INVALID_CONTEXT);
    ENUM_TEST(ALC_INVALID_ENUM);
    ENUM_TEST(ALC_INVALID_VALUE);
    ENUM_TEST(ALC_OUT_OF_MEMORY);
    ENUM_TEST(ALC_MAJOR_VERSION);
    ENUM_TEST(ALC_MINOR_VERSION);
    ENUM_TEST(ALC_ATTRIBUTES_SIZE);
    ENUM_TEST(ALC_ALL_ATTRIBUTES);
    ENUM_TEST(ALC_DEFAULT_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_EXTENSIONS);
    ENUM_TEST(ALC_CAPTURE_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_CAPTURE_SAMPLES);
    ENUM_TEST(ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
    ENUM_TEST(ALC_ALL_DEVICES_SPECIFIER);
    ENUM_TEST(ALC_CONNECTED);
    #undef ENUM_TEST

    set_alc_error(device, ALC_INVALID_VALUE);
    return (ALCenum) AL_NONE;
}

static const ALCchar *calculate_sdl_device_list(const int iscapture)
{
    /* alcGetString() has to return a const string that is not freed and might
       continue to live even if we update this list in a later query, so we
       just make a big static buffer and hope it's large enough and that other
       race conditions don't bite us. The enumeration extension shouldn't have
       reused entry points, or done this silly null-delimited string list.
       Oh well. */
    #define DEVICE_LIST_BUFFER_SIZE 512
    static ALCchar playback_list[DEVICE_LIST_BUFFER_SIZE];
    static ALCchar capture_list[DEVICE_LIST_BUFFER_SIZE];
    ALCchar *final_list = iscapture ? capture_list : playback_list;
    ALCchar *ptr = final_list;
    size_t avail = DEVICE_LIST_BUFFER_SIZE;
    size_t cpy;

    // default device is always available.
    cpy = SDL_strlcpy(ptr, iscapture ? DEFAULT_CAPTURE_DEVICE : DEFAULT_PLAYBACK_DEVICE, avail);
    SDL_assert((cpy+1) < avail);
    ptr += cpy + 1;  // skip past null char.
    avail -= cpy + 1;

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        return NULL;
    }

    if (!init_api_lock()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    grab_api_lock();

    SDL_AudioDeviceID *devices = NULL;
    int numdevs = 0;
    if (iscapture) {
        devices = SDL_GetAudioRecordingDevices(&numdevs);
    } else {
        devices = SDL_GetAudioPlaybackDevices(&numdevs);
    }

    for (int i = 0; i < numdevs; i++) {
        const char *devname = SDL_GetAudioDeviceName(devices[i]);
        if (!devname) {
            continue;
        }

        const size_t devnamelen = SDL_strlen(devname);
        // if we're out of space, we just have to drop devices we can't cram in the buffer.
        if (avail > (devnamelen + 2)) {
            cpy = SDL_strlcpy(ptr, devname, avail);
            SDL_assert(cpy == devnamelen);
            SDL_assert((cpy+1) < avail);
            ptr += cpy + 1;  // skip past null char.
            avail -= cpy + 1;
        }
    }

    SDL_free(devices);

    SDL_assert(avail >= 1);
    *ptr = '\0';

    ungrab_api_lock();

    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return final_list;

    #undef DEVICE_LIST_BUFFER_SIZE
}

// no api lock; immutable (unless it isn't, then we manually lock).
const ALCchar *alcGetString(ALCdevice *device, ALCenum param)
{
    switch (param) {
        case ALC_EXTENSIONS: {
            #define ALC_EXTENSION_ITEM(ext) " " #ext
            static ALCchar alc_extensions_string[] = ALC_EXTENSION_ITEMS;
            #undef ALC_EXTENSION_ITEM
            return alc_extensions_string + 1;  // skip that first space char
        }

        // You open the default SDL device with a NULL device name, but that is how OpenAL
        //  reports an error here, so we give it a magic identifier here instead.
        case ALC_DEFAULT_DEVICE_SPECIFIER:
            return DEFAULT_PLAYBACK_DEVICE;

        case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
            return DEFAULT_CAPTURE_DEVICE;

        case ALC_DEVICE_SPECIFIER:
            FIXME("should return NULL if device->iscapture?");
            return device ? device->name : calculate_sdl_device_list(0);

        case ALC_CAPTURE_DEVICE_SPECIFIER:
            FIXME("should return NULL if !device->iscapture?");
            return device ? device->name : calculate_sdl_device_list(1);

        case ALC_NO_ERROR: return "ALC_NO_ERROR";
        case ALC_INVALID_DEVICE: return "ALC_INVALID_DEVICE";
        case ALC_INVALID_CONTEXT:return "ALC_INVALID_CONTEXT";
        case ALC_INVALID_ENUM: return "ALC_INVALID_ENUM";
        case ALC_INVALID_VALUE: return "ALC_INVALID_VALUE";
        case ALC_OUT_OF_MEMORY: return "ALC_OUT_OF_MEMORY";

        default: break;
    }

    FIXME("other enums that should report as strings?");
    set_alc_error(device, ALC_INVALID_ENUM);
    return NULL;
}

static void _alcGetIntegerv(ALCdevice *device, const ALCenum param, const ALCsizei size, ALCint *values)
{
    if (!size || !values) {
        return;  // "A NULL destination or a zero size parameter will cause ALC to ignore the query."
    }

    ALCcontext *ctx = NULL;
    SDL_AudioSpec spec;

    switch (param) {
        case ALC_CAPTURE_SAMPLES:
            if (!device || !device->iscapture) {
                set_alc_error(device, ALC_INVALID_DEVICE);
            } else {
                *values = (ALCint) SDL_GetAudioStreamAvailable(device->capture.stream) / device->capture.framesize;
            }
            return;

        case ALC_CONNECTED:
            if (device) {
                *values = SDL_GetAtomicInt(&device->connected) ? ALC_TRUE : ALC_FALSE;
            } else {
                *values = ALC_FALSE;
                set_alc_error(device, ALC_INVALID_DEVICE);
            }
            return;

        case ALC_ATTRIBUTES_SIZE:
        case ALC_ALL_ATTRIBUTES:
            if (!device || device->iscapture) {
                *values = 0;
                set_alc_error(device, ALC_INVALID_DEVICE);
                return;
            }

            ctx = get_current_context();

            FIXME("wants 'current context of specified device', but there isn't a current context per-device...");
            if ((!ctx) || (ctx->device != device)) {
                *values = 0;
                set_alc_error(device, ALC_INVALID_CONTEXT);
                return;
            }

            if (param == ALC_ALL_ATTRIBUTES) {
                if (size < ctx->attributes_count) {
                    *values = 0;
                    set_alc_error(device, ALC_INVALID_VALUE);
                    return;
                }
                SDL_memcpy(values, ctx->attributes, ctx->attributes_count * sizeof (ALCint));
            } else {
                *values = (ALCint) ctx->attributes_count;
            }
            return;

        case ALC_MAJOR_VERSION:
            *values = OPENAL_VERSION_MAJOR;
            return;

        case ALC_MINOR_VERSION:
            *values = OPENAL_VERSION_MINOR;
            return;

        case ALC_FREQUENCY:
            if (!device || !SDL_GetAudioDeviceFormat(device->device_id, &spec, NULL)) {
                *values = 0;
                set_alc_error(device, ALC_INVALID_DEVICE);
            } else {
                *values = (ALCint) spec.freq;
            }
            return;

        default: break;
    }

    set_alc_error(device, ALC_INVALID_ENUM);
    *values = 0;
}
ENTRYPOINTVOID(alcGetIntegerv,(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values),(device,param,size,values))


// audio callback for capture devices just needs to drop data if we're overfull.
//  SDL should have handled resampling and conversion for us to the expected
//  audio format.
static void SDLCALL capture_device_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    ALCdevice *device = (ALCdevice *) userdata;

    SDL_assert(device->iscapture);
    SDL_assert(stream == device->capture.stream);

    Uint8 bitbucket[512];
    const ALCsizei framesize = device->capture.framesize;
    const ALCsizei maximum = device->capture.max_samples * framesize;
    const ALCsizei maxread = sizeof (bitbucket) / framesize;
    ALCsizei available = SDL_GetAudioStreamAvailable(stream);
    while (available > maximum) {
        const ALCsizei dumpsamps = (ALCsizei) SDL_min(maxread, (available - maximum) / framesize);
        if (!SDL_GetAudioStreamData(stream, bitbucket, dumpsamps * framesize)) {
            SDL_ClearAudioStream(stream);
            return;  // oh well.
        }
        available = SDL_GetAudioStreamAvailable(stream);
    }
}

// no api lock; this creates it and otherwise doesn't have any state that can race
ALCdevice *alcCaptureOpenDevice(const ALCchar *devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize)
{
    SDL_AudioSpec spec;
    ALCsizei framesize = 0;

    SDL_zero(spec);
    if (!alcfmt_to_sdlfmt(format, &spec.format, &spec.channels, &framesize)) {
        return NULL;
    }
    spec.freq = frequency;

    if (!devicename) {
        devicename = DEFAULT_CAPTURE_DEVICE;  // so ALC_CAPTURE_DEVICE_SPECIFIER is meaningful
    }

    ALCdevice *device = prep_alc_device(devicename, ALC_TRUE);
    if (!device) {
        return NULL;
    }

    SDL_copyp(&device->capture.spec, &spec);
    device->capture.framesize = framesize;
    device->capture.max_samples = buffersize;

    device->capture.stream = SDL_OpenAudioDeviceStream(device->device_id, &spec, capture_device_callback, device);
    if (!device->capture.stream) {
        SDL_free(device->name);
        SDL_free(device);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    device->capture.framesize = SDL_AUDIO_FRAMESIZE(spec);

    SDL_AddEventWatch(DeviceDisconnectedEventWatcher, device);

    return device;
}

// no api lock; this requires you to not destroy a device that's still in use
ALCboolean alcCaptureCloseDevice(ALCdevice *device)
{
    if (!device || !device->iscapture) {
        return ALC_FALSE;
    }

    SDL_RemoveEventWatch(DeviceDisconnectedEventWatcher, device);
    SDL_DestroyAudioStream(device->capture.stream);
    SDL_free(device->name);
    SDL_free(device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return ALC_TRUE;
}

static void _alcCaptureStart(ALCdevice *device)
{
    if (device && device->iscapture && SDL_GetAtomicInt(&device->connected)) {
        // alcCaptureStart() drops any previously-buffered data.
        SDL_ClearAudioStream(device->capture.stream);
        SDL_ResumeAudioStreamDevice(device->capture.stream);
    }
}
ENTRYPOINTVOID(alcCaptureStart,(ALCdevice *device),(device))

static void _alcCaptureStop(ALCdevice *device)
{
    if (device && device->iscapture) {
        SDL_PauseAudioStreamDevice(device->capture.stream);
    }
}
ENTRYPOINTVOID(alcCaptureStop,(ALCdevice *device),(device))

static void _alcCaptureSamples(ALCdevice *device, ALCvoid *buffer, const ALCsizei samples)
{
    if (!device || !device->iscapture) {
        return;
    }

    const ALCsizei requested_bytes = samples * device->capture.framesize;
    if (requested_bytes > SDL_GetAudioStreamAvailable(device->capture.stream)) {
        FIXME("set error state?");
        return;  // this is an error state, according to the spec.
    }

    FIXME("does ALC allow requesting a partial frame?");
    SDL_GetAudioStreamData(device->capture.stream, buffer, requested_bytes);
}
ENTRYPOINTVOID(alcCaptureSamples,(ALCdevice *device, ALCvoid *buffer, ALCsizei samples),(device,buffer,samples))



// AL implementation...

static ALenum null_context_error = AL_NO_ERROR;

static void set_al_error(ALCcontext *ctx, const ALenum error)
{
    ALenum *perr = ctx ? &ctx->error : &null_context_error;
    // can't set a new error when the previous hasn't been cleared yet.
    if (*perr == AL_NO_ERROR) {
        *perr = error;
    }
}

// !!! FIXME: buffers and sources use almost identical code for blocks
static ALsource *get_source(ALCcontext *ctx, const ALuint name, SourceBlock **_block)
{
    //SDL_Log("get_source(%d): blockidx=%d, block_offset=%d", (int) name, (int) blockidx, (int) block_offset);

    const ALsizei blockidx = (((ALsizei) name) - 1) / OPENAL_SOURCE_BLOCK_SIZE;

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        goto failed;
    } else if ((name == 0) || (blockidx < 0) || (blockidx >= ctx->num_source_blocks)) {
        set_al_error(ctx, AL_INVALID_NAME);
        goto failed;
    }

    const ALsizei block_offset = (((ALsizei) name) - 1) % OPENAL_SOURCE_BLOCK_SIZE;
    SourceBlock *block = ctx->source_blocks[blockidx];
    ALsource *source = &block->sources[block_offset];
    if (!source->allocated) {
        set_al_error(ctx, AL_INVALID_NAME);
        goto failed;
    }

    if (_block) {
        *_block = block;
    }
    return source;

failed:
    if (_block) {
        *_block = NULL;
    }
    return NULL;
}

// !!! FIXME: buffers and sources use almost identical code for blocks
static ALbuffer *get_buffer(ALCcontext *ctx, const ALuint name, BufferBlock **_block)
{
    const ALsizei blockidx = (((ALsizei) name) - 1) / OPENAL_BUFFER_BLOCK_SIZE;

    //SDL_Log("get_buffer(%d): blockidx=%d, block_offset=%d", (int) name, (int) blockidx, (int) block_offset);

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        goto failed;
    } else if ((name == 0) || (blockidx < 0) || (blockidx >= ctx->device->playback.num_buffer_blocks)) {
        set_al_error(ctx, AL_INVALID_NAME);
        goto failed;
    }

    const ALsizei block_offset = (((ALsizei) name) - 1) % OPENAL_BUFFER_BLOCK_SIZE;
    BufferBlock *block = ctx->device->playback.buffer_blocks[blockidx];
    ALbuffer *buffer = &block->buffers[block_offset];
    if (!buffer->allocated) {
        set_al_error(ctx, AL_INVALID_NAME);
        goto failed;
    }

    if (_block) {
        *_block = block;
    }
    return buffer;

failed:
    if (_block) {
        *_block = NULL;
    }
    return NULL;
}

static void _alDopplerFactor(const ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (value < 0.0f) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ctx->doppler_factor = value;
        context_needs_recalc(ctx);
    }
}
ENTRYPOINTVOID(alDopplerFactor,(ALfloat value),(value))

static void _alDopplerVelocity(const ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (value < 0.0f) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ctx->doppler_velocity = value;
        context_needs_recalc(ctx);
    }
}
ENTRYPOINTVOID(alDopplerVelocity,(ALfloat value),(value))

static void _alSpeedOfSound(const ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (value < 0.0f) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ctx->speed_of_sound = value;
        context_needs_recalc(ctx);
    }
}
ENTRYPOINTVOID(alSpeedOfSound,(ALfloat value),(value))

static void _alDistanceModel(const ALenum model)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    switch (model) {
        case AL_NONE:
        case AL_INVERSE_DISTANCE:
        case AL_INVERSE_DISTANCE_CLAMPED:
        case AL_LINEAR_DISTANCE:
        case AL_LINEAR_DISTANCE_CLAMPED:
        case AL_EXPONENT_DISTANCE:
        case AL_EXPONENT_DISTANCE_CLAMPED:
            ctx->distance_model = model;
            context_needs_recalc(ctx);
            return;
        default: break;
    }
    set_al_error(ctx, AL_INVALID_ENUM);
}
ENTRYPOINTVOID(alDistanceModel,(ALenum model),(model))


static void enable_disable_toggle(const ALenum capability, const ALboolean toggle)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    switch (capability)
    {
        case AL_SOURCE_DISTANCE_MODEL:
            ctx->source_distance_model = toggle ? ALC_TRUE : ALC_FALSE;
            context_needs_recalc(ctx);
            break;

        default: break;
    }
    set_al_error(ctx, AL_INVALID_ENUM);
}

static void _alEnable(const ALenum capability)
{
    enable_disable_toggle(capability, AL_TRUE);
}
ENTRYPOINTVOID(alEnable,(ALenum capability),(capability))


static void _alDisable(const ALenum capability)
{
    enable_disable_toggle(capability, AL_FALSE);
}
ENTRYPOINTVOID(alDisable,(ALenum capability),(capability))


static ALboolean _alIsEnabled(const ALenum capability)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return AL_FALSE;
    }

    switch (capability)
    {
        case AL_SOURCE_DISTANCE_MODEL: return ctx->source_distance_model;
        default: break;
    }
    set_al_error(ctx, AL_INVALID_ENUM);
    return AL_FALSE;
}
ENTRYPOINT(ALboolean,alIsEnabled,(ALenum capability),(capability))

static const ALchar *_alGetString(const ALenum param)
{
    switch (param) {
        case AL_EXTENSIONS: {
            #define AL_EXTENSION_ITEM(ext) " " #ext
            static ALchar al_extensions_string[] = AL_EXTENSION_ITEMS;
            #undef AL_EXTENSION_ITEM
            return al_extensions_string + 1;  // skip that first space char
        }

        case AL_VERSION: return OPENAL_VERSION_STRING;
        case AL_RENDERER: return OPENAL_RENDERER_STRING;
        case AL_VENDOR: return OPENAL_VENDOR_STRING;
        case AL_NO_ERROR: return "AL_NO_ERROR";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";

        default: break;
    }

    FIXME("other enums that should report as strings?");
    set_al_error(get_current_context(), AL_INVALID_ENUM);

    return NULL;
}
ENTRYPOINT(const ALchar *,alGetString,(ALenum param),(param))

static void _alGetBooleanv(const ALenum param, ALboolean *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  // legal no-op

    // nothing in core OpenAL 1.1 uses this
    set_al_error(ctx, AL_INVALID_ENUM);
}
ENTRYPOINTVOID(alGetBooleanv,(ALenum param, ALboolean *values),(param,values))

static void _alGetIntegerv(const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  // legal no-op

    switch (param) {
        case AL_DISTANCE_MODEL: *values = (ALint) ctx->distance_model; break;
        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetIntegerv,(ALenum param, ALint *values),(param,values))

static void _alGetFloatv(const ALenum param, ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  // legal no-op

    switch (param) {
        case AL_DOPPLER_FACTOR: *values = ctx->doppler_factor; break;
        case AL_DOPPLER_VELOCITY: *values = ctx->doppler_velocity; break;
        case AL_SPEED_OF_SOUND: *values = ctx->speed_of_sound; break;
        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetFloatv,(ALenum param, ALfloat *values),(param,values))

static void _alGetDoublev(const ALenum param, ALdouble *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  // legal no-op

    // nothing in core OpenAL 1.1 uses this
    set_al_error(ctx, AL_INVALID_ENUM);
}
ENTRYPOINTVOID(alGetDoublev,(ALenum param, ALdouble *values),(param,values))

// no api lock; just passes through to the real api
ALboolean alGetBoolean(ALenum param)
{
    ALboolean retval = AL_FALSE;
    alGetBooleanv(param, &retval);
    return retval;
}

// no api lock; just passes through to the real api
ALint alGetInteger(ALenum param)
{
    ALint retval = 0;
    alGetIntegerv(param, &retval);
    return retval;
}

// no api lock; just passes through to the real api
ALfloat alGetFloat(ALenum param)
{
    ALfloat retval = 0.0f;
    alGetFloatv(param, &retval);
    return retval;
}

// no api lock; just passes through to the real api
ALdouble alGetDouble(ALenum param)
{
    ALdouble retval = 0.0f;
    alGetDoublev(param, &retval);
    return retval;
}

static ALenum _alGetError(void)
{
    ALCcontext *ctx = get_current_context();
    ALenum *perr = ctx ? &ctx->error : &null_context_error;
    const ALenum retval = *perr;
    *perr = AL_NO_ERROR;
    return retval;
}
ENTRYPOINT(ALenum,alGetError,(void),())

// no api lock; immutable (unless we start having contexts with different extensions)
ALboolean alIsExtensionPresent(const ALchar *extname)
{
    #define AL_EXTENSION_ITEM(ext) if (SDL_strcasecmp(extname, #ext) == 0) { return AL_TRUE; }
    AL_EXTENSION_ITEMS
    #undef AL_EXTENSION_ITEM
    return AL_FALSE;
}

static void *_alGetProcAddress(const ALchar *funcname)
{
    ALCcontext *ctx = get_current_context();
    FIXME("fail if ctx == NULL?");
    if (!funcname) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return NULL;
    }

    #define FN_TEST(fn) if (SDL_strcmp(funcname, #fn) == 0) return (void *) fn
    FN_TEST(alDopplerFactor);
    FN_TEST(alDopplerVelocity);
    FN_TEST(alSpeedOfSound);
    FN_TEST(alDistanceModel);
    FN_TEST(alEnable);
    FN_TEST(alDisable);
    FN_TEST(alIsEnabled);
    FN_TEST(alGetString);
    FN_TEST(alGetBooleanv);
    FN_TEST(alGetIntegerv);
    FN_TEST(alGetFloatv);
    FN_TEST(alGetDoublev);
    FN_TEST(alGetBoolean);
    FN_TEST(alGetInteger);
    FN_TEST(alGetFloat);
    FN_TEST(alGetDouble);
    FN_TEST(alGetError);
    FN_TEST(alIsExtensionPresent);
    FN_TEST(alGetProcAddress);
    FN_TEST(alGetEnumValue);
    FN_TEST(alListenerf);
    FN_TEST(alListener3f);
    FN_TEST(alListenerfv);
    FN_TEST(alListeneri);
    FN_TEST(alListener3i);
    FN_TEST(alListeneriv);
    FN_TEST(alGetListenerf);
    FN_TEST(alGetListener3f);
    FN_TEST(alGetListenerfv);
    FN_TEST(alGetListeneri);
    FN_TEST(alGetListener3i);
    FN_TEST(alGetListeneriv);
    FN_TEST(alGenSources);
    FN_TEST(alDeleteSources);
    FN_TEST(alIsSource);
    FN_TEST(alSourcef);
    FN_TEST(alSource3f);
    FN_TEST(alSourcefv);
    FN_TEST(alSourcei);
    FN_TEST(alSource3i);
    FN_TEST(alSourceiv);
    FN_TEST(alGetSourcef);
    FN_TEST(alGetSource3f);
    FN_TEST(alGetSourcefv);
    FN_TEST(alGetSourcei);
    FN_TEST(alGetSource3i);
    FN_TEST(alGetSourceiv);
    FN_TEST(alSourcePlayv);
    FN_TEST(alSourceStopv);
    FN_TEST(alSourceRewindv);
    FN_TEST(alSourcePausev);
    FN_TEST(alSourcePlay);
    FN_TEST(alSourceStop);
    FN_TEST(alSourceRewind);
    FN_TEST(alSourcePause);
    FN_TEST(alSourceQueueBuffers);
    FN_TEST(alSourceUnqueueBuffers);
    FN_TEST(alGenBuffers);
    FN_TEST(alDeleteBuffers);
    FN_TEST(alIsBuffer);
    FN_TEST(alBufferData);
    FN_TEST(alBufferf);
    FN_TEST(alBuffer3f);
    FN_TEST(alBufferfv);
    FN_TEST(alBufferi);
    FN_TEST(alBuffer3i);
    FN_TEST(alBufferiv);
    FN_TEST(alGetBufferf);
    FN_TEST(alGetBuffer3f);
    FN_TEST(alGetBufferfv);
    FN_TEST(alGetBufferi);
    FN_TEST(alGetBuffer3i);
    FN_TEST(alGetBufferiv);
    #undef FN_TEST

    set_al_error(ctx, ALC_INVALID_VALUE);
    return NULL;
}
ENTRYPOINT(void *,alGetProcAddress,(const ALchar *funcname),(funcname))

static ALenum _alGetEnumValue(const ALchar *enumname)
{
    ALCcontext *ctx = get_current_context();
    FIXME("fail if ctx == NULL?");
    if (!enumname) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return AL_NONE;
    }

    #define ENUM_TEST(en) if (SDL_strcmp(enumname, #en) == 0) return en
    ENUM_TEST(AL_NONE);
    ENUM_TEST(AL_FALSE);
    ENUM_TEST(AL_TRUE);
    ENUM_TEST(AL_SOURCE_RELATIVE);
    ENUM_TEST(AL_CONE_INNER_ANGLE);
    ENUM_TEST(AL_CONE_OUTER_ANGLE);
    ENUM_TEST(AL_PITCH);
    ENUM_TEST(AL_POSITION);
    ENUM_TEST(AL_DIRECTION);
    ENUM_TEST(AL_VELOCITY);
    ENUM_TEST(AL_LOOPING);
    ENUM_TEST(AL_BUFFER);
    ENUM_TEST(AL_GAIN);
    ENUM_TEST(AL_MIN_GAIN);
    ENUM_TEST(AL_MAX_GAIN);
    ENUM_TEST(AL_ORIENTATION);
    ENUM_TEST(AL_SOURCE_STATE);
    ENUM_TEST(AL_INITIAL);
    ENUM_TEST(AL_PLAYING);
    ENUM_TEST(AL_PAUSED);
    ENUM_TEST(AL_STOPPED);
    ENUM_TEST(AL_BUFFERS_QUEUED);
    ENUM_TEST(AL_BUFFERS_PROCESSED);
    ENUM_TEST(AL_REFERENCE_DISTANCE);
    ENUM_TEST(AL_ROLLOFF_FACTOR);
    ENUM_TEST(AL_CONE_OUTER_GAIN);
    ENUM_TEST(AL_MAX_DISTANCE);
    ENUM_TEST(AL_SEC_OFFSET);
    ENUM_TEST(AL_SAMPLE_OFFSET);
    ENUM_TEST(AL_BYTE_OFFSET);
    ENUM_TEST(AL_SOURCE_TYPE);
    ENUM_TEST(AL_STATIC);
    ENUM_TEST(AL_STREAMING);
    ENUM_TEST(AL_UNDETERMINED);
    ENUM_TEST(AL_FORMAT_MONO8);
    ENUM_TEST(AL_FORMAT_MONO16);
    ENUM_TEST(AL_FORMAT_STEREO8);
    ENUM_TEST(AL_FORMAT_STEREO16);
    ENUM_TEST(AL_FREQUENCY);
    ENUM_TEST(AL_BITS);
    ENUM_TEST(AL_CHANNELS);
    ENUM_TEST(AL_SIZE);
    ENUM_TEST(AL_UNUSED);
    ENUM_TEST(AL_PENDING);
    ENUM_TEST(AL_PROCESSED);
    ENUM_TEST(AL_NO_ERROR);
    ENUM_TEST(AL_INVALID_NAME);
    ENUM_TEST(AL_INVALID_ENUM);
    ENUM_TEST(AL_INVALID_VALUE);
    ENUM_TEST(AL_INVALID_OPERATION);
    ENUM_TEST(AL_OUT_OF_MEMORY);
    ENUM_TEST(AL_VENDOR);
    ENUM_TEST(AL_VERSION);
    ENUM_TEST(AL_RENDERER);
    ENUM_TEST(AL_EXTENSIONS);
    ENUM_TEST(AL_DOPPLER_FACTOR);
    ENUM_TEST(AL_DOPPLER_VELOCITY);
    ENUM_TEST(AL_SPEED_OF_SOUND);
    ENUM_TEST(AL_DISTANCE_MODEL);
    ENUM_TEST(AL_INVERSE_DISTANCE);
    ENUM_TEST(AL_INVERSE_DISTANCE_CLAMPED);
    ENUM_TEST(AL_LINEAR_DISTANCE);
    ENUM_TEST(AL_LINEAR_DISTANCE_CLAMPED);
    ENUM_TEST(AL_EXPONENT_DISTANCE);
    ENUM_TEST(AL_EXPONENT_DISTANCE_CLAMPED);
    ENUM_TEST(AL_FORMAT_MONO_FLOAT32);
    ENUM_TEST(AL_FORMAT_STEREO_FLOAT32);
    ENUM_TEST(AL_FORMAT_MONO_I32);
    ENUM_TEST(AL_FORMAT_STEREO_I32);
    ENUM_TEST(AL_SOURCE_DISTANCE_MODEL);
    #undef ENUM_TEST

    set_al_error(ctx, AL_INVALID_VALUE);
    return AL_NONE;
}
ENTRYPOINT(ALenum,alGetEnumValue,(const ALchar *enumname),(enumname))

static void _alListenerfv(const ALenum param, const ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (!values) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ALboolean recalc = AL_TRUE;
        switch (param) {
            case AL_GAIN:
                ctx->listener.gain = *values;
                break;

            case AL_POSITION:
                SDL_memcpy(ctx->listener.position, values, sizeof (*values) * 3);
                break;

            case AL_VELOCITY:
                SDL_memcpy(ctx->listener.velocity, values, sizeof (*values) * 3);
                break;

            case AL_ORIENTATION:
                SDL_memcpy(&ctx->listener.orientation[0], &values[0], sizeof (*values) * 3);
                SDL_memcpy(&ctx->listener.orientation[4], &values[3], sizeof (*values) * 3);
                break;

            default:
                recalc = AL_FALSE;
                set_al_error(ctx, AL_INVALID_ENUM);
                break;
        }

        if (recalc) {
            context_needs_recalc(ctx);
        }
    }
}
ENTRYPOINTVOID(alListenerfv,(ALenum param, const ALfloat *values),(param,values))

static void _alListenerf(const ALenum param, const ALfloat value)
{
    switch (param) {
        case AL_GAIN: _alListenerfv(param, &value); break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alListenerf,(ALenum param, ALfloat value),(param,value))

static void _alListener3f(const ALenum param, const ALfloat value1, const ALfloat value2, const ALfloat value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY: {
            const ALfloat values[3] = { value1, value2, value3 };
            _alListenerfv(param, values);
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alListener3f,(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3),(param,value1,value2,value3))

static void _alListeneriv(const ALenum param, const ALint *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (!values) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ALboolean recalc = AL_TRUE;
        FIXME("Not atomic vs the mixer thread");  // maybe have a latching system?
        switch (param) {
            case AL_POSITION:
                ctx->listener.position[0] = (ALfloat) values[0];
                ctx->listener.position[1] = (ALfloat) values[1];
                ctx->listener.position[2] = (ALfloat) values[2];
                break;

            case AL_VELOCITY:
                ctx->listener.velocity[0] = (ALfloat) values[0];
                ctx->listener.velocity[1] = (ALfloat) values[1];
                ctx->listener.velocity[2] = (ALfloat) values[2];
                break;

            case AL_ORIENTATION:
                ctx->listener.orientation[0] = (ALfloat) values[0];
                ctx->listener.orientation[1] = (ALfloat) values[1];
                ctx->listener.orientation[2] = (ALfloat) values[2];
                ctx->listener.orientation[4] = (ALfloat) values[3];
                ctx->listener.orientation[5] = (ALfloat) values[4];
                ctx->listener.orientation[6] = (ALfloat) values[5];
                break;

            default:
                recalc = AL_FALSE;
                set_al_error(ctx, AL_INVALID_ENUM);
                break;
        }

        if (recalc) {
            context_needs_recalc(ctx);
        }
    }
}
ENTRYPOINTVOID(alListeneriv,(ALenum param, const ALint *values),(param,values))

static void _alListeneri(const ALenum param, const ALint value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in AL 1.1 uses this
}
ENTRYPOINTVOID(alListeneri,(ALenum param, ALint value),(param,value))

static void _alListener3i(const ALenum param, const ALint value1, const ALint value2, const ALint value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY: {
            const ALint values[3] = { value1, value2, value3 };
            _alListeneriv(param, values);
            break;
        }
        default:
            set_al_error(get_current_context(), AL_INVALID_ENUM);
            break;
    }
}
ENTRYPOINTVOID(alListener3i,(ALenum param, ALint value1, ALint value2, ALint value3),(param,value1,value2,value3))

static void _alGetListenerfv(const ALenum param, ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  // legal no-op

    switch (param) {
        case AL_GAIN:
            *values = ctx->listener.gain;
            break;

        case AL_POSITION:
            SDL_memcpy(values, ctx->listener.position, sizeof (ALfloat) * 3);
            break;

        case AL_VELOCITY:
            SDL_memcpy(values, ctx->listener.velocity, sizeof (ALfloat) * 3);
            break;

        case AL_ORIENTATION:
            SDL_memcpy(&values[0], &ctx->listener.orientation[0], sizeof (ALfloat) * 3);
            SDL_memcpy(&values[3], &ctx->listener.orientation[4], sizeof (ALfloat) * 3);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListenerfv,(ALenum param, ALfloat *values),(param,values))

static void _alGetListenerf(const ALenum param, ALfloat *value)
{
    switch (param) {
        case AL_GAIN: _alGetListenerfv(param, value); break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListenerf,(ALenum param, ALfloat *value),(param,value))


static void _alGetListener3f(const ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    ALfloat values[3];
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
            _alGetListenerfv(param, values);
            if (value1) *value1 = values[0];
            if (value2) *value2 = values[1];
            if (value3) *value3 = values[2];
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListener3f,(ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3),(param,value1,value2,value3))


static void _alGetListeneri(const ALenum param, ALint *value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in AL 1.1 uses this
}
ENTRYPOINTVOID(alGetListeneri,(ALenum param, ALint *value),(param,value))


static void _alGetListeneriv(const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) {
        return;  // legal no-op
    }

    switch (param) {
        case AL_POSITION:
            values[0] = (ALint) ctx->listener.position[0];
            values[1] = (ALint) ctx->listener.position[1];
            values[2] = (ALint) ctx->listener.position[2];
            break;

        case AL_VELOCITY:
            values[0] = (ALint) ctx->listener.velocity[0];
            values[1] = (ALint) ctx->listener.velocity[1];
            values[2] = (ALint) ctx->listener.velocity[2];
            break;

        case AL_ORIENTATION:
            values[0] = (ALint) ctx->listener.orientation[0];
            values[1] = (ALint) ctx->listener.orientation[1];
            values[2] = (ALint) ctx->listener.orientation[2];
            values[3] = (ALint) ctx->listener.orientation[4];
            values[4] = (ALint) ctx->listener.orientation[5];
            values[5] = (ALint) ctx->listener.orientation[6];
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListeneriv,(ALenum param, ALint *values),(param,values))

static void _alGetListener3i(const ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    ALint values[3];
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
            _alGetListeneriv(param, values);
            if (value1) { *value1 = values[0]; }
            if (value2) { *value2 = values[1]; }
            if (value3) { *value3 = values[2]; }
            break;

        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListener3i,(ALenum param, ALint *value1, ALint *value2, ALint *value3),(param,value1,value2,value3))

// !!! FIXME: buffers and sources use almost identical code for blocks
static void _alGenSources(const ALsizei n, ALuint *names)
{
    ALCcontext *ctx = get_current_context();

    if (n < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    } else if (n == 0) {
        return;  // not an error, but nothing to do.
    }

    SDL_AudioStream *stackstreams[16];
    SDL_AudioStream **streams = stackstreams;
    ALsource *stackobjs[16];
    ALsource **objects = stackobjs;

    if (n <= SDL_arraysize(stackobjs)) {
        SDL_memset(stackobjs, '\0', sizeof (ALsource *) * n);
        SDL_memset(stackstreams, '\0', sizeof (SDL_AudioStream *) * n);
    } else {
        objects = (ALsource **) SDL_calloc(n, sizeof (ALsource *));
        if (!objects) {
            set_al_error(ctx, AL_OUT_OF_MEMORY);
            return;
        }
        streams = (SDL_AudioStream **) SDL_calloc(n, sizeof (SDL_AudioStream *));
        if (!streams) {
            SDL_free(objects);
            set_al_error(ctx, AL_OUT_OF_MEMORY);
            return;
        }
    }

    // allocate all our SDL_AudioStreams up front, so if this fails, we haven't changed AL state.
    for (ALsizei i = 0; i < n; i++) {
        streams[i] = SDL_CreateAudioStream(&ctx->spec, &ctx->spec);
        if (!streams[i]) {
            for (ALsizei j = 0; j < i; j++) {
                SDL_DestroyAudioStream(streams[j]);
            }
            if (objects != stackobjs) { SDL_free(objects); }
            if (streams != stackstreams) { SDL_free(streams); }
            set_al_error(ctx, AL_OUT_OF_MEMORY);
            return;
        }
    }

    ALsizei found = 0;

    ALsizei totalblocks = ctx->num_source_blocks;
    ALsizei block_offset = 0;
    for (ALsizei blocki = 0; blocki < totalblocks; blocki++) {
        SourceBlock *block = ctx->source_blocks[blocki];
        block->tmp = 0;
        if (block->used < SDL_arraysize(block->sources)) {  // skip if full
            for (ALsizei i = 0; i < SDL_arraysize(block->sources); i++) {
                // if a playing source was deleted, it will still be marked mixer_accessible
                //  until the mixer thread shuffles it out. Until then, the source isn't
                //  available for reuse.
                if (!block->sources[i].allocated && !SDL_GetAtomicInt(&block->sources[i].mixer_accessible)) {
                    block->tmp++;
                    objects[found] = &block->sources[i];
                    names[found++] = (i + block_offset) + 1;  // +1 so it isn't zero.
                    if (found == n) {
                        break;
                    }
                }
            }

            if (found == n) {
                break;
            }
        }

        block_offset += SDL_arraysize(block->sources);
    }

    ALboolean out_of_memory = AL_FALSE;
    while (found < n) {  // out of blocks? Add new ones.
        // ctx->source_blocks is only accessed on the API thread under a mutex, so it's safe to realloc.
        void *ptr = SDL_realloc(ctx->source_blocks, sizeof (SourceBlock *) * (totalblocks + 1));
        if (!ptr) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->source_blocks = (SourceBlock **) ptr;

        SourceBlock *block = (SourceBlock *) malloc_simd_aligned(sizeof (SourceBlock));
        if (!block) {
            out_of_memory = AL_TRUE;
            break;
        }
        SDL_zerop(block);
        ctx->source_blocks[totalblocks] = block;
        totalblocks++;
        ctx->num_source_blocks++;

        for (ALsizei i = 0; i < SDL_arraysize(block->sources); i++) {
            block->tmp++;
            objects[found] = &block->sources[i];
            names[found++] = (i + block_offset) + 1;  // +1 so it isn't zero.
            if (found == n) {
                break;
            }
        }
        block_offset += SDL_arraysize(block->sources);
    }

    if (out_of_memory) {
        for (ALsizei i = 0; i < n; i++) {
            SDL_DestroyAudioStream(streams[i]);
        }
        if (streams != stackstreams) { SDL_free(streams); }
        if (objects != stackobjs) { SDL_free(objects); }
        SDL_memset(names, '\0', sizeof (*names) * n);
        set_al_error(ctx, AL_OUT_OF_MEMORY);
        return;
    }

    SDL_assert(found == n);  // we should have either gotten space or bailed on alloc failure

    // update the "used" field in blocks with items we are taking now.
    found = 0;
    for (ALsizei blocki = 0; found < n; blocki++) {
        SourceBlock *block = ctx->source_blocks[blocki];
        SDL_assert(blocki < totalblocks);
        const int foundhere = block->tmp;
        if (foundhere) {
            block->used += foundhere;
            found += foundhere;
            block->tmp = 0;
        }
    }

    SDL_assert(found == n);

    for (ALsizei i = 0; i < n; i++) {
        ALsource *src = objects[i];

        //SDL_Log("Generated source %u", (unsigned int) names[i]);

        SDL_assert(!src->allocated);

        // Make sure everything that wants to use SIMD is aligned for it.
        SDL_assert( (((size_t) &src->position[0]) % 16) == 0 );
        SDL_assert( (((size_t) &src->velocity[0]) % 16) == 0 );
        SDL_assert( (((size_t) &src->direction[0]) % 16) == 0 );

        SDL_zerop(src);
        SDL_SetAtomicInt(&src->state, AL_INITIAL);
        SDL_SetAtomicInt(&src->total_queued_buffers, 0);
        src->stream = streams[i];
        src->name = names[i];
        src->type = AL_UNDETERMINED;
        src->recalc = AL_TRUE;
        src->gain = 1.0f;
        src->max_gain = 1.0f;
        src->reference_distance = 1.0f;
        src->max_distance = FLT_MAX;
        src->rolloff_factor = 1.0f;
        src->pitch = 1.0f;
        src->cone_inner_angle = 360.0f;
        src->cone_outer_angle = 360.0f;
        src->distance_model = AL_INVERSE_DISTANCE_CLAMPED;
        source_needs_recalc(src);
        src->allocated = AL_TRUE;   // we officially own it.
    }

    if (objects != stackobjs) { SDL_free(objects); }
    if (streams != stackstreams) { SDL_free(streams); }
}
ENTRYPOINTVOID(alGenSources,(ALsizei n, ALuint *names),(n,names))


static void _alDeleteSources(const ALsizei n, const ALuint *names)
{
    ALCcontext *ctx = get_current_context();

    if (n < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    for (ALsizei i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name == 0) {
            // ignore it. FIXME("Spec says alDeleteBuffers() can have a zero name as a legal no-op, but this text isn't included in alDeleteSources...");
        } else {
            ALsource *source = get_source(ctx, name, NULL);
            if (!source) {
                // "If one or more of the specified names is not valid, an AL_INVALID_NAME error will be recorded, and no objects will be deleted."
                set_al_error(ctx, AL_INVALID_NAME);
                return;
            }
        }
    }

    for (ALsizei i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name != 0) {
            SourceBlock *block;
            ALsource *source = get_source(ctx, name, &block);
            SDL_assert(source != NULL);

            // "A playing source can be deleted--the source will be stopped automatically and then deleted."
            SDL_SetAtomicInt(&source->state, AL_STOPPED);
            source->allocated = AL_FALSE;
            source_release_buffer_queue(ctx, source);
            if (source->buffer) {
                SDL_assert(source->type == AL_STATIC);
                SDL_AtomicDecRef(&source->buffer->refcount);
                source->buffer = NULL;
            }
            if (source->stream) {
                SDL_DestroyAudioStream(source->stream);
                source->stream = NULL;
            }
            block->used--;
        }
    }
}
ENTRYPOINTVOID(alDeleteSources,(ALsizei n, const ALuint *names),(n,names))

static ALboolean _alIsSource(const ALuint name)
{
    ALCcontext *ctx = get_current_context();
    return (ctx && (get_source(ctx, name, NULL) != NULL)) ? AL_TRUE : AL_FALSE;
}
ENTRYPOINT(ALboolean,alIsSource,(ALuint name),(name))

static void source_set_pitch(ALCcontext *ctx, ALsource *src, const ALfloat pitch)
{
    src->pitch = pitch;   // only touched through the public API, so the api lock protects this.
    SDL_SetAudioStreamFrequencyRatio(src->stream, pitch);  // this locks the source against the mixer thread.
}

static void _alSourcefv(const ALuint name, const ALenum param, const ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    }

    switch (param) {
        case AL_GAIN: src->gain = *values; break;
        case AL_POSITION: SDL_memcpy(src->position, values, sizeof (ALfloat) * 3); break;
        case AL_VELOCITY: SDL_memcpy(src->velocity, values, sizeof (ALfloat) * 3); break;
        case AL_DIRECTION: SDL_memcpy(src->direction, values, sizeof (ALfloat) * 3); break;
        case AL_MIN_GAIN: src->min_gain = *values; break;
        case AL_MAX_GAIN: src->max_gain = *values; break;
        case AL_REFERENCE_DISTANCE: src->reference_distance = *values; break;
        case AL_ROLLOFF_FACTOR: src->rolloff_factor = *values; break;
        case AL_MAX_DISTANCE: src->max_distance = *values; break;
        case AL_PITCH: source_set_pitch(ctx, src, *values); break;
        case AL_CONE_INNER_ANGLE: src->cone_inner_angle = *values; break;
        case AL_CONE_OUTER_ANGLE: src->cone_outer_angle = *values; break;
        case AL_CONE_OUTER_GAIN: src->cone_outer_gain = *values; break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            source_set_offset(src, param, *values);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); return;

    }

    source_needs_recalc(src);
}
ENTRYPOINTVOID(alSourcefv,(ALuint name, ALenum param, const ALfloat *values),(name,param,values))

static void _alSourcef(const ALuint name, const ALenum param, const ALfloat value)
{
    switch (param) {
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_PITCH:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alSourcefv(name, param, &value);
            break;

        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSourcef,(ALuint name, ALenum param, ALfloat value),(name,param,value))

static void _alSource3f(const ALuint name, const ALenum param, const ALfloat value1, const ALfloat value2, const ALfloat value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION: {
            const ALfloat values[3] = { value1, value2, value3 };
            _alSourcefv(name, param, values);
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSource3f,(ALuint name, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3),(name,param,value1,value2,value3))

static void set_source_static_buffer(ALCcontext *ctx, ALsource *src, const ALuint bufname)
{
    const ALenum state = (const ALenum) SDL_GetAtomicInt(&src->state);
    if ((state == AL_PLAYING) || (state == AL_PAUSED)) {
        set_al_error(ctx, AL_INVALID_OPERATION);  // can't change buffer on playing/paused sources
    } else {
        ALbuffer *buffer = NULL;
        if (bufname && ((buffer = get_buffer(ctx, bufname, NULL)) == NULL)) {
            set_al_error(ctx, AL_INVALID_VALUE);
        } else {
            if (src->buffer != buffer) {
                if (src->buffer) {
                    (void) SDL_AtomicDecRef(&src->buffer->refcount);
                }
                if (buffer) {
                    SDL_AtomicIncRef(&buffer->refcount);
                }
                src->buffer = buffer;
            }

            src->type = buffer ? AL_STATIC : AL_UNDETERMINED;
            src->queue_channels = buffer ? buffer->spec.channels : 0;
            src->queue_frequency = 0;

            source_release_buffer_queue(ctx, src);
        }
    }
}

static void _alSourceiv(const ALuint name, const ALenum param, const ALint *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    }

    switch (param) {
        case AL_BUFFER: set_source_static_buffer(ctx, src, (ALuint) *values); break;
        case AL_SOURCE_RELATIVE: src->source_relative = *values ? AL_TRUE : AL_FALSE; break;
        case AL_LOOPING: src->looping = *values ? AL_TRUE : AL_FALSE; break;
        case AL_REFERENCE_DISTANCE: src->reference_distance = (ALfloat) *values; break;
        case AL_ROLLOFF_FACTOR: src->rolloff_factor = (ALfloat) *values; break;
        case AL_MAX_DISTANCE: src->max_distance = (ALfloat) *values; break;
        case AL_CONE_INNER_ANGLE: src->cone_inner_angle = (ALfloat) *values; break;
        case AL_CONE_OUTER_ANGLE: src->cone_outer_angle = (ALfloat) *values; break;

        case AL_DIRECTION:
            src->direction[0] = (ALfloat) values[0];
            src->direction[1] = (ALfloat) values[1];
            src->direction[2] = (ALfloat) values[2];
            break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            source_set_offset(src, param, (ALfloat)*values);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); return;
    }

    source_needs_recalc(src);
}
ENTRYPOINTVOID(alSourceiv,(ALuint name, ALenum param, const ALint *values),(name,param,values))

static void _alSourcei(const ALuint name, const ALenum param, const ALint value)
{
    switch (param) {
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_BUFFER:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alSourceiv(name, param, &value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSourcei,(ALuint name, ALenum param, ALint value),(name,param,value))

static void _alSource3i(const ALuint name, const ALenum param, const ALint value1, const ALint value2, const ALint value3)
{
    switch (param) {
        case AL_DIRECTION: {
            const ALint values[3] = { (ALint) value1, (ALint) value2, (ALint) value3 };
            _alSourceiv(name, param, values);
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSource3i,(ALuint name, ALenum param, ALint value1, ALint value2, ALint value3),(name,param,value1,value2,value3))

static void _alGetSourcefv(const ALuint name, const ALenum param, ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    }

    switch (param) {
        case AL_GAIN: *values = src->gain; break;
        case AL_POSITION: SDL_memcpy(values, src->position, sizeof (ALfloat) * 3); break;
        case AL_VELOCITY: SDL_memcpy(values, src->velocity, sizeof (ALfloat) * 3); break;
        case AL_DIRECTION: SDL_memcpy(values, src->direction, sizeof (ALfloat) * 3); break;
        case AL_MIN_GAIN: *values = src->min_gain; break;
        case AL_MAX_GAIN: *values = src->max_gain; break;
        case AL_REFERENCE_DISTANCE: *values = src->reference_distance; break;
        case AL_ROLLOFF_FACTOR: *values = src->rolloff_factor; break;
        case AL_MAX_DISTANCE: *values = src->max_distance; break;
        case AL_PITCH: *values = src->pitch; break;
        case AL_CONE_INNER_ANGLE: *values = src->cone_inner_angle; break;
        case AL_CONE_OUTER_ANGLE: *values = src->cone_outer_angle; break;
        case AL_CONE_OUTER_GAIN:  *values = src->cone_outer_gain; break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            *values = source_get_offset(src, param);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourcefv,(ALuint name, ALenum param, ALfloat *values),(name,param,values))

static void _alGetSourcef(const ALuint name, const ALenum param, ALfloat *value)
{
    switch (param) {
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_PITCH:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alGetSourcefv(name, param, value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourcef,(ALuint name, ALenum param, ALfloat *value),(name,param,value))

static void _alGetSource3f(const ALuint name, const ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION: {
            ALfloat values[3];
            _alGetSourcefv(name, param, values);
            if (value1) *value1 = values[0];
            if (value2) *value2 = values[1];
            if (value3) *value3 = values[2];
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSource3f,(ALuint name, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3),(name,param,value1,value2,value3))

static void _alGetSourceiv(const ALuint name, const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    }

    switch (param) {
        case AL_SOURCE_STATE: *values = (ALint) SDL_GetAtomicInt(&src->state); break;
        case AL_SOURCE_TYPE: *values = (ALint) src->type; break;
        case AL_BUFFER: *values = (ALint) (src->buffer ? src->buffer->name : 0); break;
        case AL_BUFFERS_QUEUED: *values = (ALint) SDL_GetAtomicInt(&src->total_queued_buffers); break;
        case AL_BUFFERS_PROCESSED: *values = (ALint) SDL_GetAtomicInt(&src->buffer_queue_processed.num_items); break;
        case AL_SOURCE_RELATIVE: *values = (ALint) src->source_relative; break;
        case AL_LOOPING: *values = (ALint) src->looping; break;
        case AL_REFERENCE_DISTANCE: *values = (ALint) src->reference_distance; break;
        case AL_ROLLOFF_FACTOR: *values = (ALint) src->rolloff_factor; break;
        case AL_MAX_DISTANCE: *values = (ALint) src->max_distance; break;
        case AL_CONE_INNER_ANGLE: *values = (ALint) src->cone_inner_angle; break;
        case AL_CONE_OUTER_ANGLE: *values = (ALint) src->cone_outer_angle; break;
        case AL_DIRECTION:
            values[0] = (ALint) src->direction[0];
            values[1] = (ALint) src->direction[1];
            values[2] = (ALint) src->direction[2];
            break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            *values = (ALint) source_get_offset(src, param);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourceiv,(ALuint name, ALenum param, ALint *values),(name,param,values))

static void _alGetSourcei(const ALuint name, const ALenum param, ALint *value)
{
    switch (param) {
        case AL_SOURCE_STATE:
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_BUFFER:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alGetSourceiv(name, param, value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourcei,(ALuint name, ALenum param, ALint *value),(name,param,value))

static void _alGetSource3i(const ALuint name, const ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    switch (param) {
        case AL_DIRECTION: {
            ALint values[3];
            _alGetSourceiv(name, param, values);
            if (value1) { *value1 = values[0]; }
            if (value2) { *value2 = values[1]; }
            if (value3) { *value3 = values[2]; }
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSource3i,(ALuint name, ALenum param, ALint *value1, ALint *value2, ALint *value3),(name,param,value1,value2,value3))

static void source_play(ALCcontext *ctx, const ALsizei n, const ALuint *names)
{
    if (n <= 0) {
        return;
    } else if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    SourcePlayTodo todo;
    SourcePlayTodo *todoend = &todo;
    SDL_zero(todo);

    /* Obtain our SourcePlayTodo items upfront; if this runs out of
       memory, we won't have changed any state. The mixer thread will
       put items back in the pool when done with them, so this handoff needs
       to be atomic. */
    ALboolean failed = AL_FALSE;
    for (ALsizei i = 0; i < n; i++) {
        SourcePlayTodo *item;
        void *ptr;
        do {
            ptr = SDL_GetAtomicPointer(&ctx->device->playback.source_todo_pool);
            item = (SourcePlayTodo *) ptr;
            if (!item) {
                break;
            }
            ptr = item->next;
        } while (!SDL_CompareAndSwapAtomicPointer(&ctx->device->playback.source_todo_pool, item, ptr));

        if (!item) {  // allocate a new item
            item = (SourcePlayTodo *) SDL_calloc(1, sizeof (SourcePlayTodo));
            if (!item) {
                set_al_error(ctx, AL_OUT_OF_MEMORY);
                failed = AL_TRUE;
                break;
            }
        }

        item->next = NULL;
        todoend->next = item;
        todoend = item;
    }

    if (failed) {
        // put the whole new queue back in the pool for reuse later.
        if (todo.next) {
            void *ptr;
            do {
                ptr = SDL_GetAtomicPointer(&ctx->device->playback.source_todo_pool);
                todoend->next = (SourcePlayTodo *) ptr;
            } while (!SDL_CompareAndSwapAtomicPointer(&ctx->device->playback.source_todo_pool, ptr, todo.next));
        }
        return;
    }

    FIXME("What do we do if there's an invalid source in the middle of the names vector?");
    SourcePlayTodo *todoptr = todo.next;
    for (ALsizei i = 0; i < n; i++) {
        const ALuint name = names[i];
        ALsource *src = get_source(ctx, name, NULL);
        if (src) {
            if (src->offset_latched) {
                src->offset_latched = AL_FALSE;
            } else if (SDL_GetAtomicInt(&src->state) != AL_PAUSED) {
                src->offset = 0;
            }

            /* this used to move right to AL_STOPPED if the device is
               disconnected, but now we let the mixer thread handle that to
               avoid race conditions with marking the buffer queue
               processed, etc. Strictly speaking, ALC_EXT_disconnect
               says playing a source on a disconnected device should
               "immediately" progress to STOPPED, but I'm willing to
               say that the mixer will "immediately" move it as opposed to
               it stopping when the source would be done mixing (or worse:
               hang there forever). */
            SDL_SetAtomicInt(&src->state, AL_PLAYING);

            // Mark this as visible to the mixer. This will be set back to zero by the mixer thread when it is done with the source.
            SDL_SetAtomicInt(&src->mixer_accessible, 1);

            todoptr->source = src;
            todoptr = todoptr->next;
        }
    }

    /* Send the list to the mixer atomically, so all sources start playing in sync!
       We're going to put these on a linked list called playlist_todo
       The mixer does an atomiccasptr to grab the current list, swapping
       in a NULL. Once it has the list, it's safe to do what it likes
       with it, as nothing else owns the pointers in that list. */
    void *ptr;
    do {
        ptr = SDL_GetAtomicPointer(&ctx->playlist_todo);
        todoend->next = (SourcePlayTodo *) ptr;
    } while (!SDL_CompareAndSwapAtomicPointer(&ctx->playlist_todo, ptr, todo.next));
}

static void _alSourcePlay(const ALuint name)
{
    source_play(get_current_context(), 1, &name);
}
ENTRYPOINTVOID(alSourcePlay,(ALuint name),(name))

static void _alSourcePlayv(ALsizei n, const ALuint *names)
{
    source_play(get_current_context(), n, names);
}
ENTRYPOINTVOID(alSourcePlayv,(ALsizei n, const ALuint *names),(n, names))


static void source_stop(ALCcontext *ctx, const ALuint name)
{
    ALsource *src = get_source(ctx, name, NULL);
    if (src) {
        if (SDL_GetAtomicInt(&src->state) != AL_INITIAL) {
            lock_source(src);
            SDL_SetAtomicInt(&src->state, AL_STOPPED);
            source_mark_all_buffers_processed(src);
            SDL_ClearAudioStream(src->stream);
            /*src->offset = 0;*/  FIXME("should this set src->offset to zero?");
            unlock_source(src);
        }
    }
}

static void source_rewind(ALCcontext *ctx, const ALuint name)
{
    ALsource *src = get_source(ctx, name, NULL);
    if (src) {
        lock_source(src);
        SDL_SetAtomicInt(&src->state, AL_INITIAL);
        SDL_ClearAudioStream(src->stream);
        src->offset = 0;
        unlock_source(src);
    }
}

static void source_pause(ALCcontext *ctx, const ALuint name)
{
    ALsource *src = get_source(ctx, name, NULL);
    if (src) {
        SDL_CompareAndSwapAtomicInt(&src->state, AL_PLAYING, AL_PAUSED);
    }
}

static float source_get_offset(ALsource *src, ALenum param)
{
    int offset = 0;
    int framesize = sizeof (float);
    int freq = 1;
    if (src->type == AL_STREAMING) {
        // streaming: the offset counts from the first processed buffer in the queue.
        BufferQueueItem *item = src->buffer_queue.head;
        if (item) {
            framesize = (int) (item->buffer->spec.channels * sizeof (float));
            freq = (int) (item->buffer->spec.freq);
            int proc_buf = SDL_GetAtomicInt(&src->buffer_queue_processed.num_items);
            offset = (proc_buf * item->buffer->len + src->offset);
        }
    } else if (src->buffer) {
        framesize = (int) (src->buffer->spec.channels * sizeof (float));
        freq = (int) src->buffer->spec.freq;
        offset = src->offset;
    }
    switch(param) {
        case AL_SAMPLE_OFFSET: return (float) (offset / framesize); break;
        case AL_SEC_OFFSET: return ((float) (offset / framesize)) / ((float) freq); break;
        case AL_BYTE_OFFSET: return (float) offset; break;
        default: break;
    }

    return 0.0f;
}

static void source_set_offset(ALsource *src, ALenum param, ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    lock_source(src);

    if (src->type == AL_UNDETERMINED) {  // no buffer to seek in
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (src->type == AL_STREAMING) {
        FIXME("set_offset for streaming sources not implemented");
    } else {
        const int bufflen = (int) src->buffer->len;
        const int framesize = (int) (src->buffer->spec.channels * sizeof (float));
        const int freq = src->buffer->spec.freq;
        ALboolean okay = AL_TRUE;
        int offset = -1;

        switch (param) {
            case AL_SAMPLE_OFFSET:
                offset = ((int) value) * framesize;
                break;
            case AL_SEC_OFFSET:
                offset = ((int) value) * freq * framesize;
                break;
            case AL_BYTE_OFFSET:
                offset = (((int) value) / framesize) * framesize;
                break;
            default:
                SDL_assert(!"Unexpected source offset type!");
                set_al_error(ctx, AL_INVALID_ENUM);  // this is a MojoAL bug, not an app bug, but we'll try to recover.
                okay = AL_FALSE;
                break;
        }

        if (okay) {
            if ((offset < 0) || (offset > bufflen)) {
                set_al_error(ctx, AL_INVALID_VALUE);
            } else {
                // make sure the offset lands on a sample frame boundary.
                offset -= offset % framesize;

                SDL_ClearAudioStream(src->stream);
                src->offset = offset;

                if (SDL_GetAtomicInt(&src->state) != AL_PLAYING) {
                    src->offset_latched = true;
                } else {
                    put_albuffer_to_audiostream(ctx, src->buffer, offset, src->stream);
                }
            }
        }
    }

    unlock_source(src);
}

// deal with alSourcePlay and alSourcePlayv (etc) boiler plate...
#define SOURCE_STATE_TRANSITION_OP(alfn, fn) \
    void alSource##alfn(ALuint name) { source_##fn(get_current_context(), name); } \
    void alSource##alfn##v(ALsizei n, const ALuint *sources) { \
        ALCcontext *ctx = get_current_context(); \
        if (n < 0) { \
            set_al_error(ctx, AL_INVALID_VALUE); \
        } else if (!ctx) { \
            set_al_error(ctx, AL_INVALID_OPERATION); \
        } else { \
            if (n > 1) { \
                FIXME("Can we do this without a full context lock?"); \
                SDL_LockAudioStream(ctx->stream);  /* lock the SDL device so these all start mixing in the same callback. */ \
                for (ALsizei i = 0; i < n; i++) { \
                    source_##fn(ctx, sources[i]); \
                } \
                SDL_UnlockAudioStream(ctx->stream); \
            } else if (n == 1) { \
                source_##fn(ctx, *sources); \
            } \
        } \
    }

SOURCE_STATE_TRANSITION_OP(Stop, stop)
SOURCE_STATE_TRANSITION_OP(Rewind, rewind)
SOURCE_STATE_TRANSITION_OP(Pause, pause)


static void _alSourceQueueBuffers(const ALuint name, const ALsizei nb, const ALuint *bufnames)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    } else if (src->type == AL_STATIC) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    } else if (nb < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (nb == 0) {
        return;  // not an error, but nothing to do.
    }

    ALint queue_channels = 0;
    ALsizei queue_frequency = 0;
    BufferQueueItem *queue = NULL;
    BufferQueueItem *queueend = NULL;
    ALboolean failed = AL_FALSE;

    for (ALsizei i = nb; i > 0; i--) {  // build list in reverse
        BufferQueueItem *item = NULL;
        const ALuint bufname = bufnames[i-1];
        ALbuffer *buffer = bufname ? get_buffer(ctx, bufname, NULL) : NULL;
        if (!buffer && bufname) {  // uhoh, bad buffer name!
            set_al_error(ctx, AL_INVALID_VALUE);
            failed = AL_TRUE;
            break;
        }

        if (buffer) {
            if (queue_channels == 0) {
                SDL_assert(queue_frequency == 0);
                queue_channels = buffer->spec.channels;
                queue_frequency = buffer->spec.freq;
            } else if ((queue_channels != buffer->spec.channels) || (queue_frequency != buffer->spec.freq)) {
                // the whole queue must be the same format.
                set_al_error(ctx, AL_INVALID_VALUE);
                failed = AL_TRUE;
                break;
            }
        }

        item = ctx->device->playback.buffer_queue_pool;
        if (item) {
            ctx->device->playback.buffer_queue_pool = (BufferQueueItem*)item->next;
        } else {  // allocate a new item
            item = (BufferQueueItem *) SDL_calloc(1, sizeof (BufferQueueItem));
            if (!item) {
                set_al_error(ctx, AL_OUT_OF_MEMORY);
                failed = AL_TRUE;
                break;
            }
        }

        if (buffer) {
            SDL_AtomicIncRef(&buffer->refcount);  // mark it as in-use.
        }
        item->buffer = buffer;

        SDL_assert((queue != NULL) == (queueend != NULL));
        if (queueend) {
            queueend->next = item;
        } else {
            queue = item;
        }
        queueend = item;
    }

    if (failed) {
        if (queue) {
            // Drop our claim on any buffers we planned to queue.
            BufferQueueItem *item;
            for (item = queue; item != NULL; item = (BufferQueueItem*)item->next) {
                if (item->buffer) {
                    SDL_AtomicDecRef(&item->buffer->refcount);
                }
            }

            // put the whole new queue back in the pool for reuse later.
            queueend->next = ctx->device->playback.buffer_queue_pool;
            ctx->device->playback.buffer_queue_pool = queue;
        }
        return;
    }

    FIXME("this needs to be set way sooner");

    FIXME("this used to have a source lock, think this one through");
    src->type = AL_STREAMING;

    if (!src->queue_channels) {
        src->queue_channels = queue_channels;
        src->queue_frequency = queue_frequency;
        SDL_ClearAudioStream(src->stream);
    }

    /* so we're going to put these on a linked list called just_queued,
        where things build up in reverse order, to keep this on a single
        pointer. The theory is we'll atomicgetptr the pointer, set that
        pointer as the "next" for our list, and then atomiccasptr our new
        list against the original pointer. If the CAS succeeds, we have
        a complete list, atomically set. If it fails, try again with
        the new pointer we found, updating our next pointer again. If it
        failed, it's because the pointer became NULL when the mixer thread
        grabbed the existing list.

        The mixer does an atomiccasptr to grab the current list, swapping
        in a NULL. Once it has the list, it's safe to do what it likes
        with it, as nothing else owns the pointers in that list. */

    void *ptr;
    do {
        ptr = SDL_GetAtomicPointer(&src->buffer_queue.just_queued);
        SDL_SetAtomicPointer(&queueend->next, ptr);
    } while (!SDL_CompareAndSwapAtomicPointer(&src->buffer_queue.just_queued, ptr, queue));

    SDL_AddAtomicInt(&src->total_queued_buffers, (int) nb);
    SDL_AddAtomicInt(&src->buffer_queue.num_items, (int) nb);
}
ENTRYPOINTVOID(alSourceQueueBuffers,(ALuint name, ALsizei nb, const ALuint *bufnames),(name,nb,bufnames))

static void _alSourceUnqueueBuffers(const ALuint name, const ALsizei nb, ALuint *bufnames)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    } else if (src->type == AL_STATIC) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    } else if (nb < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (nb == 0) {
        return;  // not an error, but nothing to do.
    } else if (((ALsizei) SDL_GetAtomicInt(&src->buffer_queue_processed.num_items)) < nb) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    }

    SDL_AddAtomicInt(&src->buffer_queue_processed.num_items, -((int) nb));
    SDL_AddAtomicInt(&src->total_queued_buffers, -((int) nb));

    obtain_newly_queued_buffers(&src->buffer_queue_processed);

    BufferQueueItem *queue;
    BufferQueueItem *item = queue = src->buffer_queue_processed.head;
    for (ALsizei i = 0; i < nb; i++) {
        // buffer_queue_processed.num_items said list was long enough.
        SDL_assert(item != NULL);
        item = (BufferQueueItem*) item->next;
    }
    src->buffer_queue_processed.head = item;
    if (!item) {
        src->buffer_queue_processed.tail = NULL;
    }

    BufferQueueItem *queueend = NULL;
    item = queue;
    for (ALsizei i = 0; i < nb; i++) {
        if (item->buffer) {
            SDL_AtomicDecRef(&item->buffer->refcount);
        }
        bufnames[i] = item->buffer ? item->buffer->name : 0;
        queueend = item;
        item = (BufferQueueItem*)item->next;
    }

    // put the whole new queue back in the pool for reuse later.
    SDL_assert(queueend != NULL);
    queueend->next = ctx->device->playback.buffer_queue_pool;
    ctx->device->playback.buffer_queue_pool = queue;
}
ENTRYPOINTVOID(alSourceUnqueueBuffers,(ALuint name, ALsizei nb, ALuint *bufnames),(name,nb,bufnames))

// !!! FIXME: buffers and sources use almost identical code for blocks
static void _alGenBuffers(const ALsizei n, ALuint *names)
{
    ALCcontext *ctx = get_current_context();

    if (n < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    } else if (n == 0) {
        return;  // not an error, but nothing to do.
    }

    ALbuffer *stackobjs[16];
    ALbuffer **objects = stackobjs;
    if (n <= SDL_arraysize(stackobjs)) {
        SDL_memset(stackobjs, '\0', sizeof (ALbuffer *) * n);
    } else {
        objects = (ALbuffer **) SDL_calloc(n, sizeof (ALbuffer *));
        if (!objects) {
            set_al_error(ctx, AL_OUT_OF_MEMORY);
            return;
        }
    }

    ALsizei found = 0;
    ALsizei block_offset = 0;
    ALsizei totalblocks = ctx->device->playback.num_buffer_blocks;
    for (ALsizei blocki = 0; blocki < totalblocks; blocki++) {
        BufferBlock *block = ctx->device->playback.buffer_blocks[blocki];
        block->tmp = 0;
        if (block->used < SDL_arraysize(block->buffers)) {  // skip if full
            for (ALsizei i = 0; i < SDL_arraysize(block->buffers); i++) {
                if (!block->buffers[i].allocated) {
                    block->tmp++;
                    objects[found] = &block->buffers[i];
                    names[found++] = (i + block_offset) + 1;  // +1 so it isn't zero.
                    if (found == n) {
                        break;
                    }
                }
            }

            if (found == n) {
                break;
            }
        }

        block_offset += SDL_arraysize(block->buffers);
    }

    ALboolean out_of_memory = AL_FALSE;
    while (found < n) {  // out of blocks? Add new ones.
        // ctx->buffer_blocks is only accessed on the API thread under a mutex, so it's safe to realloc.
        void *ptr = SDL_realloc(ctx->device->playback.buffer_blocks, sizeof (BufferBlock *) * (totalblocks + 1));
        if (!ptr) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->device->playback.buffer_blocks = (BufferBlock **) ptr;

        BufferBlock *block = (BufferBlock *) SDL_calloc(1, sizeof (BufferBlock));
        if (!block) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->device->playback.buffer_blocks[totalblocks] = block;
        totalblocks++;
        ctx->device->playback.num_buffer_blocks++;

        for (ALsizei i = 0; i < SDL_arraysize(block->buffers); i++) {
            block->tmp++;
            objects[found] = &block->buffers[i];
            names[found++] = (i + block_offset) + 1;  // +1 so it isn't zero.
            if (found == n) {
                break;
            }
        }
        block_offset += SDL_arraysize(block->buffers);
    }

    if (out_of_memory) {
        if (objects != stackobjs) { SDL_free(objects); }
        SDL_memset(names, '\0', sizeof (*names) * n);
        set_al_error(ctx, AL_OUT_OF_MEMORY);
        return;
    }

    SDL_assert(found == n);  // we should have either gotten space or bailed on alloc failure

    // update the "used" field in blocks with items we are taking now.
    found = 0;
    for (ALsizei blocki = 0; found < n; blocki++) {
        BufferBlock *block = ctx->device->playback.buffer_blocks[blocki];
        SDL_assert(blocki < totalblocks);
        const int foundhere = block->tmp;
        if (foundhere) {
            block->used += foundhere;
            found += foundhere;
            block->tmp = 0;
        }
    }

    SDL_assert(found == n);

    for (ALsizei i = 0; i < n; i++) {
        ALbuffer *buffer = objects[i];
        //SDL_Log("Generated buffer %u", (unsigned int) names[i]);
        SDL_assert(!buffer->allocated);
        SDL_zerop(buffer);
        buffer->name = names[i];
        buffer->spec.format = SDL_AUDIO_S16;
        buffer->spec.channels = 1;
        buffer->spec.freq = 0;
        buffer->allocated = AL_TRUE;  // we officially own it.
    }

    if (objects != stackobjs) {
        SDL_free(objects);
    }
}
ENTRYPOINTVOID(alGenBuffers,(ALsizei n, ALuint *names),(n,names))

/*
 * alGenStreamingBuffers was a pre-1.0 API. The Loki version of Unreal Tournament '99 uses it.
 * It appears to be identical to alGenBuffers, but was probably meant to be
 * a hint to the AL that the buffer's contents would be replaced frequently.
 */
static void _alGenStreamingBuffers(const ALsizei n, ALuint *names)
{
    _alGenBuffers(n, names);
}
ENTRYPOINTVOID(alGenStreamingBuffers,(ALsizei n, ALuint *names),(n,names))

static void _alDeleteBuffers(const ALsizei n, const ALuint *names)
{
    ALCcontext *ctx = get_current_context();

    if (n < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    } else if (n == 0) {
        return;  // not an error, but nothing to do.
    }

    for (ALsizei i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name == 0) {
            // ignore it.
        } else {
            ALbuffer *buffer = get_buffer(ctx, name, NULL);
            if (!buffer) {
                // "If one or more of the specified names is not valid, an AL_INVALID_NAME error will be recorded, and no objects will be deleted."
                set_al_error(ctx, AL_INVALID_NAME);
                return;
            } else if (SDL_GetAtomicInt(&buffer->refcount) != 0) {
                set_al_error(ctx, AL_INVALID_OPERATION);  // still in use
                return;
            }
        }
    }

    for (ALsizei i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name != 0) {
            BufferBlock *block;
            ALbuffer *buffer = get_buffer(ctx, name, &block);
            SDL_assert(buffer != NULL);
            if (buffer->data) {
                albuffer_in_audiostream_complete(((Uint8 *) buffer->data) - simd_alignment, buffer->data, buffer->len);
            }
            buffer->allocated = AL_FALSE;
            buffer->data = NULL;
            block->used--;
        }
    }
}
ENTRYPOINTVOID(alDeleteBuffers,(ALsizei n, const ALuint *names),(n,names))

static ALboolean _alIsBuffer(ALuint name)
{
    ALCcontext *ctx = get_current_context();
    return (ctx && (get_buffer(ctx, name, NULL) != NULL)) ? AL_TRUE : AL_FALSE;
}
ENTRYPOINT(ALboolean,alIsBuffer,(ALuint name),(name))

static void _alBufferData(const ALuint name, const ALenum alfmt, const ALvoid *data, const ALsizei size, const ALsizei freq)
{
    ALCcontext *ctx = get_current_context();
    ALbuffer *buffer = get_buffer(ctx, name, NULL);
    SDL_AudioSpec spec;
    ALCsizei framesize;

    if (!buffer) {
        return;
    } else if (size < 0) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    } else if (freq < 0) {
        return;  // not an error, but nothing to do.
    } else if (!alcfmt_to_sdlfmt(alfmt, &spec.format, &spec.channels, &framesize)) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    }

    spec.freq = (int) freq;

    FIXME("spec: 'The size given is the number of bytes, and must be logical for the format given -- an odd value for 16-bit data will always be an error, for example. An invalid size will result in an AL_INVALID_VALUE error.'");

    // increment refcount so this can't be deleted or alBufferData'd from another thread
    const int prevrefcount = SDL_AtomicIncRef(&buffer->refcount);
    SDL_assert(prevrefcount >= 0);
    if (prevrefcount != 0) {
        // this buffer is being used by some source. Unqueue it first.
        (void) SDL_AtomicDecRef(&buffer->refcount);
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    // This check was from the wild west of lock-free programming, now we shouldn't pass get_buffer() if not allocated.
    SDL_assert(buffer->allocated);

    // make a copy of the data for later. No conversion here, we'll push it through an SDL_AudioStream as needed.
    SDL_assert(sizeof (SDL_AtomicInt) <= simd_alignment);
    void *alignedptr = SDL_aligned_alloc(simd_alignment, size + simd_alignment);
    if (!alignedptr) {
        (void) SDL_AtomicDecRef(&buffer->refcount);
        set_al_error(ctx, AL_OUT_OF_MEMORY);
        return;
    }

    if (buffer->data) {
        albuffer_in_audiostream_complete(((Uint8 *) buffer->data) - simd_alignment, buffer->data, buffer->len);
    }

    SDL_SetAtomicInt((SDL_AtomicInt *) alignedptr, 1);
    void *newdata = ((Uint8 *) alignedptr) + simd_alignment;
    SDL_memcpy(newdata, data, size);
    SDL_copyp(&buffer->spec, &spec);
    buffer->data = newdata;
    buffer->len = size;
    SDL_AtomicDecRef(&buffer->refcount);  // ready to go!
}
ENTRYPOINTVOID(alBufferData,(ALuint name, ALenum alfmt, const ALvoid *data, ALsizei size, ALsizei freq),(name,alfmt,data,size,freq))

static void _alBufferfv(const ALuint name, const ALenum param, const ALfloat *values)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alBufferfv,(ALuint name, ALenum param, const ALfloat *values),(name,param,values))

static void _alBufferf(const ALuint name, const ALenum param, const ALfloat value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alBufferf,(ALuint name, ALenum param, ALfloat value),(name,param,value))

static void _alBuffer3f(const ALuint name, const ALenum param, const ALfloat value1, const ALfloat value2, const ALfloat value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alBuffer3f,(ALuint name, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3),(name,param,value1,value2,value3))

static void _alBufferiv(const ALuint name, const ALenum param, const ALint *values)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alBufferiv,(ALuint name, ALenum param, const ALint *values),(name,param,values))

static void _alBufferi(const ALuint name, const ALenum param, const ALint value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alBufferi,(ALuint name, ALenum param, ALint value),(name,param,value))

static void _alBuffer3i(const ALuint name, const ALenum param, const ALint value1, const ALint value2, const ALint value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alBuffer3i,(ALuint name, ALenum param, ALint value1, ALint value2, ALint value3),(name,param,value1,value2,value3))

static void _alGetBufferfv(const ALuint name, const ALenum param, const ALfloat *values)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alGetBufferfv,(ALuint name, ALenum param, ALfloat *values),(name,param,values))

static void _alGetBufferf(const ALuint name, const ALenum param, ALfloat *value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alGetBufferf,(ALuint name, ALenum param, ALfloat *value),(name,param,value))

static void _alGetBuffer3f(const ALuint name, const ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alGetBuffer3f,(ALuint name, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3),(name,param,value1,value2,value3))

static void _alGetBufferi(const ALuint name, const ALenum param, ALint *value)
{
    switch (param) {
        case AL_FREQUENCY:
        case AL_SIZE:
        case AL_BITS:
        case AL_CHANNELS:
            alGetBufferiv(name, param, value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetBufferi,(ALuint name, ALenum param, ALint *value),(name,param,value))

static void _alGetBuffer3i(const ALuint name, const ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM); // nothing in core OpenAL 1.1 uses this
}
ENTRYPOINTVOID(alGetBuffer3i,(ALuint name, ALenum param, ALint *value1, ALint *value2, ALint *value3),(name,param,value1,value2,value3))

static void _alGetBufferiv(const ALuint name, const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    ALbuffer *buffer = get_buffer(ctx, name, NULL);
    if (!buffer) {
        return;
    }

    switch (param) {
        case AL_FREQUENCY: *values = (ALint) buffer->spec.freq; break;
        case AL_SIZE: *values = (ALint) buffer->len; break;
        case AL_BITS: *values = (ALint) SDL_AUDIO_BITSIZE(buffer->spec.format); break;
        case AL_CHANNELS: *values = (ALint) buffer->spec.channels; break;
        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetBufferiv,(ALuint name, ALenum param, ALint *values),(name,param,values))

// end of mojoal.c ...

