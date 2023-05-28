/*
 * 7zip support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file was written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_7Z

#include "physfs_lzmasdk.h"

typedef struct
{
    ISeekInStream seekStream; /* lzma sdk i/o interface (lower level).  */
    PHYSFS_Io *io;            /* physfs i/o interface for this archive. */
    CLookToRead lookStream;   /* lzma sdk i/o interface (higher level). */
} SZIPLookToRead;

/* One SZIPentry is kept for each file in an open 7zip archive. */
typedef struct
{
    __PHYSFS_DirTreeEntry tree;   /* manages directory tree         */
    PHYSFS_uint32 dbidx;          /* index into lzma sdk database   */
} SZIPentry;

/* One SZIPinfo is kept for each open 7zip archive. */
typedef struct
{
    __PHYSFS_DirTree tree;    /* manages directory tree.           */
    PHYSFS_Io *io;            /* physfs i/o interface for this archive. */
    CSzArEx db;               /* lzma sdk archive database object. */
} SZIPinfo;


static PHYSFS_ErrorCode szipErrorCode(const SRes rc)
{
    switch (rc)
    {
        case SZ_OK: return PHYSFS_ERR_OK;
        case SZ_ERROR_DATA: return PHYSFS_ERR_CORRUPT;
        case SZ_ERROR_MEM: return PHYSFS_ERR_OUT_OF_MEMORY;
        case SZ_ERROR_CRC: return PHYSFS_ERR_CORRUPT;
        case SZ_ERROR_UNSUPPORTED: return PHYSFS_ERR_UNSUPPORTED;
        case SZ_ERROR_INPUT_EOF: return PHYSFS_ERR_CORRUPT;
        case SZ_ERROR_OUTPUT_EOF: return PHYSFS_ERR_IO;
        case SZ_ERROR_READ: return PHYSFS_ERR_IO;
        case SZ_ERROR_WRITE: return PHYSFS_ERR_IO;
        case SZ_ERROR_ARCHIVE: return PHYSFS_ERR_CORRUPT;
        case SZ_ERROR_NO_ARCHIVE: return PHYSFS_ERR_UNSUPPORTED;
        default: break;
    } /* switch */

    return PHYSFS_ERR_OTHER_ERROR;
} /* szipErrorCode */


/* LZMA SDK's ISzAlloc interface ... */

static void *SZIP_ISzAlloc_Alloc(void *p, size_t size)
{
    return allocator.Malloc(size ? size : 1);
} /* SZIP_ISzAlloc_Alloc */

static void SZIP_ISzAlloc_Free(void *p, void *address)
{
    if (address)
        allocator.Free(address);
} /* SZIP_ISzAlloc_Free */

static ISzAlloc SZIP_SzAlloc = {
    SZIP_ISzAlloc_Alloc, SZIP_ISzAlloc_Free
};


/* we implement ISeekInStream, and then wrap that in LZMA SDK's CLookToRead,
   which implements the higher-level ILookInStream on top of that, handling
   buffering and such for us. */

/* LZMA SDK's ISeekInStream interface ... */

static SRes SZIP_ISeekInStream_Read(void *p, void *buf, size_t *size)
{
    SZIPLookToRead *stream = (SZIPLookToRead *) p;
    PHYSFS_Io *io = stream->io;
    const PHYSFS_uint64 len = (PHYSFS_uint64) *size;
    const PHYSFS_sint64 rc = (len == 0) ? 0 : io->read(io, buf, len);

    if (rc < 0)
    {
        *size = 0;
        return SZ_ERROR_READ;
    } /* if */

    *size = (size_t) rc;
    return SZ_OK;
} /* SZIP_ISeekInStream_Read */

static SRes SZIP_ISeekInStream_Seek(void *p, Int64 *pos, ESzSeek origin)
{
    SZIPLookToRead *stream = (SZIPLookToRead *) p;
    PHYSFS_Io *io = stream->io;
    PHYSFS_sint64 base;
    PHYSFS_uint64 newpos;

    switch (origin)
    {
        case SZ_SEEK_SET:
            base = 0;
            break;

        case SZ_SEEK_CUR:
            base = io->tell(io);
            break;

        case SZ_SEEK_END:
            base = io->length(io);
            break;

        default:
            return SZ_ERROR_FAIL;
    } /* switch */

    if (base < 0)
        return SZ_ERROR_FAIL;
    else if ((*pos < 0) && (((Int64) base) < -*pos))
        return SZ_ERROR_FAIL;

    newpos = (PHYSFS_uint64) (((Int64) base) + *pos);
    if (!io->seek(io, newpos))
        return SZ_ERROR_FAIL;

    *pos = (Int64) newpos;
    return SZ_OK;
} /* SZIP_ISeekInStream_Seek */


static void szipInitStream(SZIPLookToRead *stream, PHYSFS_Io *io)
{
    stream->seekStream.Read = SZIP_ISeekInStream_Read;
    stream->seekStream.Seek = SZIP_ISeekInStream_Seek;

    stream->io = io;

    /* !!! FIXME: can we use lookahead? Is there value to it? */
    LookToRead_Init(&stream->lookStream);
    LookToRead_CreateVTable(&stream->lookStream, False);
    stream->lookStream.realStream = &stream->seekStream;
} /* szipInitStream */


