/*
 * Standard directory I/O support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_internal.h"

/* There's no PHYSFS_Io interface here. Use __PHYSFS_createNativeIo(). */



static char *cvtToDependent(const char *prepend, const char *path,
                            char *buf, const size_t buflen)
{
    BAIL_IF(buf == NULL, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    snprintf(buf, buflen, "%s%s", prepend ? prepend : "", path);

    #if !__PHYSFS_STANDARD_DIRSEP
    assert(__PHYSFS_platformDirSeparator != '/');
    {
        char *p;
        for (p = strchr(buf, '/'); p != NULL; p = strchr(p + 1, '/'))
            *p = __PHYSFS_platformDirSeparator;
    } /* if */
    #endif

    return buf;
} /* cvtToDependent */


#define CVT_TO_DEPENDENT(buf, pre, dir) { \
    const size_t len = ((pre) ? strlen((char *) pre) : 0) + strlen(dir) + 1; \
    buf = cvtToDependent((char*)pre,dir,(char*)__PHYSFS_smallAlloc(len),len); \
}



static void *DIR_openArchive(PHYSFS_Io *io, const char *name,
                             int forWriting, int *claimed)
{
    PHYSFS_Stat st;
    const char dirsep = __PHYSFS_platformDirSeparator;
    char *retval = NULL;
    const size_t namelen = strlen(name);
    const size_t seplen = 1;

    assert(io == NULL);  /* shouldn't create an Io for these. */
    BAIL_IF_ERRPASS(!__PHYSFS_platformStat(name, &st, 1), NULL);

    if (st.filetype != PHYSFS_FILETYPE_DIRECTORY)
        BAIL(PHYSFS_ERR_UNSUPPORTED, NULL);

    *claimed = 1;
    retval = allocator.Malloc(namelen + seplen + 1);
    BAIL_IF(retval == NULL, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    strcpy(retval, name);

    /* make sure there's a dir separator at the end of the string */
    if (retval[namelen - 1] != dirsep)
    {
        retval[namelen] = dirsep;
        retval[namelen + 1] = '\0';
    } /* if */

    return retval;
} /* DIR_openArchive */


static PHYSFS_EnumerateCallbackResult DIR_enumerate(void *opaque,
                         const char *dname, PHYSFS_EnumerateCallback cb,
                         const char *origdir, void *callbackdata)
{
    char *d;
    PHYSFS_EnumerateCallbackResult retval;
    CVT_TO_DEPENDENT(d, opaque, dname);
    BAIL_IF_ERRPASS(!d, PHYSFS_ENUM_ERROR);
    retval = __PHYSFS_platformEnumerate(d, cb, origdir, callbackdata);
    __PHYSFS_smallFree(d);
    return retval;
} /* DIR_enumerate */


static PHYSFS_Io *doOpen(void *opaque, const char *name, const int mode)
{
    PHYSFS_Io *io = NULL;
    char *f = NULL;

    CVT_TO_DEPENDENT(f, opaque, name);
    BAIL_IF_ERRPASS(!f, NULL);

    io = __PHYSFS_createNativeIo(f, mode);
    if (io == NULL)
    {
        const PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
        PHYSFS_Stat statbuf;
        __PHYSFS_platformStat(f, &statbuf, 0);  /* !!! FIXME: why are we stating here? */
        PHYSFS_setErrorCode(err);
    } /* if */

    __PHYSFS_smallFree(f);

    return io;
} /* doOpen */


static PHYSFS_Io *DIR_openRead(void *opaque, const char *filename)
{
    return doOpen(opaque, filename, 'r');
} /* DIR_openRead */


static PHYSFS_Io *DIR_openWrite(void *opaque, const char *filename)
{
    return doOpen(opaque, filename, 'w');
} /* DIR_openWrite */


static PHYSFS_Io *DIR_openAppend(void *opaque, const char *filename)
{
    return doOpen(opaque, filename, 'a');
} /* DIR_openAppend */


static int DIR_remove(void *opaque, const char *name)
{
    int retval;
    char *f;

    CVT_TO_DEPENDENT(f, opaque, name);
    BAIL_IF_ERRPASS(!f, 0);
    retval = __PHYSFS_platformDelete(f);
    __PHYSFS_smallFree(f);
    return retval;
} /* DIR_remove */


static int DIR_mkdir(void *opaque, const char *name)
{
    int retval;
    char *f;

    CVT_TO_DEPENDENT(f, opaque, name);
    BAIL_IF_ERRPASS(!f, 0);
    retval = __PHYSFS_platformMkDir(f);
    __PHYSFS_smallFree(f);
    return retval;
} /* DIR_mkdir */


static void DIR_closeArchive(void *opaque)
{
    allocator.Free(opaque);
} /* DIR_closeArchive */


static int DIR_stat(void *opaque, const char *name, PHYSFS_Stat *stat)
{
    int retval = 0;
    char *d;

    CVT_TO_DEPENDENT(d, opaque, name);
    BAIL_IF_ERRPASS(!d, 0);
    retval = __PHYSFS_platformStat(d, stat, 0);
    __PHYSFS_smallFree(d);
    return retval;
} /* DIR_stat */


const PHYSFS_Archiver __PHYSFS_Archiver_DIR =
{
    CURRENT_PHYSFS_ARCHIVER_API_VERSION,
    {
        "",
        "Non-archive, direct filesystem I/O",
        "Ryan C. Gordon <icculus@icculus.org>",
        "https://icculus.org/physfs/",
        1,  /* supportsSymlinks */
    },
    DIR_openArchive,
    DIR_enumerate,
    DIR_openRead,
    DIR_openWrite,
    DIR_openAppend,
    DIR_remove,
    DIR_mkdir,
    DIR_stat,
    DIR_closeArchive
};

/* end of physfs_archiver_dir.c ... */

