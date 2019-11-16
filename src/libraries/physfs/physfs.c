/**
 * PhysicsFS; a portable, flexible file i/o abstraction.
 *
 * Documentation is in physfs.h. It's verbose, honest.  :)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if defined(_MSC_VER)
#include <stdarg.h>

/* this code came from https://stackoverflow.com/a/8712996 */
int __PHYSFS_msvc_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

int __PHYSFS_msvc_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = __PHYSFS_msvc_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}
#endif


typedef struct __PHYSFS_DIRHANDLE__
{
    void *opaque;  /* Instance data unique to the archiver. */
    char *dirName;  /* Path to archive in platform-dependent notation. */
    char *mountPoint; /* Mountpoint in virtual file tree. */
    char *root;  /* subdirectory of archiver to use as root of archive (NULL for actual root) */
    size_t rootlen;  /* subdirectory of archiver to use as root of archive (NULL for actual root) */
    const PHYSFS_Archiver *funcs;  /* Ptr to archiver info for this handle. */
    struct __PHYSFS_DIRHANDLE__ *next;  /* linked list stuff. */
} DirHandle;


typedef struct __PHYSFS_FILEHANDLE__
{
    PHYSFS_Io *io;  /* Instance data unique to the archiver for this file. */
    PHYSFS_uint8 forReading; /* Non-zero if reading, zero if write/append */
    const DirHandle *dirHandle;  /* Archiver instance that created this */
    PHYSFS_uint8 *buffer;  /* Buffer, if set (NULL otherwise). Don't touch! */
    size_t bufsize;  /* Bufsize, if set (0 otherwise). Don't touch! */
    size_t buffill;  /* Buffer fill size. Don't touch! */
    size_t bufpos;  /* Buffer position. Don't touch! */
    struct __PHYSFS_FILEHANDLE__ *next;  /* linked list stuff. */
} FileHandle;


typedef struct __PHYSFS_ERRSTATETYPE__
{
    void *tid;
    PHYSFS_ErrorCode code;
    struct __PHYSFS_ERRSTATETYPE__ *next;
} ErrState;


/* General PhysicsFS state ... */
static int initialized = 0;
static ErrState *errorStates = NULL;
static DirHandle *searchPath = NULL;
static DirHandle *writeDir = NULL;
static FileHandle *openWriteList = NULL;
static FileHandle *openReadList = NULL;
static char *baseDir = NULL;
static char *userDir = NULL;
static char *prefDir = NULL;
static int allowSymLinks = 0;
static PHYSFS_Archiver **archivers = NULL;
static PHYSFS_ArchiveInfo **archiveInfo = NULL;
static volatile size_t numArchivers = 0;
static size_t longest_root = 0;

/* mutexes ... */
static void *errorLock = NULL;     /* protects error message list.        */
static void *stateLock = NULL;     /* protects other PhysFS static state. */

/* allocator ... */
static int externalAllocator = 0;
PHYSFS_Allocator allocator;


#ifdef PHYSFS_NEED_ATOMIC_OP_FALLBACK
static inline int __PHYSFS_atomicAdd(int *ptrval, const int val)
{
    int retval;
    __PHYSFS_platformGrabMutex(stateLock);
    retval = *ptrval;
    *ptrval = retval + val;
    __PHYSFS_platformReleaseMutex(stateLock);
    return retval;
} /* __PHYSFS_atomicAdd */

int __PHYSFS_ATOMIC_INCR(int *ptrval)
{
    return __PHYSFS_atomicAdd(ptrval, 1);
} /* __PHYSFS_ATOMIC_INCR */

int __PHYSFS_ATOMIC_DECR(int *ptrval)
{
    return __PHYSFS_atomicAdd(ptrval, -1);
} /* __PHYSFS_ATOMIC_DECR */
#endif



/* PHYSFS_Io implementation for i/o to physical filesystem... */

/* !!! FIXME: maybe refcount the paths in a string pool? */
typedef struct __PHYSFS_NativeIoInfo
{
    void *handle;
    const char *path;
    int mode;   /* 'r', 'w', or 'a' */
} NativeIoInfo;

static PHYSFS_sint64 nativeIo_read(PHYSFS_Io *io, void *buf, PHYSFS_uint64 len)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_platformRead(info->handle, buf, len);
} /* nativeIo_read */

static PHYSFS_sint64 nativeIo_write(PHYSFS_Io *io, const void *buffer,
                                    PHYSFS_uint64 len)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_platformWrite(info->handle, buffer, len);
} /* nativeIo_write */

static int nativeIo_seek(PHYSFS_Io *io, PHYSFS_uint64 offset)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_platformSeek(info->handle, offset);
} /* nativeIo_seek */

static PHYSFS_sint64 nativeIo_tell(PHYSFS_Io *io)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_platformTell(info->handle);
} /* nativeIo_tell */

static PHYSFS_sint64 nativeIo_length(PHYSFS_Io *io)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_platformFileLength(info->handle);
} /* nativeIo_length */

static PHYSFS_Io *nativeIo_duplicate(PHYSFS_Io *io)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_createNativeIo(info->path, info->mode);
} /* nativeIo_duplicate */

static int nativeIo_flush(PHYSFS_Io *io)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    return __PHYSFS_platformFlush(info->handle);
} /* nativeIo_flush */

static void nativeIo_destroy(PHYSFS_Io *io)
{
    NativeIoInfo *info = (NativeIoInfo *) io->opaque;
    __PHYSFS_platformClose(info->handle);
    allocator.Free((void *) info->path);
    allocator.Free(info);
    allocator.Free(io);
} /* nativeIo_destroy */

static const PHYSFS_Io __PHYSFS_nativeIoInterface =
{
    CURRENT_PHYSFS_IO_API_VERSION, NULL,
    nativeIo_read,
    nativeIo_write,
    nativeIo_seek,
    nativeIo_tell,
    nativeIo_length,
    nativeIo_duplicate,
    nativeIo_flush,
    nativeIo_destroy
};

PHYSFS_Io *__PHYSFS_createNativeIo(const char *path, const int mode)
{
    PHYSFS_Io *io = NULL;
    NativeIoInfo *info = NULL;
    void *handle = NULL;
    char *pathdup = NULL;

    assert((mode == 'r') || (mode == 'w') || (mode == 'a'));

    io = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    GOTO_IF(!io, PHYSFS_ERR_OUT_OF_MEMORY, createNativeIo_failed);
    info = (NativeIoInfo *) allocator.Malloc(sizeof (NativeIoInfo));
    GOTO_IF(!info, PHYSFS_ERR_OUT_OF_MEMORY, createNativeIo_failed);
    pathdup = (char *) allocator.Malloc(strlen(path) + 1);
    GOTO_IF(!pathdup, PHYSFS_ERR_OUT_OF_MEMORY, createNativeIo_failed);

    if (mode == 'r')
        handle = __PHYSFS_platformOpenRead(path);
    else if (mode == 'w')
        handle = __PHYSFS_platformOpenWrite(path);
    else if (mode == 'a')
        handle = __PHYSFS_platformOpenAppend(path);

    GOTO_IF_ERRPASS(!handle, createNativeIo_failed);

    strcpy(pathdup, path);
    info->handle = handle;
    info->path = pathdup;
    info->mode = mode;
    memcpy(io, &__PHYSFS_nativeIoInterface, sizeof (*io));
    io->opaque = info;
    return io;

createNativeIo_failed:
    if (handle != NULL) __PHYSFS_platformClose(handle);
    if (pathdup != NULL) allocator.Free(pathdup);
    if (info != NULL) allocator.Free(info);
    if (io != NULL) allocator.Free(io);
    return NULL;
} /* __PHYSFS_createNativeIo */


/* PHYSFS_Io implementation for i/o to a memory buffer... */

typedef struct __PHYSFS_MemoryIoInfo
{
    const PHYSFS_uint8 *buf;
    PHYSFS_uint64 len;
    PHYSFS_uint64 pos;
    PHYSFS_Io *parent;
    int refcount;
    void (*destruct)(void *);
} MemoryIoInfo;

static PHYSFS_sint64 memoryIo_read(PHYSFS_Io *io, void *buf, PHYSFS_uint64 len)
{
    MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
    const PHYSFS_uint64 avail = info->len - info->pos;
    assert(avail <= info->len);

    if (avail == 0)
        return 0;  /* we're at EOF; nothing to do. */

    if (len > avail)
        len = avail;

    memcpy(buf, info->buf + info->pos, (size_t) len);
    info->pos += len;
    return len;
} /* memoryIo_read */

static PHYSFS_sint64 memoryIo_write(PHYSFS_Io *io, const void *buffer,
                                    PHYSFS_uint64 len)
{
    BAIL(PHYSFS_ERR_OPEN_FOR_READING, -1);
} /* memoryIo_write */

static int memoryIo_seek(PHYSFS_Io *io, PHYSFS_uint64 offset)
{
    MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
    BAIL_IF(offset > info->len, PHYSFS_ERR_PAST_EOF, 0);
    info->pos = offset;
    return 1;
} /* memoryIo_seek */

static PHYSFS_sint64 memoryIo_tell(PHYSFS_Io *io)
{
    const MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
    return (PHYSFS_sint64) info->pos;
} /* memoryIo_tell */

static PHYSFS_sint64 memoryIo_length(PHYSFS_Io *io)
{
    const MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
    return (PHYSFS_sint64) info->len;
} /* memoryIo_length */

static PHYSFS_Io *memoryIo_duplicate(PHYSFS_Io *io)
{
    MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
    MemoryIoInfo *newinfo = NULL;
    PHYSFS_Io *parent = info->parent;
    PHYSFS_Io *retval = NULL;

    /* avoid deep copies. */
    assert((!parent) || (!((MemoryIoInfo *) parent->opaque)->parent) );

    /* share the buffer between duplicates. */
    if (parent != NULL)  /* dup the parent, increment its refcount. */
        return parent->duplicate(parent);

    /* we're the parent. */

    retval = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    newinfo = (MemoryIoInfo *) allocator.Malloc(sizeof (MemoryIoInfo));
    if (!newinfo)
    {
        allocator.Free(retval);
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    } /* if */

    __PHYSFS_ATOMIC_INCR(&info->refcount);

    memset(newinfo, '\0', sizeof (*info));
    newinfo->buf = info->buf;
    newinfo->len = info->len;
    newinfo->pos = 0;
    newinfo->parent = io;
    newinfo->refcount = 0;
    newinfo->destruct = NULL;

    memcpy(retval, io, sizeof (*retval));
    retval->opaque = newinfo;
    return retval;
} /* memoryIo_duplicate */

static int memoryIo_flush(PHYSFS_Io *io) { return 1;  /* it's read-only. */ }

static void memoryIo_destroy(PHYSFS_Io *io)
{
    MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
    PHYSFS_Io *parent = info->parent;

    if (parent != NULL)
    {
        assert(info->buf == ((MemoryIoInfo *) info->parent->opaque)->buf);
        assert(info->len == ((MemoryIoInfo *) info->parent->opaque)->len);
        assert(info->refcount == 0);
        assert(info->destruct == NULL);
        allocator.Free(info);
        allocator.Free(io);
        parent->destroy(parent);  /* decrements refcount. */
        return;
    } /* if */

    /* we _are_ the parent. */
    assert(info->refcount > 0);  /* even in a race, we hold a reference. */

    if (__PHYSFS_ATOMIC_DECR(&info->refcount) == 0)
    {
        void (*destruct)(void *) = info->destruct;
        void *buf = (void *) info->buf;
        io->opaque = NULL;  /* kill this here in case of race. */
        allocator.Free(info);
        allocator.Free(io);
        if (destruct != NULL)
            destruct(buf);
    } /* if */
} /* memoryIo_destroy */


static const PHYSFS_Io __PHYSFS_memoryIoInterface =
{
    CURRENT_PHYSFS_IO_API_VERSION, NULL,
    memoryIo_read,
    memoryIo_write,
    memoryIo_seek,
    memoryIo_tell,
    memoryIo_length,
    memoryIo_duplicate,
    memoryIo_flush,
    memoryIo_destroy
};

PHYSFS_Io *__PHYSFS_createMemoryIo(const void *buf, PHYSFS_uint64 len,
                                   void (*destruct)(void *))
{
    PHYSFS_Io *io = NULL;
    MemoryIoInfo *info = NULL;

    io = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    GOTO_IF(!io, PHYSFS_ERR_OUT_OF_MEMORY, createMemoryIo_failed);
    info = (MemoryIoInfo *) allocator.Malloc(sizeof (MemoryIoInfo));
    GOTO_IF(!info, PHYSFS_ERR_OUT_OF_MEMORY, createMemoryIo_failed);

    memset(info, '\0', sizeof (*info));
    info->buf = (const PHYSFS_uint8 *) buf;
    info->len = len;
    info->pos = 0;
    info->parent = NULL;
    info->refcount = 1;
    info->destruct = destruct;

    memcpy(io, &__PHYSFS_memoryIoInterface, sizeof (*io));
    io->opaque = info;
    return io;

createMemoryIo_failed:
    if (info != NULL) allocator.Free(info);
    if (io != NULL) allocator.Free(io);
    return NULL;
} /* __PHYSFS_createMemoryIo */


/* PHYSFS_Io implementation for i/o to a PHYSFS_File... */

static PHYSFS_sint64 handleIo_read(PHYSFS_Io *io, void *buf, PHYSFS_uint64 len)
{
    return PHYSFS_readBytes((PHYSFS_File *) io->opaque, buf, len);
} /* handleIo_read */

static PHYSFS_sint64 handleIo_write(PHYSFS_Io *io, const void *buffer,
                                    PHYSFS_uint64 len)
{
    return PHYSFS_writeBytes((PHYSFS_File *) io->opaque, buffer, len);
} /* handleIo_write */

