/*
 * High-level PhysicsFS archiver for simple unpacked file formats.
 *
 * This is a framework that basic archivers build on top of. It's for simple
 *  formats that can just hand back a list of files and the offsets of their
 *  uncompressed data. There are an alarming number of formats like this.
 *
 * RULES: Archive entries must be uncompressed. Dirs and files allowed, but no
 *  symlinks, etc. We can relax some of these rules as necessary.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

typedef struct
{
    __PHYSFS_DirTree tree;
    PHYSFS_Io *io;
} UNPKinfo;

typedef struct
{
    __PHYSFS_DirTreeEntry tree;
    PHYSFS_uint64 startPos;
    PHYSFS_uint64 size;
    PHYSFS_sint64 ctime;
    PHYSFS_sint64 mtime;
} UNPKentry;

typedef struct
{
    PHYSFS_Io *io;
    UNPKentry *entry;
    PHYSFS_uint32 curPos;
} UNPKfileinfo;


void UNPK_closeArchive(void *opaque)
{
    UNPKinfo *info = ((UNPKinfo *) opaque);
    if (info)
    {
        __PHYSFS_DirTreeDeinit(&info->tree);

        if (info->io)
            info->io->destroy(info->io);

        allocator.Free(info);
    } /* if */
} /* UNPK_closeArchive */

void UNPK_abandonArchive(void *opaque)
{
    UNPKinfo *info = ((UNPKinfo *) opaque);
    if (info)
    {
        info->io = NULL;
        UNPK_closeArchive(info);
    } /* if */
} /* UNPK_abandonArchive */

static PHYSFS_sint64 UNPK_read(PHYSFS_Io *io, void *buffer, PHYSFS_uint64 len)
{
    UNPKfileinfo *finfo = (UNPKfileinfo *) io->opaque;
    const UNPKentry *entry = finfo->entry;
    const PHYSFS_uint64 bytesLeft = (PHYSFS_uint64)(entry->size-finfo->curPos);
    PHYSFS_sint64 rc;

    if (bytesLeft < len)
        len = bytesLeft;

    rc = finfo->io->read(finfo->io, buffer, len);
    if (rc > 0)
        finfo->curPos += (PHYSFS_uint32) rc;

    return rc;
} /* UNPK_read */


static PHYSFS_sint64 UNPK_write(PHYSFS_Io *io, const void *b, PHYSFS_uint64 len)
{
    BAIL(PHYSFS_ERR_READ_ONLY, -1);
} /* UNPK_write */


static PHYSFS_sint64 UNPK_tell(PHYSFS_Io *io)
{
    return ((UNPKfileinfo *) io->opaque)->curPos;
} /* UNPK_tell */


static int UNPK_seek(PHYSFS_Io *io, PHYSFS_uint64 offset)
{
    UNPKfileinfo *finfo = (UNPKfileinfo *) io->opaque;
    const UNPKentry *entry = finfo->entry;
    int rc;

    BAIL_IF(offset >= entry->size, PHYSFS_ERR_PAST_EOF, 0);
    rc = finfo->io->seek(finfo->io, entry->startPos + offset);
    if (rc)
        finfo->curPos = (PHYSFS_uint32) offset;

    return rc;
} /* UNPK_seek */


static PHYSFS_sint64 UNPK_length(PHYSFS_Io *io)
{
    const UNPKfileinfo *finfo = (UNPKfileinfo *) io->opaque;
    return ((PHYSFS_sint64) finfo->entry->size);
} /* UNPK_length */


static PHYSFS_Io *UNPK_duplicate(PHYSFS_Io *_io)
{
    UNPKfileinfo *origfinfo = (UNPKfileinfo *) _io->opaque;
    PHYSFS_Io *io = NULL;
    PHYSFS_Io *retval = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    UNPKfileinfo *finfo = (UNPKfileinfo *) allocator.Malloc(sizeof (UNPKfileinfo));
    GOTO_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, UNPK_duplicate_failed);
    GOTO_IF(!finfo, PHYSFS_ERR_OUT_OF_MEMORY, UNPK_duplicate_failed);

    io = origfinfo->io->duplicate(origfinfo->io);
    if (!io) goto UNPK_duplicate_failed;
    finfo->io = io;
    finfo->entry = origfinfo->entry;
    finfo->curPos = 0;
    memcpy(retval, _io, sizeof (PHYSFS_Io));
    retval->opaque = finfo;
    return retval;

UNPK_duplicate_failed:
    if (finfo != NULL) allocator.Free(finfo);
    if (retval != NULL) allocator.Free(retval);
    if (io != NULL) io->destroy(io);
    return NULL;
} /* UNPK_duplicate */

static int UNPK_flush(PHYSFS_Io *io) { return 1;  /* no write support. */ }

