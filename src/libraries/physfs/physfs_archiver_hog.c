/*
 * HOG support routines for PhysicsFS.
 *
 * This driver handles Descent I/II/III HOG archives.
 *
 * The Descent I/II format is very simple:
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
 * Descent 3 moved to HOG2 format, which starts with the chars "HOG2",
 *  then 32-bits for the number of contained files, 32 bits for the offset
 *  to the first file's data, then 56 bytes of 0xFF (reserved?). Then for
 *  each file, there's 36 bytes for filename (null-terminated, rest of bytes
 *  are garbage), 32-bits unknown/reserved (always zero?), 32-bits of length
 *  of file data, 32-bits of time since Unix epoch. Then immediately following,
 *  for each file is their uncompressed content, you can find its offset
 *  by starting at the initial data offset and adding the filesize of each
 *  prior file.
 *
 * This information was found at:
 *  https://web.archive.org/web/20020213004051/http://descent-3.com/ddn/specs/hog/
 *
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 * This file written by Bradley Bell and Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_HOG

static int readui32(PHYSFS_Io *io, PHYSFS_uint32 *val)
{
    PHYSFS_uint32 v;
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &v, sizeof (v)), 0);
    *val = PHYSFS_swapULE32(v);
    return 1;
} /* readui32 */

static int hog1LoadEntries(PHYSFS_Io *io, void *arc)
{
    const PHYSFS_uint64 iolen = io->length(io);
    PHYSFS_uint32 pos = 3;

    while (pos < iolen)
    {
        PHYSFS_uint32 size;
        char name[13];

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, name, 13), 0);
        BAIL_IF_ERRPASS(!readui32(io, &size), 0);
        name[12] = '\0';  /* just in case. */
        pos += 13 + 4;

        BAIL_IF_ERRPASS(!UNPK_addEntry(arc, name, 0, -1, -1, pos, size), 0);
        pos += size;

        /* skip over entry */
        BAIL_IF_ERRPASS(!io->seek(io, pos), 0);
    } /* while */

    return 1;
} /* hogLoadEntries */

static int hog2LoadEntries(PHYSFS_Io *io, void *arc)
{
    PHYSFS_uint32 numfiles;
    PHYSFS_uint32 pos;
    PHYSFS_uint32 i;

    BAIL_IF_ERRPASS(!readui32(io, &numfiles), 0);
    BAIL_IF_ERRPASS(!readui32(io, &pos), 0);
    BAIL_IF_ERRPASS(!io->seek(io, 68), 0);  /* skip to end of header. */

    for (i = 0; i < numfiles; i++) {
        char name[37];
        PHYSFS_uint32 reserved;
        PHYSFS_uint32 size;
        PHYSFS_uint32 mtime;
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, name, 36), 0);
        BAIL_IF_ERRPASS(!readui32(io, &reserved), 0);
        BAIL_IF_ERRPASS(!readui32(io, &size), 0);
        BAIL_IF_ERRPASS(!readui32(io, &mtime), 0);
        name[36] = '\0';  /* just in case */
        BAIL_IF_ERRPASS(!UNPK_addEntry(arc, name, 0, mtime, mtime, pos, size), 0);
        pos += size;
    }

    return 1;
} /* hog2LoadEntries */


static void *HOG_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    PHYSFS_uint8 buf[3];
    void *unpkarc = NULL;
    int hog1 = 0;

    assert(io != NULL);  /* shouldn't ever happen. */
    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, buf, 3), NULL);

    if (memcmp(buf, "DHF", 3) == 0)
        hog1 = 1;  /* original HOG (Descent 1 and 2) archive */
    else
    {
        BAIL_IF(memcmp(buf, "HOG", 3) != 0, PHYSFS_ERR_UNSUPPORTED, NULL); /* Not HOG2 */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, buf, 1), NULL);
        BAIL_IF(buf[0] != '2', PHYSFS_ERR_UNSUPPORTED, NULL); /* Not HOG2 */
    } /* else */

    *claimed = 1;

    unpkarc = UNPK_openArchive(io, 0, 1);
    BAIL_IF_ERRPASS(!unpkarc, NULL);

    if (!(hog1 ? hog1LoadEntries(io, unpkarc) : hog2LoadEntries(io, unpkarc)))
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
        "Descent I/II/III HOG file format",
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

