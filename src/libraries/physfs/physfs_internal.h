/*
 * Internal function/structure declaration. Do NOT include in your
 *  application.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#ifndef _INCLUDE_PHYSFS_INTERNAL_H_
#define _INCLUDE_PHYSFS_INTERNAL_H_

#ifndef __PHYSICSFS_INTERNAL__
#error Do not include this header from your applications.
#endif

/* Turn off MSVC warnings that are aggressively anti-portability. */
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "physfs.h"

/* The holy trinity. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physfs_platforms.h"

#include <assert.h>

#define __PHYSFS_COMPILE_TIME_ASSERT(name, x) \
       typedef int __PHYSFS_compile_time_assert_##name[(x) * 2 - 1]

/* !!! FIXME: remove this when revamping stack allocation code... */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__WATCOMC__)
#include <malloc.h>
#endif

#ifdef PHYSFS_PLATFORM_SOLARIS
#include <alloca.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define PHYSFS_MINIMUM_GCC_VERSION(major, minor) \
    ( ((__GNUC__ << 16) + __GNUC_MINOR__) >= (((major) << 16) + (minor)) )
#else
#define PHYSFS_MINIMUM_GCC_VERSION(major, minor) (0)
#endif

#ifdef __cplusplus
    /* C++ always has a real inline keyword. */
#elif (defined macintosh) && !(defined __MWERKS__)
#   define inline
#elif (defined _MSC_VER)
#   define inline __inline
#endif

#if defined(PHYSFS_PLATFORM_LINUX) && !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

/* All public APIs need to be in physfs.h with a PHYSFS_DECL.
   All file-private symbols need to be marked "static".
   Everything shared between PhysicsFS sources needs to be in this
   file between the visibility pragma blocks. */
#if PHYSFS_MINIMUM_GCC_VERSION(4,0) || defined(__clang__)
#define PHYSFS_HAVE_PRAGMA_VISIBILITY 1
#endif

#if PHYSFS_HAVE_PRAGMA_VISIBILITY
#pragma GCC visibility push(hidden)
#endif

/* These are the build-in archivers. We list them all as "extern" here without
   #ifdefs to keep it tidy, but obviously you need to make sure these are
   wrapped in PHYSFS_SUPPORTS_* checks before actually referencing them. */
extern const PHYSFS_Archiver __PHYSFS_Archiver_DIR;
extern const PHYSFS_Archiver __PHYSFS_Archiver_ZIP;
extern const PHYSFS_Archiver __PHYSFS_Archiver_7Z;
extern const PHYSFS_Archiver __PHYSFS_Archiver_GRP;
extern const PHYSFS_Archiver __PHYSFS_Archiver_QPAK;
extern const PHYSFS_Archiver __PHYSFS_Archiver_HOG;
extern const PHYSFS_Archiver __PHYSFS_Archiver_MVL;
extern const PHYSFS_Archiver __PHYSFS_Archiver_WAD;
extern const PHYSFS_Archiver __PHYSFS_Archiver_SLB;
extern const PHYSFS_Archiver __PHYSFS_Archiver_ISO9660;
extern const PHYSFS_Archiver __PHYSFS_Archiver_VDF;

/* a real C99-compliant snprintf() is in Visual Studio 2015,
   but just use this everywhere for binary compatibility. */
#if defined(_MSC_VER)
int __PHYSFS_msvc_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap);
int __PHYSFS_msvc_snprintf(char *outBuf, size_t size, const char *format, ...);
#define vsnprintf __PHYSFS_msvc_vsnprintf
#define snprintf __PHYSFS_msvc_snprintf
#endif

/* Some simple wrappers around WinRT C++ interfaces we can call from C. */
#ifdef PHYSFS_PLATFORM_WINRT
const void *__PHYSFS_winrtCalcBaseDir(void);
const void *__PHYSFS_winrtCalcPrefDir(void);
#endif

