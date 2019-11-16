/*
 * QNX support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

/* This is tested against QNX 7 at the moment. */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_QNX

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

#include "physfs_internal.h"

int __PHYSFS_platformInit(void)
{
    return 1;  /* always succeed. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    /* no-op */
} /* __PHYSFS_platformDeinit */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    char *retval = (char *) allocator.Malloc(PATH_MAX+1);
    if (retval == NULL)
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    else
    {
        const int fd = open("/proc/self/exefile", O_RDONLY);
        const ssize_t br = (fd == -1) ? -1 : read(fd, retval, PATH_MAX);
        char *ptr;

        if (fd != -1)
            close(fd);

        if ((br < 0) || (br > PATH_MAX))
        {
            allocator.Free(retval);
            BAIL(PHYSFS_ERR_OS_ERROR, NULL);
        } /* if */

        retval[br] = '\0';
        ptr = strrchr(retval, '/');
        if (ptr == NULL)  /* uhoh! */
        {
            allocator.Free(retval);
            BAIL(PHYSFS_ERR_OS_ERROR, NULL);
        } /* if */

        ptr[1] = '\0';  /* chop off filename, leave dirs and '/' */

        ptr = (char *) allocator.Realloc(retval, (ptr - retval) + 2);
        if (ptr != NULL)  /* just shrinking buffer; don't care if it failed. */
            retval = ptr;
    } /* else */

    return retval;
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    /* !!! FIXME: this might be wrong; I don't know if there's a better method
        on QNX, or if it follows XDG specs, etc. */
    char *retval = NULL;
    const char *home = __PHYSFS_getUserDir();
    if (home)
    {
        const size_t len = strlen(home) + strlen(app) + 3;
        retval = (char *) allocator.Malloc(len);
        BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        snprintf(retval, len, "%s.%s/", home, app);
    } /* if */
    return retval;
} /* __PHYSFS_platformCalcPrefDir */


#if !PHYSFS_NO_CDROM_SUPPORT
#include <devctl.h>
#include <sys/dcmd_blk.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

static void checkPathForCD(const char *path, PHYSFS_StringCallback cb, void *d)
{
    struct stat statbuf;
    int fd;

    /* The devctl() thing is QNX-specific. In this case, we query what is
       probably the mountpoint for the device. statvfs() on that mountpoint
       will tell use its filesystem type. */

    if ( (stat(path, &statbuf) == 0) &&
         (S_ISBLK(statbuf.st_mode)) &&
         ((fd = open(path, O_RDONLY | O_NONBLOCK)) != -1) )
    {
        char mnt[256] = { 0 };
        const int rc = devctl(fd, DCMD_FSYS_MOUNTED_BY, mnt, sizeof (mnt), 0);
        close(fd);
        if ( (rc == EOK) && (mnt[0]) )
        {
            struct statvfs statvfsbuf;
            if (statvfs(mnt, &statvfsbuf) == 0)
            {
                /* I don't know if this is a complete or accurate list. */
                const char *fstype = statvfsbuf.f_basetype;
                const int iscd = ( (strcmp(fstype, "cd") == 0) ||
                                   (strcmp(fstype, "udf") == 0) );
                if (iscd)
                    cb(d, mnt);
            } /* if */
        } /* if */
    } /* if */
} /* checkPathForCD */

static void checkDevForCD(const char *dev, PHYSFS_StringCallback cb, void *d)
{
    size_t len;
    char *path;

    if (dev[0] == '.')  /* ignore "." and ".." */
    {
        if ((dev[1] == '\0') || ((dev[1] == '.') && (dev[2] == '\0')))
            return;
    } /* if */

    len = strlen(dev) + 6;
    path = (char *) __PHYSFS_smallAlloc(len);
    if (!path)
        return;  /* oh well. */

    snprintf(path, len, "/dev/%s", dev);
    checkPathForCD(path, cb, d);
    __PHYSFS_smallFree(path);
} /* checkDevForCD */
#endif /* !PHYSFS_NO_CDROM_SUPPORT */

void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
#if !PHYSFS_NO_CDROM_SUPPORT
    DIR *dirp = opendir("/dev");
    if (dirp)
    {
        struct dirent *dent;
        while ((dent = readdir(dirp)) != NULL)
            checkDevForCD(dent->d_name, cb, data);
        closedir(dirp);
    } /* if */
#endif
} /* __PHYSFS_platformDetectAvailableCDs */

#endif /* PHYSFS_PLATFORM_QNX */

/* end of physfs_platform_qnx.c ... */