static int handleIo_seek(PHYSFS_Io *io, PHYSFS_uint64 offset)
{
    return PHYSFS_seek((PHYSFS_File *) io->opaque, offset);
} /* handleIo_seek */

static PHYSFS_sint64 handleIo_tell(PHYSFS_Io *io)
{
    return PHYSFS_tell((PHYSFS_File *) io->opaque);
} /* handleIo_tell */

static PHYSFS_sint64 handleIo_length(PHYSFS_Io *io)
{
    return PHYSFS_fileLength((PHYSFS_File *) io->opaque);
} /* handleIo_length */

static PHYSFS_Io *handleIo_duplicate(PHYSFS_Io *io)
{
    /*
     * There's no duplicate at the PHYSFS_File level, so we break the
     *  abstraction. We're allowed to: we're physfs.c!
     */
    FileHandle *origfh = (FileHandle *) io->opaque;
    FileHandle *newfh = (FileHandle *) allocator.Malloc(sizeof (FileHandle));
    PHYSFS_Io *retval = NULL;

    GOTO_IF(!newfh, PHYSFS_ERR_OUT_OF_MEMORY, handleIo_dupe_failed);
    memset(newfh, '\0', sizeof (*newfh));

    retval = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    GOTO_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, handleIo_dupe_failed);

#if 0  /* we don't buffer the duplicate, at least not at the moment. */
    if (origfh->buffer != NULL)
    {
        newfh->buffer = (PHYSFS_uint8 *) allocator.Malloc(origfh->bufsize);
        if (!newfh->buffer)
            GOTO(PHYSFS_ERR_OUT_OF_MEMORY, handleIo_dupe_failed);
        newfh->bufsize = origfh->bufsize;
    } /* if */
#endif

    newfh->io = origfh->io->duplicate(origfh->io);
    GOTO_IF_ERRPASS(!newfh->io, handleIo_dupe_failed);

    newfh->forReading = origfh->forReading;
    newfh->dirHandle = origfh->dirHandle;

    __PHYSFS_platformGrabMutex(stateLock);
    if (newfh->forReading)
    {
        newfh->next = openReadList;
        openReadList = newfh;
    } /* if */
    else
    {
        newfh->next = openWriteList;
        openWriteList = newfh;
    } /* else */
    __PHYSFS_platformReleaseMutex(stateLock);

    memcpy(retval, io, sizeof (PHYSFS_Io));
    retval->opaque = newfh;
    return retval;
    
handleIo_dupe_failed:
    if (newfh)
    {
        if (newfh->io != NULL) newfh->io->destroy(newfh->io);
        if (newfh->buffer != NULL) allocator.Free(newfh->buffer);
        allocator.Free(newfh);
    } /* if */

    return NULL;
} /* handleIo_duplicate */

static int handleIo_flush(PHYSFS_Io *io)
{
    return PHYSFS_flush((PHYSFS_File *) io->opaque);
} /* handleIo_flush */

static void handleIo_destroy(PHYSFS_Io *io)
{
    if (io->opaque != NULL)
        PHYSFS_close((PHYSFS_File *) io->opaque);
    allocator.Free(io);
} /* handleIo_destroy */

static const PHYSFS_Io __PHYSFS_handleIoInterface =
{
    CURRENT_PHYSFS_IO_API_VERSION, NULL,
    handleIo_read,
    handleIo_write,
    handleIo_seek,
    handleIo_tell,
    handleIo_length,
    handleIo_duplicate,
    handleIo_flush,
    handleIo_destroy
};