/* atomic operations. */
#if defined(_MSC_VER) && (_MSC_VER >= 1500)
#include <intrin.h>
__PHYSFS_COMPILE_TIME_ASSERT(LongEqualsInt, sizeof (int) == sizeof (long));
#define __PHYSFS_ATOMIC_INCR(ptrval) _InterlockedIncrement((long*)(ptrval))
#define __PHYSFS_ATOMIC_DECR(ptrval) _InterlockedDecrement((long*)(ptrval))
#elif defined(__clang__) || (defined(__GNUC__) && (((__GNUC__ * 10000) + (__GNUC_MINOR__ * 100)) >= 40100))
#define __PHYSFS_ATOMIC_INCR(ptrval) __sync_fetch_and_add(ptrval, 1)
#define __PHYSFS_ATOMIC_DECR(ptrval) __sync_fetch_and_add(ptrval, -1)
#else
#define PHYSFS_NEED_ATOMIC_OP_FALLBACK 1
int __PHYSFS_ATOMIC_INCR(int *ptrval);
int __PHYSFS_ATOMIC_DECR(int *ptrval);
#endif


/*
 * Interface for small allocations. If you need a little scratch space for
 *  a throwaway buffer or string, use this. It will make small allocations
 *  on the stack if possible, and use allocator.Malloc() if they are too
 *  large. This helps reduce malloc pressure.
 * There are some rules, though:
 * NEVER return a pointer from this, as stack-allocated buffers go away
 *  when your function returns.
 * NEVER allocate in a loop, as stack-allocated pointers will pile up. Call
 *  a function that uses smallAlloc from your loop, so the allocation can
 *  free each time.
 * NEVER call smallAlloc with any complex expression (it's a macro that WILL
 *  have side effects...it references the argument multiple times). Use a
 *  variable or a literal.
 * NEVER free a pointer from this with anything but smallFree. It will not
 *  be a valid pointer to the allocator, regardless of where the memory came
 *  from.
 * NEVER realloc a pointer from this.
 * NEVER forget to use smallFree: it may not be a pointer from the stack.
 * NEVER forget to check for NULL...allocation can fail here, of course!
 */
#define __PHYSFS_SMALLALLOCTHRESHOLD 256
void *__PHYSFS_initSmallAlloc(void *ptr, const size_t len);

#define __PHYSFS_smallAlloc(bytes) ( \
    __PHYSFS_initSmallAlloc( \
        (((bytes) < __PHYSFS_SMALLALLOCTHRESHOLD) ? \
            alloca((size_t)((bytes)+sizeof(void*))) : NULL), (bytes)) \
)

void __PHYSFS_smallFree(void *ptr);


/* Use the allocation hooks. */
#define malloc(x) Do not use malloc() directly.
#define realloc(x, y) Do not use realloc() directly.
#define free(x) Do not use free() directly.
/* !!! FIXME: add alloca check here. */


/* by default, enable things, so builds can opt out of a few things they
   want to avoid. But you can build with this #defined to 0 if you would
   like to turn off everything except a handful of things you opt into. */
#ifndef PHYSFS_SUPPORTS_DEFAULT
#define PHYSFS_SUPPORTS_DEFAULT 1
#endif


#ifndef PHYSFS_SUPPORTS_ZIP
#define PHYSFS_SUPPORTS_ZIP PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_7Z
#define PHYSFS_SUPPORTS_7Z PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_GRP
#define PHYSFS_SUPPORTS_GRP PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_HOG
#define PHYSFS_SUPPORTS_HOG PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_MVL
#define PHYSFS_SUPPORTS_MVL PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_WAD
#define PHYSFS_SUPPORTS_WAD PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_QPAK
#define PHYSFS_SUPPORTS_QPAK PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_SLB
#define PHYSFS_SUPPORTS_SLB PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_ISO9660
#define PHYSFS_SUPPORTS_ISO9660 PHYSFS_SUPPORTS_DEFAULT
#endif
#ifndef PHYSFS_SUPPORTS_VDF
#define PHYSFS_SUPPORTS_VDF PHYSFS_SUPPORTS_DEFAULT
#endif

#if PHYSFS_SUPPORTS_7Z
/* 7zip support needs a global init function called at startup (no deinit). */
extern void SZIP_global_init(void);
#endif

/* The latest supported PHYSFS_Io::version value. */
#define CURRENT_PHYSFS_IO_API_VERSION 0

/* The latest supported PHYSFS_Archiver::version value. */
#define CURRENT_PHYSFS_ARCHIVER_API_VERSION 0

/* This byteorder stuff was lifted from SDL. https://www.libsdl.org/ */
#define PHYSFS_LIL_ENDIAN  1234
#define PHYSFS_BIG_ENDIAN  4321

