/*
 * Windows support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon, and made sane by Gregory S. Read.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_WINDOWS

/* Forcibly disable UNICODE macro, since we manage this ourselves. */
#ifdef UNICODE
#undef UNICODE
#endif

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#ifndef PHYSFS_PLATFORM_WINRT
#include <userenv.h>
#include <shlobj.h>
#endif

#if !defined(PHYSFS_NO_CDROM_SUPPORT)
#include <dbt.h>
#endif

#include <errno.h>
#include <ctype.h>
#include <time.h>

#ifdef allocator  /* apparently Windows 10 SDK conflicts here. */
#undef allocator
#endif

#include "physfs_internal.h"

/*
 * Users without the platform SDK don't have this defined.  The original docs
 *  for SetFilePointer() just said to compare with 0xFFFFFFFF, so this should
 *  work as desired.
 */
#define PHYSFS_INVALID_SET_FILE_POINTER  0xFFFFFFFF

/* just in case... */
#define PHYSFS_INVALID_FILE_ATTRIBUTES   0xFFFFFFFF

/* Not defined before the Vista SDK. */
#define PHYSFS_FILE_ATTRIBUTE_REPARSE_POINT 0x400
#define PHYSFS_IO_REPARSE_TAG_SYMLINK    0xA000000C


#define UTF8_TO_UNICODE_STACK(w_assignto, str) { \
    if (str == NULL) \
        w_assignto = NULL; \
    else { \
        const size_t len = (PHYSFS_uint64) ((strlen(str) + 1) * 2); \
        w_assignto = (WCHAR *) __PHYSFS_smallAlloc(len); \
        if (w_assignto != NULL) \
            PHYSFS_utf8ToUtf16(str, (PHYSFS_uint16 *) w_assignto, len); \
    } \
} \

/* Note this counts WCHARs, not codepoints! */
static PHYSFS_uint64 wStrLen(const WCHAR *wstr)
{
    PHYSFS_uint64 len = 0;
    while (*(wstr++))
        len++;
    return len;
} /* wStrLen */

static char *unicodeToUtf8Heap(const WCHAR *w_str)
{
    char *retval = NULL;
    if (w_str != NULL)
    {
        void *ptr = NULL;
        const PHYSFS_uint64 len = (wStrLen(w_str) * 4) + 1;
        retval = allocator.Malloc(len);
        BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        PHYSFS_utf8FromUtf16((const PHYSFS_uint16 *) w_str, retval, len);
        ptr = allocator.Realloc(retval, strlen(retval) + 1); /* shrink. */
        if (ptr != NULL)
            retval = (char *) ptr;
    } /* if */
    return retval;
} /* unicodeToUtf8Heap */


/* Some older APIs aren't in WinRT (only the "Ex" version, etc).
   Since non-WinRT might not have the "Ex" version, we tapdance to use
   the perfectly-fine-and-available-even-on-Win95 API on non-WinRT targets. */

static inline HANDLE winFindFirstFileW(const WCHAR *path, LPWIN32_FIND_DATAW d)
{
    #ifdef PHYSFS_PLATFORM_WINRT
    return FindFirstFileExW(path, FindExInfoStandard, d,
                            FindExSearchNameMatch, NULL, 0);
    #else
    return FindFirstFileW(path, d);
    #endif
} /* winFindFirstFileW */

static inline BOOL winInitializeCriticalSection(LPCRITICAL_SECTION lpcs)
{
    #ifdef PHYSFS_PLATFORM_WINRT
    return InitializeCriticalSectionEx(lpcs, 2000, 0);
    #else
    InitializeCriticalSection(lpcs);
    return TRUE;
    #endif
} /* winInitializeCriticalSection */

