/*
 * OS/2 support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_OS2

#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#include <os2.h>
#include <uconv.h>

#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "physfs_internal.h"

static HMODULE uconvdll = 0;
static UconvObject uconv = 0;
static int (_System *pUniCreateUconvObject)(UniChar *, UconvObject *) = NULL;
static int (_System *pUniFreeUconvObject)(UconvObject *) = NULL;
static int (_System *pUniUconvToUcs)(UconvObject,void **,size_t *, UniChar**, size_t *, size_t *) = NULL;
static int (_System *pUniUconvFromUcs)(UconvObject,UniChar **,size_t *,void **,size_t *,size_t *) = NULL;

static PHYSFS_ErrorCode errcodeFromAPIRET(const APIRET rc)
{
    switch (rc)
    {
        case NO_ERROR: return PHYSFS_ERR_OK;  /* not an error. */
        case ERROR_INTERRUPT: return PHYSFS_ERR_OK;  /* not an error. */
        case ERROR_TIMEOUT: return PHYSFS_ERR_OK;  /* not an error. */
        case ERROR_NOT_ENOUGH_MEMORY: return PHYSFS_ERR_OUT_OF_MEMORY;
        case ERROR_FILE_NOT_FOUND: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_PATH_NOT_FOUND: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_ACCESS_DENIED: return PHYSFS_ERR_PERMISSION;
        case ERROR_NOT_DOS_DISK: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_SHARING_VIOLATION: return PHYSFS_ERR_PERMISSION;
        case ERROR_CANNOT_MAKE: return PHYSFS_ERR_IO;  /* maybe this is wrong? */
        case ERROR_DEVICE_IN_USE: return PHYSFS_ERR_BUSY;
        case ERROR_OPEN_FAILED: return PHYSFS_ERR_IO;  /* maybe this is wrong? */
        case ERROR_DISK_FULL: return PHYSFS_ERR_NO_SPACE;
        case ERROR_PIPE_BUSY: return PHYSFS_ERR_BUSY;
        case ERROR_SHARING_BUFFER_EXCEEDED: return PHYSFS_ERR_IO;
        case ERROR_FILENAME_EXCED_RANGE: return PHYSFS_ERR_BAD_FILENAME;
        case ERROR_META_EXPANSION_TOO_LONG: return PHYSFS_ERR_BAD_FILENAME;
        case ERROR_TOO_MANY_HANDLES: return PHYSFS_ERR_IO;
        case ERROR_TOO_MANY_OPEN_FILES: return PHYSFS_ERR_IO;
        case ERROR_NO_MORE_SEARCH_HANDLES: return PHYSFS_ERR_IO;
        case ERROR_SEEK_ON_DEVICE: return PHYSFS_ERR_IO;
        case ERROR_NEGATIVE_SEEK: return PHYSFS_ERR_INVALID_ARGUMENT;
        case ERROR_WRITE_PROTECT: return PHYSFS_ERR_PERMISSION;
        case ERROR_WRITE_FAULT: return PHYSFS_ERR_IO;
        case ERROR_UNCERTAIN_MEDIA: return PHYSFS_ERR_IO;
        case ERROR_PROTECTION_VIOLATION: return PHYSFS_ERR_IO;
        case ERROR_BROKEN_PIPE: return PHYSFS_ERR_IO;

        /* !!! FIXME: some of these might be PHYSFS_ERR_BAD_FILENAME, etc */
        case ERROR_LOCK_VIOLATION:
        case ERROR_GEN_FAILURE:
        case ERROR_INVALID_PARAMETER:
        case ERROR_INVALID_NAME:
        case ERROR_INVALID_DRIVE:
        case ERROR_INVALID_HANDLE:
        case ERROR_INVALID_FUNCTION:
        case ERROR_INVALID_LEVEL:
        case ERROR_INVALID_CATEGORY:
        case ERROR_DUPLICATE_NAME:
        case ERROR_BUFFER_OVERFLOW:
        case ERROR_BAD_LENGTH:
        case ERROR_BAD_DRIVER_LEVEL:
        case ERROR_DIRECT_ACCESS_HANDLE:
        case ERROR_NOT_OWNER:
            return PHYSFS_ERR_OS_ERROR;

        default: break;
    } /* switch */

    return PHYSFS_ERR_OTHER_ERROR;
} /* errcodeFromAPIRET */