#ifdef __linux__
#include <endian.h>
#define PHYSFS_BYTEORDER  __BYTE_ORDER
#else /* __linux__ */
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MISPEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define PHYSFS_BYTEORDER   PHYSFS_BIG_ENDIAN
#else
#define PHYSFS_BYTEORDER   PHYSFS_LIL_ENDIAN
#endif
#endif /* __linux__ */


/*
 * When sorting the entries in an archive, we use a modified QuickSort.
 *  When there are less then PHYSFS_QUICKSORT_THRESHOLD entries left to sort,
 *  we switch over to a BubbleSort for the remainder. Tweak to taste.
 *
 * You can override this setting by defining PHYSFS_QUICKSORT_THRESHOLD
 *  before #including "physfs_internal.h".
 */
#ifndef PHYSFS_QUICKSORT_THRESHOLD
#define PHYSFS_QUICKSORT_THRESHOLD 4
#endif

/*
 * Sort an array (or whatever) of (max) elements. This uses a mixture of
 *  a QuickSort and BubbleSort internally.
 * (cmpfn) is used to determine ordering, and (swapfn) does the actual
 *  swapping of elements in the list.
 */
void __PHYSFS_sort(void *entries, size_t max,
                   int (*cmpfn)(void *, size_t, size_t),
                   void (*swapfn)(void *, size_t, size_t));

/* These get used all over for lessening code clutter. */
/* "ERRPASS" means "something else just set the error state for us" and is
    just to make it clear where the responsibility for the error state lays. */
#define BAIL(e, r) do { if (e) PHYSFS_setErrorCode(e); return r; } while (0)
#define BAIL_ERRPASS(r) do { return r; } while (0)
#define BAIL_IF(c, e, r) do { if (c) { if (e) PHYSFS_setErrorCode(e); return r; } } while (0)
#define BAIL_IF_ERRPASS(c, r) do { if (c) { return r; } } while (0)
#define BAIL_MUTEX(e, m, r) do { if (e) PHYSFS_setErrorCode(e); __PHYSFS_platformReleaseMutex(m); return r; } while (0)
#define BAIL_MUTEX_ERRPASS(m, r) do { __PHYSFS_platformReleaseMutex(m); return r; } while (0)
#define BAIL_IF_MUTEX(c, e, m, r) do { if (c) { if (e) PHYSFS_setErrorCode(e); __PHYSFS_platformReleaseMutex(m); return r; } } while (0)
#define BAIL_IF_MUTEX_ERRPASS(c, m, r) do { if (c) { __PHYSFS_platformReleaseMutex(m); return r; } } while (0)
#define GOTO(e, g) do { if (e) PHYSFS_setErrorCode(e); goto g; } while (0)
#define GOTO_ERRPASS(g) do { goto g; } while (0)
#define GOTO_IF(c, e, g) do { if (c) { if (e) PHYSFS_setErrorCode(e); goto g; } } while (0)
#define GOTO_IF_ERRPASS(c, g) do { if (c) { goto g; } } while (0)
#define GOTO_MUTEX(e, m, g) do { if (e) PHYSFS_setErrorCode(e); __PHYSFS_platformReleaseMutex(m); goto g; } while (0)
#define GOTO_MUTEX_ERRPASS(m, g) do { __PHYSFS_platformReleaseMutex(m); goto g; } while (0)
#define GOTO_IF_MUTEX(c, e, m, g) do { if (c) { if (e) PHYSFS_setErrorCode(e); __PHYSFS_platformReleaseMutex(m); goto g; } } while (0)
#define GOTO_IF_MUTEX_ERRPASS(c, m, g) do { if (c) { __PHYSFS_platformReleaseMutex(m); goto g; } } while (0)

#define __PHYSFS_ARRAYLEN(x) ( (sizeof (x)) / (sizeof (x[0])) )

#ifdef PHYSFS_NO_64BIT_SUPPORT
#define __PHYSFS_SI64(x) ((PHYSFS_sint64) (x))
#define __PHYSFS_UI64(x) ((PHYSFS_uint64) (x))
#elif (defined __GNUC__)
#define __PHYSFS_SI64(x) x##LL
#define __PHYSFS_UI64(x) x##ULL
#elif (defined _MSC_VER)
#define __PHYSFS_SI64(x) x##i64
#define __PHYSFS_UI64(x) x##ui64
#else
#define __PHYSFS_SI64(x) ((PHYSFS_sint64) (x))
#define __PHYSFS_UI64(x) ((PHYSFS_uint64) (x))
#endif