static void UNPK_destroy(PHYSFS_Io *io)
{
    UNPKfileinfo *finfo = (UNPKfileinfo *) io->opaque;
    finfo->io->destroy(finfo->io);
    allocator.Free(finfo);
    allocator.Free(io);
} /* UNPK_destroy */


static const PHYSFS_Io UNPK_Io =
{
    CURRENT_PHYSFS_IO_API_VERSION, NULL,
    UNPK_read,
    UNPK_write,
    UNPK_seek,
    UNPK_tell,
    UNPK_length,
    UNPK_duplicate,
    UNPK_flush,
    UNPK_destroy
};


static inline UNPKentry *findEntry(UNPKinfo *info, const char *path)
{
    return (UNPKentry *) __PHYSFS_DirTreeFind(&info->tree, path);
} /* findEntry */


PHYSFS_Io *UNPK_openRead(void *opaque, const char *name)
{
    PHYSFS_Io *retval = NULL;
    UNPKinfo *info = (UNPKinfo *) opaque;
    UNPKfileinfo *finfo = NULL;
    UNPKentry *entry = findEntry(info, name);

    BAIL_IF_ERRPASS(!entry, NULL);
    BAIL_IF(entry->tree.isdir, PHYSFS_ERR_NOT_A_FILE, NULL);

    retval = (PHYSFS_Io *) allocator.Malloc(sizeof (PHYSFS_Io));
    GOTO_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, UNPK_openRead_failed);

    finfo = (UNPKfileinfo *) allocator.Malloc(sizeof (UNPKfileinfo));
    GOTO_IF(!finfo, PHYSFS_ERR_OUT_OF_MEMORY, UNPK_openRead_failed);

    finfo->io = info->io->duplicate(info->io);
    GOTO_IF_ERRPASS(!finfo->io, UNPK_openRead_failed);

    if (!finfo->io->seek(finfo->io, entry->startPos))
        goto UNPK_openRead_failed;

    finfo->curPos = 0;
    finfo->entry = entry;

    memcpy(retval, &UNPK_Io, sizeof (*retval));
    retval->opaque = finfo;
    return retval;

UNPK_openRead_failed:
    if (finfo != NULL)
    {
        if (finfo->io != NULL)
            finfo->io->destroy(finfo->io);
        allocator.Free(finfo);
    } /* if */

    if (retval != NULL)
        allocator.Free(retval);

    return NULL;
} /* UNPK_openRead */


PHYSFS_Io *UNPK_openWrite(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, NULL);
} /* UNPK_openWrite */


PHYSFS_Io *UNPK_openAppend(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, NULL);
} /* UNPK_openAppend */


int UNPK_remove(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, 0);
} /* UNPK_remove */


int UNPK_mkdir(void *opaque, const char *name)
{
    BAIL(PHYSFS_ERR_READ_ONLY, 0);
} /* UNPK_mkdir */


int UNPK_stat(void *opaque, const char *path, PHYSFS_Stat *stat)
{
    UNPKinfo *info = (UNPKinfo *) opaque;
    const UNPKentry *entry = findEntry(info, path);

    BAIL_IF_ERRPASS(!entry, 0);

    if (entry->tree.isdir)
    {
        stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
        stat->filesize = 0;
    } /* if */
    else
    {
        stat->filetype = PHYSFS_FILETYPE_REGULAR;
        stat->filesize = entry->size;
    } /* else */

    stat->modtime = entry->mtime;
    stat->createtime = entry->ctime;
    stat->accesstime = -1;
    stat->readonly = 1;

    return 1;
} /* UNPK_stat */


void *UNPK_addEntry(void *opaque, char *name, const int isdir,
                    const PHYSFS_sint64 ctime, const PHYSFS_sint64 mtime,
                    const PHYSFS_uint64 pos, const PHYSFS_uint64 len)
{
    UNPKinfo *info = (UNPKinfo *) opaque;
    UNPKentry *entry;

    entry = (UNPKentry *) __PHYSFS_DirTreeAdd(&info->tree, name, isdir);
    BAIL_IF_ERRPASS(!entry, NULL);

    entry->startPos = isdir ? 0 : pos;
    entry->size = isdir ? 0 : len;
    entry->ctime = ctime;
    entry->mtime = mtime;

    return entry;
} /* UNPK_addEntry */


void *UNPK_openArchive(PHYSFS_Io *io, const int case_sensitive, const int only_usascii)
{
    UNPKinfo *info = (UNPKinfo *) allocator.Malloc(sizeof (UNPKinfo));
    BAIL_IF(!info, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    if (!__PHYSFS_DirTreeInit(&info->tree, sizeof (UNPKentry), case_sensitive, only_usascii))
    {
        allocator.Free(info);
        return NULL;
    } /* if */

    info->io = io;

    return info;
} /* UNPK_openArchive */

/* end of physfs_archiver_unpacked.c ... */