static inline HANDLE winCreateFileW(const WCHAR *wfname, const DWORD mode,
                                    const DWORD creation)
{
    const DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    #ifdef PHYSFS_PLATFORM_WINRT
    return CreateFile2(wfname, mode, share, creation, NULL);
    #else
    return CreateFileW(wfname, mode, share, NULL, creation,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    #endif
} /* winCreateFileW */

static BOOL winSetFilePointer(HANDLE h, const PHYSFS_sint64 pos,
                              PHYSFS_sint64 *_newpos, const DWORD whence)
{
    #ifdef PHYSFS_PLATFORM_WINRT
    LARGE_INTEGER lipos;
    LARGE_INTEGER linewpos;
    BOOL rc;
    lipos.QuadPart = (LONGLONG) pos;
    rc = SetFilePointerEx(h, lipos, &linewpos, whence);
    if (_newpos)
        *_newpos = (PHYSFS_sint64) linewpos.QuadPart;
    return rc;
    #else
    const LONG low = (LONG) (pos & 0xFFFFFFFF);
    LONG high = (LONG) ((pos >> 32) & 0xFFFFFFFF);
    const DWORD rc = SetFilePointer(h, low, &high, whence);
    /* 0xFFFFFFFF could be valid, so you have to check GetLastError too! */
    if (_newpos)
        *_newpos = ((PHYSFS_sint64) rc) | (((PHYSFS_sint64) high) << 32);
    if ((rc == PHYSFS_INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
        return FALSE;
    return TRUE;
    #endif
} /* winSetFilePointer */

static PHYSFS_sint64 winGetFileSize(HANDLE h)
{
    #ifdef PHYSFS_PLATFORM_WINRT
    FILE_STANDARD_INFO info;
    const BOOL rc = GetFileInformationByHandleEx(h, FileStandardInfo,
                                                 &info, sizeof (info));
    return rc ? (PHYSFS_sint64) info.EndOfFile.QuadPart : -1;
    #else
    DWORD high = 0;
    const DWORD rc = GetFileSize(h, &high);
    if ((rc == PHYSFS_INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
        return -1;
    return (PHYSFS_sint64) ((((PHYSFS_uint64) high) << 32) | rc);
    #endif
} /* winGetFileSize */


static PHYSFS_ErrorCode errcodeFromWinApiError(const DWORD err)
{
    /*
     * win32 error codes are sort of a tricky thing; Microsoft intentionally
     *  doesn't list which ones a given API might trigger, there are several
     *  with overlapping and unclear meanings...and there's 16 thousand of
     *  them in Windows 7. It looks like the ones we care about are in the
     *  first 500, but I can't say this list is perfect; we might miss
     *  important values or misinterpret others.
     *
     * Don't treat this list as anything other than a work in progress.
     */
    switch (err)
    {
        case ERROR_SUCCESS: return PHYSFS_ERR_OK;
        case ERROR_ACCESS_DENIED: return PHYSFS_ERR_PERMISSION;
        case ERROR_NETWORK_ACCESS_DENIED: return PHYSFS_ERR_PERMISSION;
        case ERROR_NOT_READY: return PHYSFS_ERR_IO;
        case ERROR_CRC: return PHYSFS_ERR_IO;
        case ERROR_SEEK: return PHYSFS_ERR_IO;
        case ERROR_SECTOR_NOT_FOUND: return PHYSFS_ERR_IO;
        case ERROR_NOT_DOS_DISK: return PHYSFS_ERR_IO;
        case ERROR_WRITE_FAULT: return PHYSFS_ERR_IO;
        case ERROR_READ_FAULT: return PHYSFS_ERR_IO;
        case ERROR_DEV_NOT_EXIST: return PHYSFS_ERR_IO;
        case ERROR_BUFFER_OVERFLOW: return PHYSFS_ERR_BAD_FILENAME;
        case ERROR_INVALID_NAME: return PHYSFS_ERR_BAD_FILENAME;
        case ERROR_BAD_PATHNAME: return PHYSFS_ERR_BAD_FILENAME;
        case ERROR_DIRECTORY: return PHYSFS_ERR_BAD_FILENAME;
        case ERROR_FILE_NOT_FOUND: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_PATH_NOT_FOUND: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_DELETE_PENDING: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_INVALID_DRIVE: return PHYSFS_ERR_NOT_FOUND;
        case ERROR_HANDLE_DISK_FULL: return PHYSFS_ERR_NO_SPACE;
        case ERROR_DISK_FULL: return PHYSFS_ERR_NO_SPACE;
        case ERROR_WRITE_PROTECT: return PHYSFS_ERR_READ_ONLY;
        case ERROR_LOCK_VIOLATION: return PHYSFS_ERR_BUSY;
        case ERROR_SHARING_VIOLATION: return PHYSFS_ERR_BUSY;
        case ERROR_CURRENT_DIRECTORY: return PHYSFS_ERR_BUSY;
        case ERROR_DRIVE_LOCKED: return PHYSFS_ERR_BUSY;
        case ERROR_PATH_BUSY: return PHYSFS_ERR_BUSY;
        case ERROR_BUSY: return PHYSFS_ERR_BUSY;
        case ERROR_NOT_ENOUGH_MEMORY: return PHYSFS_ERR_OUT_OF_MEMORY;
        case ERROR_OUTOFMEMORY: return PHYSFS_ERR_OUT_OF_MEMORY;
        case ERROR_DIR_NOT_EMPTY: return PHYSFS_ERR_DIR_NOT_EMPTY;
        default: return PHYSFS_ERR_OS_ERROR;
    } /* switch */
} /* errcodeFromWinApiError */

static inline PHYSFS_ErrorCode errcodeFromWinApi(void)
{
    return errcodeFromWinApiError(GetLastError());
} /* errcodeFromWinApi */


#if defined(PHYSFS_NO_CDROM_SUPPORT)
#define detectAvailableCDs(cb, data)
#define deinitCDThread()
#else
static HANDLE detectCDThreadHandle = NULL;
static HWND detectCDHwnd = NULL;
static volatile DWORD drivesWithMediaBitmap = 0;

typedef BOOL (WINAPI *fnSTEM)(DWORD, LPDWORD b);

static DWORD pollDiscDrives(void)
{
    /* Try to use SetThreadErrorMode(), which showed up in Windows 7. */
    HANDLE lib = LoadLibraryA("kernel32.dll");
    fnSTEM stem = NULL;
    char drive[4] = { 'x', ':', '\\', '\0' };
    DWORD oldErrorMode = 0;
    DWORD drives = 0;
    DWORD i;

    if (lib)
        stem = (fnSTEM) GetProcAddress(lib, "SetThreadErrorMode");

    if (stem)
        stem(SEM_FAILCRITICALERRORS, &oldErrorMode);
    else
        oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    
    /* Do detection. This may block if a disc is spinning up. */
    for (i = 'A'; i <= 'Z'; i++)
    {
        DWORD tmp = 0;
        drive[0] = (char) i;
        if (GetDriveTypeA(drive) != DRIVE_CDROM)
            continue;

        /* If this function succeeds, there's media in the drive */
        if (GetVolumeInformationA(drive, NULL, 0, NULL, NULL, &tmp, NULL, 0))
            drives |= (1 << (i - 'A'));
    } /* for */

    if (stem)
        stem(oldErrorMode, NULL);
    else
        SetErrorMode(oldErrorMode);

    if (lib)
        FreeLibrary(lib);

    return drives;
} /* pollDiscDrives */


static LRESULT CALLBACK detectCDWndProc(HWND hwnd, UINT msg,
                                        WPARAM wp, LPARAM lparam)
{
    PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR) lparam;
    PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME) lparam;
    const int removed = (wp == DBT_DEVICEREMOVECOMPLETE);

    if (msg == WM_DESTROY)
        return 0;
    else if ((msg != WM_DEVICECHANGE) ||
             ((wp != DBT_DEVICEARRIVAL) && (wp != DBT_DEVICEREMOVECOMPLETE)) ||
             (lpdb->dbch_devicetype != DBT_DEVTYP_VOLUME) ||
             ((lpdbv->dbcv_flags & DBTF_MEDIA) == 0))
    {
        return DefWindowProcW(hwnd, msg, wp, lparam);
    } /* else if */

    if (removed)
        drivesWithMediaBitmap &= ~lpdbv->dbcv_unitmask;
    else
        drivesWithMediaBitmap |= lpdbv->dbcv_unitmask;

    return TRUE;
} /* detectCDWndProc */


static DWORD WINAPI detectCDThread(LPVOID arg)
{
    HANDLE initialDiscDetectionComplete = *((HANDLE *) arg);
    const char *classname = "PhysicsFSDetectCDCatcher";
    const char *winname = "PhysicsFSDetectCDMsgWindow";
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    ATOM class_atom = 0;
    WNDCLASSEXA wce;
    MSG msg;

    memset(&wce, '\0', sizeof (wce));
    wce.cbSize = sizeof (wce);
    wce.lpfnWndProc = detectCDWndProc;
    wce.lpszClassName = classname;
    wce.hInstance = hInstance;
    class_atom = RegisterClassExA(&wce);
    if (class_atom == 0)
    {
        SetEvent(initialDiscDetectionComplete);  /* let main thread go on. */
        return 0;
    } /* if */

    detectCDHwnd = CreateWindowExA(0, classname, winname, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, HWND_DESKTOP, NULL, hInstance, NULL);

    if (detectCDHwnd == NULL)
    {
        SetEvent(initialDiscDetectionComplete);  /* let main thread go on. */
        UnregisterClassA(classname, hInstance);
        return 0;
    } /* if */

    /* We'll get events when discs come and go from now on. */

    /* Do initial detection, possibly blocking awhile... */
    drivesWithMediaBitmap = pollDiscDrives();

    SetEvent(initialDiscDetectionComplete);  /* let main thread go on. */

    do
    {
        const BOOL rc = GetMessageW(&msg, detectCDHwnd, 0, 0);
        if ((rc == 0) || (rc == -1))
            break;  /* don't care if WM_QUIT or error break this loop. */
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    } while (1);

    /* we've been asked to quit. */
    DestroyWindow(detectCDHwnd);
    UnregisterClassA(classname, hInstance);
    return 0;
} /* detectCDThread */

static void detectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    char drive_str[4] = { 'x', ':', '\\', '\0' };
    DWORD drives = 0;
    DWORD i;

    /*
     * If you poll a drive while a user is inserting a disc, the OS will
     *  block this thread until the drive has spun up. So we swallow the risk
     *  once for initial detection, and spin a thread that will get device
     *  events thereafter, for apps that use this interface to poll for
     *  disc insertion.
     */
    if (!detectCDThreadHandle)
    {
        HANDLE initialDetectDone = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!initialDetectDone)
            return;  /* oh well. */

        detectCDThreadHandle = CreateThread(NULL, 0, detectCDThread,
                                            &initialDetectDone, 0, NULL);
        if (detectCDThreadHandle)
            WaitForSingleObject(initialDetectDone, INFINITE);
        CloseHandle(initialDetectDone);

        if (!detectCDThreadHandle)
            return;  /* oh well. */
    } /* if */

    drives = drivesWithMediaBitmap; /* whatever the thread has seen, we take. */
    for (i = 'A'; i <= 'Z'; i++)
    {
        if (drives & (1 << (i - 'A')))
        {
            drive_str[0] = (char) i;
            cb(data, drive_str);
        } /* if */
    } /* for */
} /* detectAvailableCDs */

static void deinitCDThread(void)
{
    if (detectCDThreadHandle)
    {
        if (detectCDHwnd)
            PostMessageW(detectCDHwnd, WM_QUIT, 0, 0);
        CloseHandle(detectCDThreadHandle);
        detectCDThreadHandle = NULL;
        drivesWithMediaBitmap = 0;
    } /* if */
} /* deinitCDThread */
#endif


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    detectAvailableCDs(cb, data);
} /* __PHYSFS_platformDetectAvailableCDs */

