/*
 * Apple platform (macOS, iOS, watchOS, etc) support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_APPLE

#include <Foundation/Foundation.h>

#include "physfs_internal.h"

int __PHYSFS_platformInit(void)
{
    return 1;  /* success. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    /* no-op */
} /* __PHYSFS_platformDeinit */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    @autoreleasepool
    {
        NSString *path = [[NSBundle mainBundle] bundlePath];
        BAIL_IF(!path, PHYSFS_ERR_OS_ERROR, NULL);
        size_t len = [path lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        char *retval = (char *) allocator.Malloc(len + 2);
        BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        [path getCString:retval maxLength:len+1 encoding:NSUTF8StringEncoding];
        retval[len] = '/';
        retval[len+1] = '\0';
        return retval;  /* whew. */
    } /* @autoreleasepool */
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    @autoreleasepool
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, TRUE);
        BAIL_IF(!paths, PHYSFS_ERR_OS_ERROR, NULL);
        NSString *path = (NSString *) paths[0];
        BAIL_IF(!path, PHYSFS_ERR_OS_ERROR, NULL);
        size_t len = [path lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        const size_t applen = strlen(app);
        char *retval = (char *) allocator.Malloc(len + applen + 3);
        BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        [path getCString:retval maxLength:len+1 encoding:NSUTF8StringEncoding];
        snprintf(retval + len, applen + 3, "/%s/", app);
        return retval;  /* whew. */
    } /* @autoreleasepool */
} /* __PHYSFS_platformCalcPrefDir */


/* CD-ROM detection code... */

/*
 * Code based on sample from Apple Developer Connection:
 *  https://developer.apple.com/samplecode/Sample_Code/Devices_and_Hardware/Disks/VolumeToBSDNode/VolumeToBSDNode.c.htm
 */

#if !defined(PHYSFS_NO_CDROM_SUPPORT)

#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IODVDMedia.h>
#include <sys/mount.h>

static int darwinIsWholeMedia(io_service_t service)
{
    int retval = 0;
    CFTypeRef wholeMedia;

    if (!IOObjectConformsTo(service, kIOMediaClass))
        return 0;
        
    wholeMedia = IORegistryEntryCreateCFProperty(service,
                                                 CFSTR(kIOMediaWholeKey),
                                                 NULL, 0);
    if (wholeMedia == NULL)
        return 0;

    retval = CFBooleanGetValue(wholeMedia);
    CFRelease(wholeMedia);

    return retval;
} /* darwinIsWholeMedia */


static int darwinIsMountedDisc(char *bsdName, mach_port_t masterPort)
{
    int retval = 0;
    CFMutableDictionaryRef matchingDict;
    kern_return_t rc;
    io_iterator_t iter;
    io_service_t service;

    if ((matchingDict = IOBSDNameMatching(masterPort, 0, bsdName)) == NULL)
        return 0;

    rc = IOServiceGetMatchingServices(masterPort, matchingDict, &iter);
    if ((rc != KERN_SUCCESS) || (!iter))
        return 0;

    service = IOIteratorNext(iter);
    IOObjectRelease(iter);
    if (!service)
        return 0;

    rc = IORegistryEntryCreateIterator(service, kIOServicePlane,
             kIORegistryIterateRecursively | kIORegistryIterateParents, &iter);
    
    if (!iter)
        return 0;

    if (rc != KERN_SUCCESS)
    {
        IOObjectRelease(iter);
        return 0;
    } /* if */

    IOObjectRetain(service);  /* add an extra object reference... */

    do
    {
        if (darwinIsWholeMedia(service))
        {
            if ( (IOObjectConformsTo(service, kIOCDMediaClass)) ||
                 (IOObjectConformsTo(service, kIODVDMediaClass)) )
            {
                retval = 1;
            } /* if */
        } /* if */
        IOObjectRelease(service);
    } while ((service = IOIteratorNext(iter)) && (!retval));
                
    IOObjectRelease(iter);
    IOObjectRelease(service);

    return retval;
} /* darwinIsMountedDisc */

#endif /* !defined(PHYSFS_NO_CDROM_SUPPORT) */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
#if !defined(PHYSFS_NO_CDROM_SUPPORT)
    const char *devPrefix = "/dev/";
    const int prefixLen = strlen(devPrefix);
    mach_port_t masterPort = 0;
    struct statfs *mntbufp;
    int i, mounts;

    if (IOMasterPort(MACH_PORT_NULL, &masterPort) != KERN_SUCCESS)
        BAIL(PHYSFS_ERR_OS_ERROR, ) /*return void*/;

    mounts = getmntinfo(&mntbufp, MNT_WAIT);  /* NOT THREAD SAFE! */
    for (i = 0; i < mounts; i++)
    {
        char *dev = mntbufp[i].f_mntfromname;
        char *mnt = mntbufp[i].f_mntonname;
        if (strncmp(dev, devPrefix, prefixLen) != 0)  /* a virtual device? */
            continue;

        dev += prefixLen;
        if (darwinIsMountedDisc(dev, masterPort))
            cb(data, mnt);
    } /* for */
#endif /* !defined(PHYSFS_NO_CDROM_SUPPORT) */
} /* __PHYSFS_platformDetectAvailableCDs */

#endif /* PHYSFS_PLATFORM_APPLE */

/* end of physfs_platform_apple.m ... */

