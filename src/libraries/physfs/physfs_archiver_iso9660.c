/*
 * ISO9660 support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file originally written by Christoph Nelles, but was largely
 *  rewritten by Ryan C. Gordon (so please harass Ryan about bugs and not
 *  Christoph).
 */

/*
 * Handles CD-ROM disk images (and raw CD-ROM devices).
 *
 * Not supported:
 * - Rock Ridge (needed for sparse files, device nodes and symlinks, etc).
 * - Non 2048 Sectors
 * - TRANS.TBL (maps 8.3 filenames on old discs to long filenames).
 * - Multiextents (4gb max file size without it).
 * - UDF
 *
 * Deviations from the standard
 * - Ignores mandatory sort order
 * - Allows various invalid file names
 *
 * Problems
 * - Ambiguities in the standard
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

#if PHYSFS_SUPPORTS_ISO9660

#include <time.h>

/* ISO9660 often stores values in both big and little endian formats: little
   first, followed by big. While technically there might be different values
   in each, we just always use the littleendian ones and swap ourselves. The
   fields aren't aligned anyhow, so you have to serialize them in any case
   to avoid crashes on many CPU archs in any case. */

static int iso9660LoadEntries(PHYSFS_Io *io, const int joliet,
                              const char *base, const PHYSFS_uint64 dirstart,
                              const PHYSFS_uint64 dirend, void *unpkarc);

static int iso9660AddEntry(PHYSFS_Io *io, const int joliet, const int isdir,
                           const char *base, PHYSFS_uint8 *fname,
                           const int fnamelen, const PHYSFS_sint64 ts,
                           const PHYSFS_uint64 pos, const PHYSFS_uint64 len,
                           void *unpkarc)
{
    char *fullpath;
    char *fnamecpy;
    size_t baselen;
    size_t fullpathlen;
    void *entry;
    int i;

    if (fnamelen == 1 && ((fname[0] == 0) || (fname[0] == 1)))
        return 1;  /* Magic that represents "." and "..", ignore */

    BAIL_IF(fnamelen == 0, PHYSFS_ERR_CORRUPT, 0);
    assert(fnamelen > 0);
    assert(fnamelen <= 255);
    BAIL_IF(joliet && (fnamelen % 2), PHYSFS_ERR_CORRUPT, 0);

    /* Joliet is UCS-2, so at most UTF-8 will double the byte size */
    baselen = strlen(base);
    fullpathlen = baselen + (fnamelen * (joliet ? 2 : 1)) + 2;
    fullpath = (char *) __PHYSFS_smallAlloc(fullpathlen);
    BAIL_IF(!fullpath, PHYSFS_ERR_OUT_OF_MEMORY, 0);
    fnamecpy = fullpath;
    if (baselen > 0)
    {
        snprintf(fullpath, fullpathlen, "%s/", base);
        fnamecpy += baselen + 1;
        fullpathlen -= baselen - 1;
    } /* if */

    if (joliet)
    {
        PHYSFS_uint16 *ucs2 = (PHYSFS_uint16 *) fname;
        int total = fnamelen / 2;
        for (i = 0; i < total; i++)
            ucs2[i] = PHYSFS_swapUBE16(ucs2[i]);
        ucs2[total] = '\0';
        PHYSFS_utf8FromUcs2(ucs2, fnamecpy, fullpathlen);
    } /* if */
    else
    {
        for (i = 0; i < fnamelen; i++)
        {
            /* We assume the filenames are low-ASCII; consider the archive
               corrupt if we see something above 127, since we don't know the
               encoding. (We can change this later if we find out these exist
               and are intended to be, say, latin-1 or UTF-8 encoding). */
            BAIL_IF(fname[i] > 127, PHYSFS_ERR_CORRUPT, 0);
            fnamecpy[i] = fname[i];
        } /* for */
        fnamecpy[fnamelen] = '\0';

        if (!isdir)
        {
            /* find last SEPARATOR2 */
            char *ptr = strrchr(fnamecpy, ';');
            if (ptr && (ptr != fnamecpy))
                *(ptr--) = '\0';
            else
                ptr = fnamecpy + (fnamelen - 1);

            /* chop out any trailing '.', as done in all implementations */
            if (*ptr == '.')
                *ptr = '\0';
        } /* if */
    } /* else */

    entry = UNPK_addEntry(unpkarc, fullpath, isdir, ts, ts, pos, len);
    if ((entry) && (isdir))
    {
        if (!iso9660LoadEntries(io, joliet, fullpath, pos, pos + len, unpkarc))
            entry = NULL;  /* so we report a failure later. */
    } /* if */

    __PHYSFS_smallFree(fullpath);
    return entry != NULL;
} /* iso9660AddEntry */