#ifdef PHYSFS_PLATFORM_WINRT
static char *calcDirAppendSep(const WCHAR *wdir)
{
    size_t len;
    void *ptr;
    char *retval;
    BAIL_IF(!wdir, errcodeFromWinApi(), NULL);
    retval = unicodeToUtf8Heap(wdir);
    BAIL_IF_ERRPASS(!retval, NULL);
    len = strlen(retval);
    ptr = allocator.Realloc(retval, len + 2);
    if (!ptr)
    {
        allocator.Free(retval);
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    } /* if */
    retval = (char *) ptr;
    retval[len] = '\\';
    retval[len+1] = '\0';
    return retval;
} /* calcDirAppendSep */
#endif

char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
#ifdef PHYSFS_PLATFORM_WINRT
    return calcDirAppendSep((const WCHAR *) __PHYSFS_winrtCalcBaseDir());
#else
    char *retval = NULL;
    DWORD buflen = 64;
    LPWSTR modpath = NULL;

    while (1)
    {
        DWORD rc;
        void *ptr;

        if ( (ptr = allocator.Realloc(modpath, buflen*sizeof(WCHAR))) == NULL )
        {
            allocator.Free(modpath);
            BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
        } /* if */
        modpath = (LPWSTR) ptr;

        rc = GetModuleFileNameW(NULL, modpath, buflen);
        if (rc == 0)
        {
            allocator.Free(modpath);
            BAIL(errcodeFromWinApi(), NULL);
        } /* if */

        if (rc < buflen)
        {
            buflen = rc;
            break;
        } /* if */

        buflen *= 2;
    } /* while */

    if (buflen > 0)  /* just in case... */
    {
        WCHAR *ptr = (modpath + buflen) - 1;
        while (ptr != modpath)
        {
            if (*ptr == '\\')
                break;
            ptr--;
        } /* while */

        if ((ptr == modpath) && (*ptr != '\\'))
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);  /* oh well. */
        else
        {
            *(ptr+1) = '\0';  /* chop off filename. */
            retval = unicodeToUtf8Heap(modpath);
        } /* else */
    } /* else */
    allocator.Free(modpath);

    return retval;   /* w00t. */