static PHYSFS_Io *__PHYSFS_createHandleIo(PHYSFS_File *f)
{
    PHYSFS_Io *io = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    BAIL_IF(!io, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    memcpy(io, &__PHYSFS_handleIoInterface, sizeof (*io));
    io->opaque = f;
    return io;
} /* __PHYSFS_createHandleIo */


/* functions ... */

typedef struct
{
    char **list;
    PHYSFS_uint32 size;
    PHYSFS_ErrorCode errcode;
} EnumStringListCallbackData;

static void enumStringListCallback(void *data, const char *str)
{
    void *ptr;
    char *newstr;
    EnumStringListCallbackData *pecd = (EnumStringListCallbackData *) data;

    if (pecd->errcode)
        return;

    ptr = allocator.Realloc(pecd->list, (pecd->size + 2) * sizeof (char *));
    newstr = (char *) allocator.Malloc(strlen(str) + 1);
    if (ptr != NULL)
        pecd->list = (char **) ptr;

    if ((ptr == NULL) || (newstr == NULL))
    {
        pecd->errcode = PHYSFS_ERR_OUT_OF_MEMORY;
        pecd->list[pecd->size] = NULL;
        PHYSFS_freeList(pecd->list);
        return;
    } /* if */

    strcpy(newstr, str);
    pecd->list[pecd->size] = newstr;
    pecd->size++;
} /* enumStringListCallback */


static char **doEnumStringList(void (*func)(PHYSFS_StringCallback, void *))
{
    EnumStringListCallbackData ecd;
    memset(&ecd, '\0', sizeof (ecd));
    ecd.list = (char **) allocator.Malloc(sizeof (char *));
    BAIL_IF(!ecd.list, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    func(enumStringListCallback, &ecd);

    if (ecd.errcode)
    {
        PHYSFS_setErrorCode(ecd.errcode);
        return NULL;
    } /* if */

    ecd.list[ecd.size] = NULL;
    return ecd.list;
} /* doEnumStringList */


static void __PHYSFS_bubble_sort(void *a, size_t lo, size_t hi,
                                 int (*cmpfn)(void *, size_t, size_t),
                                 void (*swapfn)(void *, size_t, size_t))
{
    size_t i;
    int sorted;

    do
    {
        sorted = 1;
        for (i = lo; i < hi; i++)
        {
            if (cmpfn(a, i, i + 1) > 0)
            {
                swapfn(a, i, i + 1);
                sorted = 0;
            } /* if */
        } /* for */
    } while (!sorted);
} /* __PHYSFS_bubble_sort */


static void __PHYSFS_quick_sort(void *a, size_t lo, size_t hi,
                         int (*cmpfn)(void *, size_t, size_t),
                         void (*swapfn)(void *, size_t, size_t))
{
    size_t i;
    size_t j;
    size_t v;

    if ((hi - lo) <= PHYSFS_QUICKSORT_THRESHOLD)
        __PHYSFS_bubble_sort(a, lo, hi, cmpfn, swapfn);
    else
    {
        i = (hi + lo) / 2;

        if (cmpfn(a, lo, i) > 0) swapfn(a, lo, i);
        if (cmpfn(a, lo, hi) > 0) swapfn(a, lo, hi);
        if (cmpfn(a, i, hi) > 0) swapfn(a, i, hi);

        j = hi - 1;
        swapfn(a, i, j);
        i = lo;
        v = j;
        while (1)
        {
            while(cmpfn(a, ++i, v) < 0) { /* do nothing */ }
            while(cmpfn(a, --j, v) > 0) { /* do nothing */ }
            if (j < i)
                break;
            swapfn(a, i, j);
        } /* while */
        if (i != (hi-1))
            swapfn(a, i, hi-1);
        __PHYSFS_quick_sort(a, lo, j, cmpfn, swapfn);
        __PHYSFS_quick_sort(a, i+1, hi, cmpfn, swapfn);
    } /* else */
} /* __PHYSFS_quick_sort */


void __PHYSFS_sort(void *entries, size_t max,
                   int (*cmpfn)(void *, size_t, size_t),
                   void (*swapfn)(void *, size_t, size_t))
{
    /*
     * Quicksort w/ Bubblesort fallback algorithm inspired by code from here:
     *   https://www.cs.ubc.ca/spider/harrison/Java/sorting-demo.html
     */
    if (max > 0)
        __PHYSFS_quick_sort(entries, 0, max - 1, cmpfn, swapfn);
} /* __PHYSFS_sort */


static ErrState *findErrorForCurrentThread(void)
{
    ErrState *i;
    void *tid;

    if (errorLock != NULL)
        __PHYSFS_platformGrabMutex(errorLock);

    if (errorStates != NULL)
    {
        tid = __PHYSFS_platformGetThreadID();

        for (i = errorStates; i != NULL; i = i->next)
        {
            if (i->tid == tid)
            {
                if (errorLock != NULL)
                    __PHYSFS_platformReleaseMutex(errorLock);
                return i;
            } /* if */
        } /* for */
    } /* if */

    if (errorLock != NULL)
        __PHYSFS_platformReleaseMutex(errorLock);

    return NULL;   /* no error available. */
} /* findErrorForCurrentThread */


/* this doesn't reset the error state. */
static inline PHYSFS_ErrorCode currentErrorCode(void)
{
    const ErrState *err = findErrorForCurrentThread();
    return err ? err->code : PHYSFS_ERR_OK;
} /* currentErrorCode */


PHYSFS_ErrorCode PHYSFS_getLastErrorCode(void)
{
    ErrState *err = findErrorForCurrentThread();
    const PHYSFS_ErrorCode retval = (err) ? err->code : PHYSFS_ERR_OK;
    if (err)
        err->code = PHYSFS_ERR_OK;
    return retval;
} /* PHYSFS_getLastErrorCode */


PHYSFS_DECL const char *PHYSFS_getErrorByCode(PHYSFS_ErrorCode code)
{
    switch (code)
    {
        case PHYSFS_ERR_OK: return "no error";
        case PHYSFS_ERR_OTHER_ERROR: return "unknown error";
        case PHYSFS_ERR_OUT_OF_MEMORY: return "out of memory";
        case PHYSFS_ERR_NOT_INITIALIZED: return "not initialized";
        case PHYSFS_ERR_IS_INITIALIZED: return "already initialized";
        case PHYSFS_ERR_ARGV0_IS_NULL: return "argv[0] is NULL";
        case PHYSFS_ERR_UNSUPPORTED: return "unsupported";
        case PHYSFS_ERR_PAST_EOF: return "past end of file";
        case PHYSFS_ERR_FILES_STILL_OPEN: return "files still open";
        case PHYSFS_ERR_INVALID_ARGUMENT: return "invalid argument";
        case PHYSFS_ERR_NOT_MOUNTED: return "not mounted";
        case PHYSFS_ERR_NOT_FOUND: return "not found";
        case PHYSFS_ERR_SYMLINK_FORBIDDEN: return "symlinks are forbidden";
        case PHYSFS_ERR_NO_WRITE_DIR: return "write directory is not set";
        case PHYSFS_ERR_OPEN_FOR_READING: return "file open for reading";
        case PHYSFS_ERR_OPEN_FOR_WRITING: return "file open for writing";
        case PHYSFS_ERR_NOT_A_FILE: return "not a file";
        case PHYSFS_ERR_READ_ONLY: return "read-only filesystem";
        case PHYSFS_ERR_CORRUPT: return "corrupted";
        case PHYSFS_ERR_SYMLINK_LOOP: return "infinite symbolic link loop";
        case PHYSFS_ERR_IO: return "i/o error";
        case PHYSFS_ERR_PERMISSION: return "permission denied";
        case PHYSFS_ERR_NO_SPACE: return "no space available for writing";
        case PHYSFS_ERR_BAD_FILENAME: return "filename is illegal or insecure";
        case PHYSFS_ERR_BUSY: return "tried to modify a file the OS needs";
        case PHYSFS_ERR_DIR_NOT_EMPTY: return "directory isn't empty";
        case PHYSFS_ERR_OS_ERROR: return "OS reported an error";
        case PHYSFS_ERR_DUPLICATE: return "duplicate resource";
        case PHYSFS_ERR_BAD_PASSWORD: return "bad password";
        case PHYSFS_ERR_APP_CALLBACK: return "app callback reported error";
    } /* switch */

    return NULL;  /* don't know this error code. */
} /* PHYSFS_getErrorByCode */


void PHYSFS_setErrorCode(PHYSFS_ErrorCode errcode)
{
    ErrState *err;

    if (!errcode)
        return;

    err = findErrorForCurrentThread();
    if (err == NULL)
    {
        err = (ErrState *) allocator.Malloc(sizeof (ErrState));
        if (err == NULL)
            return;   /* uhh...? */

        memset(err, '\0', sizeof (ErrState));
        err->tid = __PHYSFS_platformGetThreadID();

        if (errorLock != NULL)
            __PHYSFS_platformGrabMutex(errorLock);

        err->next = errorStates;
        errorStates = err;

        if (errorLock != NULL)
            __PHYSFS_platformReleaseMutex(errorLock);
    } /* if */

    err->code = errcode;
} /* PHYSFS_setErrorCode */


const char *PHYSFS_getLastError(void)
{
    const PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
    return (err) ? PHYSFS_getErrorByCode(err) : NULL;
} /* PHYSFS_getLastError */


/* MAKE SURE that errorLock is held before calling this! */
static void freeErrorStates(void)
{
    ErrState *i;
    ErrState *next;

    for (i = errorStates; i != NULL; i = next)
    {
        next = i->next;
        allocator.Free(i);
    } /* for */

    errorStates = NULL;
} /* freeErrorStates */


void PHYSFS_getLinkedVersion(PHYSFS_Version *ver)
{
    if (ver != NULL)
    {
        ver->major = PHYSFS_VER_MAJOR;
        ver->minor = PHYSFS_VER_MINOR;
        ver->patch = PHYSFS_VER_PATCH;
    } /* if */
} /* PHYSFS_getLinkedVersion */


static const char *find_filename_extension(const char *fname)
{
    const char *retval = NULL;
    if (fname != NULL)
    {
        const char *p = strchr(fname, '.');
        retval = p;

        while (p != NULL)
        {
            p = strchr(p + 1, '.');
            if (p != NULL)
                retval = p;
        } /* while */

        if (retval != NULL)
            retval++;  /* skip '.' */
    } /* if */

    return retval;
} /* find_filename_extension */


static DirHandle *tryOpenDir(PHYSFS_Io *io, const PHYSFS_Archiver *funcs,
                             const char *d, int forWriting, int *_claimed)
{
    DirHandle *retval = NULL;
    void *opaque = NULL;

    if (io != NULL)
        BAIL_IF_ERRPASS(!io->seek(io, 0), NULL);

    opaque = funcs->openArchive(io, d, forWriting, _claimed);
    if (opaque != NULL)
    {
        retval = (DirHandle *) allocator.Malloc(sizeof (DirHandle));
        if (retval == NULL)
            funcs->closeArchive(opaque);
        else
        {
            memset(retval, '\0', sizeof (DirHandle));
            retval->mountPoint = NULL;
            retval->funcs = funcs;
            retval->opaque = opaque;
        } /* else */
    } /* if */

    return retval;
} /* tryOpenDir */


static DirHandle *openDirectory(PHYSFS_Io *io, const char *d, int forWriting)
{
    DirHandle *retval = NULL;
    PHYSFS_Archiver **i;
    const char *ext;
    int created_io = 0;
    int claimed = 0;
    PHYSFS_ErrorCode errcode;

    assert((io != NULL) || (d != NULL));

    if (io == NULL)
    {
        /* file doesn't exist, etc? Just fail out. */
        PHYSFS_Stat statbuf;
        BAIL_IF_ERRPASS(!__PHYSFS_platformStat(d, &statbuf, 1), NULL);

        /* DIR gets first shot (unlike the rest, it doesn't deal with files). */
        if (statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY)
        {
            retval = tryOpenDir(io, &__PHYSFS_Archiver_DIR, d, forWriting, &claimed);
            if (retval || claimed)
                return retval;
        } /* if */

        io = __PHYSFS_createNativeIo(d, forWriting ? 'w' : 'r');
        BAIL_IF_ERRPASS(!io, NULL);
        created_io = 1;
    } /* if */

    ext = find_filename_extension(d);
    if (ext != NULL)
    {
        /* Look for archivers with matching file extensions first... */
        for (i = archivers; (*i != NULL) && (retval == NULL) && !claimed; i++)
        {
            if (PHYSFS_utf8stricmp(ext, (*i)->info.extension) == 0)
                retval = tryOpenDir(io, *i, d, forWriting, &claimed);
        } /* for */

        /* failing an exact file extension match, try all the others... */
        for (i = archivers; (*i != NULL) && (retval == NULL) && !claimed; i++)
        {
            if (PHYSFS_utf8stricmp(ext, (*i)->info.extension) != 0)
                retval = tryOpenDir(io, *i, d, forWriting, &claimed);
        } /* for */
    } /* if */

    else  /* no extension? Try them all. */
    {
        for (i = archivers; (*i != NULL) && (retval == NULL) && !claimed; i++)
            retval = tryOpenDir(io, *i, d, forWriting, &claimed);
    } /* else */

    errcode = currentErrorCode();

    if ((!retval) && (created_io))
        io->destroy(io);

    BAIL_IF(!retval, claimed ? errcode : PHYSFS_ERR_UNSUPPORTED, NULL);
    return retval;
} /* openDirectory */


/*
 * Make a platform-independent path string sane. Doesn't actually check the
 *  file hierarchy, it just cleans up the string.
 *  (dst) must be a buffer at least as big as (src), as this is where the
 *  cleaned up string is deposited.
 * If there are illegal bits in the path (".." entries, etc) then we
 *  return zero and (dst) is undefined. Non-zero if the path was sanitized.
 */
static int sanitizePlatformIndependentPath(const char *src, char *dst)
{
    char *prev;
    char ch;

    while (*src == '/')  /* skip initial '/' chars... */
        src++;

    /* Make sure the entire string isn't "." or ".." */
    if ((strcmp(src, ".") == 0) || (strcmp(src, "..") == 0))
        BAIL(PHYSFS_ERR_BAD_FILENAME, 0);

    prev = dst;
    do
    {
        ch = *(src++);

        if ((ch == ':') || (ch == '\\'))  /* illegal chars in a physfs path. */
            BAIL(PHYSFS_ERR_BAD_FILENAME, 0);

        if (ch == '/')   /* path separator. */
        {
            *dst = '\0';  /* "." and ".." are illegal pathnames. */
            if ((strcmp(prev, ".") == 0) || (strcmp(prev, "..") == 0))
                BAIL(PHYSFS_ERR_BAD_FILENAME, 0);

            while (*src == '/')   /* chop out doubles... */
                src++;

            if (*src == '\0') /* ends with a pathsep? */
                break;  /* we're done, don't add final pathsep to dst. */

            prev = dst + 1;
        } /* if */

        *(dst++) = ch;
    } while (ch != '\0');

    return 1;
} /* sanitizePlatformIndependentPath */


static inline size_t dirHandleRootLen(const DirHandle *h)
{
    return h ? h->rootlen : 0;
} /* dirHandleRootLen */

static inline int sanitizePlatformIndependentPathWithRoot(const DirHandle *h, const char *src, char *dst)
{
    return sanitizePlatformIndependentPath(src, dst + dirHandleRootLen(h));
} /* sanitizePlatformIndependentPathWithRoot */



/*
 * Figure out if (fname) is part of (h)'s mountpoint. (fname) must be an
 *  output from sanitizePlatformIndependentPath(), so that it is in a known
 *  state.
 *
 * This only finds legitimate segments of a mountpoint. If the mountpoint is
 *  "/a/b/c" and (fname) is "/a/b/c", "/", or "/a/b/c/d", then the results are
 *  all zero. "/a/b" will succeed, though.
 */
static int partOfMountPoint(DirHandle *h, char *fname)
{
    int rc;
    size_t len, mntpntlen;

    if (h->mountPoint == NULL)
        return 0;
    else if (*fname == '\0')
        return 1;

    len = strlen(fname);
    mntpntlen = strlen(h->mountPoint);
    if (len > mntpntlen)  /* can't be a subset of mountpoint. */
        return 0;

    /* if true, must be not a match or a complete match, but not a subset. */
    if ((len + 1) == mntpntlen)
        return 0;

    rc = strncmp(fname, h->mountPoint, len); /* !!! FIXME: case insensitive? */
    if (rc != 0)
        return 0;  /* not a match. */

    /* make sure /a/b matches /a/b/ and not /a/bc ... */
    return h->mountPoint[len] == '/';
} /* partOfMountPoint */


static DirHandle *createDirHandle(PHYSFS_Io *io, const char *newDir,
                                  const char *mountPoint, int forWriting)
{
    DirHandle *dirHandle = NULL;
    char *tmpmntpnt = NULL;

    assert(newDir != NULL);  /* should have caught this higher up. */

    if (mountPoint != NULL)
    {
        const size_t len = strlen(mountPoint) + 1;
        tmpmntpnt = (char *) __PHYSFS_smallAlloc(len);
        GOTO_IF(!tmpmntpnt, PHYSFS_ERR_OUT_OF_MEMORY, badDirHandle);
        if (!sanitizePlatformIndependentPath(mountPoint, tmpmntpnt))
            goto badDirHandle;
        mountPoint = tmpmntpnt;  /* sanitized version. */
    } /* if */

    dirHandle = openDirectory(io, newDir, forWriting);
    GOTO_IF_ERRPASS(!dirHandle, badDirHandle);

    dirHandle->dirName = (char *) allocator.Malloc(strlen(newDir) + 1);
    GOTO_IF(!dirHandle->dirName, PHYSFS_ERR_OUT_OF_MEMORY, badDirHandle);
    strcpy(dirHandle->dirName, newDir);

    if ((mountPoint != NULL) && (*mountPoint != '\0'))
    {
        dirHandle->mountPoint = (char *)allocator.Malloc(strlen(mountPoint)+2);
        if (!dirHandle->mountPoint)
            GOTO(PHYSFS_ERR_OUT_OF_MEMORY, badDirHandle);
        strcpy(dirHandle->mountPoint, mountPoint);
        strcat(dirHandle->mountPoint, "/");
    } /* if */

    __PHYSFS_smallFree(tmpmntpnt);
    return dirHandle;

badDirHandle:
    if (dirHandle != NULL)
    {
        dirHandle->funcs->closeArchive(dirHandle->opaque);
        allocator.Free(dirHandle->dirName);
        allocator.Free(dirHandle->mountPoint);
        allocator.Free(dirHandle);
    } /* if */

    __PHYSFS_smallFree(tmpmntpnt);
    return NULL;
} /* createDirHandle */


/* MAKE SURE you've got the stateLock held before calling this! */
static int freeDirHandle(DirHandle *dh, FileHandle *openList)
{
    FileHandle *i;

    if (dh == NULL)
        return 1;

    for (i = openList; i != NULL; i = i->next)
        BAIL_IF(i->dirHandle == dh, PHYSFS_ERR_FILES_STILL_OPEN, 0);

    dh->funcs->closeArchive(dh->opaque);
    allocator.Free(dh->dirName);
    allocator.Free(dh->mountPoint);
    allocator.Free(dh);
    return 1;
} /* freeDirHandle */


static char *calculateBaseDir(const char *argv0)
{
    const char dirsep = __PHYSFS_platformDirSeparator;
    char *retval = NULL;
    char *ptr = NULL;

    /* Give the platform layer first shot at this. */
    retval = __PHYSFS_platformCalcBaseDir(argv0);
    if (retval != NULL)
        return retval;

    /* We need argv0 to go on. */
    BAIL_IF(argv0 == NULL, PHYSFS_ERR_ARGV0_IS_NULL, NULL);

    ptr = strrchr(argv0, dirsep);
    if (ptr != NULL)
    {
        const size_t size = ((size_t) (ptr - argv0)) + 1;
        retval = (char *) allocator.Malloc(size + 1);
        BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        memcpy(retval, argv0, size);
        retval[size] = '\0';
        return retval;
    } /* if */

    /* argv0 wasn't helpful. */
    BAIL(PHYSFS_ERR_INVALID_ARGUMENT, NULL);
} /* calculateBaseDir */


static int initializeMutexes(void)
{
    errorLock = __PHYSFS_platformCreateMutex();
    if (errorLock == NULL)
        goto initializeMutexes_failed;

    stateLock = __PHYSFS_platformCreateMutex();
    if (stateLock == NULL)
        goto initializeMutexes_failed;

    return 1;  /* success. */

initializeMutexes_failed:
    if (errorLock != NULL)
        __PHYSFS_platformDestroyMutex(errorLock);

    if (stateLock != NULL)
        __PHYSFS_platformDestroyMutex(stateLock);

    errorLock = stateLock = NULL;
    return 0;  /* failed. */
} /* initializeMutexes */


static int doRegisterArchiver(const PHYSFS_Archiver *_archiver);

static int initStaticArchivers(void)
{
    #define REGISTER_STATIC_ARCHIVER(arc) { \
        if (!doRegisterArchiver(&__PHYSFS_Archiver_##arc)) { \
            return 0; \
        } \
    }

    #if PHYSFS_SUPPORTS_ZIP
        REGISTER_STATIC_ARCHIVER(ZIP);
    #endif
    #if PHYSFS_SUPPORTS_7Z
        SZIP_global_init();
        REGISTER_STATIC_ARCHIVER(7Z);
    #endif
    #if PHYSFS_SUPPORTS_GRP
        REGISTER_STATIC_ARCHIVER(GRP);
    #endif
    #if PHYSFS_SUPPORTS_QPAK
        REGISTER_STATIC_ARCHIVER(QPAK);
    #endif
    #if PHYSFS_SUPPORTS_HOG
        REGISTER_STATIC_ARCHIVER(HOG);
    #endif
    #if PHYSFS_SUPPORTS_MVL
        REGISTER_STATIC_ARCHIVER(MVL);
    #endif
    #if PHYSFS_SUPPORTS_WAD
        REGISTER_STATIC_ARCHIVER(WAD);
    #endif
    #if PHYSFS_SUPPORTS_SLB
        REGISTER_STATIC_ARCHIVER(SLB);
    #endif
    #if PHYSFS_SUPPORTS_ISO9660
        REGISTER_STATIC_ARCHIVER(ISO9660);
    #endif
    #if PHYSFS_SUPPORTS_VDF
        REGISTER_STATIC_ARCHIVER(VDF)
    #endif

    #undef REGISTER_STATIC_ARCHIVER

    return 1;
} /* initStaticArchivers */


static void setDefaultAllocator(void);
static int doDeinit(void);

int PHYSFS_init(const char *argv0)
{
    BAIL_IF(initialized, PHYSFS_ERR_IS_INITIALIZED, 0);

    if (!externalAllocator)
        setDefaultAllocator();

    if ((allocator.Init != NULL) && (!allocator.Init())) return 0;

    if (!__PHYSFS_platformInit())
    {
        if (allocator.Deinit != NULL) allocator.Deinit();
        return 0;
    } /* if */

    /* everything below here can be cleaned up safely by doDeinit(). */

    if (!initializeMutexes()) goto initFailed;

    baseDir = calculateBaseDir(argv0);
    if (!baseDir) goto initFailed;

    userDir = __PHYSFS_platformCalcUserDir();
    if (!userDir) goto initFailed;

    /* Platform layer is required to append a dirsep. */
    assert(baseDir[strlen(baseDir) - 1] == __PHYSFS_platformDirSeparator);
    assert(userDir[strlen(userDir) - 1] == __PHYSFS_platformDirSeparator);

    if (!initStaticArchivers()) goto initFailed;

    initialized = 1;

    /* This makes sure that the error subsystem is initialized. */
    PHYSFS_setErrorCode(PHYSFS_getLastErrorCode());

    return 1;

initFailed:
    doDeinit();
    return 0;
} /* PHYSFS_init */


/* MAKE SURE you hold stateLock before calling this! */
static int closeFileHandleList(FileHandle **list)
{
    FileHandle *i;
    FileHandle *next = NULL;

    for (i = *list; i != NULL; i = next)
    {
        PHYSFS_Io *io = i->io;
        next = i->next;

        if (io->flush && !io->flush(io))
        {
            *list = i;
            return 0;
        } /* if */

        io->destroy(io);
        allocator.Free(i);
    } /* for */

    *list = NULL;
    return 1;
} /* closeFileHandleList */


/* MAKE SURE you hold the stateLock before calling this! */
static void freeSearchPath(void)
{
    DirHandle *i;
    DirHandle *next = NULL;

    closeFileHandleList(&openReadList);

    if (searchPath != NULL)
    {
        for (i = searchPath; i != NULL; i = next)
        {
            next = i->next;
            freeDirHandle(i, openReadList);
        } /* for */
        searchPath = NULL;
    } /* if */
} /* freeSearchPath */


/* MAKE SURE you hold stateLock before calling this! */
static int archiverInUse(const PHYSFS_Archiver *arc, const DirHandle *list)
{
    const DirHandle *i;
    for (i = list; i != NULL; i = i->next)
    {
        if (i->funcs == arc)
            return 1;
    } /* for */

    return 0;  /* not in use */
} /* archiverInUse */


/* MAKE SURE you hold stateLock before calling this! */
static int doDeregisterArchiver(const size_t idx)
{
    const size_t len = (numArchivers - idx) * sizeof (void *);
    PHYSFS_ArchiveInfo *info = archiveInfo[idx];
    PHYSFS_Archiver *arc = archivers[idx];

    /* make sure nothing is still using this archiver */
    if (archiverInUse(arc, searchPath) || archiverInUse(arc, writeDir))
        BAIL(PHYSFS_ERR_FILES_STILL_OPEN, 0);

    allocator.Free((void *) info->extension);
    allocator.Free((void *) info->description);
    allocator.Free((void *) info->author);
    allocator.Free((void *) info->url);
    allocator.Free((void *) arc);

    memmove(&archiveInfo[idx], &archiveInfo[idx+1], len);
    memmove(&archivers[idx], &archivers[idx+1], len);

    assert(numArchivers > 0);
    numArchivers--;

    return 1;
} /* doDeregisterArchiver */


/* Does NOT hold the state lock; we're shutting down. */
static void freeArchivers(void)
{
    while (numArchivers > 0)
    {
        if (!doDeregisterArchiver(numArchivers - 1))
            assert(!"nothing should be mounted during shutdown.");
    } /* while */

    allocator.Free(archivers);
    allocator.Free(archiveInfo);
    archivers = NULL;
    archiveInfo = NULL;
} /* freeArchivers */


static int doDeinit(void)
{
    closeFileHandleList(&openWriteList);
    BAIL_IF(!PHYSFS_setWriteDir(NULL), PHYSFS_ERR_FILES_STILL_OPEN, 0);

    freeSearchPath();
    freeArchivers();
    freeErrorStates();

    if (baseDir != NULL)
    {
        allocator.Free(baseDir);
        baseDir = NULL;
    } /* if */

    if (userDir != NULL)
    {
        allocator.Free(userDir);
        userDir = NULL;
    } /* if */

    if (prefDir != NULL)
    {
        allocator.Free(prefDir);
        prefDir = NULL;
    } /* if */

    if (archiveInfo != NULL)
    {
        allocator.Free(archiveInfo);
        archiveInfo = NULL;
    } /* if */

    if (archivers != NULL)
    {
        allocator.Free(archivers);
        archivers = NULL;
    } /* if */

    longest_root = 0;
    allowSymLinks = 0;
    initialized = 0;

    if (errorLock) __PHYSFS_platformDestroyMutex(errorLock);
    if (stateLock) __PHYSFS_platformDestroyMutex(stateLock);

    if (allocator.Deinit != NULL)
        allocator.Deinit();

    errorLock = stateLock = NULL;

    __PHYSFS_platformDeinit();

    return 1;
} /* doDeinit */


int PHYSFS_deinit(void)
{
    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, 0);
    return doDeinit();
} /* PHYSFS_deinit */


int PHYSFS_isInit(void)
{
    return initialized;
} /* PHYSFS_isInit */


char *__PHYSFS_strdup(const char *str)
{
    char *retval = (char *) allocator.Malloc(strlen(str) + 1);
    if (retval)
        strcpy(retval, str);
    return retval;
} /* __PHYSFS_strdup */


PHYSFS_uint32 __PHYSFS_hashString(const char *str, size_t len)
{
    PHYSFS_uint32 hash = 5381;
    while (len--)
        hash = ((hash << 5) + hash) ^ *(str++);
    return hash;
} /* __PHYSFS_hashString */


/* MAKE SURE you hold stateLock before calling this! */
static int doRegisterArchiver(const PHYSFS_Archiver *_archiver)
{
    const PHYSFS_uint32 maxver = CURRENT_PHYSFS_ARCHIVER_API_VERSION;
    const size_t len = (numArchivers + 2) * sizeof (void *);
    PHYSFS_Archiver *archiver = NULL;
    PHYSFS_ArchiveInfo *info = NULL;
    const char *ext = NULL;
    void *ptr = NULL;
    size_t i;

    BAIL_IF(!_archiver, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(_archiver->version > maxver, PHYSFS_ERR_UNSUPPORTED, 0);
    BAIL_IF(!_archiver->info.extension, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->info.description, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->info.author, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->info.url, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->openArchive, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->enumerate, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->openRead, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->openWrite, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->openAppend, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->remove, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->mkdir, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->closeArchive, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!_archiver->stat, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    ext = _archiver->info.extension;
    for (i = 0; i < numArchivers; i++)
    {
        if (PHYSFS_utf8stricmp(archiveInfo[i]->extension, ext) == 0)
            BAIL(PHYSFS_ERR_DUPLICATE, 0);
    } /* for */

    /* make a copy of the data. */
    archiver = (PHYSFS_Archiver *) allocator.Malloc(sizeof (*archiver));
    GOTO_IF(!archiver, PHYSFS_ERR_OUT_OF_MEMORY, regfailed);

    /* Must copy sizeof (OLD_VERSION_OF_STRUCT) when version changes! */
    memcpy(archiver, _archiver, sizeof (*archiver));

    info = (PHYSFS_ArchiveInfo *) &archiver->info;
    memset(info, '\0', sizeof (*info));  /* NULL in case an alloc fails. */
    #define CPYSTR(item) \
        info->item = __PHYSFS_strdup(_archiver->info.item); \
        GOTO_IF(!info->item, PHYSFS_ERR_OUT_OF_MEMORY, regfailed);
    CPYSTR(extension);
    CPYSTR(description);
    CPYSTR(author);
    CPYSTR(url);
    info->supportsSymlinks = _archiver->info.supportsSymlinks;
    #undef CPYSTR

    ptr = allocator.Realloc(archiveInfo, len);
    GOTO_IF(!ptr, PHYSFS_ERR_OUT_OF_MEMORY, regfailed);
    archiveInfo = (PHYSFS_ArchiveInfo **) ptr;

    ptr = allocator.Realloc(archivers, len);
    GOTO_IF(!ptr, PHYSFS_ERR_OUT_OF_MEMORY, regfailed);
    archivers = (PHYSFS_Archiver **) ptr;

    archiveInfo[numArchivers] = info;
    archiveInfo[numArchivers + 1] = NULL;

    archivers[numArchivers] = archiver;
    archivers[numArchivers + 1] = NULL;

    numArchivers++;

    return 1;

regfailed:
    if (info != NULL)
    {
        allocator.Free((void *) info->extension);
        allocator.Free((void *) info->description);
        allocator.Free((void *) info->author);
        allocator.Free((void *) info->url);
    } /* if */
    allocator.Free(archiver);

    return 0;
} /* doRegisterArchiver */


int PHYSFS_registerArchiver(const PHYSFS_Archiver *archiver)
{
    int retval;
    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, 0);
    __PHYSFS_platformGrabMutex(stateLock);
    retval = doRegisterArchiver(archiver);
    __PHYSFS_platformReleaseMutex(stateLock);
    return retval;
} /* PHYSFS_registerArchiver */


int PHYSFS_deregisterArchiver(const char *ext)
{
    size_t i;

    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, 0);
    BAIL_IF(!ext, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);
    for (i = 0; i < numArchivers; i++)
    {
        if (PHYSFS_utf8stricmp(archiveInfo[i]->extension, ext) == 0)
        {
            const int retval = doDeregisterArchiver(i);
            __PHYSFS_platformReleaseMutex(stateLock);
            return retval;
        } /* if */
    } /* for */
    __PHYSFS_platformReleaseMutex(stateLock);

    BAIL(PHYSFS_ERR_NOT_FOUND, 0);
} /* PHYSFS_deregisterArchiver */


const PHYSFS_ArchiveInfo **PHYSFS_supportedArchiveTypes(void)
{
    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, NULL);
    return (const PHYSFS_ArchiveInfo **) archiveInfo;
} /* PHYSFS_supportedArchiveTypes */