/* Do this in a separate function so we can smallAlloc without looping. */
static int szipLoadEntry(SZIPinfo *info, const PHYSFS_uint32 idx)
{
    const size_t utf16len = SzArEx_GetFileNameUtf16(&info->db, idx, NULL);
    const size_t utf16buflen = utf16len * 2;
    PHYSFS_uint16 *utf16 = (PHYSFS_uint16 *) __PHYSFS_smallAlloc(utf16buflen);
    const size_t utf8buflen = utf16len * 4;
    char *utf8 = (char *) __PHYSFS_smallAlloc(utf8buflen);
    int retval = 0;

    if (utf16 && utf8)
    {
        const int isdir = SzArEx_IsDir(&info->db, idx) != 0;
        SZIPentry *entry;
        SzArEx_GetFileNameUtf16(&info->db, idx, (UInt16 *) utf16);
        PHYSFS_utf8FromUtf16(utf16, utf8, utf8buflen);
        entry = (SZIPentry*) __PHYSFS_DirTreeAdd(&info->tree, utf8, isdir);
        retval = (entry != NULL);
        if (retval)
            entry->dbidx = idx;
    } /* if */

    __PHYSFS_smallFree(utf8);
    __PHYSFS_smallFree(utf16);

    return retval;
} /* szipLoadEntry */


static int szipLoadEntries(SZIPinfo *info)
{
    int retval = 0;

    if (__PHYSFS_DirTreeInit(&info->tree, sizeof (SZIPentry), 1, 0))
    {
        const PHYSFS_uint32 count = info->db.NumFiles;
        PHYSFS_uint32 i;
        for (i = 0; i < count; i++)
            BAIL_IF_ERRPASS(!szipLoadEntry(info, i), 0);
        retval = 1;
    } /* if */

    return retval;
} /* szipLoadEntries */


static void SZIP_closeArchive(void *opaque)
{
    SZIPinfo *info = (SZIPinfo *) opaque;
    if (info)
    {
        if (info->io)
            info->io->destroy(info->io);
        SzArEx_Free(&info->db, &SZIP_SzAlloc);
        __PHYSFS_DirTreeDeinit(&info->tree);
        allocator.Free(info);
    } /* if */
} /* SZIP_closeArchive */


static void *SZIP_openArchive(PHYSFS_Io *io, const char *name,
                              int forWriting, int *claimed)
{
    static const PHYSFS_uint8 wantedsig[] = { '7','z',0xBC,0xAF,0x27,0x1C };
    SZIPLookToRead stream;
    ISzAlloc *alloc = &SZIP_SzAlloc;
    SZIPinfo *info = NULL;
    SRes rc;
    PHYSFS_uint8 sig[6];
    PHYSFS_sint64 pos;

    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);
    pos = io->tell(io);
    BAIL_IF_ERRPASS(pos == -1, NULL);
    BAIL_IF_ERRPASS(io->read(io, sig, 6) != 6, NULL);
    *claimed = (memcmp(sig, wantedsig, 6) == 0);
    BAIL_IF_ERRPASS(!io->seek(io, pos), NULL);

    info = (SZIPinfo *) allocator.Malloc(sizeof (SZIPinfo));
    BAIL_IF(!info, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    memset(info, '\0', sizeof (*info));

    SzArEx_Init(&info->db);

    info->io = io;

    szipInitStream(&stream, io);
    rc = SzArEx_Open(&info->db, &stream.lookStream.s, alloc, alloc);
    GOTO_IF(rc != SZ_OK, szipErrorCode(rc), failed);

    GOTO_IF_ERRPASS(!szipLoadEntries(info), failed);

    return info;

failed:
    info->io = NULL;  /* don't let cleanup destroy the PHYSFS_Io. */
    SZIP_closeArchive(info);
    return NULL;
} /* SZIP_openArchive */