#endif
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
#ifdef PHYSFS_PLATFORM_WINRT
    return calcDirAppendSep((const WCHAR *) __PHYSFS_winrtCalcPrefDir());
#else
    /*
     * Vista and later has a new API for this, but SHGetFolderPath works there,
     *  and apparently just wraps the new API. This is the new way to do it:
     *
     *     SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE,
     *                          NULL, &wszPath);
     */

    WCHAR path[MAX_PATH];
    char *utf8 = NULL;
    size_t len = 0;
    char *retval = NULL;

    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                   NULL, 0, path)))
        BAIL(PHYSFS_ERR_OS_ERROR, NULL);

    utf8 = unicodeToUtf8Heap(path);
    BAIL_IF_ERRPASS(!utf8, NULL);
    len = strlen(utf8) + strlen(org) + strlen(app) + 4;
    retval = allocator.Malloc(len);
    if (!retval)
    {
        allocator.Free(utf8);
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    } /* if */

    snprintf(retval, len, "%s\\%s\\%s\\", utf8, org, app);
    allocator.Free(utf8);
    return retval;
#endif
} /* __PHYSFS_platformCalcPrefDir */


char *__PHYSFS_platformCalcUserDir(void)
{
#ifdef PHYSFS_PLATFORM_WINRT
    return calcDirAppendSep((const WCHAR *) __PHYSFS_winrtCalcPrefDir());
#else
    typedef BOOL (WINAPI *fnGetUserProfDirW)(HANDLE, LPWSTR, LPDWORD);
    fnGetUserProfDirW pGetDir = NULL;
    HANDLE lib = NULL;
    HANDLE accessToken = NULL;       /* Security handle to process */
    char *retval = NULL;

    lib = LoadLibraryA("userenv.dll");
    BAIL_IF(!lib, errcodeFromWinApi(), NULL);
    pGetDir=(fnGetUserProfDirW) GetProcAddress(lib,"GetUserProfileDirectoryW");
    GOTO_IF(!pGetDir, errcodeFromWinApi(), done);

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &accessToken))
        GOTO(errcodeFromWinApi(), done);
    else
    {
        DWORD psize = 0;
        LPWSTR wstr = NULL;
        BOOL rc = 0;

        /*
         * Should fail. Will write the size of the profile path in
         *  psize. Also note that the second parameter can't be
         *  NULL or the function fails on Windows XP, but has to be NULL on
         *  Windows 10 or it will fail.  :(
         */
        rc = pGetDir(accessToken, NULL, &psize);
        GOTO_IF(rc, PHYSFS_ERR_OS_ERROR, done);  /* should have failed! */

        if (psize == 0)  /* probably on Windows XP, try a different way. */
        {
            WCHAR x = 0;
            rc = pGetDir(accessToken, &x, &psize);
            GOTO_IF(rc, PHYSFS_ERR_OS_ERROR, done);  /* should have failed! */
            GOTO_IF(!psize, PHYSFS_ERR_OS_ERROR, done);  /* Uhoh... */
        } /* if */

        /* Allocate memory for the profile directory */
        wstr = (LPWSTR) __PHYSFS_smallAlloc((psize + 1) * sizeof (WCHAR));
        if (wstr != NULL)
        {
            if (pGetDir(accessToken, wstr, &psize))
            {
                /* Make sure it ends in a dirsep. We allocated +1 for this. */
                if (wstr[psize - 2] != '\\')
                {
                    wstr[psize - 1] = '\\';
                    wstr[psize - 0] = '\0';
                } /* if */
                retval = unicodeToUtf8Heap(wstr);
            } /* if */
            __PHYSFS_smallFree(wstr);
        } /* if */
    } /* if */