void PHYSFS_freeList(void *list)
{
    void **i;
    if (list != NULL)
    {
        for (i = (void **) list; *i != NULL; i++)
            allocator.Free(*i);

        allocator.Free(list);
    } /* if */
} /* PHYSFS_freeList */


const char *PHYSFS_getDirSeparator(void)
{
    static char retval[2] = { __PHYSFS_platformDirSeparator, '\0' };
    return retval;
} /* PHYSFS_getDirSeparator */


char **PHYSFS_getCdRomDirs(void)
{
    return doEnumStringList(__PHYSFS_platformDetectAvailableCDs);
} /* PHYSFS_getCdRomDirs */


void PHYSFS_getCdRomDirsCallback(PHYSFS_StringCallback callback, void *data)
{
    __PHYSFS_platformDetectAvailableCDs(callback, data);
} /* PHYSFS_getCdRomDirsCallback */


const char *PHYSFS_getPrefDir(const char *org, const char *app)
{
    const char dirsep = __PHYSFS_platformDirSeparator;
    PHYSFS_Stat statbuf;
    char *ptr = NULL;
    char *endstr = NULL;

    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, 0);
    BAIL_IF(!org, PHYSFS_ERR_INVALID_ARGUMENT, NULL);
    BAIL_IF(*org == '\0', PHYSFS_ERR_INVALID_ARGUMENT, NULL);
    BAIL_IF(!app, PHYSFS_ERR_INVALID_ARGUMENT, NULL);
    BAIL_IF(*app == '\0', PHYSFS_ERR_INVALID_ARGUMENT, NULL);

    allocator.Free(prefDir);
    prefDir = __PHYSFS_platformCalcPrefDir(org, app);
    BAIL_IF_ERRPASS(!prefDir, NULL);

    assert(strlen(prefDir) > 0);
    endstr = prefDir + (strlen(prefDir) - 1);
    assert(*endstr == dirsep);
    *endstr = '\0';  /* mask out the final dirsep for now. */

    if (!__PHYSFS_platformStat(prefDir, &statbuf, 1))
    {
        for (ptr = strchr(prefDir, dirsep); ptr; ptr = strchr(ptr+1, dirsep))
        {
            *ptr = '\0';
            __PHYSFS_platformMkDir(prefDir);
            *ptr = dirsep;
        } /* for */

        if (!__PHYSFS_platformMkDir(prefDir))
        {
            allocator.Free(prefDir);
            prefDir = NULL;
        } /* if */
    } /* if */

    *endstr = dirsep;  /* readd the final dirsep. */

    return prefDir;
} /* PHYSFS_getPrefDir */


const char *PHYSFS_getBaseDir(void)
{
    return baseDir;   /* this is calculated in PHYSFS_init()... */
} /* PHYSFS_getBaseDir */


const char *__PHYSFS_getUserDir(void)  /* not deprecated internal version. */
{
    return userDir;   /* this is calculated in PHYSFS_init()... */
} /* __PHYSFS_getUserDir */


const char *PHYSFS_getUserDir(void)
{
    return __PHYSFS_getUserDir();
} /* PHYSFS_getUserDir */


const char *PHYSFS_getWriteDir(void)
{
    const char *retval = NULL;

    __PHYSFS_platformGrabMutex(stateLock);
    if (writeDir != NULL)
        retval = writeDir->dirName;
    __PHYSFS_platformReleaseMutex(stateLock);

    return retval;
} /* PHYSFS_getWriteDir */


int PHYSFS_setWriteDir(const char *newDir)
{
    int retval = 1;

    __PHYSFS_platformGrabMutex(stateLock);

    if (writeDir != NULL)
    {
        BAIL_IF_MUTEX_ERRPASS(!freeDirHandle(writeDir, openWriteList),
                            stateLock, 0);
        writeDir = NULL;
    } /* if */

    if (newDir != NULL)
    {
        writeDir = createDirHandle(NULL, newDir, NULL, 1);
        retval = (writeDir != NULL);
    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);

    return retval;
} /* PHYSFS_setWriteDir */


int PHYSFS_setRoot(const char *archive, const char *subdir)
{
    DirHandle *i;

    BAIL_IF(!archive, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);

    for (i = searchPath; i != NULL; i = i->next)
    {
        if ((i->dirName != NULL) && (strcmp(archive, i->dirName) == 0))
        {
            if (!subdir || (strcmp(subdir, "/") == 0))
            {
                if (i->root)
                    allocator.Free(i->root);
                i->root = NULL;
                i->rootlen = 0;
            } /* if */
            else
            {
                const size_t len = strlen(subdir) + 1;
                char *ptr = (char *) allocator.Malloc(len);
                BAIL_IF_MUTEX(!ptr, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);
                if (!sanitizePlatformIndependentPath(subdir, ptr))
                {
                    allocator.Free(ptr);
                    BAIL_MUTEX_ERRPASS(stateLock, 0);
                } /* if */

                if (i->root)
                    allocator.Free(i->root);
                i->root = ptr;
                i->rootlen = len;

                if (longest_root < len)
                    longest_root = len;
            } /* else */

            break;
        } /* if */
    } /* for */

    __PHYSFS_platformReleaseMutex(stateLock);
    return 1;
} /* PHYSFS_setRoot */


