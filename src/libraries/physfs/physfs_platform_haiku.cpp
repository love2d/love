/*
 * Haiku platform-dependent support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_HAIKU

#include <os/kernel/OS.h>
#include <os/app/Roster.h>
#include <os/storage/Volume.h>
#include <os/storage/VolumeRoster.h>
#include <os/storage/Directory.h>
#include <os/storage/Entry.h>
#include <os/storage/Path.h>
#include <os/kernel/fs_info.h>
#include <os/device/scsi.h>

#include <errno.h>
#include <unistd.h>

#include "physfs_internal.h"

int __PHYSFS_platformInit(void)
{
    return 1;  /* always succeed. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    /* no-op */
} /* __PHYSFS_platformDeinit */


static char *getMountPoint(const char *devname, char *buf, size_t bufsize)
{
    BVolumeRoster mounts;
    BVolume vol;

    mounts.Rewind();
    while (mounts.GetNextVolume(&vol) == B_NO_ERROR)
    {
        fs_info fsinfo;
        fs_stat_dev(vol.Device(), &fsinfo);
        if (strcmp(devname, fsinfo.device_name) == 0)
        {
            BDirectory directory;
            BEntry entry;
            BPath path;
            const char *str;

            if ( (vol.GetRootDirectory(&directory) < B_OK) ||
                 (directory.GetEntry(&entry) < B_OK) ||
                 (entry.GetPath(&path) < B_OK) ||
                 ( (str = path.Path()) == NULL) )
                return NULL;

            strncpy(buf, str, bufsize-1);
            buf[bufsize-1] = '\0';
            return buf;
        } /* if */
    } /* while */

    return NULL;
} /* getMountPoint */


    /*
     * This function is lifted from Simple Directmedia Layer (SDL):
     *  https://www.libsdl.org/  ... this is zlib-licensed code, too.
     */
static void tryDir(const char *d, PHYSFS_StringCallback callback, void *data)
{
    BDirectory dir;
    dir.SetTo(d);
    if (dir.InitCheck() != B_NO_ERROR)
        return;

    dir.Rewind();
    BEntry entry;
    while (dir.GetNextEntry(&entry) >= 0)
    {
        BPath path;
        const char *name;
        entry_ref e;

        if (entry.GetPath(&path) != B_NO_ERROR)
            continue;

        name = path.Path();

        if (entry.GetRef(&e) != B_NO_ERROR)
            continue;

        if (entry.IsDirectory())
        {
            if (strcmp(e.name, "floppy") != 0)
                tryDir(name, callback, data);
            continue;
        } /* if */

        const int devfd = open(name, O_RDONLY);
        if (devfd < 0)
            continue;

        device_geometry g;
        const int rc = ioctl(devfd, B_GET_GEOMETRY, &g, sizeof (g));
        close(devfd);
        if (rc < 0)
            continue;

        if (g.device_type != B_CD)
            continue;

        char mntpnt[B_FILE_NAME_LENGTH];
        if (getMountPoint(name, mntpnt, sizeof (mntpnt)))
            callback(data, mntpnt);
    } /* while */
} /* tryDir */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    tryDir("/dev/disk", cb, data);
} /* __PHYSFS_platformDetectAvailableCDs */


static team_id getTeamID(void)
{
    thread_info info;
    thread_id tid = find_thread(NULL);
    get_thread_info(tid, &info);
    return info.team;
} /* getTeamID */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    image_info info;
    int32 cookie = 0;

    while (get_next_image_info(0, &cookie, &info) == B_OK)
    {
        if (info.type == B_APP_IMAGE)
            break;
    } /* while */

    BEntry entry(info.name, true);
    BPath path;
    status_t rc = entry.GetPath(&path);  /* (path) now has binary's path. */
    assert(rc == B_OK);
    rc = path.GetParent(&path); /* chop filename, keep directory. */
    assert(rc == B_OK);
    const char *str = path.Path();
    assert(str != NULL);
    const size_t len = strlen(str);
    char *retval = (char *) allocator.Malloc(len + 2);
    BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, str);
    retval[len] = '/';
    retval[len+1] = '\0';
    return retval;
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    const char *userdir = __PHYSFS_getUserDir();
    const char *append = "config/settings/";
    const size_t len = strlen(userdir) + strlen(append) + strlen(app) + 2;
    char *retval = (char *) allocator.Malloc(len);
    BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    snprintf(retval, len, "%s%s%s/", userdir, append, app);
    return retval;
} /* __PHYSFS_platformCalcPrefDir */

#endif  /* PHYSFS_PLATFORM_HAIKU */

/* end of physfs_platform_haiku.cpp ... */