/*
 * Check if a ui64 will fit in the platform's address space.
 *  The initial sizeof check will optimize this macro out entirely on
 *  64-bit (and larger?!) platforms, and the other condition will
 *  return zero or non-zero if the variable will fit in the platform's
 *  size_t, suitable to pass to malloc. This is kinda messy, but effective.
 */
#define __PHYSFS_ui64FitsAddressSpace(s) ( \
    (sizeof (PHYSFS_uint64) <= sizeof (size_t)) || \
    ((s) < (__PHYSFS_UI64(0xFFFFFFFFFFFFFFFF) >> (64-(sizeof(size_t)*8)))) \
)

/*
 * Like strdup(), but uses the current PhysicsFS allocator.
 */
char *__PHYSFS_strdup(const char *str);

/*
 * Give a hash value for a C string (uses djb's xor hashing algorithm).
 */
PHYSFS_uint32 __PHYSFS_hashString(const char *str, size_t len);


/*
 * The current allocator. Not valid before PHYSFS_init is called!
 */
extern PHYSFS_Allocator __PHYSFS_AllocatorHooks;

/* convenience macro to make this less cumbersome internally... */
#define allocator __PHYSFS_AllocatorHooks

/*
 * Create a PHYSFS_Io for a file in the physical filesystem.
 *  This path is in platform-dependent notation. (mode) must be 'r', 'w', or
 *  'a' for Read, Write, or Append.
 */
PHYSFS_Io *__PHYSFS_createNativeIo(const char *path, const int mode);

/*
 * Create a PHYSFS_Io for a buffer of memory (READ-ONLY). If you already
 *  have one of these, just use its duplicate() method, and it'll increment
 *  its refcount without allocating a copy of the buffer.
 */
PHYSFS_Io *__PHYSFS_createMemoryIo(const void *buf, PHYSFS_uint64 len,
                                   void (*destruct)(void *));


/*
 * Read (len) bytes from (io) into (buf). Returns non-zero on success,
 *  zero on i/o error. Literally: "return (io->read(io, buf, len) == len);"
 */
int __PHYSFS_readAll(PHYSFS_Io *io, void *buf, const size_t len);


/* These are shared between some archivers. */

void UNPK_abandonArchive(void *opaque);
void UNPK_closeArchive(void *opaque);
void *UNPK_openArchive(PHYSFS_Io *io);
void *UNPK_addEntry(void *opaque, char *name, const int isdir,
                    const PHYSFS_sint64 ctime, const PHYSFS_sint64 mtime,
                    const PHYSFS_uint64 pos, const PHYSFS_uint64 len);
PHYSFS_Io *UNPK_openRead(void *opaque, const char *name);
PHYSFS_Io *UNPK_openWrite(void *opaque, const char *name);
PHYSFS_Io *UNPK_openAppend(void *opaque, const char *name);
int UNPK_remove(void *opaque, const char *name);
int UNPK_mkdir(void *opaque, const char *name);
int UNPK_stat(void *opaque, const char *fn, PHYSFS_Stat *st);
#define UNPK_enumerate __PHYSFS_DirTreeEnumerate



/* Optional API many archivers use this to manage their directory tree. */
/* !!! FIXME: document this better. */

typedef struct __PHYSFS_DirTreeEntry
{
    char *name;                              /* Full path in archive.        */
    struct __PHYSFS_DirTreeEntry *hashnext;  /* next item in hash bucket.    */
    struct __PHYSFS_DirTreeEntry *children;  /* linked list of kids, if dir. */
    struct __PHYSFS_DirTreeEntry *sibling;   /* next item in same dir.       */
    int isdir;
} __PHYSFS_DirTreeEntry;

typedef struct __PHYSFS_DirTree
{
    __PHYSFS_DirTreeEntry *root;    /* root of directory tree.             */
    __PHYSFS_DirTreeEntry **hash;  /* all entries hashed for fast lookup. */
    size_t hashBuckets;            /* number of buckets in hash.          */
    size_t entrylen;    /* size in bytes of entries (including subclass). */
} __PHYSFS_DirTree;