static PHYSFS_Io *SZIP_openRead(void *opaque, const char *path)
{
    /* !!! FIXME: the current lzma sdk C API only allows you to decompress
       !!! FIXME:  the entire file at once, which isn't ideal. Fix this in the
       !!! FIXME:  SDK and then convert this all to a streaming interface. */

    SZIPinfo *info = (SZIPinfo *) opaque;
    SZIPentry *entry = (SZIPentry *) __PHYSFS_DirTreeFind(&info->tree, path);
    ISzAlloc *alloc = &SZIP_SzAlloc;
    SZIPLookToRead stream;
    PHYSFS_Io *retval = NULL;
    PHYSFS_Io *io = NULL;
    UInt32 blockIndex = 0xFFFFFFFF;
    Byte *outBuffer = NULL;
    size_t outBufferSize = 0;
    size_t offset = 0;
    size_t outSizeProcessed = 0;
    void *buf = NULL;
    SRes rc;

    BAIL_IF_ERRPASS(!entry, NULL);
    BAIL_IF(entry->tree.isdir, PHYSFS_ERR_NOT_A_FILE, NULL);

    io = info->io->duplicate(info->io);
    GOTO_IF_ERRPASS(!io, SZIP_openRead_failed);

    szipInitStream(&stream, io);

    rc = SzArEx_Extract(&info->db, &stream.lookStream.s, entry->dbidx,
                        &blockIndex, &outBuffer, &outBufferSize, &offset,
                        &outSizeProcessed, alloc, alloc);
    GOTO_IF(rc != SZ_OK, szipErrorCode(rc), SZIP_openRead_failed);
    GOTO_IF(outBuffer == NULL, PHYSFS_ERR_OUT_OF_MEMORY, SZIP_openRead_failed);

    io->destroy(io);
    io = NULL;

    buf = allocator.Malloc(outSizeProcessed ? outSizeProcessed : 1);
    GOTO_IF(buf == NULL, PHYSFS_ERR_OUT_OF_MEMORY, SZIP_openRead_failed);

    if (outSizeProcessed > 0)
        memcpy(buf, outBuffer + offset, outSizeProcessed);

    alloc->Free(alloc, outBuffer);
    outBuffer = NULL;

    retval = __PHYSFS_createMemoryIo(buf, outSizeProcessed, allocator.Free);
    GOTO_IF_ERRPASS(!retval, SZIP_openRead_failed);

    return retval;

SZIP_openRead_failed:
    if (io != NULL)
        io->destroy(io);

    if (buf)
        allocator.Free(buf);

    if (outBuffer)
        alloc->Free(alloc, outBuffer);

    return NULL;
} /* SZIP_openRead */


static PHYSFS_Io *SZIP_openWrite(void *opaque, const char *filename)
{
    BAIL(PHYSFS_ERR_READ_ONLY, NULL);
} /* SZIP_openWrite */


static PHYSFS_Io *SZIP_openAppend(void *opaque, const char *filename)
{
    BAIL(PHYSFS_ERR_READ_ONLY, NULL);
} /* SZIP_openAppend */


static int SZIP_remove(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, 0);
} /* SZIP_remove */


static int SZIP_mkdir(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, 0);
} /* SZIP_mkdir */


static inline PHYSFS_uint64 lzmasdkTimeToPhysfsTime(const CNtfsFileTime *t)
{
    const PHYSFS_uint64 winEpochToUnixEpoch = __PHYSFS_UI64(0x019DB1DED53E8000);
    const PHYSFS_uint64 nanosecToMillisec = __PHYSFS_UI64(10000000);
    const PHYSFS_uint64 quad = (((PHYSFS_uint64) t->High) << 32) | t->Low;
    return (quad - winEpochToUnixEpoch) / nanosecToMillisec;
} /* lzmasdkTimeToPhysfsTime */


static int SZIP_stat(void *opaque, const char *path, PHYSFS_Stat *stat)
{
    SZIPinfo *info = (SZIPinfo *) opaque;
    SZIPentry *entry;
    PHYSFS_uint32 idx;

    entry = (SZIPentry *) __PHYSFS_DirTreeFind(&info->tree, path);
    BAIL_IF_ERRPASS(!entry, 0);
    idx = entry->dbidx;

    if (entry->tree.isdir)
    {
        stat->filesize = -1;
	    stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
    } /* if */
    else
    {
        stat->filesize = (PHYSFS_sint64) SzArEx_GetFileSize(&info->db, idx);
	    stat->filetype = PHYSFS_FILETYPE_REGULAR;
    } /* else */

    if (info->db.MTime.Vals != NULL)
	    stat->modtime = lzmasdkTimeToPhysfsTime(&info->db.MTime.Vals[idx]);
    else if (info->db.CTime.Vals != NULL)
	    stat->modtime = lzmasdkTimeToPhysfsTime(&info->db.CTime.Vals[idx]);
    else
	    stat->modtime = -1;

    if (info->db.CTime.Vals != NULL)
	    stat->createtime = lzmasdkTimeToPhysfsTime(&info->db.CTime.Vals[idx]);
    else if (info->db.MTime.Vals != NULL)
	    stat->createtime = lzmasdkTimeToPhysfsTime(&info->db.MTime.Vals[idx]);
    else
	    stat->createtime = -1;

	stat->accesstime = -1;
	stat->readonly = 1;

    return 1;
} /* SZIP_stat */


void SZIP_global_init(void)
{
    /* this just needs to calculate some things, so it only ever
       has to run once, even after a deinit. */
    static int generatedTable = 0;
    if (!generatedTable)
    {
        generatedTable = 1;
        CrcGenerateTable();
    } /* if */
} /* SZIP_global_init */


const PHYSFS_Archiver __PHYSFS_Archiver_7Z =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "7Z",
        "7zip archives",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/physfs/",
        0,  /* supportsSymlinks */
    },
    SZIP_openArchive,
    __PHYSFS_DirTreeEnumerate,
    SZIP_openRead,
    SZIP_openWrite,
    SZIP_openAppend,
    SZIP_remove,
    SZIP_mkdir,
    SZIP_stat,
    SZIP_closeArchive
};

#endif  /* defined PHYSFS_SUPPORTS_7Z */

/* end of physfs_archiver_7z.c ... */