static char *cvtUtf8ToCodepage(const char *utf8str)
{
    const size_t len = strlen(utf8str) + 1;
    const size_t uc2buflen = len * sizeof (UniChar);
    UniChar *uc2ptr = (UniChar *) __PHYSFS_smallAlloc(uc2buflen);
    UniChar *uc2str = uc2ptr;
    char *cpptr = NULL;
    char *cpstr = NULL;
    size_t subs = 0;
    size_t unilen;

    BAIL_IF(!uc2str, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    PHYSFS_utf8ToUcs2(utf8str, (PHYSFS_uint16 *) uc2str, uc2buflen);
    for (unilen = 0; uc2str[unilen]; unilen++) { /* spin */ }
    unilen++;  /* null terminator. */

    if (!uconvdll)
    {
        /* There's really not much we can do on older OS/2s except pray this
           is latin1-compatible. */
        size_t i;
        cpptr = (char *) allocator.Malloc(unilen);
        cpstr = cpptr;
        GOTO_IF(!cpptr, PHYSFS_ERR_OUT_OF_MEMORY, failed);
        for (i = 0; i < unilen; i++)
        {
            const UniChar ch = uc2str[i];
            GOTO_IF(ch > 0xFF, PHYSFS_ERR_BAD_FILENAME, failed);
            cpptr[i] = (char) ((unsigned char) ch);
        } /* for */

        __PHYSFS_smallFree(uc2ptr);
        return cpstr;
    } /* if */
    else
    {
        int rc;
        size_t cplen = unilen * 4; /* overallocate, just in case. */
        cpptr = (char *) allocator.Malloc(cplen);
        GOTO_IF(!cpptr, PHYSFS_ERR_OUT_OF_MEMORY, failed);
        cpstr = cpptr;

        rc = pUniUconvFromUcs(uconv, &uc2str, &unilen, (void **) &cpstr, &cplen, &subs);
        GOTO_IF(rc != ULS_SUCCESS, PHYSFS_ERR_BAD_FILENAME, failed);
        GOTO_IF(subs > 0, PHYSFS_ERR_BAD_FILENAME, failed);
        assert(unilen == 0);

        __PHYSFS_smallFree(uc2ptr);
        return cpptr;
    } /* else */

failed:
    __PHYSFS_smallFree(uc2ptr);
    allocator.Free(cpptr);

    return NULL;
} /* cvtUtf8ToCodepage */

static char *cvtCodepageToUtf8(const char *cpstr)
{
    const size_t len = strlen(cpstr) + 1;
    char *retvalbuf = (char *) allocator.Malloc(len * 4);
    char *retval = NULL;

    BAIL_IF(!retvalbuf, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    if (!uconvdll)
    {
        /* There's really not much we can do on older OS/2s except pray this
           is latin1-compatible. */
        retval = retvalbuf;
        PHYSFS_utf8FromLatin1(cpstr, retval, len * 4);
    } /* if */
    else
    {
        int rc;
        size_t cplen = len;
        size_t unilen = len;
        size_t subs = 0;
        UniChar *uc2ptr = __PHYSFS_smallAlloc(len * sizeof (UniChar));
        UniChar *uc2str = uc2ptr;

        BAIL_IF(!uc2ptr, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        rc = pUniUconvToUcs(uconv, (void **) &cpstr, &cplen, &uc2str, &unilen, &subs);
        GOTO_IF(rc != ULS_SUCCESS, PHYSFS_ERR_BAD_FILENAME, done);
        GOTO_IF(subs > 0, PHYSFS_ERR_BAD_FILENAME, done);
        assert(cplen == 0);
        retval = retvalbuf;
        PHYSFS_utf8FromUcs2((const PHYSFS_uint16 *) uc2ptr, retval, len * 4);
        done:
        __PHYSFS_smallFree(uc2ptr);
    } /* else */

    return retval;
} /* cvtCodepageToUtf8 */


/* (be gentle, this function isn't very robust.) */
static char *cvtPathToCorrectCase(char *buf)
{
    char *retval = buf;
    char *fname = buf + 3;            /* point to first element. */
    char *ptr = strchr(fname, '\\');  /* find end of first element. */

    buf[0] = toupper(buf[0]);  /* capitalize drive letter. */

    /*
     * Go through each path element, and enumerate its parent dir until
     *  a case-insensitive match is found. If one is (and it SHOULD be)
     *  then overwrite the original element with the correct case.
     * If there's an error, or the path has vanished for some reason, it
     *  won't hurt to have the original case, so we just keep going.
     */
    while ((fname != NULL) && (*fname != '\0'))
    {
        char spec[CCHMAXPATH];
        FILEFINDBUF3 fb;
        HDIR hdir = HDIR_CREATE;
        ULONG count = 1;
        APIRET rc;

        *(fname - 1) = '\0';  /* isolate parent dir string. */

        strcpy(spec, buf);      /* copy isolated parent dir... */
        strcat(spec, "\\*.*");  /*  ...and add wildcard search spec. */

        if (ptr != NULL)  /* isolate element to find (fname is the start). */
            *ptr = '\0';

        rc = DosFindFirst(spec, &hdir, FILE_DIRECTORY,
                          &fb, sizeof (fb), &count, FIL_STANDARD);
        if (rc == NO_ERROR)
        {
            while (count == 1)  /* while still entries to enumerate... */
            {
                int cmp;
                char *utf8 = cvtCodepageToUtf8(fb.achName);
                if (!utf8) /* ugh, maybe we'll get lucky with the C runtime. */
                    cmp = stricmp(fb.achName, fname);
                else
                {
                    cmp = PHYSFS_utf8stricmp(utf8, fname);
                    allocator.Free(utf8);
                } /* else */

                if (cmp == 0)
                {
                    strcpy(fname, fb.achName);
                    break;  /* there it is. Overwrite and stop searching. */
                } /* if */

                DosFindNext(hdir, &fb, sizeof (fb), &count);
            } /* while */
            DosFindClose(hdir);
        } /* if */

        *(fname - 1) = '\\';   /* unisolate parent dir. */
        fname = ptr;           /* point to next element. */
        if (ptr != NULL)
        {
            *ptr = '\\';       /* unisolate element. */
            ptr = strchr(++fname, '\\');  /* find next element. */
        } /* if */
    } /* while */

    return retval;
} /* cvtPathToCorrectCase */

static void prepUnicodeSupport(void)
{
    /* really old OS/2 might not have Unicode support _at all_, so load
       the system library and do without if it doesn't exist. */
    int ok = 0;                                                   
    char buf[CCHMAXPATH];
    UniChar defstr[] = { 0 };
    if (DosLoadModule(buf, sizeof (buf) - 1, "uconv", &uconvdll) == NO_ERROR)
    {
        #define LOAD(x) (DosQueryProcAddr(uconvdll,0,#x,(PFN*)&p##x)==NO_ERROR)
        ok = LOAD(UniCreateUconvObject) &&
             LOAD(UniFreeUconvObject) &&
             LOAD(UniUconvToUcs) &&
             LOAD(UniUconvFromUcs);
        #undef LOAD
    } /* else */

    if (!ok || (pUniCreateUconvObject(defstr, &uconv) != ULS_SUCCESS))
    {
        /* oh well, live without it. */
        if (uconvdll)
        {
            if (uconv)
                pUniFreeUconvObject(uconv);
            DosFreeModule(uconvdll);
            uconvdll = 0;
        } /* if */
    } /* if */
} /* prepUnicodeSupport */


int __PHYSFS_platformInit(void)
{
    prepUnicodeSupport();
    return 1;  /* ready to go! */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    if (uconvdll)
    {
        pUniFreeUconvObject(uconv);
        uconv = 0;
        DosFreeModule(uconvdll);
        uconvdll = 0;
    } /* if */
} /* __PHYSFS_platformDeinit */


static int discIsInserted(ULONG drive)
{
    int rc;
    char buf[20];
    DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);
    rc = DosQueryFSInfo(drive + 1, FSIL_VOLSER, buf, sizeof (buf));
    DosError(FERR_ENABLEHARDERR | FERR_ENABLEEXCEPTION);
    return (rc == NO_ERROR);
} /* is_cdrom_inserted */


/* looks like "CD01" in ASCII (littleendian)...used for an ioctl. */
#define CD01 0x31304443

static int isCdRomDrive(ULONG drive)
{
    PHYSFS_uint32 param, data;
    ULONG ul1, ul2;
    APIRET rc;
    HFILE hfile = NULLHANDLE;
    char drivename[3] = { 0, ':', '\0' };

    drivename[0] = 'A' + drive;

    rc = DosOpen(drivename, &hfile, &ul1, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW,
                 OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE, NULL);
    if (rc != NO_ERROR)
        return 0;

    data = 0;
    param = PHYSFS_swapULE32(CD01);
    ul1 = ul2 = sizeof (PHYSFS_uint32);
    rc = DosDevIOCtl(hfile, IOCTL_CDROMDISK, CDROMDISK_GETDRIVER,
                     &param, sizeof (param), &ul1, &data, sizeof (data), &ul2);

    DosClose(hfile);
    return ((rc == NO_ERROR) && (PHYSFS_swapULE32(data) == CD01));
} /* isCdRomDrive */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    ULONG dummy = 0;
    ULONG drivemap = 0;
    ULONG i, bit;
    const APIRET rc = DosQueryCurrentDisk(&dummy, &drivemap);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc),);

    for (i = 0, bit = 1; i < 26; i++, bit <<= 1)
    {
        if (drivemap & bit)  /* this logical drive exists. */
        {
            if ((isCdRomDrive(i)) && (discIsInserted(i)))
            {
                char drive[4] = "x:\\";
                drive[0] = ('A' + i);
                cb(data, drive);
            } /* if */
        } /* if */
    } /* for */
} /* __PHYSFS_platformDetectAvailableCDs */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    char *retval = NULL;
    char buf[CCHMAXPATH];
    APIRET rc;
    PTIB ptib;
    PPIB ppib;
    PHYSFS_sint32 len;

    rc = DosGetInfoBlocks(&ptib, &ppib);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);
    rc = DosQueryModuleName(ppib->pib_hmte, sizeof (buf), (PCHAR) buf);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);
    retval = cvtCodepageToUtf8(buf);
    BAIL_IF_ERRPASS(!retval, NULL);

    /* chop off filename, leave path. */
    for (len = strlen(retval) - 1; len >= 0; len--)
    {
        if (retval[len] == '\\')
        {
            retval[len + 1] = '\0';
            break;
        } /* if */
    } /* for */

    assert(len > 0);  /* should have been a "x:\\" on the front on string. */

    /* The string is capitalized! Figure out the REAL case... */
    return cvtPathToCorrectCase(retval);
} /* __PHYSFS_platformCalcBaseDir */