done:
    if (accessToken)
        CloseHandle(accessToken);
    FreeLibrary(lib);
    return retval;  /* We made it: hit the showers. */
#endif
} /* __PHYSFS_platformCalcUserDir */


int __PHYSFS_platformInit(void)
{
    return 1;  /* It's all good */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    deinitCDThread();
} /* __PHYSFS_platformDeinit */


void *__PHYSFS_platformGetThreadID(void)
{
    return ( (void *) ((size_t) GetCurrentThreadId()) );
} /* __PHYSFS_platformGetThreadID */


PHYSFS_EnumerateCallbackResult __PHYSFS_platformEnumerate(const char *dirname,
                               PHYSFS_EnumerateCallback callback,
                               const char *origdir, void *callbackdata)
{
    PHYSFS_EnumerateCallbackResult retval = PHYSFS_ENUM_OK;
    HANDLE dir = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW entw;
    size_t len = strlen(dirname);
    char *searchPath = NULL;
    WCHAR *wSearchPath = NULL;

    /* Allocate a new string for path, maybe '\\', "*", and NULL terminator */
    searchPath = (char *) __PHYSFS_smallAlloc(len + 3);
    BAIL_IF(!searchPath, PHYSFS_ERR_OUT_OF_MEMORY, PHYSFS_ENUM_ERROR);

    /* Copy current dirname */
    strcpy(searchPath, dirname);

    /* if there's no '\\' at the end of the path, stick one in there. */
    if (searchPath[len - 1] != '\\')
    {
        searchPath[len++] = '\\';
        searchPath[len] = '\0';
    } /* if */

    /* Append the "*" to the end of the string */
    strcat(searchPath, "*");

    UTF8_TO_UNICODE_STACK(wSearchPath, searchPath);
    __PHYSFS_smallFree(searchPath);
    BAIL_IF_ERRPASS(!wSearchPath, PHYSFS_ENUM_ERROR);

    dir = winFindFirstFileW(wSearchPath, &entw);
    __PHYSFS_smallFree(wSearchPath);
    BAIL_IF(dir==INVALID_HANDLE_VALUE, errcodeFromWinApi(), PHYSFS_ENUM_ERROR);

    do
    {
        const WCHAR *fn = entw.cFileName;
        char *utf8;

        if (fn[0] == '.')  /* ignore "." and ".." */
        {
            if ((fn[1] == '\0') || ((fn[1] == '.') && (fn[2] == '\0')))
                continue;
        } /* if */

        utf8 = unicodeToUtf8Heap(fn);
        if (utf8 == NULL)
            retval = -1;
        else
        {
            retval = callback(callbackdata, origdir, utf8);
            allocator.Free(utf8);
            if (retval == PHYSFS_ENUM_ERROR)
                PHYSFS_setErrorCode(PHYSFS_ERR_APP_CALLBACK);
        } /* else */
    } while ((retval == PHYSFS_ENUM_OK) && (FindNextFileW(dir, &entw) != 0));

    FindClose(dir);

    return retval;
} /* __PHYSFS_platformEnumerate */