static int doMount(PHYSFS_Io *io, const char *fname,
                   const char *mountPoint, int appendToPath)
{
    DirHandle *dh;
    DirHandle *prev = NULL;
    DirHandle *i;

    BAIL_IF(!fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    if (mountPoint == NULL)
        mountPoint = "/";

    __PHYSFS_platformGrabMutex(stateLock);

    for (i = searchPath; i != NULL; i = i->next)
    {
        /* already in search path? */
        if ((i->dirName != NULL) && (strcmp(fname, i->dirName) == 0))
            BAIL_MUTEX_ERRPASS(stateLock, 1);
        prev = i;
    } /* for */

    dh = createDirHandle(io, fname, mountPoint, 0);
    BAIL_IF_MUTEX_ERRPASS(!dh, stateLock, 0);

    if (appendToPath)
    {
        if (prev == NULL)
            searchPath = dh;
        else
            prev->next = dh;
    } /* if */
    else
    {
        dh->next = searchPath;
        searchPath = dh;
    } /* else */

    __PHYSFS_platformReleaseMutex(stateLock);
    return 1;
} /* doMount */


int PHYSFS_mountIo(PHYSFS_Io *io, const char *fname,
                   const char *mountPoint, int appendToPath)
{
    BAIL_IF(!io, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(io->version != 0, PHYSFS_ERR_UNSUPPORTED, 0);
    return doMount(io, fname, mountPoint, appendToPath);
} /* PHYSFS_mountIo */


int PHYSFS_mountMemory(const void *buf, PHYSFS_uint64 len, void (*del)(void *),
                       const char *fname, const char *mountPoint,
                       int appendToPath)
{
    int retval = 0;
    PHYSFS_Io *io = NULL;

    BAIL_IF(!buf, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    io = __PHYSFS_createMemoryIo(buf, len, del);
    BAIL_IF_ERRPASS(!io, 0);
    retval = doMount(io, fname, mountPoint, appendToPath);
    if (!retval)
    {
        /* docs say not to call (del) in case of failure, so cheat. */
        MemoryIoInfo *info = (MemoryIoInfo *) io->opaque;
        info->destruct = NULL;
        io->destroy(io);
    } /* if */

    return retval;
} /* PHYSFS_mountMemory */


int PHYSFS_mountHandle(PHYSFS_File *file, const char *fname,
                       const char *mountPoint, int appendToPath)
{
    int retval = 0;
    PHYSFS_Io *io = NULL;

    BAIL_IF(!file, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    io = __PHYSFS_createHandleIo(file);
    BAIL_IF_ERRPASS(!io, 0);
    retval = doMount(io, fname, mountPoint, appendToPath);
    if (!retval)
    {
        /* docs say not to destruct in case of failure, so cheat. */
        io->opaque = NULL;
        io->destroy(io);
    } /* if */

    return retval;
} /* PHYSFS_mountHandle */


int PHYSFS_mount(const char *newDir, const char *mountPoint, int appendToPath)
{
    BAIL_IF(!newDir, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    return doMount(NULL, newDir, mountPoint, appendToPath);
} /* PHYSFS_mount */


int PHYSFS_addToSearchPath(const char *newDir, int appendToPath)
{
    return PHYSFS_mount(newDir, NULL, appendToPath);
} /* PHYSFS_addToSearchPath */


int PHYSFS_removeFromSearchPath(const char *oldDir)
{
    return PHYSFS_unmount(oldDir);
} /* PHYSFS_removeFromSearchPath */


int PHYSFS_unmount(const char *oldDir)
{
    DirHandle *i;
    DirHandle *prev = NULL;
    DirHandle *next = NULL;

    BAIL_IF(oldDir == NULL, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);
    for (i = searchPath; i != NULL; i = i->next)
    {
        if (strcmp(i->dirName, oldDir) == 0)
        {
            next = i->next;
            BAIL_IF_MUTEX_ERRPASS(!freeDirHandle(i, openReadList),
                                stateLock, 0);

            if (prev == NULL)
                searchPath = next;
            else
                prev->next = next;

            BAIL_MUTEX_ERRPASS(stateLock, 1);
        } /* if */
        prev = i;
    } /* for */

    BAIL_MUTEX(PHYSFS_ERR_NOT_MOUNTED, stateLock, 0);
} /* PHYSFS_unmount */


char **PHYSFS_getSearchPath(void)
{
    return doEnumStringList(PHYSFS_getSearchPathCallback);
} /* PHYSFS_getSearchPath */


const char *PHYSFS_getMountPoint(const char *dir)
{
    DirHandle *i;
    __PHYSFS_platformGrabMutex(stateLock);
    for (i = searchPath; i != NULL; i = i->next)
    {
        if (strcmp(i->dirName, dir) == 0)
        {
            const char *retval = ((i->mountPoint) ? i->mountPoint : "/");
            __PHYSFS_platformReleaseMutex(stateLock);
            return retval;
        } /* if */
    } /* for */
    __PHYSFS_platformReleaseMutex(stateLock);

    BAIL(PHYSFS_ERR_NOT_MOUNTED, NULL);
} /* PHYSFS_getMountPoint */


void PHYSFS_getSearchPathCallback(PHYSFS_StringCallback callback, void *data)
{
    DirHandle *i;

    __PHYSFS_platformGrabMutex(stateLock);

    for (i = searchPath; i != NULL; i = i->next)
        callback(data, i->dirName);

    __PHYSFS_platformReleaseMutex(stateLock);
} /* PHYSFS_getSearchPathCallback */


typedef struct setSaneCfgEnumData
{
    const char *archiveExt;
    size_t archiveExtLen;
    int archivesFirst;
    PHYSFS_ErrorCode errcode;
} setSaneCfgEnumData;

static PHYSFS_EnumerateCallbackResult setSaneCfgEnumCallback(void *_data,
                                                const char *dir, const char *f)
{
    setSaneCfgEnumData *data = (setSaneCfgEnumData *) _data;
    const size_t extlen = data->archiveExtLen;
    const size_t l = strlen(f);
    const char *ext;

    if ((l > extlen) && (f[l - extlen - 1] == '.'))
    {
        ext = f + (l - extlen);
        if (PHYSFS_utf8stricmp(ext, data->archiveExt) == 0)
        {
            const char dirsep = __PHYSFS_platformDirSeparator;
            const char *d = PHYSFS_getRealDir(f);
            const size_t allocsize = strlen(d) + l + 2;
            char *str = (char *) __PHYSFS_smallAlloc(allocsize);
            if (str == NULL)
                data->errcode = PHYSFS_ERR_OUT_OF_MEMORY;
            else
            {
                snprintf(str, allocsize, "%s%c%s", d, dirsep, f);
                if (!PHYSFS_mount(str, NULL, data->archivesFirst == 0))
                    data->errcode = currentErrorCode();
                __PHYSFS_smallFree(str);
            } /* else */
        } /* if */
    } /* if */

    /* !!! FIXME: if we want to abort on errors... */
    /*return (data->errcode != PHYSFS_ERR_OK) ? PHYSFS_ENUM_ERROR : PHYSFS_ENUM_OK;*/

    return PHYSFS_ENUM_OK;  /* keep going */
} /* setSaneCfgEnumCallback */


int PHYSFS_setSaneConfig(const char *organization, const char *appName,
                         const char *archiveExt, int includeCdRoms,
                         int archivesFirst)
{
    const char *basedir;
    const char *prefdir;

    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, 0);

    prefdir = PHYSFS_getPrefDir(organization, appName);
    BAIL_IF_ERRPASS(!prefdir, 0);

    basedir = PHYSFS_getBaseDir();
    BAIL_IF_ERRPASS(!basedir, 0);

    BAIL_IF(!PHYSFS_setWriteDir(prefdir), PHYSFS_ERR_NO_WRITE_DIR, 0);

    /* !!! FIXME: these can fail and we should report that... */

    /* Put write dir first in search path... */
    PHYSFS_mount(prefdir, NULL, 0);

    /* Put base path on search path... */
    PHYSFS_mount(basedir, NULL, 1);

    /* handle CD-ROMs... */
    if (includeCdRoms)
    {
        char **cds = PHYSFS_getCdRomDirs();
        char **i;
        for (i = cds; *i != NULL; i++)
            PHYSFS_mount(*i, NULL, 1);
        PHYSFS_freeList(cds);
    } /* if */

    /* Root out archives, and add them to search path... */
    if (archiveExt != NULL)
    {
        setSaneCfgEnumData data;
        memset(&data, '\0', sizeof (data));
        data.archiveExt = archiveExt;
        data.archiveExtLen = strlen(archiveExt);
        data.archivesFirst = archivesFirst;
        data.errcode = PHYSFS_ERR_OK;
        if (!PHYSFS_enumerate("/", setSaneCfgEnumCallback, &data))
        {
            /* !!! FIXME: use this if we're reporting errors.
            PHYSFS_ErrorCode errcode = currentErrorCode();
            if (errcode == PHYSFS_ERR_APP_CALLBACK)
                errcode = data->errcode; */
        } /* if */
    } /* if */

    return 1;
} /* PHYSFS_setSaneConfig */


void PHYSFS_permitSymbolicLinks(int allow)
{
    allowSymLinks = allow;
} /* PHYSFS_permitSymbolicLinks */


int PHYSFS_symbolicLinksPermitted(void)
{
    return allowSymLinks;
} /* PHYSFS_symbolicLinksPermitted */


/*
 * Verify that (fname) (in platform-independent notation), in relation
 *  to (h) is secure. That means that each element of fname is checked
 *  for symlinks (if they aren't permitted). This also allows for quick
 *  rejection of files that exist outside an archive's mountpoint.
 *
 * With some exceptions (like PHYSFS_mkdir(), which builds multiple subdirs
 *  at a time), you should always pass zero for "allowMissing" for efficiency.
 *
 * (fname) must point to an output from sanitizePlatformIndependentPath(),
 *  since it will make sure that path names are in the right format for
 *  passing certain checks. It will also do checks for "insecure" pathnames
 *  like ".." which should be done once instead of once per archive. This also
 *  gives us license to treat (fname) as scratch space in this function.
 *
 * (fname)'s buffer must have enough space available before it for this
 *  function to prepend any root directory for this DirHandle.
 *
 * Returns non-zero if string is safe, zero if there's a security issue.
 *  PHYSFS_getLastError() will specify what was wrong. (*fname) will be
 *  updated to point past any mount point elements so it is prepared to
 *  be used with the archiver directly.
 */
static int verifyPath(DirHandle *h, char **_fname, int allowMissing)
{
    char *fname = *_fname;
    int retval = 1;
    char *start;
    char *end;

    if ((*fname == '\0') && (!h->root))  /* quick rejection. */
        return 1;

    /* !!! FIXME: This codeblock sucks. */
    if (h->mountPoint != NULL)  /* NULL mountpoint means "/". */
    {
        size_t mntpntlen = strlen(h->mountPoint);
        size_t len = strlen(fname);
        assert(mntpntlen > 1); /* root mount points should be NULL. */
        /* not under the mountpoint, so skip this archive. */
        BAIL_IF(len < mntpntlen-1, PHYSFS_ERR_NOT_FOUND, 0);
        /* !!! FIXME: Case insensitive? */
        retval = strncmp(h->mountPoint, fname, mntpntlen-1);
        BAIL_IF(retval != 0, PHYSFS_ERR_NOT_FOUND, 0);
        if (len > mntpntlen-1)  /* corner case... */
            BAIL_IF(fname[mntpntlen-1]!='/', PHYSFS_ERR_NOT_FOUND, 0);
        fname += mntpntlen-1;  /* move to start of actual archive path. */
        if (*fname == '/')
            fname++;
        *_fname = fname;  /* skip mountpoint for later use. */
        retval = 1;  /* may be reset, below. */
    } /* if */

    /* prepend the root directory, if any. */
    if (h->root)
    {
        const int isempty = (*fname == '\0');
        fname -= h->rootlen - 1;
        strcpy(fname, h->root);
        if (!isempty)
            fname[h->rootlen - 2] = '/';
        *_fname = fname;
    } /* if */

    start = fname;
    if (!allowSymLinks)
    {
        while (1)
        {
            PHYSFS_Stat statbuf;
            int rc = 0;
            end = strchr(start, '/');

            if (end != NULL) *end = '\0';
            rc = h->funcs->stat(h->opaque, fname, &statbuf);
            if (rc)
                rc = (statbuf.filetype == PHYSFS_FILETYPE_SYMLINK);
            else if (currentErrorCode() == PHYSFS_ERR_NOT_FOUND)
                retval = 0;

            if (end != NULL) *end = '/';

            /* insecure path (has a disallowed symlink in it)? */
            BAIL_IF(rc, PHYSFS_ERR_SYMLINK_FORBIDDEN, 0);

            /* break out early if path element is missing. */
            if (!retval)
            {
                /*
                 * We need to clear it if it's the last element of the path,
                 *  since this might be a non-existant file we're opening
                 *  for writing...
                 */
                if ((end == NULL) || (allowMissing))
                    retval = 1;
                break;
            } /* if */

            if (end == NULL)
                break;

            start = end + 1;
        } /* while */
    } /* if */

    return retval;
} /* verifyPath */


/* This must hold the stateLock before calling. */
static int doMkdir(const char *_dname, char *dname)
{
    DirHandle *h = writeDir;
    char *start;
    char *end;
    int retval = 0;
    int exists = 1;  /* force existance check on first path element. */

    assert(h != NULL);

    BAIL_IF_ERRPASS(!sanitizePlatformIndependentPathWithRoot(h, _dname, dname), 0);
    BAIL_IF_ERRPASS(!verifyPath(h, &dname, 1), 0);

    start = dname;
    while (1)
    {
        end = strchr(start, '/');
        if (end != NULL)
            *end = '\0';

        /* only check for existance if all parent dirs existed, too... */
        if (exists)
        {
            PHYSFS_Stat statbuf;
            const int rc = h->funcs->stat(h->opaque, dname, &statbuf);
            if ((!rc) && (currentErrorCode() == PHYSFS_ERR_NOT_FOUND))
                exists = 0;
            retval = ((rc) && (statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY));
        } /* if */

        if (!exists)
            retval = h->funcs->mkdir(h->opaque, dname);

        if (!retval)
            break;

        if (end == NULL)
            break;

        *end = '/';
        start = end + 1;
    } /* while */

    return retval;
} /* doMkdir */


int PHYSFS_mkdir(const char *_dname)
{
    int retval = 0;
    char *dname;
    size_t len;

    BAIL_IF(!_dname, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);
    BAIL_IF_MUTEX(!writeDir, PHYSFS_ERR_NO_WRITE_DIR, stateLock, 0);
    len = strlen(_dname) + dirHandleRootLen(writeDir) + 1;
    dname = (char *) __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!dname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);
    retval = doMkdir(_dname, dname);
    __PHYSFS_platformReleaseMutex(stateLock);
    __PHYSFS_smallFree(dname);
    return retval;
} /* PHYSFS_mkdir */


/* This must hold the stateLock before calling. */
static int doDelete(const char *_fname, char *fname)
{
    DirHandle *h = writeDir;
    BAIL_IF_ERRPASS(!sanitizePlatformIndependentPathWithRoot(h, _fname, fname), 0);
    BAIL_IF_ERRPASS(!verifyPath(h, &fname, 0), 0);
    return h->funcs->remove(h->opaque, fname);
} /* doDelete */


int PHYSFS_delete(const char *_fname)
{
    int retval;
    char *fname;
    size_t len;

    __PHYSFS_platformGrabMutex(stateLock);
    BAIL_IF_MUTEX(!writeDir, PHYSFS_ERR_NO_WRITE_DIR, stateLock, 0);
    len = strlen(_fname) + dirHandleRootLen(writeDir) + 1;
    fname = (char *) __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!fname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);
    retval = doDelete(_fname, fname);
    __PHYSFS_platformReleaseMutex(stateLock);
    __PHYSFS_smallFree(fname);
    return retval;
} /* PHYSFS_delete */


static DirHandle *getRealDirHandle(const char *_fname)
{
    DirHandle *retval = NULL;
    char *allocated_fname = NULL;
    char *fname = NULL;
    size_t len;

    BAIL_IF(!_fname, PHYSFS_ERR_INVALID_ARGUMENT, NULL);

    __PHYSFS_platformGrabMutex(stateLock);
    len = strlen(_fname) + longest_root + 1;
    allocated_fname = __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!allocated_fname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, NULL);
    fname = allocated_fname + longest_root;
    if (sanitizePlatformIndependentPath(_fname, fname))
    {
        DirHandle *i;
        for (i = searchPath; i != NULL; i = i->next)
        {
            char *arcfname = fname;
            if (partOfMountPoint(i, arcfname))
            {
                retval = i;
                break;
            } /* if */
            else if (verifyPath(i, &arcfname, 0))
            {
                PHYSFS_Stat statbuf;
                if (i->funcs->stat(i->opaque, arcfname, &statbuf))
                {
                    retval = i;
                    break;
                } /* if */
            } /* if */
        } /* for */
    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);
    __PHYSFS_smallFree(allocated_fname);
    return retval;
} /* getRealDirHandle */

const char *PHYSFS_getRealDir(const char *fname)
{
    DirHandle *dh = getRealDirHandle(fname);
    return dh ? dh->dirName : NULL;
} /* PHYSFS_getRealDir */


static int locateInStringList(const char *str,
                              char **list,
                              PHYSFS_uint32 *pos)
{
    PHYSFS_uint32 len = *pos;
    PHYSFS_uint32 half_len;
    PHYSFS_uint32 lo = 0;
    PHYSFS_uint32 middle;
    int cmp;

    while (len > 0)
    {
        half_len = len >> 1;
        middle = lo + half_len;
        cmp = strcmp(list[middle], str);

        if (cmp == 0)  /* it's in the list already. */
            return 1;
        else if (cmp > 0)
            len = half_len;
        else
        {
            lo = middle + 1;
            len -= half_len + 1;
        } /* else */
    } /* while */

    *pos = lo;
    return 0;
} /* locateInStringList */


static PHYSFS_EnumerateCallbackResult enumFilesCallback(void *data,
                                        const char *origdir, const char *str)
{
    PHYSFS_uint32 pos;
    void *ptr;
    char *newstr;
    EnumStringListCallbackData *pecd = (EnumStringListCallbackData *) data;

    /*
     * See if file is in the list already, and if not, insert it in there
     *  alphabetically...
     */
    pos = pecd->size;
    if (locateInStringList(str, pecd->list, &pos))
        return PHYSFS_ENUM_OK;  /* already in the list, but keep going. */

    ptr = allocator.Realloc(pecd->list, (pecd->size + 2) * sizeof (char *));
    newstr = (char *) allocator.Malloc(strlen(str) + 1);
    if (ptr != NULL)
        pecd->list = (char **) ptr;

    if ((ptr == NULL) || (newstr == NULL))
    {
        if (newstr)
            allocator.Free(newstr);

        pecd->errcode = PHYSFS_ERR_OUT_OF_MEMORY;
        return PHYSFS_ENUM_ERROR;  /* better luck next time. */
    } /* if */

    strcpy(newstr, str);

    if (pos != pecd->size)
    {
        memmove(&pecd->list[pos+1], &pecd->list[pos],
                 sizeof (char *) * ((pecd->size) - pos));
    } /* if */

    pecd->list[pos] = newstr;
    pecd->size++;

    return PHYSFS_ENUM_OK;
} /* enumFilesCallback */


char **PHYSFS_enumerateFiles(const char *path)
{
    EnumStringListCallbackData ecd;
    memset(&ecd, '\0', sizeof (ecd));
    ecd.list = (char **) allocator.Malloc(sizeof (char *));
    BAIL_IF(!ecd.list, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    if (!PHYSFS_enumerate(path, enumFilesCallback, &ecd))
    {
        const PHYSFS_ErrorCode errcode = currentErrorCode();
        PHYSFS_uint32 i;
        for (i = 0; i < ecd.size; i++)
            allocator.Free(ecd.list[i]);
        allocator.Free(ecd.list);
        BAIL_IF(errcode == PHYSFS_ERR_APP_CALLBACK, ecd.errcode, NULL);
        return NULL;
    } /* if */

    ecd.list[ecd.size] = NULL;
    return ecd.list;
} /* PHYSFS_enumerateFiles */


/*
 * Broke out to seperate function so we can use stack allocation gratuitously.
 */
static PHYSFS_EnumerateCallbackResult enumerateFromMountPoint(DirHandle *i,
                                    const char *arcfname,
                                    PHYSFS_EnumerateCallback callback,
                                    const char *_fname, void *data)
{
    PHYSFS_EnumerateCallbackResult retval;
    const size_t len = strlen(arcfname);
    char *ptr = NULL;
    char *end = NULL;
    const size_t slen = strlen(i->mountPoint) + 1;
    char *mountPoint = (char *) __PHYSFS_smallAlloc(slen);

    BAIL_IF(!mountPoint, PHYSFS_ERR_OUT_OF_MEMORY, PHYSFS_ENUM_ERROR);

    strcpy(mountPoint, i->mountPoint);
    ptr = mountPoint + ((len) ? len + 1 : 0);
    end = strchr(ptr, '/');
    assert(end);  /* should always find a terminating '/'. */
    *end = '\0';
    retval = callback(data, _fname, ptr);
    __PHYSFS_smallFree(mountPoint);

    BAIL_IF(retval == PHYSFS_ENUM_ERROR, PHYSFS_ERR_APP_CALLBACK, retval);
    return retval;
} /* enumerateFromMountPoint */


typedef struct SymlinkFilterData
{
    PHYSFS_EnumerateCallback callback;
    void *callbackData;
    DirHandle *dirhandle;
    const char *arcfname;
    PHYSFS_ErrorCode errcode;
} SymlinkFilterData;

static PHYSFS_EnumerateCallbackResult enumCallbackFilterSymLinks(void *_data,
                                    const char *origdir, const char *fname)
{
    SymlinkFilterData *data = (SymlinkFilterData *) _data;
    const DirHandle *dh = data->dirhandle;
    const char *arcfname = data->arcfname;
    PHYSFS_Stat statbuf;
    const char *trimmedDir = (*arcfname == '/') ? (arcfname + 1) : arcfname;
    const size_t slen = strlen(trimmedDir) + strlen(fname) + 2;
    char *path = (char *) __PHYSFS_smallAlloc(slen);
    PHYSFS_EnumerateCallbackResult retval = PHYSFS_ENUM_OK;

    if (path == NULL)
    {
        data->errcode = PHYSFS_ERR_OUT_OF_MEMORY;
        return PHYSFS_ENUM_ERROR;
    } /* if */

    snprintf(path, slen, "%s%s%s", trimmedDir, *trimmedDir ? "/" : "", fname);

    if (!dh->funcs->stat(dh->opaque, path, &statbuf))
    {
        data->errcode = currentErrorCode();
        retval = PHYSFS_ENUM_ERROR;
    } /* if */
    else
    {
        /* Pass it on to the application if it's not a symlink. */
        if (statbuf.filetype != PHYSFS_FILETYPE_SYMLINK)
        {
            retval = data->callback(data->callbackData, origdir, fname);
            if (retval == PHYSFS_ENUM_ERROR)
                data->errcode = PHYSFS_ERR_APP_CALLBACK;
        } /* if */
    } /* else */

    __PHYSFS_smallFree(path);

    return retval;
} /* enumCallbackFilterSymLinks */


int PHYSFS_enumerate(const char *_fn, PHYSFS_EnumerateCallback cb, void *data)
{
    PHYSFS_EnumerateCallbackResult retval = PHYSFS_ENUM_OK;
    size_t len;
    char *allocated_fname;
    char *fname;

    BAIL_IF(!_fn, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!cb, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);

    len = strlen(_fn) + longest_root + 1;
    allocated_fname = (char *) __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!allocated_fname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);
    fname = allocated_fname + longest_root;
    if (!sanitizePlatformIndependentPath(_fn, fname))
        retval = PHYSFS_ENUM_STOP;
    else
    {
        DirHandle *i;
        SymlinkFilterData filterdata;

        if (!allowSymLinks)
        {
            memset(&filterdata, '\0', sizeof (filterdata));
            filterdata.callback = cb;
            filterdata.callbackData = data;
        } /* if */

        for (i = searchPath; (retval == PHYSFS_ENUM_OK) && i; i = i->next)
        {
            char *arcfname = fname;

            if (partOfMountPoint(i, arcfname))
                retval = enumerateFromMountPoint(i, arcfname, cb, _fn, data);

            else if (verifyPath(i, &arcfname, 0))
            {
                PHYSFS_Stat statbuf;
                if (!i->funcs->stat(i->opaque, arcfname, &statbuf))
                {
                    if (currentErrorCode() == PHYSFS_ERR_NOT_FOUND)
                        continue;  /* no such dir in this archive, skip it. */
                } /* if */

                if (statbuf.filetype != PHYSFS_FILETYPE_DIRECTORY)
                    continue;  /* not a directory in this archive, skip it. */

                else if ((!allowSymLinks) && (i->funcs->info.supportsSymlinks))
                {
                    filterdata.dirhandle = i;
                    filterdata.arcfname = arcfname;
                    filterdata.errcode = PHYSFS_ERR_OK;
                    retval = i->funcs->enumerate(i->opaque, arcfname,
                                                 enumCallbackFilterSymLinks,
                                                 _fn, &filterdata);
                    if (retval == PHYSFS_ENUM_ERROR)
                    {
                        if (currentErrorCode() == PHYSFS_ERR_APP_CALLBACK)
                            PHYSFS_setErrorCode(filterdata.errcode);
                    } /* if */
                } /* else if */
                else
                {
                    retval = i->funcs->enumerate(i->opaque, arcfname,
                                                 cb, _fn, data);
                } /* else */
            } /* else if */
        } /* for */

    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);

    __PHYSFS_smallFree(allocated_fname);

    return (retval == PHYSFS_ENUM_ERROR) ? 0 : 1;
} /* PHYSFS_enumerate */


typedef struct
{
    PHYSFS_EnumFilesCallback callback;
    void *data;
} LegacyEnumFilesCallbackData;

static PHYSFS_EnumerateCallbackResult enumFilesCallbackAlwaysSucceed(void *d,
                                    const char *origdir, const char *fname)
{
    LegacyEnumFilesCallbackData *cbdata = (LegacyEnumFilesCallbackData *) d;
    cbdata->callback(cbdata->data, origdir, fname);
    return PHYSFS_ENUM_OK;
} /* enumFilesCallbackAlwaysSucceed */

void PHYSFS_enumerateFilesCallback(const char *fname,
                                   PHYSFS_EnumFilesCallback callback,
                                   void *data)
{
    LegacyEnumFilesCallbackData cbdata;
    cbdata.callback = callback;
    cbdata.data = data;
    (void) PHYSFS_enumerate(fname, enumFilesCallbackAlwaysSucceed, &cbdata);
} /* PHYSFS_enumerateFilesCallback */


int PHYSFS_exists(const char *fname)
{
    return (getRealDirHandle(fname) != NULL);
} /* PHYSFS_exists */


PHYSFS_sint64 PHYSFS_getLastModTime(const char *fname)
{
    PHYSFS_Stat statbuf;
    BAIL_IF_ERRPASS(!PHYSFS_stat(fname, &statbuf), -1);
    return statbuf.modtime;
} /* PHYSFS_getLastModTime */


int PHYSFS_isDirectory(const char *fname)
{
    PHYSFS_Stat statbuf;
    BAIL_IF_ERRPASS(!PHYSFS_stat(fname, &statbuf), 0);
    return (statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY);
} /* PHYSFS_isDirectory */


int PHYSFS_isSymbolicLink(const char *fname)
{
    PHYSFS_Stat statbuf;
    BAIL_IF_ERRPASS(!PHYSFS_stat(fname, &statbuf), 0);
    return (statbuf.filetype == PHYSFS_FILETYPE_SYMLINK);
} /* PHYSFS_isSymbolicLink */


static PHYSFS_File *doOpenWrite(const char *_fname, const int appending)
{
    FileHandle *fh = NULL;
    DirHandle *h;
    size_t len;
    char *fname;

    BAIL_IF(!_fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);

    h = writeDir;
    BAIL_IF_MUTEX(!h, PHYSFS_ERR_NO_WRITE_DIR, stateLock, 0);

    len = strlen(_fname) + dirHandleRootLen(h) + 1;
    fname = (char *) __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!fname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);

    if (sanitizePlatformIndependentPathWithRoot(h, _fname, fname))
    {
        PHYSFS_Io *io = NULL;
        char *arcfname = fname;
        if (verifyPath(h, &arcfname, 0))
        {
            const PHYSFS_Archiver *f = h->funcs;
            if (appending)
                io = f->openAppend(h->opaque, arcfname);
            else
                io = f->openWrite(h->opaque, arcfname);

            if (io)
            {
                fh = (FileHandle *) allocator.Malloc(sizeof (FileHandle));
                if (fh == NULL)
                {
                    io->destroy(io);
                    PHYSFS_setErrorCode(PHYSFS_ERR_OUT_OF_MEMORY);
                } /* if */
                else
                {
                    memset(fh, '\0', sizeof (FileHandle));
                    fh->io = io;
                    fh->dirHandle = h;
                    fh->next = openWriteList;
                    openWriteList = fh;
                } /* else */
            } /* if */
        } /* if */
    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);

    __PHYSFS_smallFree(fname);
    return ((PHYSFS_File *) fh);
} /* doOpenWrite */