static int iso9660LoadEntries(PHYSFS_Io *io, const int joliet,
                              const char *base, const PHYSFS_uint64 dirstart,
                              const PHYSFS_uint64 dirend, void *unpkarc)
{
    PHYSFS_uint64 readpos = dirstart;

    while (1)
    {
        PHYSFS_uint8 recordlen;
        PHYSFS_uint8 extattrlen;
        PHYSFS_uint32 extent;
        PHYSFS_uint32 datalen;
        PHYSFS_uint8 ignore[4];
        PHYSFS_uint8 year, month, day, hour, minute, second, offset;
        PHYSFS_uint8 flags;
        PHYSFS_uint8 fnamelen;
        PHYSFS_uint8 fname[256];
        PHYSFS_sint64 timestamp;
        struct tm t;
        int isdir;
        int multiextent;

        BAIL_IF_ERRPASS(!io->seek(io, readpos), 0);

        /* recordlen = 0 -> no more entries or fill entry */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &recordlen, 1), 0);
        if (recordlen > 0)
            readpos += recordlen;  /* ready to seek to next record. */
        else
        {
            PHYSFS_uint64 nextpos;

            /* if we are in the last sector of the directory & it's 0 -> end */
            if ((dirend - 2048) <= (readpos - 1))
                break; /* finished */

            /* else skip to the next sector & continue; */
            nextpos = (((readpos - 1) / 2048) + 1) * 2048;

            /* whoops, can't make forward progress! */
            BAIL_IF(nextpos == readpos, PHYSFS_ERR_CORRUPT, 0);

            readpos = nextpos;
            continue;  /* start back at upper loop. */
        } /* else */

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &extattrlen, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &extent, 4), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* extent be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &datalen, 4), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* datalen be */

        /* record timestamp */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &year, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &month, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &day, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &hour, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &minute, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &second, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &offset, 1), 0);

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &flags, 1), 0);
        isdir = (flags & (1 << 1)) != 0;
        multiextent = (flags & (1 << 7)) != 0;
        BAIL_IF(multiextent, PHYSFS_ERR_UNSUPPORTED, 0);  /* !!! FIXME */

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 1), 0); /* unit size */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 1), 0); /* interleave gap */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0); /* seqnum le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0); /* seqnum be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &fnamelen, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, fname, fnamelen), 0);

        t.tm_sec = second;
        t.tm_min = minute;
        t.tm_hour = hour;
        t.tm_mday = day;
        t.tm_mon = month - 1;
        t.tm_year = year;
        t.tm_wday = 0;
        t.tm_yday = 0;
        t.tm_isdst = -1;
        timestamp = (PHYSFS_sint64) mktime(&t);

        extent += extattrlen;  /* skip extended attribute record. */

        /* infinite loop, corrupt file? */
        BAIL_IF((extent * 2048) == dirstart, PHYSFS_ERR_CORRUPT, 0);

        if (!iso9660AddEntry(io, joliet, isdir, base, fname, fnamelen,
                             timestamp, extent * 2048, datalen, unpkarc))
        {
            return 0;
        } /* if */
    } /* while */

    return 1;
} /* iso9660LoadEntries */