int __PHYSFS_DirTreeInit(__PHYSFS_DirTree *dt, const size_t entrylen);
void *__PHYSFS_DirTreeAdd(__PHYSFS_DirTree *dt, char *name, const int isdir);
void *__PHYSFS_DirTreeFind(__PHYSFS_DirTree *dt, const char *path);
PHYSFS_EnumerateCallbackResult __PHYSFS_DirTreeEnumerate(void *opaque,
                              const char *dname, PHYSFS_EnumerateCallback cb,
                              const char *origdir, void *callbackdata);
void __PHYSFS_DirTreeDeinit(__PHYSFS_DirTree *dt);



/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*------------                                              ----------------*/
/*------------  You MUST implement the following functions  ----------------*/
/*------------        if porting to a new platform.         ----------------*/
/*------------     (see platform/unix.c for an example)     ----------------*/
/*------------                                              ----------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


/*
 * The dir separator; '/' on unix, '\\' on win32, ":" on MacOS, etc...
 *  Obviously, this isn't a function. If you need more than one char for this,
 *  you'll need to pull some old pieces of PhysicsFS out of revision control.
 */
#if defined(PHYSFS_PLATFORM_WINDOWS) || defined(PHYSFS_PLATFORM_OS2)
#define __PHYSFS_platformDirSeparator '\\'
#else
#define __PHYSFS_STANDARD_DIRSEP 1
#define __PHYSFS_platformDirSeparator '/'
#endif

/*
 * Initialize the platform. This is called when PHYSFS_init() is called from
 *  the application.
 *
 * Return zero if there was a catastrophic failure (which prevents you from
 *  functioning at all), and non-zero otherwise.
 */
int __PHYSFS_platformInit(void);


/*
 * Deinitialize the platform. This is called when PHYSFS_deinit() is called
 *  from the application. You can use this to clean up anything you've
 *  allocated in your platform driver.
 */
void __PHYSFS_platformDeinit(void);


/*
 * Open a file for reading. (filename) is in platform-dependent notation. The
 *  file pointer should be positioned on the first byte of the file.
 *
 * The return value will be some platform-specific datatype that is opaque to
 *  the caller; it could be a (FILE *) under Unix, or a (HANDLE *) under win32.
 *
 * The same file can be opened for read multiple times, and each should have
 *  a unique file handle; this is frequently employed to prevent race
 *  conditions in the archivers.
 *
 * Call PHYSFS_setErrorCode() and return (NULL) if the file can't be opened.
 */
void *__PHYSFS_platformOpenRead(const char *filename);


/*
 * Open a file for writing. (filename) is in platform-dependent notation. If
 *  the file exists, it should be truncated to zero bytes, and if it doesn't
 *  exist, it should be created as a zero-byte file. The file pointer should
 *  be positioned on the first byte of the file.
 *
 * The return value will be some platform-specific datatype that is opaque to
 *  the caller; it could be a (FILE *) under Unix, or a (HANDLE *) under win32,
 *  etc.
 *
 * Opening a file for write multiple times has undefined results.
 *
 * Call PHYSFS_setErrorCode() and return (NULL) if the file can't be opened.
 */
void *__PHYSFS_platformOpenWrite(const char *filename);


/*
 * Open a file for appending. (filename) is in platform-dependent notation. If
 *  the file exists, the file pointer should be place just past the end of the
 *  file, so that the first write will be one byte after the current end of
 *  the file. If the file doesn't exist, it should be created as a zero-byte
 *  file. The file pointer should be positioned on the first byte of the file.
 *
 * The return value will be some platform-specific datatype that is opaque to
 *  the caller; it could be a (FILE *) under Unix, or a (HANDLE *) under win32,
 *  etc.
 *
 * Opening a file for append multiple times has undefined results.
 *
 * Call PHYSFS_setErrorCode() and return (NULL) if the file can't be opened.
 */
void *__PHYSFS_platformOpenAppend(const char *filename);

/*
 * Read more data from a platform-specific file handle. (opaque) should be
 *  cast to whatever data type your platform uses. Read a maximum of (len)
 *  8-bit bytes to the area pointed to by (buf). If there isn't enough data
 *  available, return the number of bytes read, and position the file pointer
 *  immediately after those bytes.
 *  On success, return (len) and position the file pointer immediately past
 *  the end of the last read byte. Return (-1) if there is a catastrophic
 *  error, and call PHYSFS_setErrorCode() to describe the problem; the file
 *  pointer should not move in such a case. A partial read is success; only
 *  return (-1) on total failure; presumably, the next read call after a
 *  partial read will fail as such.
 */
