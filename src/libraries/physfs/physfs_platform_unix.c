/*
 * Unix support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_UNIX

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#if PHYSFS_NO_CDROM_SUPPORT
#elif PHYSFS_PLATFORM_LINUX
#  define PHYSFS_HAVE_MNTENT_H 1
#elif defined __CYGWIN__
#  define PHYSFS_HAVE_MNTENT_H 1
#elif PHYSFS_PLATFORM_SOLARIS
#  define PHYSFS_HAVE_SYS_MNTTAB_H 1
#elif PHYSFS_PLATFORM_BSD
#  define PHYSFS_HAVE_SYS_UCRED_H 1
#else
#  warning No CD-ROM support included. Either define your platform here,
#  warning  or define PHYSFS_NO_CDROM_SUPPORT=1 to confirm this is intentional.
#endif

#ifdef PHYSFS_HAVE_SYS_UCRED_H
#  ifdef PHYSFS_HAVE_MNTENT_H
#    undef PHYSFS_HAVE_MNTENT_H /* don't do both... */
#  endif
#  include <sys/mount.h>
#  include <sys/ucred.h>
#endif

#ifdef PHYSFS_HAVE_MNTENT_H
#include <mntent.h>
#endif

#ifdef PHYSFS_HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif

#ifdef PHYSFS_PLATFORM_FREEBSD
#include <sys/sysctl.h>
#endif


#include "physfs_internal.h"

int __PHYSFS_platformInit(void)
{
    return 1;  /* always succeed. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    /* no-op */
} /* __PHYSFS_platformDeinit */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
#if (defined PHYSFS_NO_CDROM_SUPPORT)
    /* no-op. */

#elif (defined PHYSFS_HAVE_SYS_UCRED_H)
    int i;
    struct statfs *mntbufp = NULL;
    int mounts = getmntinfo(&mntbufp, MNT_NOWAIT);

    for (i = 0; i < mounts; i++)
    {
        int add_it = 0;

        if (strcmp(mntbufp[i].f_fstypename, "iso9660") == 0)
            add_it = 1;
        else if (strcmp( mntbufp[i].f_fstypename, "cd9660") == 0)
            add_it = 1;

        /* add other mount types here */

        if (add_it)
            cb(data, mntbufp[i].f_mntonname);
    } /* for */

#elif (defined PHYSFS_HAVE_MNTENT_H)
    FILE *mounts = NULL;
    struct mntent *ent = NULL;

    mounts = setmntent("/etc/mtab", "r");
    BAIL_IF(mounts == NULL, PHYSFS_ERR_IO, /*return void*/);

    while ( (ent = getmntent(mounts)) != NULL )
    {
        int add_it = 0;
        if (strcmp(ent->mnt_type, "iso9660") == 0)
            add_it = 1;
        else if (strcmp(ent->mnt_type, "udf") == 0)
            add_it = 1;

        /* !!! FIXME: these might pick up floppy drives, right? */
        else if (strcmp(ent->mnt_type, "auto") == 0)
            add_it = 1;
        else if (strcmp(ent->mnt_type, "supermount") == 0)
            add_it = 1;

        /* add other mount types here */

        if (add_it)
            cb(data, ent->mnt_dir);
    } /* while */

    endmntent(mounts);

#elif (defined PHYSFS_HAVE_SYS_MNTTAB_H)
    FILE *mounts = fopen(MNTTAB, "r");
    struct mnttab ent;

    BAIL_IF(mounts == NULL, PHYSFS_ERR_IO, /*return void*/);
    while (getmntent(mounts, &ent) == 0)
    {
        int add_it = 0;
        if (strcmp(ent.mnt_fstype, "hsfs") == 0)
            add_it = 1;

        /* add other mount types here */

        if (add_it)
            cb(data, ent.mnt_mountp);
    } /* while */

    fclose(mounts);
#endif
} /* __PHYSFS_platformDetectAvailableCDs */


/*
 * See where program (bin) resides in the $PATH specified by (envr).
 *  returns a copy of the first element in envr that contains it, or NULL
 *  if it doesn't exist or there were other problems. PHYSFS_SetError() is
 *  called if we have a problem.
 *
 * (envr) will be scribbled over, and you are expected to allocator.Free() the
 *  return value when you're done with it.
 */
static char *findBinaryInPath(const char *bin, char *envr)
{
    size_t alloc_size = 0;
    char *exe = NULL;
    char *start = envr;
    char *ptr;

    assert(bin != NULL);
    assert(envr != NULL);

    do
    {
        size_t size;
        size_t binlen;

        ptr = strchr(start, ':');  /* find next $PATH separator. */
        if (ptr)
            *ptr = '\0';

        binlen = strlen(bin);
        size = strlen(start) + binlen + 2;
        if (size >= alloc_size)
        {
            char *x = (char *) allocator.Realloc(exe, size);
            if (!x)
            {
                if (exe != NULL)
                    allocator.Free(exe);
                BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
            } /* if */

            alloc_size = size;
            exe = x;
        } /* if */

        /* build full binary path... */
        strcpy(exe, start);
        if ((exe[0] == '\0') || (exe[strlen(exe) - 1] != '/'))
            strcat(exe, "/");
        strcat(exe, bin);

        if (access(exe, X_OK) == 0)  /* Exists as executable? We're done. */
        {
            exe[(size - binlen) - 1] = '\0'; /* chop off filename, leave '/' */
            return exe;
        } /* if */

        start = ptr + 1;  /* start points to beginning of next element. */
    } while (ptr != NULL);

    if (exe != NULL)
        allocator.Free(exe);

    return NULL;  /* doesn't exist in path. */
} /* findBinaryInPath */