PHYSFS_File *PHYSFS_openWrite(const char *filename)
{
    return doOpenWrite(filename, 0);
} /* PHYSFS_openWrite */


PHYSFS_File *PHYSFS_openAppend(const char *filename)
{
    return doOpenWrite(filename, 1);
} /* PHYSFS_openAppend */


PHYSFS_File *PHYSFS_openRead(const char *_fname)
{
    FileHandle *fh = NULL;
    char *allocated_fname;
    char *fname;
    size_t len;

    BAIL_IF(!_fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    __PHYSFS_platformGrabMutex(stateLock);

    BAIL_IF_MUTEX(!searchPath, PHYSFS_ERR_NOT_FOUND, stateLock, 0);

    len = strlen(_fname) + longest_root + 1;
    allocated_fname = (char *) __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!allocated_fname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);
    fname = allocated_fname + longest_root;

    if (sanitizePlatformIndependentPath(_fname, fname))
    {
        PHYSFS_Io *io = NULL;
        DirHandle *i;

        for (i = searchPath; i != NULL; i = i->next)
        {
            char *arcfname = fname;
            if (verifyPath(i, &arcfname, 0))
            {
                io = i->funcs->openRead(i->opaque, arcfname);
                if (io)
                    break;
            } /* if */
        } /* for */

        if (io)
        {
            fh = (FileHandle *) allocator.Malloc(sizeof (FileHandle));
            if (fh == NULL)
            {
                io->destroy(io);
                PHYSFS_setErrorCode(PHYSFS_ERR_OUT_OF_MEMORY);
            } /* if */

            memset(fh, '\0', sizeof (FileHandle));
            fh->io = io;
            fh->forReading = 1;
            fh->dirHandle = i;
            fh->next = openReadList;
            openReadList = fh;
        } /* if */
    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);
    __PHYSFS_smallFree(allocated_fname);
    return ((PHYSFS_File *) fh);
} /* PHYSFS_openRead */


