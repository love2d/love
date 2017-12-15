/*
 * HOG support routines for PhysicsFS.
 *
 * This driver handles Descent I/II HOG archives.
 *
 * The format is very simple:
 *
 *   The file always starts with the 3-byte signature "DHF" (Descent
 *   HOG file). After that the files of a HOG are just attached after
 *   another, divided by a 17 bytes header, which specifies the name
 *   and length (in bytes) of the forthcoming file! So you just read
 *   the header with its information of how big the following file is,
 *   and then skip exact that number of bytes to get to the next file
 *   in that HOG.
 *
 *    char sig[3] = {'D', 'H', 'F'}; // "DHF"=Descent HOG File
 *
 *    struct {
 *     char file_name[13]; // Filename, padded to 13 bytes with 0s
 *     int file_size; // filesize in bytes
 *     char data[file_size]; // The file data
 *    } FILE_STRUCT; // Repeated until the end of the file.
 *
 * (That info is from http://www.descent2.com/ddn/specs/hog/)
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Bradley Bell.
 *  Based on grp.c by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_HOG

static int hogLoadEntries(PHYSFS_Io *io, void *arc)
{
    const PHYSFS_uint64 iolen = io->length(io);
    PHYSFS_uint32 pos = 3;

    while (pos < iolen)
    {
        PHYSFS_uint32 size;
        char name[13];

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, name, 13), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &size, 4), 0);
        name[12] = '\0';  /* just in case. */
        pos += 13 + 4;

        size = PHYSFS_swapULE32(size);
        BAIL_IF_ERRPASS(!UNPK_addEntry(arc, name, 0, -1, -1, pos, size), 0);
        pos += size;

        /* skip over entry */
        BAIL_IF_ERRPASS(!io->seek(io, pos), 0);
    } /* while */

    return 1;
} /* hogLoadEntries */


static void *HOG_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    PHYSFS_uint8 buf[3];
    void *unpkarc = NULL;

    assert(io != NULL);  /* shouldn't ever happen. */
    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, buf, 3), NULL);
    BAIL_IF(memcmp(buf, "DHF", 3) != 0, PHYSFS_ERR_UNSUPPORTED, NULL);

    *claimed = 1;

    unpkarc = UNPK_openArchive(io);
    BAIL_IF_ERRPASS(!unpkarc, NULL);

    if (!hogLoadEntries(io, unpkarc))
    {
        UNPK_abandonArchive(unpkarc);
        return NULL;
    } /* if */

    return unpkarc;
} /* HOG_openArchive */


const PHYSFS_Archiver __PHYSFS_Archiver_HOG =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "HOG",
        "Descent I/II HOG file format",
        "Bradley Bell <btb@icculus.org>",
        "https://icculus.org/physfs/",
        0,  /* supportsSymlinks */
    },
    HOG_openArchive,
    UNPK_enumerate,
    UNPK_openRead,
    UNPK_openWrite,
    UNPK_openAppend,
    UNPK_remove,
    UNPK_mkdir,
    UNPK_stat,
    UNPK_closeArchive
};

#endif  /* defined PHYSFS_SUPPORTS_HOG */

/* end of physfs_archiver_hog.c ... */