char *__PHYSFS_platformCalcUserDir(void)
{
    return __PHYSFS_platformCalcBaseDir(NULL);  /* !!! FIXME: ? */
} /* __PHYSFS_platformCalcUserDir */

char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    return __PHYSFS_platformCalcBaseDir(NULL);  /* !!! FIXME: ? */
} /* __PHYSFS_platformCalcPrefDir */

PHYSFS_EnumerateCallbackResult __PHYSFS_platformEnumerate(const char *dirname,
                               PHYSFS_EnumerateCallback callback,
                               const char *origdir, void *callbackdata)
{                                        
    PHYSFS_EnumerateCallbackResult retval = PHYSFS_ENUM_OK;
    size_t utf8len = strlen(dirname);
    char *utf8 = (char *) __PHYSFS_smallAlloc(utf8len + 5);
    char *cpspec = NULL;
    FILEFINDBUF3 fb;
    HDIR hdir = HDIR_CREATE;
    ULONG count = 1;
    APIRET rc;

    BAIL_IF(!utf8, PHYSFS_ERR_OUT_OF_MEMORY, PHYSFS_ENUM_ERROR);

    strcpy(utf8, dirname);
    if (utf8[utf8len - 1] != '\\')
        strcpy(utf8 + utf8len, "\\*.*");
    else
        strcpy(utf8 + utf8len, "*.*");

    cpspec = cvtUtf8ToCodepage(utf8);
    __PHYSFS_smallFree(utf8);
    BAIL_IF_ERRPASS(!cpspec, PHYSFS_ENUM_ERROR);

    rc = DosFindFirst(cpspec, &hdir,
                      FILE_DIRECTORY | FILE_ARCHIVED |
                      FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM,
                      &fb, sizeof (fb), &count, FIL_STANDARD);
    allocator.Free(cpspec);

    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), PHYSFS_ENUM_ERROR);

    while (count == 1)
    {
        if ((strcmp(fb.achName, ".") != 0) && (strcmp(fb.achName, "..") != 0))
        {
            utf8 = cvtCodepageToUtf8(fb.achName);
            if (!utf8)
                retval = PHYSFS_ENUM_ERROR;
            else
            {
                retval = callback(callbackdata, origdir, utf8);
                allocator.Free(utf8);
                if (retval == PHYSFS_ENUM_ERROR)
                    PHYSFS_setErrorCode(PHYSFS_ERR_APP_CALLBACK);
            } /* else */
        } /* if */

        if (retval != PHYSFS_ENUM_OK)
            break;

        DosFindNext(hdir, &fb, sizeof (fb), &count);
    } /* while */

    DosFindClose(hdir);

    return retval;
} /* __PHYSFS_platformEnumerate */