static char *readSymLink(const char *path)
{
    ssize_t len = 64;
    ssize_t rc = -1;
    char *retval = NULL;

    while (1)
    {
         char *ptr = (char *) allocator.Realloc(retval, (size_t) len);
         if (ptr == NULL)
             break;   /* out of memory. */
         retval = ptr;

         rc = readlink(path, retval, len);
         if (rc == -1)
             break;  /* not a symlink, i/o error, etc. */

         else if (rc < len)
         {
             retval[rc] = '\0';  /* readlink doesn't null-terminate. */
             return retval;  /* we're good to go. */
         } /* else if */

         len *= 2;  /* grow buffer, try again. */
    } /* while */

    if (retval != NULL)
        allocator.Free(retval);
    return NULL;
} /* readSymLink */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    char *retval = NULL;
    const char *envr = NULL;

    /* Try to avoid using argv0 unless forced to. Try system-specific stuff. */

    #if defined(PHYSFS_PLATFORM_FREEBSD)
    {
        char fullpath[PATH_MAX];
        size_t buflen = sizeof (fullpath);
        int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
        if (sysctl(mib, 4, fullpath, &buflen, NULL, 0) != -1)
            retval = __PHYSFS_strdup(fullpath);
    }
    #endif

    /* If there's a Linux-like /proc filesystem, you can get the full path to
     *  the current process from a symlink in there.
     */

    if (!retval && (access("/proc", F_OK) == 0))
    {
        retval = readSymLink("/proc/self/exe");
        if (!retval) retval = readSymLink("/proc/curproc/file");
        if (!retval) retval = readSymLink("/proc/curproc/exe");
        if (!retval) retval = readSymLink("/proc/self/path/a.out");
        if (retval == NULL)
        {
            /* older kernels don't have /proc/self ... try PID version... */
            const unsigned long long pid = (unsigned long long) getpid();
            char path[64];
            const int rc = (int) snprintf(path,sizeof(path),"/proc/%llu/exe",pid);
            if ( (rc > 0) && (rc < sizeof(path)) )
                retval = readSymLink(path);
        } /* if */
    } /* if */

    #if defined(PHYSFS_PLATFORM_SOLARIS)
    if (!retval)  /* try getexecname() if /proc didn't pan out. This may not be an absolute path! */
    {
        const char *path = getexecname();
        if ((path != NULL) && (path[0] == '/'))  /* must be absolute path... */
            retval = __PHYSFS_strdup(path);
    } /* if */
    #endif

    if (retval != NULL)  /* chop off filename. */
    {
        char *ptr = strrchr(retval, '/');
        if (ptr != NULL)
            *(ptr+1) = '\0';
        else  /* shouldn't happen, but just in case... */
        {
            allocator.Free(retval);
            retval = NULL;
        } /* else */
    } /* if */

    /* No /proc/self/exe, etc, but we have an argv[0] we can parse? */
    if ((retval == NULL) && (argv0 != NULL))
    {
        /* fast path: default behaviour can handle this. */
        if (strchr(argv0, '/') != NULL)
            return NULL;  /* higher level parses out real path from argv0. */

        /* If there's no dirsep on argv0, then look through $PATH for it. */
        envr = getenv("PATH");
        if (envr != NULL)
        {
            char *path = (char *) __PHYSFS_smallAlloc(strlen(envr) + 1);
            BAIL_IF(!path, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
            strcpy(path, envr);
            retval = findBinaryInPath(argv0, path);
            __PHYSFS_smallFree(path);
        } /* if */
    } /* if */

    if (retval != NULL)
    {
        /* try to shrink buffer... */
        char *ptr = (char *) allocator.Realloc(retval, strlen(retval) + 1);
        if (ptr != NULL)
            retval = ptr;  /* oh well if it failed. */
    } /* if */

    return retval;
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    /*
     * We use XDG's base directory spec, even if you're not on Linux.
     *  This isn't strictly correct, but the results are relatively sane
     *  in any case.
     *
     * https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
     */
    const char *envr = getenv("XDG_DATA_HOME");
    const char *append = "/";
    char *retval = NULL;
    size_t len = 0;

    if (!envr)
    {
        /* You end up with "$HOME/.local/share/Game Name 2" */
        envr = __PHYSFS_getUserDir();
        BAIL_IF_ERRPASS(!envr, NULL);  /* oh well. */
        append = ".local/share/";
    } /* if */

    len = strlen(envr) + strlen(append) + strlen(app) + 2;
    retval = (char *) allocator.Malloc(len);
    BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    snprintf(retval, len, "%s%s%s/", envr, append, app);
    return retval;
} /* __PHYSFS_platformCalcPrefDir */

#endif /* PHYSFS_PLATFORM_UNIX */

/* end of physfs_platform_unix.c ... */