static int parseVolumeDescriptor(PHYSFS_Io *io, PHYSFS_uint64 *_rootpos,
                                 PHYSFS_uint64 *_rootlen, int *_joliet,
                                 int *_claimed)
{
    PHYSFS_uint64 pos = 32768; /* start at the Primary Volume Descriptor */
    int found = 0;
    int done = 0;

    *_joliet = 0;

    while (!done)
    {
        PHYSFS_uint8 type;
        PHYSFS_uint8 identifier[5];
        PHYSFS_uint8 version;
        PHYSFS_uint8 flags;
        PHYSFS_uint8 escapeseqs[32];
        PHYSFS_uint8 ignore[32];
        PHYSFS_uint16 blocksize;
        PHYSFS_uint32 extent;
        PHYSFS_uint32 datalen;

        BAIL_IF_ERRPASS(!io->seek(io, pos), 0);
        pos += 2048;  /* each volume descriptor is 2048 bytes */

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &type, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &identifier, 5), 0);

        if (memcmp(identifier, "CD001", 5) != 0)  /* maybe not an iso? */
        {
            BAIL_IF(!*_claimed, PHYSFS_ERR_UNSUPPORTED, 0);
            continue;  /* just skip this one */
        } /* if */

        *_claimed = 1; /* okay, this is probably an iso. */

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &version, 1), 0);  /* version */
        BAIL_IF(version != 1, PHYSFS_ERR_UNSUPPORTED, 0);

        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &flags, 1), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 32), 0);  /* system id */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 32), 0);  /* volume id */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 8), 0);  /* reserved */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0);  /* space le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0);  /* space be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, escapeseqs, 32), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0);  /* setsize le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0);  /* setsize be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0);  /* seq num le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0);  /* seq num be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &blocksize, 2), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 2), 0); /* blocklen be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* pthtablen le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* pthtablen be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* pthtabpos le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* optpthtabpos le */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* pthtabpos be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* optpthtabpos be */

        /* root directory record... */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 1), 0); /* len */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 1), 0); /* attr len */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &extent, 4), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* extent be */
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, &datalen, 4), 0);
        BAIL_IF_ERRPASS(!__PHYSFS_readAll(io, ignore, 4), 0); /* datalen be */

        /* !!! FIXME: deal with this properly. */
        blocksize = PHYSFS_swapULE32(blocksize);
        BAIL_IF(blocksize && (blocksize != 2048), PHYSFS_ERR_UNSUPPORTED, 0);

        switch (type)
        {
            case 1:  /* Primary Volume Descriptor */
            case 2:  /* Supplementary Volume Descriptor */
                if (found < type)
                {
                    *_rootpos = PHYSFS_swapULE32(extent) * 2048;
                    *_rootlen = PHYSFS_swapULE32(datalen);
                    found = type;

                    if (found == 2)  /* possible Joliet volume */
                    {
                        const PHYSFS_uint8 *s = escapeseqs;
                        *_joliet = !(flags & 1) &&
                            (s[0] == 0x25) && (s[1] == 0x2F) &&
                            ((s[2] == 0x40) || (s[2] == 0x43) || (s[2] == 0x45));
                    } /* if */
                } /* if */
                break;

            case 255: /* type 255 terminates the volume descriptor list */
                done = 1;
                break;

            default:
                break;  /* skip unknown types. */
        } /* switch */
    } /* while */

    BAIL_IF(!found, PHYSFS_ERR_CORRUPT, 0);

    return 1;
} /* parseVolumeDescriptor */


static void *ISO9660_openArchive(PHYSFS_Io *io, const char *filename,
                                 int forWriting, int *claimed)
{
    PHYSFS_uint64 rootpos = 0;
    PHYSFS_uint64 len = 0;
    int joliet = 0;
    void *unpkarc = NULL;

    assert(io != NULL);  /* shouldn't ever happen. */

    BAIL_IF(forWriting, PHYSFS_ERR_READ_ONLY, NULL);

    if (!parseVolumeDescriptor(io, &rootpos, &len, &joliet, claimed))
        return NULL;

    unpkarc = UNPK_openArchive(io);
    BAIL_IF_ERRPASS(!unpkarc, NULL);

    if (!iso9660LoadEntries(io, joliet, "", rootpos, rootpos + len, unpkarc))
    {
        UNPK_abandonArchive(unpkarc);
        return NULL;
    } /* if */

    return unpkarc;
} /* ISO9660_openArchive */


const PHYSFS_Archiver __PHYSFS_Archiver_ISO9660 =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "ISO",
        "ISO9660 image file",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/physfs/",
        0,  /* supportsSymlinks */
    },
    ISO9660_openArchive,
    UNPK_enumerate,
    UNPK_openRead,
    UNPK_openWrite,
    UNPK_openAppend,
    UNPK_remove,
    UNPK_mkdir,
    UNPK_stat,
    UNPK_closeArchive
};

#endif  /* defined PHYSFS_SUPPORTS_ISO9660 */

/* end of physfs_archiver_iso9660.c ... */

