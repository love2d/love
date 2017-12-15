/*
 * VDF support routines for PhysicsFS.
 *
 * This driver handles Gothic I/II VDF archives.
 * This format (but not this driver) was designed by Piranha Bytes for
 *  use wih the ZenGin engine.
 *
 * This file was written by Francesco Bertolaccini, based on the UNPK archiver
 *  by Ryan C. Gordon and the works of degenerated1123 and Nico Bendlin.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_VDF

#include <time.h>

#define VDF_COMMENT_LENGTH 256
#define VDF_SIGNATURE_LENGTH 16
#define VDF_ENTRY_NAME_LENGTH 64
#define VDF_ENTRY_DIR 0x80000000

static const char* VDF_SIGNATURE_G1 = "PSVDSC_V2.00\r\n\r\n";
static const char* VDF_SIGNATURE_G2 = "PSVDSC_V2.00\n\r\n\r";


static inline int readui32(PHYSFS_Io *io, PHYSFS_uint32 *val)
{
    PHYSFS_uint32 v;
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &v, sizeof (v)), 0);
    *val = PHYSFS_swapULE32(v);
    return 1;
} /* readui32 */


static PHYSFS_sint64 vdfDosTimeToEpoch(const PHYSFS_uint32 dostime)
{
    /* VDF stores timestamps as 32bit DOS dates: the seconds are counted in
       2-seconds intervals and the years are counted since 1 Jan. 1980 */
    struct tm t;
    memset(&t, '\0', sizeof (t));
    t.tm_year = ((int) ((dostime >> 25) & 0x7F)) + 80; /* 1980 to 1900 */
    t.tm_mon = ((int) ((dostime >> 21) & 0xF)) - 1;  /* 1-12 to 0-11 */
    t.tm_mday = (int) ((dostime >> 16) & 0x1F);
    t.tm_hour = (int) ((dostime >> 11) & 0x1F);
    t.tm_min = (int) ((dostime >> 5) & 0x3F);
    t.tm_sec = ((int) ((dostime >> 0) & 0x1F)) * 2;  /* 2 seconds to 1. */
    return (PHYSFS_sint64) mktime(&t);
} /* vdfDosTimeToEpoch */


static int vdfLoadEntries(PHYSFS_Io *io, const PHYSFS_uint32 count,
                          const PHYSFS_sint64 ts, void *arc)
{
    PHYSFS_uint32 i;

    for (i = 0; i < count; i++)
    {
        char name[VDF_ENTRY_NAME_LENGTH + 1];
        int namei;
        PHYSFS_uint32 jump, size, type, attr;

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, name, sizeof (name) - 1), 0);
        BAIL_IF_ERRPASS(!readui32(io, &jump), 0);
        BAIL_IF_ERRPASS(!readui32(io, &size), 0);
        BAIL_IF_ERRPASS(!readui32(io, &type), 0);
        BAIL_IF_ERRPASS(!readui32(io, &attr), 0);

        /* Trim whitespace off the end of the filename */
        name[VDF_ENTRY_NAME_LENGTH] = '\0';  /* always null-terminated. */
        for (namei = VDF_ENTRY_NAME_LENGTH - 1; namei >= 0; namei--)
        {
            /* We assume the filenames are low-ASCII; consider the archive
               corrupt if we see something above 127, since we don't know the
               encoding. (We can change this later if we find out these exist
               and are intended to be, say, latin-1 or UTF-8 encoding). */
            BAIL_IF(((PHYSFS_uint8) name[namei]) > 127, PHYSFS_ERR_CORRUPT, 0);

            if (name[namei] == ' ')
                name[namei] = '\0';
            else
                break;
        } /* for */

        BAIL_IF(!name[0], PHYSFS_ERR_CORRUPT, 0);
        if (!(type & VDF_ENTRY_DIR)) {
            BAIL_IF_ERRPASS(!UNPK_addEntry(arc, name, 0, ts, ts, jump, size), 0);
        }
    } /* for */

    return 1;
} /* vdfLoadEntries */


static void *VDF_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    PHYSFS_uint8 ignore[16];
    PHYSFS_uint8 sig[VDF_SIGNATURE_LENGTH];
    PHYSFS_uint32 count, timestamp, version, dataSize, rootCatOffset;
    void *unpkarc;

    assert(io != NULL); /* shouldn't ever happen. */

    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);

    /* skip the 256-byte comment field. */
    BAIL_IF_ERRPASS(!io->seek(io, VDF_COMMENT_LENGTH), NULL);

    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, sig, sizeof (sig)), NULL);

    if ((memcmp(sig, VDF_SIGNATURE_G1, VDF_SIGNATURE_LENGTH) != 0) &&
        (memcmp(sig, VDF_SIGNATURE_G2, VDF_SIGNATURE_LENGTH) != 0))
    {
        BAIL(PHYSFS_ERR_UNSUPPORTED, NULL);
    } /* if */

    *claimed = 1;

    BAIL_IF_ERRPASS(!readui32(io, &count), NULL);
    BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), NULL);  /* numFiles */
    BAIL_IF_ERRPASS(!readui32(io, &timestamp), NULL);
    BAIL_IF_ERRPASS(!readui32(io, &dataSize), NULL);  /* dataSize */
    BAIL_IF_ERRPASS(!readui32(io, &rootCatOffset), NULL);  /* rootCatOff */
    BAIL_IF_ERRPASS(!readui32(io, &version), NULL);

    BAIL_IF(version != 0x50, PHYSFS_ERR_UNSUPPORTED, NULL);

    BAIL_IF_ERRPASS(!io->seek(io, rootCatOffset), NULL);

    unpkarc = UNPK_openArchive(io);
    BAIL_IF_ERRPASS(!unpkarc, NULL);

    if (!vdfLoadEntries(io, count, vdfDosTimeToEpoch(timestamp), unpkarc))
    {
        UNPK_abandonArchive(unpkarc);
        return NULL;
    } /* if */

    return unpkarc;
} /* VDF_openArchive */


const PHYSFS_Archiver __PHYSFS_Archiver_VDF =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "VDF",
        "Gothic I/II engine format",
        "Francesco Bertolaccini <bertolaccinifrancesco@gmail.com>",
        "https://github.com/frabert",
        0,  /* supportsSymlinks */
    },
    VDF_openArchive,
    UNPK_enumerate,
    UNPK_openRead,
    UNPK_openWrite,
    UNPK_openAppend,
    UNPK_remove,
    UNPK_mkdir,
    UNPK_stat,
    UNPK_closeArchive
};

#endif /* defined PHYSFS_SUPPORTS_VDF */

/* end of physfs_archiver_vdf.c ... */