PHYSFS_sint64 __PHYSFS_platformRead(void *opaque, void *buf, PHYSFS_uint64 len);

/*
 * Write more data to a platform-specific file handle. (opaque) should be
 *  cast to whatever data type your platform uses. Write a maximum of (len)
 *  8-bit bytes from the area pointed to by (buffer). If there is a problem,
 *  return the number of bytes written, and position the file pointer
 *  immediately after those bytes. Return (-1) if there is a catastrophic
 *  error, and call PHYSFS_setErrorCode() to describe the problem; the file
 *  pointer should not move in such a case. A partial write is success; only
 *  return (-1) on total failure; presumably, the next write call after a
 *  partial write will fail as such.
 */
PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buffer,
                                     PHYSFS_uint64 len);

/*
 * Set the file pointer to a new position. (opaque) should be cast to
 *  whatever data type your platform uses. (pos) specifies the number
 *  of 8-bit bytes to seek to from the start of the file. Seeking past the
 *  end of the file is an error condition, and you should check for it.
 *
 * Not all file types can seek; this is to be expected by the caller.
 *
 * On error, call PHYSFS_setErrorCode() and return zero. On success, return
 *  a non-zero value.
 */
int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos);


/*
 * Get the file pointer's position, in an 8-bit byte offset from the start of
 *  the file. (opaque) should be cast to whatever data type your platform
 *  uses.
 *
 * Not all file types can "tell"; this is to be expected by the caller.
 *
 * On error, call PHYSFS_setErrorCode() and return -1. On success, return >= 0.
 */
PHYSFS_sint64 __PHYSFS_platformTell(void *opaque);


/*
 * Determine the current size of a file, in 8-bit bytes, from an open file.
 *
 * The caller expects that this information may not be available for all
 *  file types on all platforms.
 *
 * Return -1 if you can't do it, and call PHYSFS_setErrorCode(). Otherwise,
 *  return the file length in 8-bit bytes.
 */
PHYSFS_sint64 __PHYSFS_platformFileLength(void *handle);


/*
 * Read filesystem metadata for a specific path.
 *
 * This needs to fill in all the fields of (stat). For fields that might not
 *  mean anything on a platform (access time, perhaps), choose a reasonable
 *  default. if (follow), we want to follow symlinks and stat what they
 *  link to and not the link itself.
 *
 *  Return zero on failure, non-zero on success.
 */
int __PHYSFS_platformStat(const char *fn, PHYSFS_Stat *stat, const int follow);

/*
 * Flush any pending writes to disk. (opaque) should be cast to whatever data
 *  type your platform uses. Be sure to check for errors; the caller expects
 *  that this function can fail if there was a flushing error, etc.
 *
 *  Return zero on failure, non-zero on success.
 */
int __PHYSFS_platformFlush(void *opaque);

/*
 * Close file and deallocate resources. (opaque) should be cast to whatever
 *  data type your platform uses. This should close the file in any scenario:
 *  flushing is a separate function call, and this function should never fail.
 *
 * You should clean up all resources associated with (opaque); the pointer
 *  will be considered invalid after this call.
 */
void __PHYSFS_platformClose(void *opaque);

/*
 * Platform implementation of PHYSFS_getCdRomDirsCallback()...
 *  CD directories are discovered and reported to the callback one at a time.
 *  Pointers passed to the callback are assumed to be invalid to the
 *  application after the callback returns, so you can free them or whatever.
 *  Callback does not assume results will be sorted in any meaningful way.
 */
void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data);

/*
 * Calculate the base dir, if your platform needs special consideration.
 *  Just return NULL if the standard routines will suffice. (see
 *  calculateBaseDir() in physfs.c ...)
 * Your string must end with a dir separator if you don't return NULL.
 *  Caller will allocator.Free() the retval if it's not NULL.
 */
char *__PHYSFS_platformCalcBaseDir(const char *argv0);