char *__PHYSFS_platformCurrentDir(void)
{
    char *retval;
    char *cpstr;
    char *utf8;
    ULONG currentDisk;
    ULONG dummy;
    ULONG pathSize = 0;
    APIRET rc;
    BYTE byte;

    rc = DosQueryCurrentDisk(&currentDisk, &dummy);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), NULL);

    /* The first call just tells us how much space we need for the string. */
    rc = DosQueryCurrentDir(currentDisk, &byte, &pathSize);
    pathSize++; /* Add space for null terminator. */
    cpstr = (char *) __PHYSFS_smallAlloc(pathSize);
    BAIL_IF(cpstr == NULL, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    /* Actually get the string this time. */
    rc = DosQueryCurrentDir(currentDisk, (PBYTE) cpstr, &pathSize);
    if (rc != NO_ERROR)
    {
        __PHYSFS_smallFree(cpstr);
        BAIL(errcodeFromAPIRET(rc), NULL);
    } /* if */

    utf8 = cvtCodepageToUtf8(cpstr);
    __PHYSFS_smallFree(cpstr);
    BAIL_IF_ERRPASS(utf8 == NULL, NULL);

    /* +4 for "x:\\" drive selector and null terminator. */
    retval = (char *) allocator.Malloc(strlen(utf8) + 4);
    if (retval == NULL)
    {
        allocator.Free(utf8);
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    } /* if */

    retval[0] = ('A' + (currentDisk - 1));
    retval[1] = ':';
    retval[2] = '\\';
    strcpy(retval + 3, utf8);

    allocator.Free(utf8);

    return retval;
} /* __PHYSFS_platformCurrentDir */


