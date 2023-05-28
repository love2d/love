/*
 * SLB support routines for PhysicsFS.
 *
 * This driver handles SLB archives ("slab files"). This uncompressed format
 * is used in I-War / Independence War and Independence War: Defiance.
 *
 * The format begins with four zero bytes (version?), the file count and the
 * location of the table of contents. Each ToC entry contains a 64-byte buffer
 * containing a zero-terminated filename, the offset of the data, and its size.
 * All the filenames begin with the separator character '\'. 
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 * This file written by Aleksi Nurmi, based on the GRP archiver by
 * Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_SLB

static int slbLoadEntries(PHYSFS_Io *io, const PHYSFS_uint32 count, void *arc)
{
    PHYSFS_uint32 i;
    for (i = 0; i < count; i++)
    {
        PHYSFS_uint32 pos;
        PHYSFS_uint32 size;
        char name[64];
        char backslash;
        char *ptr;

        /* don't include the '\' in the beginning */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &backslash, 1), 0);
        BAIL_IF(backslash != '\\', PHYSFS_ERR_CORRUPT, 0);

        /* read the rest of the buffer, 63 bytes */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, name, 63), 0);
        name[63] = '\0'; /* in case the name lacks the null terminator */

        /* convert backslashes */
        for (ptr = name; *ptr; ptr++)
        {
            if (*ptr == '\\')
                *ptr = '/';
        } /* for */

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &pos, 4), 0);
        pos = PHYSFS_swapULE32(pos);

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &size, 4), 0);
        size = PHYSFS_swapULE32(size);

        BAIL_IF_ERRPASS(!UNPK_addEntry(arc, name, 0, -1, -1, pos, size), 0);
    } /* for */

    return 1;
} /* slbLoadEntries */


static void *SLB_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    PHYSFS_uint32 version;
    PHYSFS_uint32 count;
    PHYSFS_uint32 tocPos;
    void *unpkarc;

    /* There's no identifier on an SLB file, so we assume it's _not_ if the
       file count or tocPos is zero. Beyond that, we'll assume it's
       bogus/corrupt if the entries' filenames don't start with '\' or the
       tocPos is past the end of the file (seek will fail). This probably
       covers all meaningful cases where we would accidentally accept a non-SLB
       file with this archiver. */

    assert(io != NULL);  /* shouldn't ever happen. */

    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);

    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &version, sizeof (version)), NULL);
    version = PHYSFS_swapULE32(version);
    BAIL_IF(version != 0, PHYSFS_ERR_UNSUPPORTED, NULL);

    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &count, sizeof (count)), NULL);
    count = PHYSFS_swapULE32(count);
    BAIL_IF(!count, PHYSFS_ERR_UNSUPPORTED, NULL);

    /* offset of the table of contents */
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &tocPos, sizeof (tocPos)), NULL);
    tocPos = PHYSFS_swapULE32(tocPos);
    BAIL_IF(!tocPos, PHYSFS_ERR_UNSUPPORTED, NULL);
    
    /* seek to the table of contents */
    BAIL_IF_ERRPASS(!io->seek(io, tocPos), NULL);

    /* !!! FIXME: check case_sensitive and only_usascii params for this archive. */
    unpkarc = UNPK_openArchive(io, 1, 0);
    BAIL_IF_ERRPASS(!unpkarc, NULL);

    if (!slbLoadEntries(io, count, unpkarc))
    {
        UNPK_abandonArchive(unpkarc);
        return NULL;
    } /* if */

    *claimed = 1;  /* oh well. */

    return unpkarc;
} /* SLB_openArchive */


const PHYSFS_Archiver __PHYSFS_Archiver_SLB =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "SLB",
        "I-War / Independence War Slab file",
        "Aleksi Nurmi <aleksi.nurmi@gmail.com>",
        "https://bitbucket.org/ahnurmi/",
        0,  /* supportsSymlinks */
    },
    SLB_openArchive,
    UNPK_enumerate,
    UNPK_openRead,
    UNPK_openWrite,
    UNPK_openAppend,
    UNPK_remove,
    UNPK_mkdir,
    UNPK_stat,
    UNPK_closeArchive
};

#endif  /* defined PHYSFS_SUPPORTS_SLB */

/* end of physfs_archiver_slb.c ... */