static int closeHandleInOpenList(FileHandle **list, FileHandle *handle)
{
    FileHandle *prev = NULL;
    FileHandle *i;

    for (i = *list; i != NULL; i = i->next)
    {
        if (i == handle)  /* handle is in this list? */
        {
            PHYSFS_Io *io = handle->io;
            PHYSFS_uint8 *tmp = handle->buffer;

            /* send our buffer to io... */
            if (!handle->forReading)
            {
                if (!PHYSFS_flush((PHYSFS_File *) handle))
                    return -1;

                /* ...then have io send it to the disk... */
                else if (io->flush && !io->flush(io))
                    return -1;
            } /* if */

            /* ...then close the underlying file. */
            io->destroy(io);

            if (tmp != NULL)  /* free any associated buffer. */
                allocator.Free(tmp);

            if (prev == NULL)
                *list = handle->next;
            else
                prev->next = handle->next;

            allocator.Free(handle);
            return 1;
        } /* if */
        prev = i;
    } /* for */

    return 0;
} /* closeHandleInOpenList */


int PHYSFS_close(PHYSFS_File *_handle)
{
    FileHandle *handle = (FileHandle *) _handle;
    int rc;

    __PHYSFS_platformGrabMutex(stateLock);

    /* -1 == close failure. 0 == not found. 1 == success. */
    rc = closeHandleInOpenList(&openReadList, handle);
    BAIL_IF_MUTEX_ERRPASS(rc == -1, stateLock, 0);
    if (!rc)
    {
        rc = closeHandleInOpenList(&openWriteList, handle);
        BAIL_IF_MUTEX_ERRPASS(rc == -1, stateLock, 0);
    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);
    BAIL_IF(!rc, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    return 1;
} /* PHYSFS_close */


static PHYSFS_sint64 doBufferedRead(FileHandle *fh, void *_buffer, size_t len)
{
    PHYSFS_uint8 *buffer = (PHYSFS_uint8 *) _buffer;
    PHYSFS_sint64 retval = 0;

    while (len > 0)
    {
        const size_t avail = fh->buffill - fh->bufpos;
        if (avail > 0)  /* data available in the buffer. */
        {
            const size_t cpy = (len < avail) ? len : avail;
            memcpy(buffer, fh->buffer + fh->bufpos, cpy);
            assert(len >= cpy);
            buffer += cpy;
            len -= cpy;
            fh->bufpos += cpy;
            retval += cpy;
        } /* if */

        else   /* buffer is empty, refill it. */
        {
            PHYSFS_Io *io = fh->io;
            const PHYSFS_sint64 rc = io->read(io, fh->buffer, fh->bufsize);
            fh->bufpos = 0;
            if (rc > 0)
                fh->buffill = (size_t) rc;
            else
            {
                fh->buffill = 0;
                if (retval == 0)  /* report already-read data, or failure. */
                    retval = rc;
                break;
            } /* else */
        } /* else */
    } /* while */

    return retval;
} /* doBufferedRead */


PHYSFS_sint64 PHYSFS_read(PHYSFS_File *handle, void *buffer,
                          PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    const PHYSFS_uint64 len = ((PHYSFS_uint64) size) * ((PHYSFS_uint64) count);
    const PHYSFS_sint64 retval = PHYSFS_readBytes(handle, buffer, len);
    return ( (retval <= 0) ? retval : (retval / ((PHYSFS_sint64) size)) );
} /* PHYSFS_read */


PHYSFS_sint64 PHYSFS_readBytes(PHYSFS_File *handle, void *buffer,
                               PHYSFS_uint64 _len)
{
    const size_t len = (size_t) _len;
    FileHandle *fh = (FileHandle *) handle;

#ifdef PHYSFS_NO_64BIT_SUPPORT
    const PHYSFS_uint64 maxlen = __PHYSFS_UI64(0x7FFFFFFF);
#else
    const PHYSFS_uint64 maxlen = __PHYSFS_UI64(0x7FFFFFFFFFFFFFFF);
#endif

    if (!__PHYSFS_ui64FitsAddressSpace(_len))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, -1);

    BAIL_IF(_len > maxlen, PHYSFS_ERR_INVALID_ARGUMENT, -1);
    BAIL_IF(!fh->forReading, PHYSFS_ERR_OPEN_FOR_WRITING, -1);
    BAIL_IF_ERRPASS(len == 0, 0);
    if (fh->buffer)
        return doBufferedRead(fh, buffer, len);

    return fh->io->read(fh->io, buffer, len);
} /* PHYSFS_readBytes */


static PHYSFS_sint64 doBufferedWrite(PHYSFS_File *handle, const void *buffer,
                                     const size_t len)
{
    FileHandle *fh = (FileHandle *) handle;

    /* whole thing fits in the buffer? */
    if ((fh->buffill + len) < fh->bufsize)
    {
        memcpy(fh->buffer + fh->buffill, buffer, len);
        fh->buffill += len;
        return (PHYSFS_sint64) len;
    } /* if */

    /* would overflow buffer. Flush and then write the new objects, too. */
    BAIL_IF_ERRPASS(!PHYSFS_flush(handle), -1);
    return fh->io->write(fh->io, buffer, len);
} /* doBufferedWrite */


PHYSFS_sint64 PHYSFS_write(PHYSFS_File *handle, const void *buffer,
                           PHYSFS_uint32 size, PHYSFS_uint32 count)
{
    const PHYSFS_uint64 len = ((PHYSFS_uint64) size) * ((PHYSFS_uint64) count);
    const PHYSFS_sint64 retval = PHYSFS_writeBytes(handle, buffer, len);
    return ( (retval <= 0) ? retval : (retval / ((PHYSFS_sint64) size)) );
} /* PHYSFS_write */


PHYSFS_sint64 PHYSFS_writeBytes(PHYSFS_File *handle, const void *buffer,
                                PHYSFS_uint64 _len)
{
    const size_t len = (size_t) _len;
    FileHandle *fh = (FileHandle *) handle;

#ifdef PHYSFS_NO_64BIT_SUPPORT
    const PHYSFS_uint64 maxlen = __PHYSFS_UI64(0x7FFFFFFF);
#else
    const PHYSFS_uint64 maxlen = __PHYSFS_UI64(0x7FFFFFFFFFFFFFFF);
#endif

    if (!__PHYSFS_ui64FitsAddressSpace(_len))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, -1);

    BAIL_IF(_len > maxlen, PHYSFS_ERR_INVALID_ARGUMENT, -1);
    BAIL_IF(fh->forReading, PHYSFS_ERR_OPEN_FOR_READING, -1);
    BAIL_IF_ERRPASS(len == 0, 0);
    if (fh->buffer)
        return doBufferedWrite(handle, buffer, len);

    return fh->io->write(fh->io, buffer, len);
} /* PHYSFS_write */


int PHYSFS_eof(PHYSFS_File *handle)
{
    FileHandle *fh = (FileHandle *) handle;

    if (!fh->forReading)  /* never EOF on files opened for write/append. */
        return 0;

    /* can't be eof if buffer isn't empty */
    if (fh->bufpos == fh->buffill)
    {
        /* check the Io. */
        PHYSFS_Io *io = fh->io;
        const PHYSFS_sint64 pos = io->tell(io);
        const PHYSFS_sint64 len = io->length(io);
        if ((pos < 0) || (len < 0))
            return 0;  /* beats me. */
        return (pos >= len);
    } /* if */

    return 0;
} /* PHYSFS_eof */


PHYSFS_sint64 PHYSFS_tell(PHYSFS_File *handle)
{
    FileHandle *fh = (FileHandle *) handle;
    const PHYSFS_sint64 pos = fh->io->tell(fh->io);
    const PHYSFS_sint64 retval = fh->forReading ?
                                 (pos - fh->buffill) + fh->bufpos :
                                 (pos + fh->buffill);
    return retval;
} /* PHYSFS_tell */


int PHYSFS_seek(PHYSFS_File *handle, PHYSFS_uint64 pos)
{
    FileHandle *fh = (FileHandle *) handle;
    BAIL_IF_ERRPASS(!PHYSFS_flush(handle), 0);

    if (fh->buffer && fh->forReading)
    {
        /* avoid throwing away our precious buffer if seeking within it. */
        PHYSFS_sint64 offset = pos - PHYSFS_tell(handle);
        if ( /* seeking within the already-buffered range? */
             /* forward? */
            ((offset >= 0) && (((size_t)offset) <= fh->buffill-fh->bufpos)) ||
            /* backward? */
            ((offset < 0) && (((size_t) -offset) <= fh->bufpos)) )
        {
            fh->bufpos = (size_t) (((PHYSFS_sint64) fh->bufpos) + offset);
            return 1; /* successful seek */
        } /* if */
    } /* if */

    /* we have to fall back to a 'raw' seek. */
    fh->buffill = fh->bufpos = 0;
    return fh->io->seek(fh->io, pos);
} /* PHYSFS_seek */


PHYSFS_sint64 PHYSFS_fileLength(PHYSFS_File *handle)
{
    PHYSFS_Io *io = ((FileHandle *) handle)->io;
    return io->length(io);
} /* PHYSFS_filelength */