int __PHYSFS_platformMkDir(const char *filename)
{
    APIRET rc;
    char *cpstr = cvtUtf8ToCodepage(filename);
    BAIL_IF_ERRPASS(!cpstr, 0);
    rc = DosCreateDir(cpstr, NULL);
    allocator.Free(cpstr);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);
    return 1;
} /* __PHYSFS_platformMkDir */


static HFILE openFile(const char *filename, const ULONG flags, const ULONG mode)
{
    char *cpfname = cvtUtf8ToCodepage(filename);
    ULONG action = 0;
    HFILE hfile = NULLHANDLE;
    APIRET rc;

    BAIL_IF_ERRPASS(!cpfname, 0);

    rc = DosOpen(cpfname, &hfile, &action, 0, FILE_NORMAL, flags, mode, NULL);
    allocator.Free(cpfname);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);

    return hfile;
} /* openFile */

void *__PHYSFS_platformOpenRead(const char *filename)
{
    /*
     * File must be opened SHARE_DENYWRITE and ACCESS_READONLY, otherwise
     *  DosQueryFileInfo() will fail if we try to get a file length, etc.
     */
    return (void *) openFile(filename,
                        OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_FAIL_IF_NEW,
                        OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_LOCALITY |
                        OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                        OPEN_ACCESS_READONLY);
} /* __PHYSFS_platformOpenRead */