int __PHYSFS_platformMkDir(const char *path)
{
    WCHAR *wpath;
    DWORD rc;
    UTF8_TO_UNICODE_STACK(wpath, path);
    rc = CreateDirectoryW(wpath, NULL);
    __PHYSFS_smallFree(wpath);
    BAIL_IF(rc == 0, errcodeFromWinApi(), 0);
    return 1;
} /* __PHYSFS_platformMkDir */


static HANDLE doOpen(const char *fname, DWORD mode, DWORD creation)
{
    HANDLE fileh;
    WCHAR *wfname;

    UTF8_TO_UNICODE_STACK(wfname, fname);
    BAIL_IF(!wfname, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    fileh = winCreateFileW(wfname, mode, creation);
    __PHYSFS_smallFree(wfname);

    if (fileh == INVALID_HANDLE_VALUE)
        BAIL(errcodeFromWinApi(), INVALID_HANDLE_VALUE);

    return fileh;
} /* doOpen */


void *__PHYSFS_platformOpenRead(const char *filename)
{
    HANDLE h = doOpen(filename, GENERIC_READ, OPEN_EXISTING);
    return (h == INVALID_HANDLE_VALUE) ? NULL : (void *) h;
} /* __PHYSFS_platformOpenRead */


void *__PHYSFS_platformOpenWrite(const char *filename)
{
    HANDLE h = doOpen(filename, GENERIC_WRITE, CREATE_ALWAYS);
    return (h == INVALID_HANDLE_VALUE) ? NULL : (void *) h;
} /* __PHYSFS_platformOpenWrite */


void *__PHYSFS_platformOpenAppend(const char *filename)
{
    HANDLE h = doOpen(filename, GENERIC_WRITE, OPEN_ALWAYS);
    BAIL_IF_ERRPASS(h == INVALID_HANDLE_VALUE, NULL);

    if (!winSetFilePointer(h, 0, NULL, FILE_END))
    {
        const PHYSFS_ErrorCode err = errcodeFromWinApi();
        CloseHandle(h);
        BAIL(err, NULL);
    } /* if */

    return (void *) h;
} /* __PHYSFS_platformOpenAppend */


PHYSFS_sint64 __PHYSFS_platformRead(void *opaque, void *buf, PHYSFS_uint64 len)
{
    HANDLE h = (HANDLE) opaque;
    PHYSFS_sint64 totalRead = 0;

    if (!__PHYSFS_ui64FitsAddressSpace(len))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, -1);

    while (len > 0)
    {
        const DWORD thislen = (len > 0xFFFFFFFF) ? 0xFFFFFFFF : (DWORD) len;
        DWORD numRead = 0;
        if (!ReadFile(h, buf, thislen, &numRead, NULL))
            BAIL(errcodeFromWinApi(), -1);
        len -= (PHYSFS_uint64) numRead;
        totalRead += (PHYSFS_sint64) numRead;
        if (numRead != thislen)
            break;
    } /* while */

    return totalRead;
} /* __PHYSFS_platformRead */


PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buffer,
                                     PHYSFS_uint64 len)
{
    HANDLE h = (HANDLE) opaque;
    PHYSFS_sint64 totalWritten = 0;

    if (!__PHYSFS_ui64FitsAddressSpace(len))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, -1);

    while (len > 0)
    {
        const DWORD thislen = (len > 0xFFFFFFFF) ? 0xFFFFFFFF : (DWORD) len;
        DWORD numWritten = 0;
        if (!WriteFile(h, buffer, thislen, &numWritten, NULL))
            BAIL(errcodeFromWinApi(), -1);
        len -= (PHYSFS_uint64) numWritten;
        totalWritten += (PHYSFS_sint64) numWritten;
        if (numWritten != thislen)
            break;
    } /* while */

    return totalWritten;
} /* __PHYSFS_platformWrite */


int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos)
{
    HANDLE h = (HANDLE) opaque;
    const PHYSFS_sint64 spos = (PHYSFS_sint64) pos;
    BAIL_IF(!winSetFilePointer(h,spos,NULL,FILE_BEGIN), errcodeFromWinApi(), 0);
    return 1;  /* No error occured */
} /* __PHYSFS_platformSeek */


PHYSFS_sint64 __PHYSFS_platformTell(void *opaque)
{
    HANDLE h = (HANDLE) opaque;
    PHYSFS_sint64 pos = 0;
    BAIL_IF(!winSetFilePointer(h,0,&pos,FILE_CURRENT), errcodeFromWinApi(), -1);
    return pos;
} /* __PHYSFS_platformTell */


PHYSFS_sint64 __PHYSFS_platformFileLength(void *opaque)
{
    HANDLE h = (HANDLE) opaque;
    const PHYSFS_sint64 retval = winGetFileSize(h);
    BAIL_IF(retval < 0, errcodeFromWinApi(), -1);
    return retval;
} /* __PHYSFS_platformFileLength */


int __PHYSFS_platformFlush(void *opaque)
{
    HANDLE h = (HANDLE) opaque;
    BAIL_IF(!FlushFileBuffers(h), errcodeFromWinApi(), 0);
    return 1;
} /* __PHYSFS_platformFlush */


void __PHYSFS_platformClose(void *opaque)
{
    HANDLE h = (HANDLE) opaque;
    (void) CloseHandle(h); /* ignore errors. You should have flushed! */
} /* __PHYSFS_platformClose */


static int doPlatformDelete(LPWSTR wpath)
{
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExW(wpath, GetFileExInfoStandard, &info))
        BAIL(errcodeFromWinApi(), 0);
    else
    {
        const int isdir = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        const BOOL rc = isdir ? RemoveDirectoryW(wpath) : DeleteFileW(wpath);
        BAIL_IF(!rc, errcodeFromWinApi(), 0);
    } /* else */
    return 1;   /* if you made it here, it worked. */
} /* doPlatformDelete */


int __PHYSFS_platformDelete(const char *path)
{
    int retval = 0;
    LPWSTR wpath = NULL;
    UTF8_TO_UNICODE_STACK(wpath, path);
    BAIL_IF(!wpath, PHYSFS_ERR_OUT_OF_MEMORY, 0);
    retval = doPlatformDelete(wpath);
    __PHYSFS_smallFree(wpath);
    return retval;
} /* __PHYSFS_platformDelete */


void *__PHYSFS_platformCreateMutex(void)
{
    LPCRITICAL_SECTION lpcs;
    lpcs = (LPCRITICAL_SECTION) allocator.Malloc(sizeof (CRITICAL_SECTION));
    BAIL_IF(!lpcs, PHYSFS_ERR_OUT_OF_MEMORY, NULL);

    if (!winInitializeCriticalSection(lpcs))
    {
        allocator.Free(lpcs);
        BAIL(errcodeFromWinApi(), NULL);
    } /* if */

    return lpcs;
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    DeleteCriticalSection((LPCRITICAL_SECTION) mutex);
    allocator.Free(mutex);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    EnterCriticalSection((LPCRITICAL_SECTION) mutex);
    return 1;
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    LeaveCriticalSection((LPCRITICAL_SECTION) mutex);
} /* __PHYSFS_platformReleaseMutex */