int PHYSFS_setBuffer(PHYSFS_File *handle, PHYSFS_uint64 _bufsize)
{
    FileHandle *fh = (FileHandle *) handle;
    const size_t bufsize = (size_t) _bufsize;

    if (!__PHYSFS_ui64FitsAddressSpace(_bufsize))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, 0);

    BAIL_IF_ERRPASS(!PHYSFS_flush(handle), 0);

    /*
     * For reads, we need to move the file pointer to where it would be
     *  if we weren't buffering, so that the next read will get the
     *  right chunk of stuff from the file. PHYSFS_flush() handles writes.
     */
    if ((fh->forReading) && (fh->buffill != fh->bufpos))
    {
        PHYSFS_uint64 pos;
        const PHYSFS_sint64 curpos = fh->io->tell(fh->io);
        BAIL_IF_ERRPASS(curpos == -1, 0);
        pos = ((curpos - fh->buffill) + fh->bufpos);
        BAIL_IF_ERRPASS(!fh->io->seek(fh->io, pos), 0);
    } /* if */

    if (bufsize == 0)  /* delete existing buffer. */
    {
        if (fh->buffer)
        {
            allocator.Free(fh->buffer);
            fh->buffer = NULL;
        } /* if */
    } /* if */

    else
    {
        PHYSFS_uint8 *newbuf;
        newbuf = (PHYSFS_uint8 *) allocator.Realloc(fh->buffer, bufsize);
        BAIL_IF(!newbuf, PHYSFS_ERR_OUT_OF_MEMORY, 0);
        fh->buffer = newbuf;
    } /* else */

    fh->bufsize = bufsize;
    fh->buffill = fh->bufpos = 0;
    return 1;
} /* PHYSFS_setBuffer */


int PHYSFS_flush(PHYSFS_File *handle)
{
    FileHandle *fh = (FileHandle *) handle;
    PHYSFS_Io *io;
    PHYSFS_sint64 rc;

    if ((fh->forReading) || (fh->bufpos == fh->buffill))
        return 1;  /* open for read or buffer empty are successful no-ops. */

    /* dump buffer to disk. */
    io = fh->io;
    rc = io->write(io, fh->buffer + fh->bufpos, fh->buffill - fh->bufpos);
    BAIL_IF_ERRPASS(rc <= 0, 0);
    fh->bufpos = fh->buffill = 0;
    return 1;
} /* PHYSFS_flush */


int PHYSFS_stat(const char *_fname, PHYSFS_Stat *stat)
{
    int retval = 0;
    char *allocated_fname;
    char *fname;
    size_t len;

    BAIL_IF(!_fname, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    BAIL_IF(!stat, PHYSFS_ERR_INVALID_ARGUMENT, 0);

    /* set some sane defaults... */
    stat->filesize = -1;
    stat->modtime = -1;
    stat->createtime = -1;
    stat->accesstime = -1;
    stat->filetype = PHYSFS_FILETYPE_OTHER;
    stat->readonly = 1;

    __PHYSFS_platformGrabMutex(stateLock);
    len = strlen(_fname) + longest_root + 1;
    allocated_fname = (char *) __PHYSFS_smallAlloc(len);
    BAIL_IF_MUTEX(!allocated_fname, PHYSFS_ERR_OUT_OF_MEMORY, stateLock, 0);
    fname = allocated_fname + longest_root;

    if (sanitizePlatformIndependentPath(_fname, fname))
    {
        if (*fname == '\0')
        {
            stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
            stat->readonly = !writeDir; /* Writeable if we have a writeDir */
            retval = 1;
        } /* if */
        else
        {
            DirHandle *i;
            int exists = 0;
            for (i = searchPath; ((i != NULL) && (!exists)); i = i->next)
            {
                char *arcfname = fname;
                exists = partOfMountPoint(i, arcfname);
                if (exists)
                {
                    stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
                    stat->readonly = 1;
                    retval = 1;
                } /* if */
                else if (verifyPath(i, &arcfname, 0))
                {
                    retval = i->funcs->stat(i->opaque, arcfname, stat);
                    if ((retval) || (currentErrorCode() != PHYSFS_ERR_NOT_FOUND))
                        exists = 1;
                } /* else if */
            } /* for */
        } /* else */
    } /* if */

    __PHYSFS_platformReleaseMutex(stateLock);
    __PHYSFS_smallFree(allocated_fname);
    return retval;
} /* PHYSFS_stat */


int __PHYSFS_readAll(PHYSFS_Io *io, void *buf, const size_t _len)
{
    const PHYSFS_uint64 len = (PHYSFS_uint64) _len;
    return (io->read(io, buf, len) == len);
} /* __PHYSFS_readAll */


void *__PHYSFS_initSmallAlloc(void *ptr, const size_t len)
{
    void *useHeap = ((ptr == NULL) ? ((void *) 1) : ((void *) 0));
    if (useHeap)  /* too large for stack allocation or alloca() failed. */
        ptr = allocator.Malloc(len+sizeof (void *));

    if (ptr != NULL)
    {
        void **retval = (void **) ptr;
        /*printf("%s alloc'd (%lld) bytes at (%p).\n",
                useHeap ? "heap" : "stack", (long long) len, ptr);*/
        *retval = useHeap;
        return retval + 1;
    } /* if */

    return NULL;  /* allocation failed. */
} /* __PHYSFS_initSmallAlloc */


void __PHYSFS_smallFree(void *ptr)
{
    if (ptr != NULL)
    {
        void **block = ((void **) ptr) - 1;
        const int useHeap = (*block != NULL);
        if (useHeap)
            allocator.Free(block);
        /*printf("%s free'd (%p).\n", useHeap ? "heap" : "stack", block);*/
    } /* if */
} /* __PHYSFS_smallFree */


int PHYSFS_setAllocator(const PHYSFS_Allocator *a)
{
    BAIL_IF(initialized, PHYSFS_ERR_IS_INITIALIZED, 0);
    externalAllocator = (a != NULL);
    if (externalAllocator)
        memcpy(&allocator, a, sizeof (PHYSFS_Allocator));

    return 1;
} /* PHYSFS_setAllocator */


const PHYSFS_Allocator *PHYSFS_getAllocator(void)
{
    BAIL_IF(!initialized, PHYSFS_ERR_NOT_INITIALIZED, NULL);
    return &allocator;
} /* PHYSFS_getAllocator */


static void *mallocAllocatorMalloc(PHYSFS_uint64 s)
{
    if (!__PHYSFS_ui64FitsAddressSpace(s))
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    #undef malloc
    return malloc((size_t) s);
} /* mallocAllocatorMalloc */


static void *mallocAllocatorRealloc(void *ptr, PHYSFS_uint64 s)
{
    if (!__PHYSFS_ui64FitsAddressSpace(s))
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    #undef realloc
    return realloc(ptr, (size_t) s);
} /* mallocAllocatorRealloc */


static void mallocAllocatorFree(void *ptr)
{
    #undef free
    free(ptr);
} /* mallocAllocatorFree */


static void setDefaultAllocator(void)
{
    assert(!externalAllocator);
    allocator.Init = NULL;
    allocator.Deinit = NULL;
    allocator.Malloc = mallocAllocatorMalloc;
    allocator.Realloc = mallocAllocatorRealloc;
    allocator.Free = mallocAllocatorFree;
} /* setDefaultAllocator */


int __PHYSFS_DirTreeInit(__PHYSFS_DirTree *dt, const size_t entrylen)
{
    static char rootpath[2] = { '/', '\0' };
    size_t alloclen;

    assert(entrylen >= sizeof (__PHYSFS_DirTreeEntry));

    memset(dt, '\0', sizeof (*dt));

    dt->root = (__PHYSFS_DirTreeEntry *) allocator.Malloc(entrylen);
    BAIL_IF(!dt->root, PHYSFS_ERR_OUT_OF_MEMORY, 0);
    memset(dt->root, '\0', entrylen);
    dt->root->name = rootpath;
    dt->root->isdir = 1;
    dt->hashBuckets = 64;
    if (!dt->hashBuckets)
        dt->hashBuckets = 1;
    dt->entrylen = entrylen;

    alloclen = dt->hashBuckets * sizeof (__PHYSFS_DirTreeEntry *);
    dt->hash = (__PHYSFS_DirTreeEntry **) allocator.Malloc(alloclen);
    BAIL_IF(!dt->hash, PHYSFS_ERR_OUT_OF_MEMORY, 0);
    memset(dt->hash, '\0', alloclen);

    return 1;
} /* __PHYSFS_DirTreeInit */


static inline PHYSFS_uint32 hashPathName(__PHYSFS_DirTree *dt, const char *name)
{
    return __PHYSFS_hashString(name, strlen(name)) % dt->hashBuckets;
} /* hashPathName */


/* Fill in missing parent directories. */
static __PHYSFS_DirTreeEntry *addAncestors(__PHYSFS_DirTree *dt, char *name)
{
    __PHYSFS_DirTreeEntry *retval = dt->root;
    char *sep = strrchr(name, '/');

    if (sep)
    {
        *sep = '\0';  /* chop off last piece. */
        retval = (__PHYSFS_DirTreeEntry *) __PHYSFS_DirTreeFind(dt, name);

        if (retval != NULL)
        {
            *sep = '/';
            BAIL_IF(!retval->isdir, PHYSFS_ERR_CORRUPT, NULL);
            return retval;  /* already hashed. */
        } /* if */

        /* okay, this is a new dir. Build and hash us. */
        retval = (__PHYSFS_DirTreeEntry*)__PHYSFS_DirTreeAdd(dt, name, 1);
        *sep = '/';
    } /* if */

    return retval;
} /* addAncestors */


void *__PHYSFS_DirTreeAdd(__PHYSFS_DirTree *dt, char *name, const int isdir)
{
    __PHYSFS_DirTreeEntry *retval = __PHYSFS_DirTreeFind(dt, name);
    if (!retval)
    {
        const size_t alloclen = strlen(name) + 1 + dt->entrylen;
        PHYSFS_uint32 hashval;
        __PHYSFS_DirTreeEntry *parent = addAncestors(dt, name);
        BAIL_IF_ERRPASS(!parent, NULL);
        assert(dt->entrylen >= sizeof (__PHYSFS_DirTreeEntry));
        retval = (__PHYSFS_DirTreeEntry *) allocator.Malloc(alloclen);
        BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        memset(retval, '\0', dt->entrylen);
        retval->name = ((char *) retval) + dt->entrylen;
        strcpy(retval->name, name);
        hashval = hashPathName(dt, name);
        retval->hashnext = dt->hash[hashval];
        dt->hash[hashval] = retval;
        retval->sibling = parent->children;
        retval->isdir = isdir;
        parent->children = retval;
    } /* if */

    return retval;
} /* __PHYSFS_DirTreeAdd */


/* Find the __PHYSFS_DirTreeEntry for a path in platform-independent notation. */
void *__PHYSFS_DirTreeFind(__PHYSFS_DirTree *dt, const char *path)
{
    PHYSFS_uint32 hashval;
    __PHYSFS_DirTreeEntry *prev = NULL;
    __PHYSFS_DirTreeEntry *retval;

    if (*path == '\0')
        return dt->root;

    hashval = hashPathName(dt, path);
    for (retval = dt->hash[hashval]; retval; retval = retval->hashnext)
    {
        if (strcmp(retval->name, path) == 0)
        {
            if (prev != NULL)  /* move this to the front of the list */
            {
                prev->hashnext = retval->hashnext;
                retval->hashnext = dt->hash[hashval];
                dt->hash[hashval] = retval;
            } /* if */

            return retval;
        } /* if */

        prev = retval;
    } /* for */

    BAIL(PHYSFS_ERR_NOT_FOUND, NULL);
} /* __PHYSFS_DirTreeFind */

PHYSFS_EnumerateCallbackResult __PHYSFS_DirTreeEnumerate(void *opaque,
                              const char *dname, PHYSFS_EnumerateCallback cb,
                              const char *origdir, void *callbackdata)
{
    PHYSFS_EnumerateCallbackResult retval = PHYSFS_ENUM_OK;
    __PHYSFS_DirTree *tree = (__PHYSFS_DirTree *) opaque;
    const __PHYSFS_DirTreeEntry *entry = __PHYSFS_DirTreeFind(tree, dname);
    BAIL_IF(!entry, PHYSFS_ERR_NOT_FOUND, PHYSFS_ENUM_ERROR);

    entry = entry->children;

    while (entry && (retval == PHYSFS_ENUM_OK))
    {
        const char *name = entry->name;
        const char *ptr = strrchr(name, '/');
        retval = cb(callbackdata, origdir, ptr ? ptr + 1 : name);
        BAIL_IF(retval == PHYSFS_ENUM_ERROR, PHYSFS_ERR_APP_CALLBACK, retval);
        entry = entry->sibling;
    } /* while */

    return retval;
} /* __PHYSFS_DirTreeEnumerate */


void __PHYSFS_DirTreeDeinit(__PHYSFS_DirTree *dt)
{
    if (!dt)
        return;

    if (dt->root)
    {
        assert(dt->root->sibling == NULL);
        assert(dt->hash || (dt->root->children == NULL));
        allocator.Free(dt->root);
    } /* if */

    if (dt->hash)
    {
        size_t i;
        for (i = 0; i < dt->hashBuckets; i++)
        {
            __PHYSFS_DirTreeEntry *entry;
            __PHYSFS_DirTreeEntry *next;
            for (entry = dt->hash[i]; entry; entry = next)
            {
                next = entry->hashnext;
                allocator.Free(entry);
            } /* for */
        } /* for */
        allocator.Free(dt->hash);
    } /* if */
} /* __PHYSFS_DirTreeDeinit */

/* end of physfs.c ... */