void *__PHYSFS_platformOpenWrite(const char *filename)
{
    return (void *) openFile(filename,
                        OPEN_ACTION_REPLACE_IF_EXISTS |
                        OPEN_ACTION_CREATE_IF_NEW,
                        OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_LOCALITY |
                        OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE);
} /* __PHYSFS_platformOpenWrite */


void *__PHYSFS_platformOpenAppend(const char *filename)
{
    APIRET rc;
    ULONG dummy = 0;
    HFILE hfile;

    /*
     * File must be opened SHARE_DENYWRITE and ACCESS_READWRITE, otherwise
     *  DosQueryFileInfo() will fail if we try to get a file length, etc.
     */
    hfile = openFile(filename,
                        OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW,
                        OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_LOCALITY |
                        OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                        OPEN_ACCESS_READWRITE);
    BAIL_IF_ERRPASS(!hfile, NULL);

    rc = DosSetFilePtr(hfile, 0, FILE_END, &dummy);
    if (rc != NO_ERROR)
    {
        DosClose(hfile);
        BAIL(errcodeFromAPIRET(rc), NULL);
    } /* if */

    return ((void *) hfile);
} /* __PHYSFS_platformOpenAppend */


PHYSFS_sint64 __PHYSFS_platformRead(void *opaque, void *buf, PHYSFS_uint64 len)
{
    ULONG br = 0;
    APIRET rc;
    BAIL_IF(!__PHYSFS_ui64FitsAddressSpace(len),PHYSFS_ERR_INVALID_ARGUMENT,-1);
    rc = DosRead((HFILE) opaque, buf, (ULONG) len, &br);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), (br > 0) ? ((PHYSFS_sint64) br) : -1);
    return (PHYSFS_sint64) br;
} /* __PHYSFS_platformRead */


PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buf,
                                     PHYSFS_uint64 len)
{
    ULONG bw = 0;
    APIRET rc;
    BAIL_IF(!__PHYSFS_ui64FitsAddressSpace(len),PHYSFS_ERR_INVALID_ARGUMENT,-1);
    rc = DosWrite((HFILE) opaque, (void *) buf, (ULONG) len, &bw);    
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), (bw > 0) ? ((PHYSFS_sint64) bw) : -1);
    return (PHYSFS_sint64) bw;
} /* __PHYSFS_platformWrite */


int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos)
{
    ULONG dummy;
    HFILE hfile = (HFILE) opaque;
    LONG dist = (LONG) pos;
    APIRET rc;

    /* hooray for 32-bit filesystem limits!  :) */
    BAIL_IF((PHYSFS_uint64) dist != pos, PHYSFS_ERR_INVALID_ARGUMENT, 0);
    rc = DosSetFilePtr(hfile, dist, FILE_BEGIN, &dummy);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);
    return 1;
} /* __PHYSFS_platformSeek */


PHYSFS_sint64 __PHYSFS_platformTell(void *opaque)
{
    ULONG pos;
    HFILE hfile = (HFILE) opaque;
    const APIRET rc = DosSetFilePtr(hfile, 0, FILE_CURRENT, &pos);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), -1);
    return ((PHYSFS_sint64) pos);
} /* __PHYSFS_platformTell */


PHYSFS_sint64 __PHYSFS_platformFileLength(void *opaque)
{
    FILESTATUS3 fs;
    HFILE hfile = (HFILE) opaque;
    const APIRET rc = DosQueryFileInfo(hfile, FIL_STANDARD, &fs, sizeof (fs));
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), -1);
    return ((PHYSFS_sint64) fs.cbFile);
} /* __PHYSFS_platformFileLength */


int __PHYSFS_platformFlush(void *opaque)
{
    const APIRET rc = DosResetBuffer((HFILE) opaque);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);
    return 1;
} /* __PHYSFS_platformFlush */


void __PHYSFS_platformClose(void *opaque)
{
    DosClose((HFILE) opaque);  /* ignore errors. You should have flushed! */
} /* __PHYSFS_platformClose */