static PHYSFS_sint64 FileTimeToPhysfsTime(const FILETIME *ft)
{
    SYSTEMTIME st_utc;
    SYSTEMTIME st_localtz;
    TIME_ZONE_INFORMATION tzi;
    DWORD tzid;
    PHYSFS_sint64 retval;
    struct tm tm;
    BOOL rc;

    BAIL_IF(!FileTimeToSystemTime(ft, &st_utc), errcodeFromWinApi(), -1);
    tzid = GetTimeZoneInformation(&tzi);
    BAIL_IF(tzid == TIME_ZONE_ID_INVALID, errcodeFromWinApi(), -1);
    rc = SystemTimeToTzSpecificLocalTime(&tzi, &st_utc, &st_localtz);
    BAIL_IF(!rc, errcodeFromWinApi(), -1);

    /* Convert to a format that mktime() can grok... */
    tm.tm_sec = st_localtz.wSecond;
    tm.tm_min = st_localtz.wMinute;
    tm.tm_hour = st_localtz.wHour;
    tm.tm_mday = st_localtz.wDay;
    tm.tm_mon = st_localtz.wMonth - 1;
    tm.tm_year = st_localtz.wYear - 1900;
    tm.tm_wday = -1 /*st_localtz.wDayOfWeek*/;
    tm.tm_yday = -1;
    tm.tm_isdst = -1;

    /* Convert to a format PhysicsFS can grok... */
    retval = (PHYSFS_sint64) mktime(&tm);
    BAIL_IF(retval == -1, PHYSFS_ERR_OS_ERROR, -1);
    return retval;
} /* FileTimeToPhysfsTime */


/* check for symlinks. These exist in NTFS 3.1 (WinXP), even though
   they aren't really available to userspace before Vista. I wonder
   what would happen if you put an NTFS disk with a symlink on it
   into an XP machine, though; would this flag get set?
   NTFS symlinks are a form of "reparse point" (junction, volume mount,
   etc), so if the REPARSE_POINT attribute is set, check for the symlink
   tag thereafter. This assumes you already read in the file attributes. */
static int isSymlink(const WCHAR *wpath, const DWORD attr)
{
    WIN32_FIND_DATAW w32dw;
    HANDLE h;

    if ((attr & PHYSFS_FILE_ATTRIBUTE_REPARSE_POINT) == 0)
        return 0;  /* not a reparse point? Definitely not a symlink. */

    h = winFindFirstFileW(wpath, &w32dw);
    if (h == INVALID_HANDLE_VALUE)
        return 0;  /* ...maybe the file just vanished...? */

    FindClose(h);
    return (w32dw.dwReserved0 == PHYSFS_IO_REPARSE_TAG_SYMLINK);
} /* isSymlink */


int __PHYSFS_platformStat(const char *filename, PHYSFS_Stat *st, const int follow)
{
    WIN32_FILE_ATTRIBUTE_DATA winstat;
    WCHAR *wstr = NULL;
    DWORD err = 0;
    BOOL rc = 0;
    int issymlink = 0;

    UTF8_TO_UNICODE_STACK(wstr, filename);
    BAIL_IF(!wstr, PHYSFS_ERR_OUT_OF_MEMORY, 0);
    rc = GetFileAttributesExW(wstr, GetFileExInfoStandard, &winstat);

    if (!rc)
        err = GetLastError();
    else  /* check for symlink while wstr is still available */
        issymlink = !follow && isSymlink(wstr, winstat.dwFileAttributes);

    __PHYSFS_smallFree(wstr);
    BAIL_IF(!rc, errcodeFromWinApiError(err), 0);

    st->modtime = FileTimeToPhysfsTime(&winstat.ftLastWriteTime);
    st->accesstime = FileTimeToPhysfsTime(&winstat.ftLastAccessTime);
    st->createtime = FileTimeToPhysfsTime(&winstat.ftCreationTime);

    if (issymlink)
    {
        st->filetype = PHYSFS_FILETYPE_SYMLINK;
        st->filesize = 0;
    } /* if */

    else if (winstat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        st->filetype = PHYSFS_FILETYPE_DIRECTORY;
        st->filesize = 0;
    } /* else if */

    else if (winstat.dwFileAttributes & (FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_DEVICE))
    {
        st->filetype = PHYSFS_FILETYPE_OTHER;
        st->filesize = (((PHYSFS_uint64) winstat.nFileSizeHigh) << 32) | winstat.nFileSizeLow;
    } /* else if */

    else
    {
        st->filetype = PHYSFS_FILETYPE_REGULAR;
        st->filesize = (((PHYSFS_uint64) winstat.nFileSizeHigh) << 32) | winstat.nFileSizeLow;
    } /* else */

    st->readonly = ((winstat.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0);

    return 1;
} /* __PHYSFS_platformStat */

#endif  /* PHYSFS_PLATFORM_WINDOWS */

/* end of physfs_platform_windows.c ... */