/*
 * Get the platform-specific user dir.
 * As of PhysicsFS 2.1, returning NULL means fatal error.
 * Your string must end with a dir separator if you don't return NULL.
 *  Caller will allocator.Free() the retval if it's not NULL.
 */
char *__PHYSFS_platformCalcUserDir(void);


/* This is the cached version from PHYSFS_init(). This is a fast call. */
const char *__PHYSFS_getUserDir(void);  /* not deprecated internal version. */


/*
 * Get the platform-specific pref dir.
 * Returning NULL means fatal error.
 * Your string must end with a dir separator if you don't return NULL.
 *  Caller will allocator.Free() the retval if it's not NULL.
 *  Caller will make missing directories if necessary; this just reports
 *   the final path.
 */
char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app);


/*
 * Return a pointer that uniquely identifies the current thread.
 *  On a platform without threading, (0x1) will suffice. These numbers are
 *  arbitrary; the only requirement is that no two threads have the same
 *  pointer.
 */
void *__PHYSFS_platformGetThreadID(void);


/*
 * Enumerate a directory of files. This follows the rules for the
 *  PHYSFS_Archiver::enumerate() method, except that the (dirName) that is
 *  passed to this function is converted to platform-DEPENDENT notation by
 *  the caller. The PHYSFS_Archiver version uses platform-independent
 *  notation. Note that ".", "..", and other meta-entries should always
 *  be ignored.
 */
PHYSFS_EnumerateCallbackResult __PHYSFS_platformEnumerate(const char *dirname,
                               PHYSFS_EnumerateCallback callback,
                               const char *origdir, void *callbackdata);

/*
 * Make a directory in the actual filesystem. (path) is specified in
 *  platform-dependent notation. On error, return zero and set the error
 *  message. Return non-zero on success.
 */
int __PHYSFS_platformMkDir(const char *path);


/*
 * Remove a file or directory entry in the actual filesystem. (path) is
 *  specified in platform-dependent notation. Note that this deletes files
 *  _and_ directories, so you might need to do some determination.
 *  Non-empty directories should report an error and not delete themselves
 *  or their contents.
 *
 * Deleting a symlink should remove the link, not what it points to.
 *
 * On error, return zero and set the error message. Return non-zero on success.
 */
int __PHYSFS_platformDelete(const char *path);


/*
 * Create a platform-specific mutex. This can be whatever datatype your
 *  platform uses for mutexes, but it is cast to a (void *) for abstractness.
 *
 * Return (NULL) if you couldn't create one. Systems without threads can
 *  return any arbitrary non-NULL value.
 */
void *__PHYSFS_platformCreateMutex(void);

/*
 * Destroy a platform-specific mutex, and clean up any resources associated
 *  with it. (mutex) is a value previously returned by
 *  __PHYSFS_platformCreateMutex(). This can be a no-op on single-threaded
 *  platforms.
 */
void __PHYSFS_platformDestroyMutex(void *mutex);

/*
 * Grab possession of a platform-specific mutex. Mutexes should be recursive;
 *  that is, the same thread should be able to call this function multiple
 *  times in a row without causing a deadlock. This function should block 
 *  until a thread can gain possession of the mutex.
 *
 * Return non-zero if the mutex was grabbed, zero if there was an 
 *  unrecoverable problem grabbing it (this should not be a matter of 
 *  timing out! We're talking major system errors; block until the mutex 
 *  is available otherwise.)
 *
 * _DO NOT_ call PHYSFS_setErrorCode() in here! Since setErrorCode calls this
 *  function, you'll cause an infinite recursion. This means you can't
 *  use the BAIL_*MACRO* macros, either.
 */
int __PHYSFS_platformGrabMutex(void *mutex);

/*
 * Relinquish possession of the mutex when this method has been called 
 *  once for each time that platformGrabMutex was called. Once possession has
 *  been released, the next thread in line to grab the mutex (if any) may
 *  proceed.
 *
 * _DO NOT_ call PHYSFS_setErrorCode() in here! Since setErrorCode calls this
 *  function, you'll cause an infinite recursion. This means you can't
 *  use the BAIL_*MACRO* macros, either.
 */
void __PHYSFS_platformReleaseMutex(void *mutex);

#if PHYSFS_HAVE_PRAGMA_VISIBILITY
#pragma GCC visibility pop
#endif

#ifdef __cplusplus
}
#endif

#endif

/* end of physfs_internal.h ... */