int __PHYSFS_platformDelete(const char *path)
{
    char *cppath = cvtUtf8ToCodepage(path);
    FILESTATUS3 fs;
    APIRET rc;
    int retval = 0;

    BAIL_IF_ERRPASS(!cppath, 0);
    rc = DosQueryPathInfo(cppath, FIL_STANDARD, &fs, sizeof (fs));
    GOTO_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), done);
    rc = (fs.attrFile & FILE_DIRECTORY) ? DosDeleteDir(path) : DosDelete(path);
    GOTO_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), done);
    retval = 1;  /* success */

done:
    allocator.Free(cppath);
    return retval;
} /* __PHYSFS_platformDelete */


/* Convert to a format PhysicsFS can grok... */
PHYSFS_sint64 os2TimeToUnixTime(const FDATE *date, const FTIME *time)
{
    struct tm tm;

    tm.tm_sec = ((PHYSFS_uint32) time->twosecs) * 2;                        
    tm.tm_min = time->minutes;
    tm.tm_hour = time->hours;
    tm.tm_mday = date->day;
    tm.tm_mon = date->month;
    tm.tm_year = ((PHYSFS_uint32) date->year) + 80;
    tm.tm_wday = -1 /*st_localtz.wDayOfWeek*/;
    tm.tm_yday = -1;
    tm.tm_isdst = -1;

    return (PHYSFS_sint64) mktime(&tm);
} /* os2TimeToUnixTime */


int __PHYSFS_platformStat(const char *filename, PHYSFS_Stat *stat, const int follow)
{
    char *cpfname = cvtUtf8ToCodepage(filename);
    FILESTATUS3 fs;
    int retval = 0;
    APIRET rc;

    BAIL_IF_ERRPASS(!cpfname, 0);

    rc = DosQueryPathInfo(cpfname, FIL_STANDARD, &fs, sizeof (fs));
    GOTO_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), done);

    if (fs.attrFile & FILE_DIRECTORY)
    {
        stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
        stat->filesize = 0;
    } /* if */
    else
    {
        stat->filetype = PHYSFS_FILETYPE_REGULAR;
        stat->filesize = fs.cbFile;
    } /* else */

    stat->modtime = os2TimeToUnixTime(&fs.fdateLastWrite, &fs.ftimeLastWrite);
    if (stat->modtime < 0)
        stat->modtime = 0;

    stat->accesstime = os2TimeToUnixTime(&fs.fdateLastAccess, &fs.ftimeLastAccess);
    if (stat->accesstime < 0)
        stat->accesstime = 0;

    stat->createtime = os2TimeToUnixTime(&fs.fdateCreation, &fs.ftimeCreation);
    if (stat->createtime < 0)
        stat->createtime = 0;

    stat->readonly = ((fs.attrFile & FILE_READONLY) == FILE_READONLY);
    return 1;  /* success */

done:
    allocator.Free(cpfname); 
    return retval;
} /* __PHYSFS_platformStat */


void *__PHYSFS_platformGetThreadID(void)
{
    PTIB ptib;
    PPIB ppib;

    /*
     * Allegedly, this API never fails, but we'll punt and return a
     *  default value (zero might as well do) if it does.
     */
    const APIRET rc = DosGetInfoBlocks(&ptib, &ppib);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), 0);
    return ((void *) ptib->tib_ordinal);
} /* __PHYSFS_platformGetThreadID */


void *__PHYSFS_platformCreateMutex(void)
{
    HMTX hmtx = NULLHANDLE;
    const APIRET rc = DosCreateMutexSem(NULL, &hmtx, 0, 0);
    BAIL_IF(rc != NO_ERROR, errcodeFromAPIRET(rc), NULL);
    return ((void *) hmtx);
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    DosCloseMutexSem((HMTX) mutex);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    /* Do _NOT_ set the physfs error message in here! */
    return (DosRequestMutexSem((HMTX) mutex, SEM_INDEFINITE_WAIT) == NO_ERROR);
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    DosReleaseMutexSem((HMTX) mutex);
} /* __PHYSFS_platformReleaseMutex */

#endif  /* PHYSFS_PLATFORM_OS2 */

/* end of physfs_platform_os2.c ... */
