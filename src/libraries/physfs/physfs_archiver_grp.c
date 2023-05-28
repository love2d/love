/*
 * GRP support routines for PhysicsFS.
 *
 * This driver handles BUILD engine archives ("groupfiles"). This format
 *  (but not this driver) was put together by Ken Silverman.
 *
 * The format is simple enough. In Ken's words:
 *
 *    What's the .GRP file format?
 *
 *     The ".grp" file format is just a collection of a lot of files stored
 *     into 1 big one. I tried to make the format as simple as possible: The
 *     first 12 bytes contains my name, "KenSilverman". The next 4 bytes is
 *     the number of files that were compacted into the group file. Then for
 *     each file, there is a 16 byte structure, where the first 12 bytes are
 *     the filename, and the last 4 bytes are the file's size. The rest of
 *     the group file is just the raw data packed one after the other in the
 *     same order as the list of files.
 *
 * (That info is from http://www.advsys.net/ken/build.htm ...)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_GRP

static int grpLoadEntries(PHYSFS_Io *io, const PHYSFS_uint32 count, void *arc)
{
    PHYSFS_uint32 pos = 16 + (16 * count);  /* past sig+metadata. */
    PHYSFS_uint32 i;

    for (i = 0; i < count; i++)
    {
        char *ptr;
        char name[13];
        PHYSFS_uint32 size;
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, name, 12), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &size, 4), 0);

        name[12] = '\0';  /* name isn't null-terminated in file. */
        if ((ptr = strchr(name, ' ')) != NULL)
            *ptr = '\0';  /* trim extra spaces. */

        size = PHYSFS_swapULE32(size);
        BAIL_IF_ERRPASS(!UNPK_addEntry(arc, name, 0, -1, -1, pos, size), 0);

        pos += size;
    } /* for */

    return 1;
} /* grpLoadEntries */


static void *GRP_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    PHYSFS_uint8 buf[12];
    PHYSFS_uint32 count = 0;
    void *unpkarc = NULL;

    assert(io != NULL);  /* shouldn't ever happen. */

    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);

    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, buf, sizeof (buf)), NULL);
    if (memcmp(buf, "KenSilverman", sizeof (buf)) != 0)
        BAIL(PHYSFS_ERR_UNSUPPORTED, NULL);

    *claimed = 1;

    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &count, sizeof(count)), NULL);
    count = PHYSFS_swapULE32(count);

    unpkarc = UNPK_openArchive(io, 0, 1);
    BAIL_IF_ERRPASS(!unpkarc, NULL);

    if (!grpLoadEntries(io, count, unpkarc))
    {
        UNPK_abandonArchive(unpkarc);
        return NULL;
    } /* if */

    return unpkarc;
} /* GRP_openArchive */


const PHYSFS_Archiver __PHYSFS_Archiver_GRP =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "GRP",
        "Build engine Groupfile format",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/physfs/",
        0,  /* supportsSymlinks */
    },
    GRP_openArchive,
    UNPK_enumerate,
    UNPK_openRead,
    UNPK_openWrite,
    UNPK_openAppend,
    UNPK_remove,
    UNPK_mkdir,
    UNPK_stat,
    UNPK_closeArchive
};

#endif  /* defined PHYSFS_SUPPORTS_GRP */

/* end of physfs_archiver_grp.c ... */

