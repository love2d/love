/**
 * \file physfs.h
 *
 * Main header file for PhysicsFS.
 */

/**
 * \mainpage PhysicsFS
 *
 * The latest version of PhysicsFS can be found at:
 *     https://icculus.org/physfs/
 *
 * PhysicsFS; a portable, flexible file i/o abstraction.
 *
 * This API gives you access to a system file system in ways superior to the
 *  stdio or system i/o calls. The brief benefits:
 *
 *   - It's portable.
 *   - It's safe. No file access is permitted outside the specified dirs.
 *   - It's flexible. Archives (.ZIP files) can be used transparently as
 *      directory structures.
 *
 * With PhysicsFS, you have a single writing directory and multiple
 *  directories (the "search path") for reading. You can think of this as a
 *  filesystem within a filesystem. If (on Windows) you were to set the
 *  writing directory to "C:\MyGame\MyWritingDirectory", then no PHYSFS calls
 *  could touch anything above this directory, including the "C:\MyGame" and
 *  "C:\" directories. This prevents an application's internal scripting
 *  language from piddling over c:\\config.sys, for example. If you'd rather
 *  give PHYSFS full access to the system's REAL file system, set the writing
 *  dir to "C:\", but that's generally A Bad Thing for several reasons.
 *
 * Drive letters are hidden in PhysicsFS once you set up your initial paths.
 *  The search path creates a single, hierarchical directory structure.
 *  Not only does this lend itself well to general abstraction with archives,
 *  it also gives better support to operating systems like MacOS and Unix.
 *  Generally speaking, you shouldn't ever hardcode a drive letter; not only
 *  does this hurt portability to non-Microsoft OSes, but it limits your win32
 *  users to a single drive, too. Use the PhysicsFS abstraction functions and
 *  allow user-defined configuration options, too. When opening a file, you
 *  specify it like it was on a Unix filesystem: if you want to write to
 *  "C:\MyGame\MyConfigFiles\game.cfg", then you might set the write dir to
 *  "C:\MyGame" and then open "MyConfigFiles/game.cfg". This gives an
 *  abstraction across all platforms. Specifying a file in this way is termed
 *  "platform-independent notation" in this documentation. Specifying a
 *  a filename in a form such as "C:\mydir\myfile" or
 *  "MacOS hard drive:My Directory:My File" is termed "platform-dependent
 *  notation". The only time you use platform-dependent notation is when
 *  setting up your write directory and search path; after that, all file
 *  access into those directories are done with platform-independent notation.
 *
 * All files opened for writing are opened in relation to the write directory,
 *  which is the root of the writable filesystem. When opening a file for
 *  reading, PhysicsFS goes through the search path. This is NOT the
 *  same thing as the PATH environment variable. An application using
 *  PhysicsFS specifies directories to be searched which may be actual
 *  directories, or archive files that contain files and subdirectories of
 *  their own. See the end of these docs for currently supported archive
 *  formats.
 *
 * Once the search path is defined, you may open files for reading. If you've
 *  got the following search path defined (to use a win32 example again):
 *
 *  - C:\\mygame
 *  - C:\\mygame\\myuserfiles
 *  - D:\\mygamescdromdatafiles
 *  - C:\\mygame\\installeddatafiles.zip
 *
 * Then a call to PHYSFS_openRead("textfiles/myfile.txt") (note the directory
 *  separator, lack of drive letter, and lack of dir separator at the start of
 *  the string; this is platform-independent notation) will check for
 *  C:\\mygame\\textfiles\\myfile.txt, then
 *  C:\\mygame\\myuserfiles\\textfiles\\myfile.txt, then
 *  D:\\mygamescdromdatafiles\\textfiles\\myfile.txt, then, finally, for
 *  textfiles\\myfile.txt inside of C:\\mygame\\installeddatafiles.zip.
 *  Remember that most archive types and platform filesystems store their
 *  filenames in a case-sensitive manner, so you should be careful to specify
 *  it correctly.
 *
 * Files opened through PhysicsFS may NOT contain "." or ".." or ":" as dir
 *  elements. Not only are these meaningless on MacOS Classic and/or Unix,
 *  they are a security hole. Also, symbolic links (which can be found in
 *  some archive types and directly in the filesystem on Unix platforms) are
 *  NOT followed until you call PHYSFS_permitSymbolicLinks(). That's left to
 *  your own discretion, as following a symlink can allow for access outside
 *  the write dir and search paths. For portability, there is no mechanism for
 *  creating new symlinks in PhysicsFS.
 *
 * The write dir is not included in the search path unless you specifically
 *  add it. While you CAN change the write dir as many times as you like,
 *  you should probably set it once and stick to it. Remember that your
 *  program will not have permission to write in every directory on Unix and
 *  NT systems.
 *
 * All files are opened in binary mode; there is no endline conversion for
 *  textfiles. Other than that, PhysicsFS has some convenience functions for
 *  platform-independence. There is a function to tell you the current
 *  platform's dir separator ("\\" on windows, "/" on Unix, ":" on MacOS),
 *  which is needed only to set up your search/write paths. There is a
 *  function to tell you what CD-ROM drives contain accessible discs, and a
 *  function to recommend a good search path, etc.
 *
 * A recommended order for the search path is the write dir, then the base dir,
 *  then the cdrom dir, then any archives discovered. Quake 3 does something
 *  like this, but moves the archives to the start of the search path. Build
 *  Engine games, like Duke Nukem 3D and Blood, place the archives last, and
 *  use the base dir for both searching and writing. There is a helper
 *  function (PHYSFS_setSaneConfig()) that puts together a basic configuration
 *  for you, based on a few parameters. Also see the comments on
 *  PHYSFS_getBaseDir(), and PHYSFS_getPrefDir() for info on what those
 *  are and how they can help you determine an optimal search path.
 *
 * PhysicsFS 2.0 adds the concept of "mounting" archives to arbitrary points
 *  in the search path. If a zipfile contains "maps/level.map" and you mount
 *  that archive at "mods/mymod", then you would have to open
 *  "mods/mymod/maps/level.map" to access the file, even though "mods/mymod"
 *  isn't actually specified in the .zip file. Unlike the Unix mentality of
 *  mounting a filesystem, "mods/mymod" doesn't actually have to exist when
 *  mounting the zipfile. It's a "virtual" directory. The mounting mechanism
 *  allows the developer to seperate archives in the tree and avoid trampling
 *  over files when added new archives, such as including mod support in a
 *  game...keeping external content on a tight leash in this manner can be of
 *  utmost importance to some applications.
 *
 * PhysicsFS is mostly thread safe. The errors returned by
 *  PHYSFS_getLastErrorCode() are unique by thread, and library-state-setting
 *  functions are mutex'd. For efficiency, individual file accesses are 
 *  not locked, so you can not safely read/write/seek/close/etc the same 
 *  file from two threads at the same time. Other race conditions are bugs 
 *  that should be reported/patched.
 *
 * While you CAN use stdio/syscall file access in a program that has PHYSFS_*
 *  calls, doing so is not recommended, and you can not directly use system
 *  filehandles with PhysicsFS and vice versa (but as of PhysicsFS 2.1, you
 *  can wrap them in a PHYSFS_Io interface yourself if you wanted to).
 *
 * Note that archives need not be named as such: if you have a ZIP file and
 *  rename it with a .PKG extension, the file will still be recognized as a
 *  ZIP archive by PhysicsFS; the file's contents are used to determine its
 *  type where possible.
 *
 * Currently supported archive types:
 *   - .ZIP (pkZip/WinZip/Info-ZIP compatible)
 *   - .7Z  (7zip archives)
 *   - .ISO (ISO9660 files, CD-ROM images)
 *   - .GRP (Build Engine groupfile archives)
 *   - .PAK (Quake I/II archive format)
 *   - .HOG (Descent I/II HOG file archives)
 *   - .MVL (Descent II movielib archives)
 *   - .WAD (DOOM engine archives)
 *   - .VDF (Gothic I/II engine archives)
 *   - .SLB (Independence War archives)
 *
 * String policy for PhysicsFS 2.0 and later:
 *
 * PhysicsFS 1.0 could only deal with null-terminated ASCII strings. All high
 *  ASCII chars resulted in undefined behaviour, and there was no Unicode
 *  support at all. PhysicsFS 2.0 supports Unicode without breaking binary
 *  compatibility with the 1.0 API by using UTF-8 encoding of all strings
 *  passed in and out of the library.
 *
 * All strings passed through PhysicsFS are in null-terminated UTF-8 format.
 *  This means that if all you care about is English (ASCII characters <= 127)
 *  then you just use regular C strings. If you care about Unicode (and you
 *  should!) then you need to figure out what your platform wants, needs, and
 *  offers. If you are on Windows before Win2000 and build with Unicode
 *  support, your TCHAR strings are two bytes per character (this is called
 *  "UCS-2 encoding"). Any modern Windows uses UTF-16, which is two bytes
 *  per character for most characters, but some characters are four. You
 *  should convert them to UTF-8 before handing them to PhysicsFS with
 *  PHYSFS_utf8FromUtf16(), which handles both UTF-16 and UCS-2. If you're
 *  using Unix or Mac OS X, your wchar_t strings are four bytes per character
 *  ("UCS-4 encoding", sometimes called "UTF-32"). Use PHYSFS_utf8FromUcs4().
 *  Mac OS X can give you UTF-8 directly from a CFString or NSString, and many
 *  Unixes generally give you C strings in UTF-8 format everywhere. If you
 *  have a single-byte high ASCII charset, like so-many European "codepages"
 *  you may be out of luck. We'll convert from "Latin1" to UTF-8 only, and
 *  never back to Latin1. If you're above ASCII 127, all bets are off: move
 *  to Unicode or use your platform's facilities. Passing a C string with
 *  high-ASCII data that isn't UTF-8 encoded will NOT do what you expect!
 *
 * Naturally, there's also PHYSFS_utf8ToUcs2(), PHYSFS_utf8ToUtf16(), and
 *  PHYSFS_utf8ToUcs4() to get data back into a format you like. Behind the
 *  scenes, PhysicsFS will use Unicode where possible: the UTF-8 strings on
 *  Windows will be converted and used with the multibyte Windows APIs, for
 *  example.
 *
 * PhysicsFS offers basic encoding conversion support, but not a whole string
 *  library. Get your stuff into whatever format you can work with.
 *
 * Most platforms supported by PhysicsFS 2.1 and later fully support Unicode.
 *  Some older platforms have been dropped (Windows 95, Mac OS 9). Some, like
 *  OS/2, might be able to convert to a local codepage or will just fail to
 *  open/create the file. Modern OSes (macOS, Linux, Windows, etc) should all
 *  be fine.
 *
 * Many game-specific archivers are seriously unprepared for Unicode (the
 *  Descent HOG/MVL and Build Engine GRP archivers, for example, only offer a
 *  DOS 8.3 filename, for example). Nothing can be done for these, but they
 *  tend to be legacy formats for existing content that was all ASCII (and
 *  thus, valid UTF-8) anyhow. Other formats, like .ZIP, don't explicitly
 *  offer Unicode support, but unofficially expect filenames to be UTF-8
 *  encoded, and thus Just Work. Most everything does the right thing without
 *  bothering you, but it's good to be aware of these nuances in case they
 *  don't.
 *
 *
 * Other stuff:
 *
 * Please see the file LICENSE.txt in the source's root directory for
 *  licensing and redistribution rights.
 *
 * Please see the file CREDITS.txt in the source's "docs" directory for
 *  a more or less complete list of who's responsible for this.
 *
 *  \author Ryan C. Gordon.
 */

#ifndef _INCLUDE_PHYSFS_H_
#define _INCLUDE_PHYSFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(PHYSFS_DECL)
/* do nothing. */
#elif defined(_MSC_VER)
#define PHYSFS_DECL __declspec(dllexport)
#elif defined(__SUNPRO_C)
#define PHYSFS_DECL __global
#elif ((__GNUC__ >= 3) && (!defined(__EMX__)) && (!defined(sun)))
#define PHYSFS_DECL __attribute__((visibility("default")))
#else
#define PHYSFS_DECL
#endif

#if defined(PHYSFS_DEPRECATED)
/* do nothing. */
#elif (__GNUC__ >= 4)  /* technically, this arrived in gcc 3.1, but oh well. */
#define PHYSFS_DEPRECATED __attribute__((deprecated))
#else
#define PHYSFS_DEPRECATED
#endif

#if 0  /* !!! FIXME: look into this later. */
#if defined(PHYSFS_CALL)
/* do nothing. */
#elif defined(__WIN32__) && !defined(__GNUC__)
#define PHYSFS_CALL __cdecl
#elif defined(__OS2__) || defined(OS2) /* should work across all compilers. */
#define PHYSFS_CALL _System
#else
#define PHYSFS_CALL
#endif
#endif

/**
 * \typedef PHYSFS_uint8
 * \brief An unsigned, 8-bit integer type.
 */
typedef unsigned char         PHYSFS_uint8;

/**
 * \typedef PHYSFS_sint8
 * \brief A signed, 8-bit integer type.
 */
typedef signed char           PHYSFS_sint8;

/**
 * \typedef PHYSFS_uint16
 * \brief An unsigned, 16-bit integer type.
 */
typedef unsigned short        PHYSFS_uint16;

/**
 * \typedef PHYSFS_sint16
 * \brief A signed, 16-bit integer type.
 */
typedef signed short          PHYSFS_sint16;

/**
 * \typedef PHYSFS_uint32
 * \brief An unsigned, 32-bit integer type.
 */
typedef unsigned int          PHYSFS_uint32;

/**
 * \typedef PHYSFS_sint32
 * \brief A signed, 32-bit integer type.
 */
typedef signed int            PHYSFS_sint32;

/**
 * \typedef PHYSFS_uint64
 * \brief An unsigned, 64-bit integer type.
 * \warning on platforms without any sort of 64-bit datatype, this is
 *           equivalent to PHYSFS_uint32!
 */

/**
 * \typedef PHYSFS_sint64
 * \brief A signed, 64-bit integer type.
 * \warning on platforms without any sort of 64-bit datatype, this is
 *           equivalent to PHYSFS_sint32!
 */


#if (defined PHYSFS_NO_64BIT_SUPPORT)  /* oh well. */
typedef PHYSFS_uint32         PHYSFS_uint64;
typedef PHYSFS_sint32         PHYSFS_sint64;
#elif (defined _MSC_VER)
typedef signed __int64        PHYSFS_sint64;
typedef unsigned __int64      PHYSFS_uint64;
#else
typedef unsigned long long    PHYSFS_uint64;
typedef signed long long      PHYSFS_sint64;
#endif


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/* Make sure the types really have the right sizes */
#define PHYSFS_COMPILE_TIME_ASSERT(name, x) \
       typedef int PHYSFS_compile_time_assert_##name[(x) * 2 - 1]

PHYSFS_COMPILE_TIME_ASSERT(uint8IsOneByte, sizeof(PHYSFS_uint8) == 1);
PHYSFS_COMPILE_TIME_ASSERT(sint8IsOneByte, sizeof(PHYSFS_sint8) == 1);
PHYSFS_COMPILE_TIME_ASSERT(uint16IsTwoBytes, sizeof(PHYSFS_uint16) == 2);
PHYSFS_COMPILE_TIME_ASSERT(sint16IsTwoBytes, sizeof(PHYSFS_sint16) == 2);
PHYSFS_COMPILE_TIME_ASSERT(uint32IsFourBytes, sizeof(PHYSFS_uint32) == 4);
PHYSFS_COMPILE_TIME_ASSERT(sint32IsFourBytes, sizeof(PHYSFS_sint32) == 4);

#ifndef PHYSFS_NO_64BIT_SUPPORT
PHYSFS_COMPILE_TIME_ASSERT(uint64IsEightBytes, sizeof(PHYSFS_uint64) == 8);
PHYSFS_COMPILE_TIME_ASSERT(sint64IsEightBytes, sizeof(PHYSFS_sint64) == 8);
#endif

#undef PHYSFS_COMPILE_TIME_ASSERT

#endif  /* DOXYGEN_SHOULD_IGNORE_THIS */


/**
 * \struct PHYSFS_File
 * \brief A PhysicsFS file handle.
 *
 * You get a pointer to one of these when you open a file for reading,
 *  writing, or appending via PhysicsFS.
 *
 * As you can see from the lack of meaningful fields, you should treat this
 *  as opaque data. Don't try to manipulate the file handle, just pass the
 *  pointer you got, unmolested, to various PhysicsFS APIs.
 *
 * \sa PHYSFS_openRead
 * \sa PHYSFS_openWrite
 * \sa PHYSFS_openAppend
 * \sa PHYSFS_close
 * \sa PHYSFS_read
 * \sa PHYSFS_write
 * \sa PHYSFS_seek
 * \sa PHYSFS_tell
 * \sa PHYSFS_eof
 * \sa PHYSFS_setBuffer
 * \sa PHYSFS_flush
 */
typedef struct PHYSFS_File
{
    void *opaque;  /**< That's all you get. Don't touch. */
} PHYSFS_File;


/**
 * \def PHYSFS_file
 * \brief 1.0 API compatibility define.
 *
 * PHYSFS_file is identical to PHYSFS_File. This #define is here for backwards
 *  compatibility with the 1.0 API, which had an inconsistent capitalization
 *  convention in this case. New code should use PHYSFS_File, as this #define
 *  may go away someday.
 *
 * \sa PHYSFS_File
 */
#define PHYSFS_file PHYSFS_File


/**
 * \struct PHYSFS_ArchiveInfo
 * \brief Information on various PhysicsFS-supported archives.
 *
 * This structure gives you details on what sort of archives are supported
 *  by this implementation of PhysicsFS. Archives tend to be things like
 *  ZIP files and such.
 *
 * \warning Not all binaries are created equal! PhysicsFS can be built with
 *          or without support for various archives. You can check with
 *          PHYSFS_supportedArchiveTypes() to see if your archive type is
 *          supported.
 *
 * \sa PHYSFS_supportedArchiveTypes
 * \sa PHYSFS_registerArchiver
 * \sa PHYSFS_deregisterArchiver
 */
typedef struct PHYSFS_ArchiveInfo
{
    const char *extension;   /**< Archive file extension: "ZIP", for example. */
    const char *description; /**< Human-readable archive description. */
    const char *author;      /**< Person who did support for this archive. */
    const char *url;         /**< URL related to this archive */
    int supportsSymlinks;    /**< non-zero if archive offers symbolic links. */
} PHYSFS_ArchiveInfo;


/**
 * \struct PHYSFS_Version
 * \brief Information the version of PhysicsFS in use.
 *
 * Represents the library's version as three levels: major revision
 *  (increments with massive changes, additions, and enhancements),
 *  minor revision (increments with backwards-compatible changes to the
 *  major revision), and patchlevel (increments with fixes to the minor
 *  revision).
 *
 * \sa PHYSFS_VERSION
 * \sa PHYSFS_getLinkedVersion
 */
typedef struct PHYSFS_Version
{
    PHYSFS_uint8 major; /**< major revision */
    PHYSFS_uint8 minor; /**< minor revision */
    PHYSFS_uint8 patch; /**< patchlevel */
} PHYSFS_Version;


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#define PHYSFS_VER_MAJOR 3
#define PHYSFS_VER_MINOR 1
#define PHYSFS_VER_PATCH 0
#endif  /* DOXYGEN_SHOULD_IGNORE_THIS */


/* PhysicsFS state stuff ... */

/**
 * \def PHYSFS_VERSION(x)
 * \brief Macro to determine PhysicsFS version program was compiled against.
 *
 * This macro fills in a PHYSFS_Version structure with the version of the
 *  library you compiled against. This is determined by what header the
 *  compiler uses. Note that if you dynamically linked the library, you might
 *  have a slightly newer or older version at runtime. That version can be
 *  determined with PHYSFS_getLinkedVersion(), which, unlike PHYSFS_VERSION,
 *  is not a macro.
 *
 * \param x A pointer to a PHYSFS_Version struct to initialize.
 *
 * \sa PHYSFS_Version
 * \sa PHYSFS_getLinkedVersion
 */
#define PHYSFS_VERSION(x) \
{ \
    (x)->major = PHYSFS_VER_MAJOR; \
    (x)->minor = PHYSFS_VER_MINOR; \
    (x)->patch = PHYSFS_VER_PATCH; \
}


/**
 * \fn void PHYSFS_getLinkedVersion(PHYSFS_Version *ver)
 * \brief Get the version of PhysicsFS that is linked against your program.
 *
 * If you are using a shared library (DLL) version of PhysFS, then it is
 *  possible that it will be different than the version you compiled against.
 *
 * This is a real function; the macro PHYSFS_VERSION tells you what version
 *  of PhysFS you compiled against:
 *
 * \code
 * PHYSFS_Version compiled;
 * PHYSFS_Version linked;
 *
 * PHYSFS_VERSION(&compiled);
 * PHYSFS_getLinkedVersion(&linked);
 * printf("We compiled against PhysFS version %d.%d.%d ...\n",
 *           compiled.major, compiled.minor, compiled.patch);
 * printf("But we linked against PhysFS version %d.%d.%d.\n",
 *           linked.major, linked.minor, linked.patch);
 * \endcode
 *
 * This function may be called safely at any time, even before PHYSFS_init().
 *
 * \sa PHYSFS_VERSION
 */
PHYSFS_DECL void PHYSFS_getLinkedVersion(PHYSFS_Version *ver);


/**
 * \fn int PHYSFS_init(const char *argv0)
 * \brief Initialize the PhysicsFS library.
 *
 * This must be called before any other PhysicsFS function.
 *
 * This should be called prior to any attempts to change your process's
 *  current working directory.
 *
 *   \param argv0 the argv[0] string passed to your program's mainline.
 *          This may be NULL on most platforms (such as ones without a
 *          standard main() function), but you should always try to pass
 *          something in here. Unix-like systems such as Linux _need_ to
 *          pass argv[0] from main() in here.
 *  \return nonzero on success, zero on error. Specifics of the error can be
 *          gleaned from PHYSFS_getLastError().
 *
 * \sa PHYSFS_deinit
 * \sa PHYSFS_isInit
 */
PHYSFS_DECL int PHYSFS_init(const char *argv0);


/**
 * \fn int PHYSFS_deinit(void)
 * \brief Deinitialize the PhysicsFS library.
 *
 * This closes any files opened via PhysicsFS, blanks the search/write paths,
 *  frees memory, and invalidates all of your file handles.
 *
 * Note that this call can FAIL if there's a file open for writing that
 *  refuses to close (for example, the underlying operating system was
 *  buffering writes to network filesystem, and the fileserver has crashed,
 *  or a hard drive has failed, etc). It is usually best to close all write
 *  handles yourself before calling this function, so that you can gracefully
 *  handle a specific failure.
 *
 * Once successfully deinitialized, PHYSFS_init() can be called again to
 *  restart the subsystem. All default API states are restored at this
 *  point, with the exception of any custom allocator you might have
 *  specified, which survives between initializations.
 *
 *  \return nonzero on success, zero on error. Specifics of the error can be
 *          gleaned from PHYSFS_getLastError(). If failure, state of PhysFS is
 *          undefined, and probably badly screwed up.
 *
 * \sa PHYSFS_init
 * \sa PHYSFS_isInit
 */
PHYSFS_DECL int PHYSFS_deinit(void);


/**
 * \fn const PHYSFS_ArchiveInfo **PHYSFS_supportedArchiveTypes(void)
 * \brief Get a list of supported archive types.
 *
 * Get a list of archive types supported by this implementation of PhysicFS.
 *  These are the file formats usable for search path entries. This is for
 *  informational purposes only. Note that the extension listed is merely
 *  convention: if we list "ZIP", you can open a PkZip-compatible archive
 *  with an extension of "XYZ", if you like.
 *
 * The returned value is an array of pointers to PHYSFS_ArchiveInfo structures,
 *  with a NULL entry to signify the end of the list:
 *
 * \code
 * PHYSFS_ArchiveInfo **i;
 *
 * for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
 * {
 *     printf("Supported archive: [%s], which is [%s].\n",
 *              (*i)->extension, (*i)->description);
 * }
 * \endcode
 *
 * The return values are pointers to internal memory, and should
 *  be considered READ ONLY, and never freed. The returned values are
 *  valid until the next call to PHYSFS_deinit(), PHYSFS_registerArchiver(),
 *  or PHYSFS_deregisterArchiver().
 *
 *   \return READ ONLY Null-terminated array of READ ONLY structures.
 *
 * \sa PHYSFS_registerArchiver
 * \sa PHYSFS_deregisterArchiver
 */
PHYSFS_DECL const PHYSFS_ArchiveInfo **PHYSFS_supportedArchiveTypes(void);


/**
 * \fn void PHYSFS_freeList(void *listVar)
 * \brief Deallocate resources of lists returned by PhysicsFS.
 *
 * Certain PhysicsFS functions return lists of information that are
 *  dynamically allocated. Use this function to free those resources.
 *
 * It is safe to pass a NULL here, but doing so will cause a crash in versions
 *  before PhysicsFS 2.1.0.
 *
 *   \param listVar List of information specified as freeable by this function.
 *                  Passing NULL is safe; it is a valid no-op.
 *
 * \sa PHYSFS_getCdRomDirs
 * \sa PHYSFS_enumerateFiles
 * \sa PHYSFS_getSearchPath
 */
PHYSFS_DECL void PHYSFS_freeList(void *listVar);


/**
 * \fn const char *PHYSFS_getLastError(void)
 * \brief Get human-readable error information.
 *
 * \deprecated Use PHYSFS_getLastErrorCode() and PHYSFS_getErrorByCode() instead.
 *
 * \warning As of PhysicsFS 2.1, this function has been nerfed.
 *          Before PhysicsFS 2.1, this function was the only way to get
 *          error details beyond a given function's basic return value.
 *          This was meant to be a human-readable string in one of several
 *          languages, and was not useful for application parsing. This was
 *          a problem, because the developer and not the user chose the
 *          language at compile time, and the PhysicsFS maintainers had
 *          to (poorly) maintain a significant amount of localization work.
 *          The app couldn't parse the strings, even if they counted on a
 *          specific language, since some were dynamically generated.
 *          In 2.1 and later, this always returns a static string in
 *          English; you may use it as a key string for your own
 *          localizations if you like, as we'll promise not to change
 *          existing error strings. Also, if your application wants to
 *          look at specific errors, we now offer a better option:
 *          use PHYSFS_getLastErrorCode() instead.
 *
 * Get the last PhysicsFS error message as a human-readable, null-terminated
 *  string. This will return NULL if there's been no error since the last call
 *  to this function. The pointer returned by this call points to an internal
 *  buffer. Each thread has a unique error state associated with it, but each
 *  time a new error message is set, it will overwrite the previous one
 *  associated with that thread. It is safe to call this function at anytime,
 *  even before PHYSFS_init().
 *
 * PHYSFS_getLastError() and PHYSFS_getLastErrorCode() both reset the same
 *  thread-specific error state. Calling one will wipe out the other's
 *  data. If you need both, call PHYSFS_getLastErrorCode(), then pass that
 *  value to PHYSFS_getErrorByCode().
 *
 * As of PhysicsFS 2.1, this function only presents text in the English
 *  language, but the strings are static, so you can use them as keys into
 *  your own localization dictionary. These strings are meant to be passed on
 *  directly to the user.
 *
 * Generally, applications should only concern themselves with whether a
 *  given function failed; however, if your code require more specifics, you
 *  should use PHYSFS_getLastErrorCode() instead of this function.
 *
 *   \return READ ONLY string of last error message.
 *
 * \sa PHYSFS_getLastErrorCode
 * \sa PHYSFS_getErrorByCode
 */
PHYSFS_DECL const char *PHYSFS_getLastError(void) PHYSFS_DEPRECATED;


/**
 * \fn const char *PHYSFS_getDirSeparator(void)
 * \brief Get platform-dependent dir separator string.
 *
 * This returns "\\" on win32, "/" on Unix, and ":" on MacOS. It may be more
 *  than one character, depending on the platform, and your code should take
 *  that into account. Note that this is only useful for setting up the
 *  search/write paths, since access into those dirs always use '/'
 *  (platform-independent notation) to separate directories. This is also
 *  handy for getting platform-independent access when using stdio calls.
 *
 *   \return READ ONLY null-terminated string of platform's dir separator.
 */
PHYSFS_DECL const char *PHYSFS_getDirSeparator(void);


/**
 * \fn void PHYSFS_permitSymbolicLinks(int allow)
 * \brief Enable or disable following of symbolic links.
 *
 * Some physical filesystems and archives contain files that are just pointers
 *  to other files. On the physical filesystem, opening such a link will
 *  (transparently) open the file that is pointed to.
 *
 * By default, PhysicsFS will check if a file is really a symlink during open
 *  calls and fail if it is. Otherwise, the link could take you outside the
 *  write and search paths, and compromise security.
 *
 * If you want to take that risk, call this function with a non-zero parameter.
 *  Note that this is more for sandboxing a program's scripting language, in
 *  case untrusted scripts try to compromise the system. Generally speaking,
 *  a user could very well have a legitimate reason to set up a symlink, so
 *  unless you feel there's a specific danger in allowing them, you should
 *  permit them.
 *
 * Symlinks are only explicitly checked when dealing with filenames
 *  in platform-independent notation. That is, when setting up your
 *  search and write paths, etc, symlinks are never checked for.
 *
 * Please note that PHYSFS_stat() will always check the path specified; if
 *  that path is a symlink, it will not be followed in any case. If symlinks
 *  aren't permitted through this function, PHYSFS_stat() ignores them, and
 *  would treat the query as if the path didn't exist at all.
 *
 * Symbolic link permission can be enabled or disabled at any time after
 *  you've called PHYSFS_init(), and is disabled by default.
 *
 *   \param allow nonzero to permit symlinks, zero to deny linking.
 *
 * \sa PHYSFS_symbolicLinksPermitted
 */
PHYSFS_DECL void PHYSFS_permitSymbolicLinks(int allow);


/**
 * \fn char **PHYSFS_getCdRomDirs(void)
 * \brief Get an array of paths to available CD-ROM drives.
 *
 * The dirs returned are platform-dependent ("D:\" on Win32, "/cdrom" or
 *  whatnot on Unix). Dirs are only returned if there is a disc ready and
 *  accessible in the drive. So if you've got two drives (D: and E:), and only
 *  E: has a disc in it, then that's all you get. If the user inserts a disc
 *  in D: and you call this function again, you get both drives. If, on a
 *  Unix box, the user unmounts a disc and remounts it elsewhere, the next
 *  call to this function will reflect that change.
 *
 * This function refers to "CD-ROM" media, but it really means "inserted disc
 *  media," such as DVD-ROM, HD-DVD, CDRW, and Blu-Ray discs. It looks for
 *  filesystems, and as such won't report an audio CD, unless there's a
 *  mounted filesystem track on it.
 *
 * The returned value is an array of strings, with a NULL entry to signify the
 *  end of the list:
 *
 * \code
 * char **cds = PHYSFS_getCdRomDirs();
 * char **i;
 *
 * for (i = cds; *i != NULL; i++)
 *     printf("cdrom dir [%s] is available.\n", *i);
 *
 * PHYSFS_freeList(cds);
 * \endcode
 *
 * This call may block while drives spin up. Be forewarned.
 *
 * When you are done with the returned information, you may dispose of the
 *  resources by calling PHYSFS_freeList() with the returned pointer.
 *
 *   \return Null-terminated array of null-terminated strings.
 *
 * \sa PHYSFS_getCdRomDirsCallback
 */
PHYSFS_DECL char **PHYSFS_getCdRomDirs(void);


/**
 * \fn const char *PHYSFS_getBaseDir(void)
 * \brief Get the path where the application resides.
 *
 * Helper function.
 *
 * Get the "base dir". This is the directory where the application was run
 *  from, which is probably the installation directory, and may or may not
 *  be the process's current working directory.
 *
 * You should probably use the base dir in your search path.
 *
 *  \return READ ONLY string of base dir in platform-dependent notation.
 *
 * \sa PHYSFS_getPrefDir
 */
PHYSFS_DECL const char *PHYSFS_getBaseDir(void);


/**
 * \fn const char *PHYSFS_getUserDir(void)
 * \brief Get the path where user's home directory resides.
 *
 * \deprecated As of PhysicsFS 2.1, you probably want PHYSFS_getPrefDir().
 *
 * Helper function.
 *
 * Get the "user dir". This is meant to be a suggestion of where a specific
 *  user of the system can store files. On Unix, this is her home directory.
 *  On systems with no concept of multiple home directories (MacOS, win95),
 *  this will default to something like "C:\mybasedir\users\username"
 *  where "username" will either be the login name, or "default" if the
 *  platform doesn't support multiple users, either.
 *
 *  \return READ ONLY string of user dir in platform-dependent notation.
 *
 * \sa PHYSFS_getBaseDir
 * \sa PHYSFS_getPrefDir
 */
PHYSFS_DECL const char *PHYSFS_getUserDir(void) PHYSFS_DEPRECATED;


/**
 * \fn const char *PHYSFS_getWriteDir(void)
 * \brief Get path where PhysicsFS will allow file writing.
 *
 * Get the current write dir. The default write dir is NULL.
 *
 *  \return READ ONLY string of write dir in platform-dependent notation,
 *           OR NULL IF NO WRITE PATH IS CURRENTLY SET.
 *
 * \sa PHYSFS_setWriteDir
 */
PHYSFS_DECL const char *PHYSFS_getWriteDir(void);


/**
 * \fn int PHYSFS_setWriteDir(const char *newDir)
 * \brief Tell PhysicsFS where it may write files.
 *
 * Set a new write dir. This will override the previous setting.
 *
 * This call will fail (and fail to change the write dir) if the current
 *  write dir still has files open in it.
 *
 *   \param newDir The new directory to be the root of the write dir,
 *                   specified in platform-dependent notation. Setting to NULL
 *                   disables the write dir, so no files can be opened for
 *                   writing via PhysicsFS.
 *  \return non-zero on success, zero on failure. All attempts to open a file
 *           for writing via PhysicsFS will fail until this call succeeds.
 *           Use PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_getWriteDir
 */
PHYSFS_DECL int PHYSFS_setWriteDir(const char *newDir);


/**
 * \fn int PHYSFS_addToSearchPath(const char *newDir, int appendToPath)
 * \brief Add an archive or directory to the search path.
 *
 * \deprecated As of PhysicsFS 2.0, use PHYSFS_mount() instead. This
 *             function just wraps it anyhow.
 *
 * This function is equivalent to:
 *
 * \code
 *  PHYSFS_mount(newDir, NULL, appendToPath);
 * \endcode
 *
 * You must use this and not PHYSFS_mount if binary compatibility with
 *  PhysicsFS 1.0 is important (which it may not be for many people).
 *
 * \sa PHYSFS_mount
 * \sa PHYSFS_removeFromSearchPath
 * \sa PHYSFS_getSearchPath
 */
PHYSFS_DECL int PHYSFS_addToSearchPath(const char *newDir, int appendToPath)
                                        PHYSFS_DEPRECATED;

/**
 * \fn int PHYSFS_removeFromSearchPath(const char *oldDir)
 * \brief Remove a directory or archive from the search path.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_unmount() instead. This
 *             function just wraps it anyhow. There's no functional difference
 *             except the vocabulary changed from "adding to the search path"
 *             to "mounting" when that functionality was extended, and thus
 *             the preferred way to accomplish this function's work is now
 *             called "unmounting."
 *
 * This function is equivalent to:
 *
 * \code
 *  PHYSFS_unmount(oldDir);
 * \endcode
 *
 * You must use this and not PHYSFS_unmount if binary compatibility with
 *  PhysicsFS 1.0 is important (which it may not be for many people).
 *
 * \sa PHYSFS_addToSearchPath
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_unmount
 */
PHYSFS_DECL int PHYSFS_removeFromSearchPath(const char *oldDir)
                                            PHYSFS_DEPRECATED;


/**
 * \fn char **PHYSFS_getSearchPath(void)
 * \brief Get the current search path.
 *
 * The default search path is an empty list.
 *
 * The returned value is an array of strings, with a NULL entry to signify the
 *  end of the list:
 *
 * \code
 * char **i;
 *
 * for (i = PHYSFS_getSearchPath(); *i != NULL; i++)
 *     printf("[%s] is in the search path.\n", *i);
 * \endcode
 *
 * When you are done with the returned information, you may dispose of the
 *  resources by calling PHYSFS_freeList() with the returned pointer.
 *
 *   \return Null-terminated array of null-terminated strings. NULL if there
 *            was a problem (read: OUT OF MEMORY).
 *
 * \sa PHYSFS_getSearchPathCallback
 * \sa PHYSFS_addToSearchPath
 * \sa PHYSFS_removeFromSearchPath
 */
PHYSFS_DECL char **PHYSFS_getSearchPath(void);


/**
 * \fn int PHYSFS_setSaneConfig(const char *organization, const char *appName, const char *archiveExt, int includeCdRoms, int archivesFirst)
 * \brief Set up sane, default paths.
 *
 * Helper function.
 *
 * The write dir will be set to the pref dir returned by
 *  \code PHYSFS_getPrefDir(organization, appName) \endcode, which is
 *  created if it doesn't exist.
 *
 * The above is sufficient to make sure your program's configuration directory
 *  is separated from other clutter, and platform-independent.
 *
 *  The search path will be:
 *
 *    - The Write Dir (created if it doesn't exist)
 *    - The Base Dir (PHYSFS_getBaseDir())
 *    - All found CD-ROM dirs (optionally)
 *
 * These directories are then searched for files ending with the extension
 *  (archiveExt), which, if they are valid and supported archives, will also
 *  be added to the search path. If you specified "PKG" for (archiveExt), and
 *  there's a file named data.PKG in the base dir, it'll be checked. Archives
 *  can either be appended or prepended to the search path in alphabetical
 *  order, regardless of which directories they were found in. All archives
 *  are mounted in the root of the virtual file system ("/").
 *
 * All of this can be accomplished from the application, but this just does it
 *  all for you. Feel free to add more to the search path manually, too.
 *
 *    \param organization Name of your company/group/etc to be used as a
 *                         dirname, so keep it small, and no-frills.
 *
 *    \param appName Program-specific name of your program, to separate it
 *                   from other programs using PhysicsFS.
 *
 *    \param archiveExt File extension used by your program to specify an
 *                      archive. For example, Quake 3 uses "pk3", even though
 *                      they are just zipfiles. Specify NULL to not dig out
 *                      archives automatically. Do not specify the '.' char;
 *                      If you want to look for ZIP files, specify "ZIP" and
 *                      not ".ZIP" ... the archive search is case-insensitive.
 *
 *    \param includeCdRoms Non-zero to include CD-ROMs in the search path, and
 *                         (if (archiveExt) != NULL) search them for archives.
 *                         This may cause a significant amount of blocking
 *                         while discs are accessed, and if there are no discs
 *                         in the drive (or even not mounted on Unix systems),
 *                         then they may not be made available anyhow. You may
 *                         want to specify zero and handle the disc setup
 *                         yourself.
 *
 *    \param archivesFirst Non-zero to prepend the archives to the search path.
 *                         Zero to append them. Ignored if !(archiveExt).
 *
 *  \return nonzero on success, zero on error. Use PHYSFS_getLastErrorCode()
 *          to obtain the specific error.
 */
PHYSFS_DECL int PHYSFS_setSaneConfig(const char *organization,
                                     const char *appName,
                                     const char *archiveExt,
                                     int includeCdRoms,
                                     int archivesFirst);


/* Directory management stuff ... */

/**
 * \fn int PHYSFS_mkdir(const char *dirName)
 * \brief Create a directory.
 *
 * This is specified in platform-independent notation in relation to the
 *  write dir. All missing parent directories are also created if they
 *  don't exist.
 *
 * So if you've got the write dir set to "C:\mygame\writedir" and call
 *  PHYSFS_mkdir("downloads/maps") then the directories
 *  "C:\mygame\writedir\downloads" and "C:\mygame\writedir\downloads\maps"
 *  will be created if possible. If the creation of "maps" fails after we
 *  have successfully created "downloads", then the function leaves the
 *  created directory behind and reports failure.
 *
 *   \param dirName New dir to create.
 *  \return nonzero on success, zero on error. Use
 *          PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_delete
 */
PHYSFS_DECL int PHYSFS_mkdir(const char *dirName);


/**
 * \fn int PHYSFS_delete(const char *filename)
 * \brief Delete a file or directory.
 *
 * (filename) is specified in platform-independent notation in relation to the
 *  write dir.
 *
 * A directory must be empty before this call can delete it.
 *
 * Deleting a symlink will remove the link, not what it points to, regardless
 *  of whether you "permitSymLinks" or not.
 *
 * So if you've got the write dir set to "C:\mygame\writedir" and call
 *  PHYSFS_delete("downloads/maps/level1.map") then the file
 *  "C:\mygame\writedir\downloads\maps\level1.map" is removed from the
 *  physical filesystem, if it exists and the operating system permits the
 *  deletion.
 *
 * Note that on Unix systems, deleting a file may be successful, but the
 *  actual file won't be removed until all processes that have an open
 *  filehandle to it (including your program) close their handles.
 *
 * Chances are, the bits that make up the file still exist, they are just
 *  made available to be written over at a later point. Don't consider this
 *  a security method or anything.  :)
 *
 *   \param filename Filename to delete.
 *  \return nonzero on success, zero on error. Use PHYSFS_getLastErrorCode()
 *          to obtain the specific error.
 */
PHYSFS_DECL int PHYSFS_delete(const char *filename);


/**
 * \fn const char *PHYSFS_getRealDir(const char *filename)
 * \brief Figure out where in the search path a file resides.
 *
 * The file is specified in platform-independent notation. The returned
 *  filename will be the element of the search path where the file was found,
 *  which may be a directory, or an archive. Even if there are multiple
 *  matches in different parts of the search path, only the first one found
 *  is used, just like when opening a file.
 *
 * So, if you look for "maps/level1.map", and C:\\mygame is in your search
 *  path and C:\\mygame\\maps\\level1.map exists, then "C:\mygame" is returned.
 *
 * If a any part of a match is a symbolic link, and you've not explicitly
 *  permitted symlinks, then it will be ignored, and the search for a match
 *  will continue.
 *
 * If you specify a fake directory that only exists as a mount point, it'll
 *  be associated with the first archive mounted there, even though that
 *  directory isn't necessarily contained in a real archive.
 *
 * \warning This will return NULL if there is no real directory associated
 *          with (filename). Specifically, PHYSFS_mountIo(),
 *          PHYSFS_mountMemory(), and PHYSFS_mountHandle() will return NULL
 *          even if the filename is found in the search path. Plan accordingly.
 *
 *     \param filename file to look for.
 *    \return READ ONLY string of element of search path containing the
 *             the file in question. NULL if not found.
 */
PHYSFS_DECL const char *PHYSFS_getRealDir(const char *filename);


/**
 * \fn char **PHYSFS_enumerateFiles(const char *dir)
 * \brief Get a file listing of a search path's directory.
 *
 * \warning In PhysicsFS versions prior to 2.1, this function would return
 *          as many items as it could in the face of a failure condition
 *          (out of memory, disk i/o error, etc). Since this meant apps
 *          couldn't distinguish between complete success and partial failure,
 *          and since the function could always return NULL to report
 *          catastrophic failures anyway, in PhysicsFS 2.1 this function's
 *          policy changed: it will either return a list of complete results
 *          or it will return NULL for any failure of any kind, so we can
 *          guarantee that the enumeration ran to completion and has no gaps
 *          in its results.
 *
 * Matching directories are interpolated. That is, if "C:\mydir" is in the
 *  search path and contains a directory "savegames" that contains "x.sav",
 *  "y.sav", and "z.sav", and there is also a "C:\userdir" in the search path
 *  that has a "savegames" subdirectory with "w.sav", then the following code:
 *
 * \code
 * char **rc = PHYSFS_enumerateFiles("savegames");
 * char **i;
 *
 * for (i = rc; *i != NULL; i++)
 *     printf(" * We've got [%s].\n", *i);
 *
 * PHYSFS_freeList(rc);
 * \endcode
 *
 *  \...will print:
 *
 * \verbatim
 * We've got [x.sav].
 * We've got [y.sav].
 * We've got [z.sav].
 * We've got [w.sav].\endverbatim
 *
 * Feel free to sort the list however you like. However, the returned data
 *  will always contain no duplicates, and will be always sorted in alphabetic
 *  (rather: case-sensitive Unicode) order for you.
 *
 * Don't forget to call PHYSFS_freeList() with the return value from this
 *  function when you are done with it.
 *
 *    \param dir directory in platform-independent notation to enumerate.
 *   \return Null-terminated array of null-terminated strings, or NULL for
 *           failure cases.
 *
 * \sa PHYSFS_enumerate
 */
PHYSFS_DECL char **PHYSFS_enumerateFiles(const char *dir);


/**
 * \fn int PHYSFS_exists(const char *fname)
 * \brief Determine if a file exists in the search path.
 *
 * Reports true if there is an entry anywhere in the search path by the
 *  name of (fname).
 *
 * Note that entries that are symlinks are ignored if
 *  PHYSFS_permitSymbolicLinks(1) hasn't been called, so you
 *  might end up further down in the search path than expected.
 *
 *    \param fname filename in platform-independent notation.
 *   \return non-zero if filename exists. zero otherwise.
 */
PHYSFS_DECL int PHYSFS_exists(const char *fname);


/**
 * \fn int PHYSFS_isDirectory(const char *fname)
 * \brief Determine if a file in the search path is really a directory.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_stat() instead. This
 *             function just wraps it anyhow.
 *
 * Determine if the first occurence of (fname) in the search path is
 *  really a directory entry.
 *
 * Note that entries that are symlinks are ignored if
 *  PHYSFS_permitSymbolicLinks(1) hasn't been called, so you
 *  might end up further down in the search path than expected.
 *
 *    \param fname filename in platform-independent notation.
 *   \return non-zero if filename exists and is a directory.  zero otherwise.
 *
 * \sa PHYSFS_stat
 * \sa PHYSFS_exists
 */
PHYSFS_DECL int PHYSFS_isDirectory(const char *fname) PHYSFS_DEPRECATED;


/**
 * \fn int PHYSFS_isSymbolicLink(const char *fname)
 * \brief Determine if a file in the search path is really a symbolic link.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_stat() instead. This
 *             function just wraps it anyhow.
 *
 * Determine if the first occurence of (fname) in the search path is
 *  really a symbolic link.
 *
 * Note that entries that are symlinks are ignored if
 *  PHYSFS_permitSymbolicLinks(1) hasn't been called, and as such,
 *  this function will always return 0 in that case.
 *
 *    \param fname filename in platform-independent notation.
 *   \return non-zero if filename exists and is a symlink.  zero otherwise.
 *
 * \sa PHYSFS_stat
 * \sa PHYSFS_exists
 */
PHYSFS_DECL int PHYSFS_isSymbolicLink(const char *fname) PHYSFS_DEPRECATED;


/**
 * \fn PHYSFS_sint64 PHYSFS_getLastModTime(const char *filename)
 * \brief Get the last modification time of a file.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_stat() instead. This
 *             function just wraps it anyhow.
 *
 * The modtime is returned as a number of seconds since the Unix epoch
 *  (midnight, Jan 1, 1970). The exact derivation and accuracy of this time
 *  depends on the particular archiver. If there is no reasonable way to
 *  obtain this information for a particular archiver, or there was some sort
 *  of error, this function returns (-1).
 *
 * You must use this and not PHYSFS_stat() if binary compatibility with
 *  PhysicsFS 2.0 is important (which it may not be for many people).
 *
 *   \param filename filename to check, in platform-independent notation.
 *  \return last modified time of the file. -1 if it can't be determined.
 *
 * \sa PHYSFS_stat
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_getLastModTime(const char *filename)
                                                PHYSFS_DEPRECATED;


/* i/o stuff... */

/**
 * \fn PHYSFS_File *PHYSFS_openWrite(const char *filename)
 * \brief Open a file for writing.
 *
 * Open a file for writing, in platform-independent notation and in relation
 *  to the write dir as the root of the writable filesystem. The specified
 *  file is created if it doesn't exist. If it does exist, it is truncated to
 *  zero bytes, and the writing offset is set to the start.
 *
 * Note that entries that are symlinks are ignored if
 *  PHYSFS_permitSymbolicLinks(1) hasn't been called, and opening a
 *  symlink with this function will fail in such a case.
 *
 *   \param filename File to open.
 *  \return A valid PhysicsFS filehandle on success, NULL on error. Use
 *          PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_openRead
 * \sa PHYSFS_openAppend
 * \sa PHYSFS_write
 * \sa PHYSFS_close
 */
PHYSFS_DECL PHYSFS_File *PHYSFS_openWrite(const char *filename);


/**
 * \fn PHYSFS_File *PHYSFS_openAppend(const char *filename)
 * \brief Open a file for appending.
 *
 * Open a file for writing, in platform-independent notation and in relation
 *  to the write dir as the root of the writable filesystem. The specified
 *  file is created if it doesn't exist. If it does exist, the writing offset
 *  is set to the end of the file, so the first write will be the byte after
 *  the end.
 *
 * Note that entries that are symlinks are ignored if
 *  PHYSFS_permitSymbolicLinks(1) hasn't been called, and opening a
 *  symlink with this function will fail in such a case.
 *
 *   \param filename File to open.
 *  \return A valid PhysicsFS filehandle on success, NULL on error. Use
 *          PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_openRead
 * \sa PHYSFS_openWrite
 * \sa PHYSFS_write
 * \sa PHYSFS_close
 */
PHYSFS_DECL PHYSFS_File *PHYSFS_openAppend(const char *filename);


/**
 * \fn PHYSFS_File *PHYSFS_openRead(const char *filename)
 * \brief Open a file for reading.
 *
 * Open a file for reading, in platform-independent notation. The search path
 *  is checked one at a time until a matching file is found, in which case an
 *  abstract filehandle is associated with it, and reading may be done.
 *  The reading offset is set to the first byte of the file.
 *
 * Note that entries that are symlinks are ignored if
 *  PHYSFS_permitSymbolicLinks(1) hasn't been called, and opening a
 *  symlink with this function will fail in such a case.
 *
 *   \param filename File to open.
 *  \return A valid PhysicsFS filehandle on success, NULL on error.
 *          Use PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_openWrite
 * \sa PHYSFS_openAppend
 * \sa PHYSFS_read
 * \sa PHYSFS_close
 */
PHYSFS_DECL PHYSFS_File *PHYSFS_openRead(const char *filename);


/**
 * \fn int PHYSFS_close(PHYSFS_File *handle)
 * \brief Close a PhysicsFS filehandle.
 *
 * This call is capable of failing if the operating system was buffering
 *  writes to the physical media, and, now forced to write those changes to
 *  physical media, can not store the data for some reason. In such a case,
 *  the filehandle stays open. A well-written program should ALWAYS check the
 *  return value from the close call in addition to every writing call!
 *
 *   \param handle handle returned from PHYSFS_open*().
 *  \return nonzero on success, zero on error. Use PHYSFS_getLastErrorCode()
 *          to obtain the specific error.
 *
 * \sa PHYSFS_openRead
 * \sa PHYSFS_openWrite
 * \sa PHYSFS_openAppend
 */
PHYSFS_DECL int PHYSFS_close(PHYSFS_File *handle);


/**
 * \fn PHYSFS_sint64 PHYSFS_read(PHYSFS_File *handle, void *buffer, PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
 * \brief Read data from a PhysicsFS filehandle
 *
 * The file must be opened for reading.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_readBytes() instead. This
 *             function just wraps it anyhow. This function never clarified
 *             what would happen if you managed to read a partial object, so
 *             working at the byte level makes this cleaner for everyone,
 *             especially now that PHYSFS_Io interfaces can be supplied by the
 *             application.
 *
 *   \param handle handle returned from PHYSFS_openRead().
 *   \param buffer buffer to store read data into.
 *   \param objSize size in bytes of objects being read from (handle).
 *   \param objCount number of (objSize) objects to read from (handle).
 *  \return number of objects read. PHYSFS_getLastErrorCode() can shed light
 *          on the reason this might be < (objCount), as can PHYSFS_eof().
 *          -1 if complete failure.
 *
 * \sa PHYSFS_readBytes
 * \sa PHYSFS_eof
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_read(PHYSFS_File *handle,
                                      void *buffer,
                                      PHYSFS_uint32 objSize,
                                      PHYSFS_uint32 objCount)
                                        PHYSFS_DEPRECATED;

/**
 * \fn PHYSFS_sint64 PHYSFS_write(PHYSFS_File *handle, const void *buffer, PHYSFS_uint32 objSize, PHYSFS_uint32 objCount)
 * \brief Write data to a PhysicsFS filehandle
 *
 * The file must be opened for writing.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_writeBytes() instead. This
 *             function just wraps it anyhow. This function never clarified
 *             what would happen if you managed to write a partial object, so
 *             working at the byte level makes this cleaner for everyone,
 *             especially now that PHYSFS_Io interfaces can be supplied by the
 *             application.
 *
 *   \param handle retval from PHYSFS_openWrite() or PHYSFS_openAppend().
 *   \param buffer buffer of bytes to write to (handle).
 *   \param objSize size in bytes of objects being written to (handle).
 *   \param objCount number of (objSize) objects to write to (handle).
 *  \return number of objects written. PHYSFS_getLastErrorCode() can shed
 *          light on the reason this might be < (objCount). -1 if complete
 *          failure.
 *
 * \sa PHYSFS_writeBytes
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_write(PHYSFS_File *handle,
                                       const void *buffer,
                                       PHYSFS_uint32 objSize,
                                       PHYSFS_uint32 objCount)
                                        PHYSFS_DEPRECATED;


/* File position stuff... */

/**
 * \fn int PHYSFS_eof(PHYSFS_File *handle)
 * \brief Check for end-of-file state on a PhysicsFS filehandle.
 *
 * Determine if the end of file has been reached in a PhysicsFS filehandle.
 *
 *   \param handle handle returned from PHYSFS_openRead().
 *  \return nonzero if EOF, zero if not.
 *
 * \sa PHYSFS_read
 * \sa PHYSFS_tell
 */
PHYSFS_DECL int PHYSFS_eof(PHYSFS_File *handle);


/**
 * \fn PHYSFS_sint64 PHYSFS_tell(PHYSFS_File *handle)
 * \brief Determine current position within a PhysicsFS filehandle.
 *
 *   \param handle handle returned from PHYSFS_open*().
 *  \return offset in bytes from start of file. -1 if error occurred.
 *           Use PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_seek
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_tell(PHYSFS_File *handle);


/**
 * \fn int PHYSFS_seek(PHYSFS_File *handle, PHYSFS_uint64 pos)
 * \brief Seek to a new position within a PhysicsFS filehandle.
 *
 * The next read or write will occur at that place. Seeking past the
 *  beginning or end of the file is not allowed, and causes an error.
 *
 *   \param handle handle returned from PHYSFS_open*().
 *   \param pos number of bytes from start of file to seek to.
 *  \return nonzero on success, zero on error. Use PHYSFS_getLastErrorCode()
 *          to obtain the specific error.
 *
 * \sa PHYSFS_tell
 */
PHYSFS_DECL int PHYSFS_seek(PHYSFS_File *handle, PHYSFS_uint64 pos);


/**
 * \fn PHYSFS_sint64 PHYSFS_fileLength(PHYSFS_File *handle)
 * \brief Get total length of a file in bytes.
 *
 * Note that if another process/thread is writing to this file at the same
 *  time, then the information this function supplies could be incorrect
 *  before you get it. Use with caution, or better yet, don't use at all.
 *
 *   \param handle handle returned from PHYSFS_open*().
 *  \return size in bytes of the file. -1 if can't be determined.
 *
 * \sa PHYSFS_tell
 * \sa PHYSFS_seek
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_fileLength(PHYSFS_File *handle);


/* Buffering stuff... */

/**
 * \fn int PHYSFS_setBuffer(PHYSFS_File *handle, PHYSFS_uint64 bufsize)
 * \brief Set up buffering for a PhysicsFS file handle.
 *
 * Define an i/o buffer for a file handle. A memory block of (bufsize) bytes
 *  will be allocated and associated with (handle).
 *
 * For files opened for reading, up to (bufsize) bytes are read from (handle)
 *  and stored in the internal buffer. Calls to PHYSFS_read() will pull
 *  from this buffer until it is empty, and then refill it for more reading.
 *  Note that compressed files, like ZIP archives, will decompress while
 *  buffering, so this can be handy for offsetting CPU-intensive operations.
 *  The buffer isn't filled until you do your next read.
 *
 * For files opened for writing, data will be buffered to memory until the
 *  buffer is full or the buffer is flushed. Closing a handle implicitly
 *  causes a flush...check your return values!
 *
 * Seeking, etc transparently accounts for buffering.
 *
 * You can resize an existing buffer by calling this function more than once
 *  on the same file. Setting the buffer size to zero will free an existing
 *  buffer.
 *
 * PhysicsFS file handles are unbuffered by default.
 *
 * Please check the return value of this function! Failures can include
 *  not being able to seek backwards in a read-only file when removing the
 *  buffer, not being able to allocate the buffer, and not being able to
 *  flush the buffer to disk, among other unexpected problems.
 *
 *   \param handle handle returned from PHYSFS_open*().
 *   \param bufsize size, in bytes, of buffer to allocate.
 *  \return nonzero if successful, zero on error.
 *
 * \sa PHYSFS_flush
 * \sa PHYSFS_read
 * \sa PHYSFS_write
 * \sa PHYSFS_close
 */
PHYSFS_DECL int PHYSFS_setBuffer(PHYSFS_File *handle, PHYSFS_uint64 bufsize);


/**
 * \fn int PHYSFS_flush(PHYSFS_File *handle)
 * \brief Flush a buffered PhysicsFS file handle.
 *
 * For buffered files opened for writing, this will put the current contents
 *  of the buffer to disk and flag the buffer as empty if possible.
 *
 * For buffered files opened for reading or unbuffered files, this is a safe
 *  no-op, and will report success.
 *
 *   \param handle handle returned from PHYSFS_open*().
 *  \return nonzero if successful, zero on error.
 *
 * \sa PHYSFS_setBuffer
 * \sa PHYSFS_close
 */
PHYSFS_DECL int PHYSFS_flush(PHYSFS_File *handle);


/* Byteorder stuff... */

/**
 * \fn PHYSFS_sint16 PHYSFS_swapSLE16(PHYSFS_sint16 val)
 * \brief Swap littleendian signed 16 to platform's native byte order.
 *
 * Take a 16-bit signed value in littleendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_sint16 PHYSFS_swapSLE16(PHYSFS_sint16 val);


/**
 * \fn PHYSFS_uint16 PHYSFS_swapULE16(PHYSFS_uint16 val)
 * \brief Swap littleendian unsigned 16 to platform's native byte order.
 *
 * Take a 16-bit unsigned value in littleendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_uint16 PHYSFS_swapULE16(PHYSFS_uint16 val);

/**
 * \fn PHYSFS_sint32 PHYSFS_swapSLE32(PHYSFS_sint32 val)
 * \brief Swap littleendian signed 32 to platform's native byte order.
 *
 * Take a 32-bit signed value in littleendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_sint32 PHYSFS_swapSLE32(PHYSFS_sint32 val);


/**
 * \fn PHYSFS_uint32 PHYSFS_swapULE32(PHYSFS_uint32 val)
 * \brief Swap littleendian unsigned 32 to platform's native byte order.
 *
 * Take a 32-bit unsigned value in littleendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_uint32 PHYSFS_swapULE32(PHYSFS_uint32 val);

/**
 * \fn PHYSFS_sint64 PHYSFS_swapSLE64(PHYSFS_sint64 val)
 * \brief Swap littleendian signed 64 to platform's native byte order.
 *
 * Take a 64-bit signed value in littleendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 *
 * \warning Remember, PHYSFS_sint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_swapSLE64(PHYSFS_sint64 val);


/**
 * \fn PHYSFS_uint64 PHYSFS_swapULE64(PHYSFS_uint64 val)
 * \brief Swap littleendian unsigned 64 to platform's native byte order.
 *
 * Take a 64-bit unsigned value in littleendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 *
 * \warning Remember, PHYSFS_uint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL PHYSFS_uint64 PHYSFS_swapULE64(PHYSFS_uint64 val);


/**
 * \fn PHYSFS_sint16 PHYSFS_swapSBE16(PHYSFS_sint16 val)
 * \brief Swap bigendian signed 16 to platform's native byte order.
 *
 * Take a 16-bit signed value in bigendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_sint16 PHYSFS_swapSBE16(PHYSFS_sint16 val);


/**
 * \fn PHYSFS_uint16 PHYSFS_swapUBE16(PHYSFS_uint16 val)
 * \brief Swap bigendian unsigned 16 to platform's native byte order.
 *
 * Take a 16-bit unsigned value in bigendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_uint16 PHYSFS_swapUBE16(PHYSFS_uint16 val);

/**
 * \fn PHYSFS_sint32 PHYSFS_swapSBE32(PHYSFS_sint32 val)
 * \brief Swap bigendian signed 32 to platform's native byte order.
 *
 * Take a 32-bit signed value in bigendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_sint32 PHYSFS_swapSBE32(PHYSFS_sint32 val);


/**
 * \fn PHYSFS_uint32 PHYSFS_swapUBE32(PHYSFS_uint32 val)
 * \brief Swap bigendian unsigned 32 to platform's native byte order.
 *
 * Take a 32-bit unsigned value in bigendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 */
PHYSFS_DECL PHYSFS_uint32 PHYSFS_swapUBE32(PHYSFS_uint32 val);


/**
 * \fn PHYSFS_sint64 PHYSFS_swapSBE64(PHYSFS_sint64 val)
 * \brief Swap bigendian signed 64 to platform's native byte order.
 *
 * Take a 64-bit signed value in bigendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 *
 * \warning Remember, PHYSFS_sint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_swapSBE64(PHYSFS_sint64 val);


/**
 * \fn PHYSFS_uint64 PHYSFS_swapUBE64(PHYSFS_uint64 val)
 * \brief Swap bigendian unsigned 64 to platform's native byte order.
 *
 * Take a 64-bit unsigned value in bigendian format and convert it to
 *  the platform's native byte order.
 *
 *    \param val value to convert
 *   \return converted value.
 *
 * \warning Remember, PHYSFS_uint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL PHYSFS_uint64 PHYSFS_swapUBE64(PHYSFS_uint64 val);


/**
 * \fn int PHYSFS_readSLE16(PHYSFS_File *file, PHYSFS_sint16 *val)
 * \brief Read and convert a signed 16-bit littleendian value.
 *
 * Convenience function. Read a signed 16-bit littleendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_readSLE16(PHYSFS_File *file, PHYSFS_sint16 *val);


/**
 * \fn int PHYSFS_readULE16(PHYSFS_File *file, PHYSFS_uint16 *val)
 * \brief Read and convert an unsigned 16-bit littleendian value.
 *
 * Convenience function. Read an unsigned 16-bit littleendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 */
PHYSFS_DECL int PHYSFS_readULE16(PHYSFS_File *file, PHYSFS_uint16 *val);


/**
 * \fn int PHYSFS_readSBE16(PHYSFS_File *file, PHYSFS_sint16 *val)
 * \brief Read and convert a signed 16-bit bigendian value.
 *
 * Convenience function. Read a signed 16-bit bigendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_readSBE16(PHYSFS_File *file, PHYSFS_sint16 *val);


/**
 * \fn int PHYSFS_readUBE16(PHYSFS_File *file, PHYSFS_uint16 *val)
 * \brief Read and convert an unsigned 16-bit bigendian value.
 *
 * Convenience function. Read an unsigned 16-bit bigendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 */
PHYSFS_DECL int PHYSFS_readUBE16(PHYSFS_File *file, PHYSFS_uint16 *val);


/**
 * \fn int PHYSFS_readSLE32(PHYSFS_File *file, PHYSFS_sint32 *val)
 * \brief Read and convert a signed 32-bit littleendian value.
 *
 * Convenience function. Read a signed 32-bit littleendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_readSLE32(PHYSFS_File *file, PHYSFS_sint32 *val);


/**
 * \fn int PHYSFS_readULE32(PHYSFS_File *file, PHYSFS_uint32 *val)
 * \brief Read and convert an unsigned 32-bit littleendian value.
 *
 * Convenience function. Read an unsigned 32-bit littleendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 */
PHYSFS_DECL int PHYSFS_readULE32(PHYSFS_File *file, PHYSFS_uint32 *val);


/**
 * \fn int PHYSFS_readSBE32(PHYSFS_File *file, PHYSFS_sint32 *val)
 * \brief Read and convert a signed 32-bit bigendian value.
 *
 * Convenience function. Read a signed 32-bit bigendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_readSBE32(PHYSFS_File *file, PHYSFS_sint32 *val);


/**
 * \fn int PHYSFS_readUBE32(PHYSFS_File *file, PHYSFS_uint32 *val)
 * \brief Read and convert an unsigned 32-bit bigendian value.
 *
 * Convenience function. Read an unsigned 32-bit bigendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 */
PHYSFS_DECL int PHYSFS_readUBE32(PHYSFS_File *file, PHYSFS_uint32 *val);


/**
 * \fn int PHYSFS_readSLE64(PHYSFS_File *file, PHYSFS_sint64 *val)
 * \brief Read and convert a signed 64-bit littleendian value.
 *
 * Convenience function. Read a signed 64-bit littleendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_sint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_readSLE64(PHYSFS_File *file, PHYSFS_sint64 *val);


/**
 * \fn int PHYSFS_readULE64(PHYSFS_File *file, PHYSFS_uint64 *val)
 * \brief Read and convert an unsigned 64-bit littleendian value.
 *
 * Convenience function. Read an unsigned 64-bit littleendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_uint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_readULE64(PHYSFS_File *file, PHYSFS_uint64 *val);


/**
 * \fn int PHYSFS_readSBE64(PHYSFS_File *file, PHYSFS_sint64 *val)
 * \brief Read and convert a signed 64-bit bigendian value.
 *
 * Convenience function. Read a signed 64-bit bigendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_sint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_readSBE64(PHYSFS_File *file, PHYSFS_sint64 *val);


/**
 * \fn int PHYSFS_readUBE64(PHYSFS_File *file, PHYSFS_uint64 *val)
 * \brief Read and convert an unsigned 64-bit bigendian value.
 *
 * Convenience function. Read an unsigned 64-bit bigendian value from a
 *  file and convert it to the platform's native byte order.
 *
 *    \param file PhysicsFS file handle from which to read.
 *    \param val pointer to where value should be stored.
 *   \return zero on failure, non-zero on success. If successful, (*val) will
 *           store the result. On failure, you can find out what went wrong
 *           from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_uint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_readUBE64(PHYSFS_File *file, PHYSFS_uint64 *val);


/**
 * \fn int PHYSFS_writeSLE16(PHYSFS_File *file, PHYSFS_sint16 val)
 * \brief Convert and write a signed 16-bit littleendian value.
 *
 * Convenience function. Convert a signed 16-bit value from the platform's
 *  native byte order to littleendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeSLE16(PHYSFS_File *file, PHYSFS_sint16 val);


/**
 * \fn int PHYSFS_writeULE16(PHYSFS_File *file, PHYSFS_uint16 val)
 * \brief Convert and write an unsigned 16-bit littleendian value.
 *
 * Convenience function. Convert an unsigned 16-bit value from the platform's
 *  native byte order to littleendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeULE16(PHYSFS_File *file, PHYSFS_uint16 val);


/**
 * \fn int PHYSFS_writeSBE16(PHYSFS_File *file, PHYSFS_sint16 val)
 * \brief Convert and write a signed 16-bit bigendian value.
 *
 * Convenience function. Convert a signed 16-bit value from the platform's
 *  native byte order to bigendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeSBE16(PHYSFS_File *file, PHYSFS_sint16 val);


/**
 * \fn int PHYSFS_writeUBE16(PHYSFS_File *file, PHYSFS_uint16 val)
 * \brief Convert and write an unsigned 16-bit bigendian value.
 *
 * Convenience function. Convert an unsigned 16-bit value from the platform's
 *  native byte order to bigendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeUBE16(PHYSFS_File *file, PHYSFS_uint16 val);


/**
 * \fn int PHYSFS_writeSLE32(PHYSFS_File *file, PHYSFS_sint32 val)
 * \brief Convert and write a signed 32-bit littleendian value.
 *
 * Convenience function. Convert a signed 32-bit value from the platform's
 *  native byte order to littleendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeSLE32(PHYSFS_File *file, PHYSFS_sint32 val);


/**
 * \fn int PHYSFS_writeULE32(PHYSFS_File *file, PHYSFS_uint32 val)
 * \brief Convert and write an unsigned 32-bit littleendian value.
 *
 * Convenience function. Convert an unsigned 32-bit value from the platform's
 *  native byte order to littleendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeULE32(PHYSFS_File *file, PHYSFS_uint32 val);


/**
 * \fn int PHYSFS_writeSBE32(PHYSFS_File *file, PHYSFS_sint32 val)
 * \brief Convert and write a signed 32-bit bigendian value.
 *
 * Convenience function. Convert a signed 32-bit value from the platform's
 *  native byte order to bigendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeSBE32(PHYSFS_File *file, PHYSFS_sint32 val);


/**
 * \fn int PHYSFS_writeUBE32(PHYSFS_File *file, PHYSFS_uint32 val)
 * \brief Convert and write an unsigned 32-bit bigendian value.
 *
 * Convenience function. Convert an unsigned 32-bit value from the platform's
 *  native byte order to bigendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 */
PHYSFS_DECL int PHYSFS_writeUBE32(PHYSFS_File *file, PHYSFS_uint32 val);


/**
 * \fn int PHYSFS_writeSLE64(PHYSFS_File *file, PHYSFS_sint64 val)
 * \brief Convert and write a signed 64-bit littleendian value.
 *
 * Convenience function. Convert a signed 64-bit value from the platform's
 *  native byte order to littleendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_sint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_writeSLE64(PHYSFS_File *file, PHYSFS_sint64 val);


/**
 * \fn int PHYSFS_writeULE64(PHYSFS_File *file, PHYSFS_uint64 val)
 * \brief Convert and write an unsigned 64-bit littleendian value.
 *
 * Convenience function. Convert an unsigned 64-bit value from the platform's
 *  native byte order to littleendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_uint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_writeULE64(PHYSFS_File *file, PHYSFS_uint64 val);


/**
 * \fn int PHYSFS_writeSBE64(PHYSFS_File *file, PHYSFS_sint64 val)
 * \brief Convert and write a signed 64-bit bigending value.
 *
 * Convenience function. Convert a signed 64-bit value from the platform's
 *  native byte order to bigendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_sint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_writeSBE64(PHYSFS_File *file, PHYSFS_sint64 val);


/**
 * \fn int PHYSFS_writeUBE64(PHYSFS_File *file, PHYSFS_uint64 val)
 * \brief Convert and write an unsigned 64-bit bigendian value.
 *
 * Convenience function. Convert an unsigned 64-bit value from the platform's
 *  native byte order to bigendian and write it to a file.
 *
 *    \param file PhysicsFS file handle to which to write.
 *    \param val Value to convert and write.
 *   \return zero on failure, non-zero on success. On failure, you can
 *           find out what went wrong from PHYSFS_getLastErrorCode().
 *
 * \warning Remember, PHYSFS_uint64 is only 32 bits on platforms without
 *          any sort of 64-bit support.
 */
PHYSFS_DECL int PHYSFS_writeUBE64(PHYSFS_File *file, PHYSFS_uint64 val);


/* Everything above this line is part of the PhysicsFS 1.0 API. */

/**
 * \fn int PHYSFS_isInit(void)
 * \brief Determine if the PhysicsFS library is initialized.
 *
 * Once PHYSFS_init() returns successfully, this will return non-zero.
 *  Before a successful PHYSFS_init() and after PHYSFS_deinit() returns
 *  successfully, this will return zero. This function is safe to call at
 *  any time.
 *
 *  \return non-zero if library is initialized, zero if library is not.
 *
 * \sa PHYSFS_init
 * \sa PHYSFS_deinit
 */
PHYSFS_DECL int PHYSFS_isInit(void);


/**
 * \fn int PHYSFS_symbolicLinksPermitted(void)
 * \brief Determine if the symbolic links are permitted.
 *
 * This reports the setting from the last call to PHYSFS_permitSymbolicLinks().
 *  If PHYSFS_permitSymbolicLinks() hasn't been called since the library was
 *  last initialized, symbolic links are implicitly disabled.
 *
 *  \return non-zero if symlinks are permitted, zero if not.
 *
 * \sa PHYSFS_permitSymbolicLinks
 */
PHYSFS_DECL int PHYSFS_symbolicLinksPermitted(void);


/**
 * \struct PHYSFS_Allocator
 * \brief PhysicsFS allocation function pointers.
 *
 * (This is for limited, hardcore use. If you don't immediately see a need
 *  for it, you can probably ignore this forever.)
 *
 * You create one of these structures for use with PHYSFS_setAllocator.
 *  Allocators are assumed to be reentrant by the caller; please mutex
 *  accordingly.
 *
 * Allocations are always discussed in 64-bits, for future expansion...we're
 *  on the cusp of a 64-bit transition, and we'll probably be allocating 6
 *  gigabytes like it's nothing sooner or later, and I don't want to change
 *  this again at that point. If you're on a 32-bit platform and have to
 *  downcast, it's okay to return NULL if the allocation is greater than
 *  4 gigabytes, since you'd have to do so anyhow.
 *
 * \sa PHYSFS_setAllocator
 */
typedef struct PHYSFS_Allocator
{
    int (*Init)(void);   /**< Initialize. Can be NULL. Zero on failure. */
    void (*Deinit)(void);  /**< Deinitialize your allocator. Can be NULL. */
    void *(*Malloc)(PHYSFS_uint64);  /**< Allocate like malloc(). */
    void *(*Realloc)(void *, PHYSFS_uint64); /**< Reallocate like realloc(). */
    void (*Free)(void *); /**< Free memory from Malloc or Realloc. */
} PHYSFS_Allocator;


/**
 * \fn int PHYSFS_setAllocator(const PHYSFS_Allocator *allocator)
 * \brief Hook your own allocation routines into PhysicsFS.
 *
 * (This is for limited, hardcore use. If you don't immediately see a need
 *  for it, you can probably ignore this forever.)
 *
 * By default, PhysicsFS will use whatever is reasonable for a platform
 *  to manage dynamic memory (usually ANSI C malloc/realloc/free, but
 *  some platforms might use something else), but in some uncommon cases, the
 *  app might want more control over the library's memory management. This
 *  lets you redirect PhysicsFS to use your own allocation routines instead.
 *  You can only call this function before PHYSFS_init(); if the library is
 *  initialized, it'll reject your efforts to change the allocator mid-stream.
 *  You may call this function after PHYSFS_deinit() if you are willing to
 *  shut down the library and restart it with a new allocator; this is a safe
 *  and supported operation. The allocator remains intact between deinit/init
 *  calls. If you want to return to the platform's default allocator, pass a
 *  NULL in here.
 *
 * If you aren't immediately sure what to do with this function, you can
 *  safely ignore it altogether.
 *
 *    \param allocator Structure containing your allocator's entry points.
 *   \return zero on failure, non-zero on success. This call only fails
 *           when used between PHYSFS_init() and PHYSFS_deinit() calls.
 */
PHYSFS_DECL int PHYSFS_setAllocator(const PHYSFS_Allocator *allocator);


/**
 * \fn int PHYSFS_mount(const char *newDir, const char *mountPoint, int appendToPath)
 * \brief Add an archive or directory to the search path.
 *
 * If this is a duplicate, the entry is not added again, even though the
 *  function succeeds. You may not add the same archive to two different
 *  mountpoints: duplicate checking is done against the archive and not the
 *  mountpoint.
 *
 * When you mount an archive, it is added to a virtual file system...all files
 *  in all of the archives are interpolated into a single hierachical file
 *  tree. Two archives mounted at the same place (or an archive with files
 *  overlapping another mountpoint) may have overlapping files: in such a case,
 *  the file earliest in the search path is selected, and the other files are
 *  inaccessible to the application. This allows archives to be used to
 *  override previous revisions; you can use the mounting mechanism to place
 *  archives at a specific point in the file tree and prevent overlap; this
 *  is useful for downloadable mods that might trample over application data
 *  or each other, for example.
 *
 * The mountpoint does not need to exist prior to mounting, which is different
 *  than those familiar with the Unix concept of "mounting" may expect.
 *  As well, more than one archive can be mounted to the same mountpoint, or
 *  mountpoints and archive contents can overlap...the interpolation mechanism
 *  still functions as usual.
 *
 * Specifying a symbolic link to an archive or directory is allowed here,
 *  regardless of the state of PHYSFS_permitSymbolicLinks(). That function
 *  only deals with symlinks inside the mounted directory or archive.
 *
 *   \param newDir directory or archive to add to the path, in
 *                   platform-dependent notation.
 *   \param mountPoint Location in the interpolated tree that this archive
 *                     will be "mounted", in platform-independent notation.
 *                     NULL or "" is equivalent to "/".
 *   \param appendToPath nonzero to append to search path, zero to prepend.
 *  \return nonzero if added to path, zero on failure (bogus archive, dir
 *          missing, etc). Use PHYSFS_getLastErrorCode() to obtain
 *          the specific error.
 *
 * \sa PHYSFS_removeFromSearchPath
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_getMountPoint
 * \sa PHYSFS_mountIo
 */
PHYSFS_DECL int PHYSFS_mount(const char *newDir,
                             const char *mountPoint,
                             int appendToPath);

/**
 * \fn int PHYSFS_getMountPoint(const char *dir)
 * \brief Determine a mounted archive's mountpoint.
 *
 * You give this function the name of an archive or dir you successfully
 *  added to the search path, and it reports the location in the interpolated
 *  tree where it is mounted. Files mounted with a NULL mountpoint or through
 *  PHYSFS_addToSearchPath() will report "/". The return value is READ ONLY
 *  and valid until the archive is removed from the search path.
 *
 *   \param dir directory or archive previously added to the path, in
 *              platform-dependent notation. This must match the string
 *              used when adding, even if your string would also reference
 *              the same file with a different string of characters.
 *  \return READ-ONLY string of mount point if added to path, NULL on failure
 *          (bogus archive, etc). Use PHYSFS_getLastErrorCode() to obtain the
 *          specific error.
 *
 * \sa PHYSFS_removeFromSearchPath
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_getMountPoint
 */
PHYSFS_DECL const char *PHYSFS_getMountPoint(const char *dir);


/**
 * \typedef PHYSFS_StringCallback
 * \brief Function signature for callbacks that report strings.
 *
 * These are used to report a list of strings to an original caller, one
 *  string per callback. All strings are UTF-8 encoded. Functions should not
 *  try to modify or free the string's memory.
 *
 * These callbacks are used, starting in PhysicsFS 1.1, as an alternative to
 *  functions that would return lists that need to be cleaned up with
 *  PHYSFS_freeList(). The callback means that the library doesn't need to
 *  allocate an entire list and all the strings up front.
 *
 * Be aware that promises data ordering in the list versions are not
 *  necessarily so in the callback versions. Check the documentation on
 *  specific APIs, but strings may not be sorted as you expect.
 *
 *    \param data User-defined data pointer, passed through from the API
 *                that eventually called the callback.
 *    \param str The string data about which the callback is meant to inform.
 *
 * \sa PHYSFS_getCdRomDirsCallback
 * \sa PHYSFS_getSearchPathCallback
 */
typedef void (*PHYSFS_StringCallback)(void *data, const char *str);


/**
 * \typedef PHYSFS_EnumFilesCallback
 * \brief Function signature for callbacks that enumerate files.
 *
 * \warning As of PhysicsFS 2.1, Use PHYSFS_EnumerateCallback with
 *  PHYSFS_enumerate() instead; it gives you more control over the process.
 *
 * These are used to report a list of directory entries to an original caller,
 *  one file/dir/symlink per callback. All strings are UTF-8 encoded.
 *  Functions should not try to modify or free any string's memory.
 *
 * These callbacks are used, starting in PhysicsFS 1.1, as an alternative to
 *  functions that would return lists that need to be cleaned up with
 *  PHYSFS_freeList(). The callback means that the library doesn't need to
 *  allocate an entire list and all the strings up front.
 *
 * Be aware that promised data ordering in the list versions are not
 *  necessarily so in the callback versions. Check the documentation on
 *  specific APIs, but strings may not be sorted as you expect and you might
 *  get duplicate strings.
 *
 *    \param data User-defined data pointer, passed through from the API
 *                that eventually called the callback.
 *    \param origdir A string containing the full path, in platform-independent
 *                   notation, of the directory containing this file. In most
 *                   cases, this is the directory on which you requested
 *                   enumeration, passed in the callback for your convenience.
 *    \param fname The filename that is being enumerated. It may not be in
 *                 alphabetical order compared to other callbacks that have
 *                 fired, and it will not contain the full path. You can
 *                 recreate the fullpath with $origdir/$fname ... The file
 *                 can be a subdirectory, a file, a symlink, etc.
 *
 * \sa PHYSFS_enumerateFilesCallback
 */
typedef void (*PHYSFS_EnumFilesCallback)(void *data, const char *origdir,
                                         const char *fname);


/**
 * \fn void PHYSFS_getCdRomDirsCallback(PHYSFS_StringCallback c, void *d)
 * \brief Enumerate CD-ROM directories, using an application-defined callback.
 *
 * Internally, PHYSFS_getCdRomDirs() just calls this function and then builds
 *  a list before returning to the application, so functionality is identical
 *  except for how the information is represented to the application.
 *
 * Unlike PHYSFS_getCdRomDirs(), this function does not return an array.
 *  Rather, it calls a function specified by the application once per
 *  detected disc:
 *
 * \code
 *
 * static void foundDisc(void *data, const char *cddir)
 * {
 *     printf("cdrom dir [%s] is available.\n", cddir);
 * }
 *
 * // ...
 * PHYSFS_getCdRomDirsCallback(foundDisc, NULL);
 * \endcode
 *
 * This call may block while drives spin up. Be forewarned.
 *
 *    \param c Callback function to notify about detected drives.
 *    \param d Application-defined data passed to callback. Can be NULL.
 *
 * \sa PHYSFS_StringCallback
 * \sa PHYSFS_getCdRomDirs
 */
PHYSFS_DECL void PHYSFS_getCdRomDirsCallback(PHYSFS_StringCallback c, void *d);


/**
 * \fn void PHYSFS_getSearchPathCallback(PHYSFS_StringCallback c, void *d)
 * \brief Enumerate the search path, using an application-defined callback.
 *
 * Internally, PHYSFS_getSearchPath() just calls this function and then builds
 *  a list before returning to the application, so functionality is identical
 *  except for how the information is represented to the application.
 *
 * Unlike PHYSFS_getSearchPath(), this function does not return an array.
 *  Rather, it calls a function specified by the application once per
 *  element of the search path:
 *
 * \code
 *
 * static void printSearchPath(void *data, const char *pathItem)
 * {
 *     printf("[%s] is in the search path.\n", pathItem);
 * }
 *
 * // ...
 * PHYSFS_getSearchPathCallback(printSearchPath, NULL);
 * \endcode
 *
 * Elements of the search path are reported in order search priority, so the
 *  first archive/dir that would be examined when looking for a file is the
 *  first element passed through the callback.
 *
 *    \param c Callback function to notify about search path elements.
 *    \param d Application-defined data passed to callback. Can be NULL.
 *
 * \sa PHYSFS_StringCallback
 * \sa PHYSFS_getSearchPath
 */
PHYSFS_DECL void PHYSFS_getSearchPathCallback(PHYSFS_StringCallback c, void *d);


/**
 * \fn void PHYSFS_enumerateFilesCallback(const char *dir, PHYSFS_EnumFilesCallback c, void *d)
 * \brief Get a file listing of a search path's directory, using an application-defined callback.
 *
 * \deprecated As of PhysicsFS 2.1, use PHYSFS_enumerate() instead. This
 *  function has no way to report errors (or to have the callback signal an
 *  error or request a stop), so if data will be lost, your callback has no
 *  way to direct the process, and your calling app has no way to know.
 *
 * As of PhysicsFS 2.1, this function just wraps PHYSFS_enumerate() and
 *  ignores errors. Consider using PHYSFS_enumerate() or
 *  PHYSFS_enumerateFiles() instead.
 *
 * \sa PHYSFS_enumerate
 * \sa PHYSFS_enumerateFiles
 * \sa PHYSFS_EnumFilesCallback
 */
PHYSFS_DECL void PHYSFS_enumerateFilesCallback(const char *dir,
                                               PHYSFS_EnumFilesCallback c,
                                               void *d) PHYSFS_DEPRECATED;

/**
 * \fn void PHYSFS_utf8FromUcs4(const PHYSFS_uint32 *src, char *dst, PHYSFS_uint64 len)
 * \brief Convert a UCS-4 string to a UTF-8 string.
 *
 * \warning This function will not report an error if there are invalid UCS-4
 *          values in the source string. It will replace them with a '?'
 *          character and continue on.
 *
 * UCS-4 (aka UTF-32) strings are 32-bits per character: \c wchar_t on Unix.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is the same size as the source buffer. UTF-8
 *  never uses more than 32-bits per character, so while it may shrink a UCS-4
 *  string, it will never expand it.
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UTF-8
 *  sequence at the end. If the buffer length is 0, this function does nothing.
 *
 *   \param src Null-terminated source string in UCS-4 format.
 *   \param dst Buffer to store converted UTF-8 string.
 *   \param len Size, in bytes, of destination buffer.
 */
PHYSFS_DECL void PHYSFS_utf8FromUcs4(const PHYSFS_uint32 *src, char *dst,
                                     PHYSFS_uint64 len);

/**
 * \fn void PHYSFS_utf8ToUcs4(const char *src, PHYSFS_uint32 *dst, PHYSFS_uint64 len)
 * \brief Convert a UTF-8 string to a UCS-4 string.
 *
 * \warning This function will not report an error if there are invalid UTF-8
 *          sequences in the source string. It will replace them with a '?'
 *          character and continue on.
 *
 * UCS-4 (aka UTF-32) strings are 32-bits per character: \c wchar_t on Unix.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is four times the size of the source buffer.
 *  UTF-8 uses from one to four bytes per character, but UCS-4 always uses
 *  four, so an entirely low-ASCII string will quadruple in size!
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UCS-4
 *  sequence at the end. If the buffer length is 0, this function does nothing.
 *
 *   \param src Null-terminated source string in UTF-8 format.
 *   \param dst Buffer to store converted UCS-4 string.
 *   \param len Size, in bytes, of destination buffer.
 */
PHYSFS_DECL void PHYSFS_utf8ToUcs4(const char *src, PHYSFS_uint32 *dst,
                                   PHYSFS_uint64 len);

/**
 * \fn void PHYSFS_utf8FromUcs2(const PHYSFS_uint16 *src, char *dst, PHYSFS_uint64 len)
 * \brief Convert a UCS-2 string to a UTF-8 string.
 *
 * \warning you almost certainly should use PHYSFS_utf8FromUtf16(), which
 *  became available in PhysicsFS 2.1, unless you know what you're doing.
 *
 * \warning This function will not report an error if there are invalid UCS-2
 *          values in the source string. It will replace them with a '?'
 *          character and continue on.
 *
 * UCS-2 strings are 16-bits per character: \c TCHAR on Windows, when building
 *  with Unicode support. Please note that modern versions of Windows use
 *  UTF-16, which is an extended form of UCS-2, and not UCS-2 itself. You
 *  almost certainly want PHYSFS_utf8FromUtf16() instead.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is double the size of the source buffer.
 *  UTF-8 never uses more than 32-bits per character, so while it may shrink
 *  a UCS-2 string, it may also expand it.
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UTF-8
 *  sequence at the end. If the buffer length is 0, this function does nothing.
 *
 *   \param src Null-terminated source string in UCS-2 format.
 *   \param dst Buffer to store converted UTF-8 string.
 *   \param len Size, in bytes, of destination buffer.
 *
 * \sa PHYSFS_utf8FromUtf16
 */
PHYSFS_DECL void PHYSFS_utf8FromUcs2(const PHYSFS_uint16 *src, char *dst,
                                     PHYSFS_uint64 len);

/**
 * \fn PHYSFS_utf8ToUcs2(const char *src, PHYSFS_uint16 *dst, PHYSFS_uint64 len)
 * \brief Convert a UTF-8 string to a UCS-2 string.
 *
 * \warning you almost certainly should use PHYSFS_utf8ToUtf16(), which
 *  became available in PhysicsFS 2.1, unless you know what you're doing.
 *
 * \warning This function will not report an error if there are invalid UTF-8
 *          sequences in the source string. It will replace them with a '?'
 *          character and continue on.
 *
 * UCS-2 strings are 16-bits per character: \c TCHAR on Windows, when building
 *  with Unicode support. Please note that modern versions of Windows use
 *  UTF-16, which is an extended form of UCS-2, and not UCS-2 itself. You
 *  almost certainly want PHYSFS_utf8ToUtf16() instead, but you need to
 *  understand how that changes things, too.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is double the size of the source buffer.
 *  UTF-8 uses from one to four bytes per character, but UCS-2 always uses
 *  two, so an entirely low-ASCII string will double in size!
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UCS-2
 *  sequence at the end. If the buffer length is 0, this function does nothing.
 *
 *   \param src Null-terminated source string in UTF-8 format.
 *   \param dst Buffer to store converted UCS-2 string.
 *   \param len Size, in bytes, of destination buffer.
 *
 * \sa PHYSFS_utf8ToUtf16
 */
PHYSFS_DECL void PHYSFS_utf8ToUcs2(const char *src, PHYSFS_uint16 *dst,
                                   PHYSFS_uint64 len);

/**
 * \fn void PHYSFS_utf8FromLatin1(const char *src, char *dst, PHYSFS_uint64 len)
 * \brief Convert a UTF-8 string to a Latin1 string.
 *
 * Latin1 strings are 8-bits per character: a popular "high ASCII" encoding.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is double the size of the source buffer.
 *  UTF-8 expands latin1 codepoints over 127 from 1 to 2 bytes, so the string
 *  may grow in some cases.
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UTF-8
 *  sequence at the end. If the buffer length is 0, this function does nothing.
 *
 * Please note that we do not supply a UTF-8 to Latin1 converter, since Latin1
 *  can't express most Unicode codepoints. It's a legacy encoding; you should
 *  be converting away from it at all times.
 *
 *   \param src Null-terminated source string in Latin1 format.
 *   \param dst Buffer to store converted UTF-8 string.
 *   \param len Size, in bytes, of destination buffer.
 */
PHYSFS_DECL void PHYSFS_utf8FromLatin1(const char *src, char *dst,
                                       PHYSFS_uint64 len);

/* Everything above this line is part of the PhysicsFS 2.0 API. */

/**
 * \fn int PHYSFS_caseFold(const PHYSFS_uint32 from, PHYSFS_uint32 *to)
 * \brief "Fold" a Unicode codepoint to a lowercase equivalent.
 *
 * (This is for limited, hardcore use. If you don't immediately see a need
 *  for it, you can probably ignore this forever.)
 *
 * This will convert a Unicode codepoint into its lowercase equivalent.
 *  Bogus codepoints and codepoints without a lowercase equivalent will
 *  be returned unconverted.
 *
 * Note that you might get multiple codepoints in return! The German Eszett,
 *  for example, will fold down to two lowercase latin 's' codepoints. The
 *  theory is that if you fold two strings, one with an Eszett and one with
 *  "SS" down, they will match.
 *
 * \warning Anyone that is a student of Unicode knows about the "Turkish I"
 *          problem. This API does not handle it. Assume this one letter
 *          in all of Unicode will definitely fold sort of incorrectly. If
 *          you don't know what this is about, you can probably ignore this
 *          problem for most of the planet, but perfection is impossible.
 *
 *   \param from The codepoint to fold.
 *   \param to Buffer to store the folded codepoint values into. This should
 *             point to space for at least 3 PHYSFS_uint32 slots.
 *  \return The number of codepoints the folding produced. Between 1 and 3.
 */
PHYSFS_DECL int PHYSFS_caseFold(const PHYSFS_uint32 from, PHYSFS_uint32 *to);


/**
 * \fn int PHYSFS_utf8stricmp(const char *str1, const char *str2)
 * \brief Case-insensitive compare of two UTF-8 strings.
 *
 * This is a strcasecmp/stricmp replacement that expects both strings
 *  to be in UTF-8 encoding. It will do "case folding" to decide if the
 *  Unicode codepoints in the strings match.
 *
 * If both strings are exclusively low-ASCII characters, this will do the
 *  right thing, as that is also valid UTF-8. If there are any high-ASCII
 *  chars, this will not do what you expect!
 *
 * It will report which string is "greater than" the other, but be aware that
 *  this doesn't necessarily mean anything: 'a' may be "less than" 'b', but
 *  a Japanese kuten has no meaningful alphabetically relationship to
 *  a Greek lambda, but being able to assign a reliable "value" makes sorting
 *  algorithms possible, if not entirely sane. Most cases should treat the
 *  return value as "equal" or "not equal".
 *
 * Like stricmp, this expects both strings to be NULL-terminated.
 *
 *   \param str1 First string to compare.
 *   \param str2 Second string to compare.
 *  \return -1 if str1 is "less than" str2, 1 if "greater than", 0 if equal.
 */
PHYSFS_DECL int PHYSFS_utf8stricmp(const char *str1, const char *str2);

/**
 * \fn int PHYSFS_utf16stricmp(const PHYSFS_uint16 *str1, const PHYSFS_uint16 *str2)
 * \brief Case-insensitive compare of two UTF-16 strings.
 *
 * This is a strcasecmp/stricmp replacement that expects both strings
 *  to be in UTF-16 encoding. It will do "case folding" to decide if the
 *  Unicode codepoints in the strings match.
 *
 * It will report which string is "greater than" the other, but be aware that
 *  this doesn't necessarily mean anything: 'a' may be "less than" 'b', but
 *  a Japanese kuten has no meaningful alphabetically relationship to
 *  a Greek lambda, but being able to assign a reliable "value" makes sorting
 *  algorithms possible, if not entirely sane. Most cases should treat the
 *  return value as "equal" or "not equal".
 *
 * Like stricmp, this expects both strings to be NULL-terminated.
 *
 *   \param str1 First string to compare.
 *   \param str2 Second string to compare.
 *  \return -1 if str1 is "less than" str2, 1 if "greater than", 0 if equal.
 */
PHYSFS_DECL int PHYSFS_utf16stricmp(const PHYSFS_uint16 *str1,
                                    const PHYSFS_uint16 *str2);

/**
 * \fn int PHYSFS_ucs4stricmp(const PHYSFS_uint32 *str1, const PHYSFS_uint32 *str2)
 * \brief Case-insensitive compare of two UCS-4 strings.
 *
 * This is a strcasecmp/stricmp replacement that expects both strings
 *  to be in UCS-4 (aka UTF-32) encoding. It will do "case folding" to decide
 *  if the Unicode codepoints in the strings match.
 *
 * It will report which string is "greater than" the other, but be aware that
 *  this doesn't necessarily mean anything: 'a' may be "less than" 'b', but
 *  a Japanese kuten has no meaningful alphabetically relationship to
 *  a Greek lambda, but being able to assign a reliable "value" makes sorting
 *  algorithms possible, if not entirely sane. Most cases should treat the
 *  return value as "equal" or "not equal".
 *
 * Like stricmp, this expects both strings to be NULL-terminated.
 *
 *   \param str1 First string to compare.
 *   \param str2 Second string to compare.
 *  \return -1 if str1 is "less than" str2, 1 if "greater than", 0 if equal.
 */
PHYSFS_DECL int PHYSFS_ucs4stricmp(const PHYSFS_uint32 *str1,
                                   const PHYSFS_uint32 *str2);


/**
 * \typedef PHYSFS_EnumerateCallback
 * \brief Possible return values from PHYSFS_EnumerateCallback.
 *
 * These values dictate if an enumeration callback should continue to fire,
 *  or stop (and why it is stopping).
 *
 * \sa PHYSFS_EnumerateCallback
 * \sa PHYSFS_enumerate
 */
typedef enum PHYSFS_EnumerateCallbackResult
{
    PHYSFS_ENUM_ERROR = -1,   /**< Stop enumerating, report error to app. */
    PHYSFS_ENUM_STOP = 0,     /**< Stop enumerating, report success to app. */
    PHYSFS_ENUM_OK = 1        /**< Keep enumerating, no problems */
} PHYSFS_EnumerateCallbackResult;

/**
 * \typedef PHYSFS_EnumerateCallback
 * \brief Function signature for callbacks that enumerate and return results.
 *
 * This is the same thing as PHYSFS_EnumFilesCallback from PhysicsFS 2.0,
 *  except it can return a result from the callback: namely: if you're looking
 *  for something specific, once you find it, you can tell PhysicsFS to stop
 *  enumerating further. This is used with PHYSFS_enumerate(), which we
 *  hopefully got right this time.  :)
 *
 *    \param data User-defined data pointer, passed through from the API
 *                that eventually called the callback.
 *    \param origdir A string containing the full path, in platform-independent
 *                   notation, of the directory containing this file. In most
 *                   cases, this is the directory on which you requested
 *                   enumeration, passed in the callback for your convenience.
 *    \param fname The filename that is being enumerated. It may not be in
 *                 alphabetical order compared to other callbacks that have
 *                 fired, and it will not contain the full path. You can
 *                 recreate the fullpath with $origdir/$fname ... The file
 *                 can be a subdirectory, a file, a symlink, etc.
 *   \return A value from PHYSFS_EnumerateCallbackResult.
 *           All other values are (currently) undefined; don't use them.
 *
 * \sa PHYSFS_enumerate
 * \sa PHYSFS_EnumerateCallbackResult
 */
typedef PHYSFS_EnumerateCallbackResult (*PHYSFS_EnumerateCallback)(void *data,
                                       const char *origdir, const char *fname);

/**
 * \fn int PHYSFS_enumerate(const char *dir, PHYSFS_EnumerateCallback c, void *d)
 * \brief Get a file listing of a search path's directory, using an application-defined callback, with errors reported.
 *
 * Internally, PHYSFS_enumerateFiles() just calls this function and then builds
 *  a list before returning to the application, so functionality is identical
 *  except for how the information is represented to the application.
 *
 * Unlike PHYSFS_enumerateFiles(), this function does not return an array.
 *  Rather, it calls a function specified by the application once per
 *  element of the search path:
 *
 * \code
 *
 * static int printDir(void *data, const char *origdir, const char *fname)
 * {
 *     printf(" * We've got [%s] in [%s].\n", fname, origdir);
 *     return 1;  // give me more data, please.
 * }
 *
 * // ...
 * PHYSFS_enumerate("/some/path", printDir, NULL);
 * \endcode
 *
 * Items sent to the callback are not guaranteed to be in any order whatsoever.
 *  There is no sorting done at this level, and if you need that, you should
 *  probably use PHYSFS_enumerateFiles() instead, which guarantees
 *  alphabetical sorting. This form reports whatever is discovered in each
 *  archive before moving on to the next. Even within one archive, we can't
 *  guarantee what order it will discover data. <em>Any sorting you find in
 *  these callbacks is just pure luck. Do not rely on it.</em> As this walks
 *  the entire list of archives, you may receive duplicate filenames.
 *
 * This API and the callbacks themselves are capable of reporting errors.
 *  Prior to this API, callbacks had to accept every enumerated item, even if
 *  they were only looking for a specific thing and wanted to stop after that,
 *  or had a serious error and couldn't alert anyone. Furthermore, if
 *  PhysicsFS itself had a problem (disk error or whatnot), it couldn't report
 *  it to the calling app, it would just have to skip items or stop
 *  enumerating outright, and the caller wouldn't know it had lost some data
 *  along the way.
 *
 * Now the caller can be sure it got a complete data set, and its callback has
 *  control if it wants enumeration to stop early. See the documentation for
 *  PHYSFS_EnumerateCallback for details on how your callback should behave.
 *
 *    \param dir Directory, in platform-independent notation, to enumerate.
 *    \param c Callback function to notify about search path elements.
 *    \param d Application-defined data passed to callback. Can be NULL.
 *   \return non-zero on success, zero on failure. Use
 *           PHYSFS_getLastErrorCode() to obtain the specific error. If the
 *           callback returns PHYSFS_ENUM_STOP to stop early, this will be
 *           considered success. Callbacks returning PHYSFS_ENUM_ERROR will
 *           make this function return zero and set the error code to
 *           PHYSFS_ERR_APP_CALLBACK.
 *
 * \sa PHYSFS_EnumerateCallback
 * \sa PHYSFS_enumerateFiles
 */
PHYSFS_DECL int PHYSFS_enumerate(const char *dir, PHYSFS_EnumerateCallback c,
                                 void *d);


/**
 * \fn int PHYSFS_unmount(const char *oldDir)
 * \brief Remove a directory or archive from the search path.
 *
 * This is functionally equivalent to PHYSFS_removeFromSearchPath(), but that
 *  function is deprecated to keep the vocabulary paired with PHYSFS_mount().
 *
 * This must be a (case-sensitive) match to a dir or archive already in the
 *  search path, specified in platform-dependent notation.
 *
 * This call will fail (and fail to remove from the path) if the element still
 *  has files open in it.
 *
 * \warning This function wants the path to the archive or directory that was
 *          mounted (the same string used for the "newDir" argument of
 *          PHYSFS_addToSearchPath or any of the mount functions), not the
 *          path where it is mounted in the tree (the "mountPoint" argument
 *          to any of the mount functions).
 *
 *    \param oldDir dir/archive to remove.
 *   \return nonzero on success, zero on failure. Use
 *           PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_mount
 */
PHYSFS_DECL int PHYSFS_unmount(const char *oldDir);


/**
 * \fn const PHYSFS_Allocator *PHYSFS_getAllocator(void)
 * \brief Discover the current allocator.
 *
 * (This is for limited, hardcore use. If you don't immediately see a need
 *  for it, you can probably ignore this forever.)
 *
 * This function exposes the function pointers that make up the currently used
 *  allocator. This can be useful for apps that want to access PhysicsFS's
 *  internal, default allocation routines, as well as for external code that
 *  wants to share the same allocator, even if the application specified their
 *  own.
 *
 * This call is only valid between PHYSFS_init() and PHYSFS_deinit() calls;
 *  it will return NULL if the library isn't initialized. As we can't
 *  guarantee the state of the internal allocators unless the library is
 *  initialized, you shouldn't use any allocator returned here after a call
 *  to PHYSFS_deinit().
 *
 * Do not call the returned allocator's Init() or Deinit() methods under any
 *  circumstances.
 *
 * If you aren't immediately sure what to do with this function, you can
 *  safely ignore it altogether.
 *
 *  \return Current allocator, as set by PHYSFS_setAllocator(), or PhysicsFS's
 *          internal, default allocator if no application defined allocator
 *          is currently set. Will return NULL if the library is not
 *          initialized.
 *
 * \sa PHYSFS_Allocator
 * \sa PHYSFS_setAllocator
 */
PHYSFS_DECL const PHYSFS_Allocator *PHYSFS_getAllocator(void);


/**
 * \enum PHYSFS_FileType
 * \brief Type of a File
 *
 * Possible types of a file.
 *
 * \sa PHYSFS_stat
 */
typedef enum PHYSFS_FileType
{
	PHYSFS_FILETYPE_REGULAR, /**< a normal file */
	PHYSFS_FILETYPE_DIRECTORY, /**< a directory */
	PHYSFS_FILETYPE_SYMLINK, /**< a symlink */
	PHYSFS_FILETYPE_OTHER /**< something completely different like a device */
} PHYSFS_FileType;

/**
 * \struct PHYSFS_Stat
 * \brief Meta data for a file or directory
 *
 * Container for various meta data about a file in the virtual file system.
 *  PHYSFS_stat() uses this structure for returning the information. The time
 *  data will be either the number of seconds since the Unix epoch (midnight,
 *  Jan 1, 1970), or -1 if the information isn't available or applicable.
 *  The (filesize) field is measured in bytes.
 *  The (readonly) field tells you whether the archive thinks a file is
 *  not writable, but tends to be only an estimate (for example, your write
 *  dir might overlap with a .zip file, meaning you _can_ successfully open
 *  that path for writing, as it gets created elsewhere.
 *
 * \sa PHYSFS_stat
 * \sa PHYSFS_FileType
 */
typedef struct PHYSFS_Stat
{
	PHYSFS_sint64 filesize; /**< size in bytes, -1 for non-files and unknown */
	PHYSFS_sint64 modtime;  /**< last modification time */
	PHYSFS_sint64 createtime; /**< like modtime, but for file creation time */
	PHYSFS_sint64 accesstime; /**< like modtime, but for file access time */
	PHYSFS_FileType filetype; /**< File? Directory? Symlink? */
	int readonly; /**< non-zero if read only, zero if writable. */
} PHYSFS_Stat;

/**
 * \fn int PHYSFS_stat(const char *fname, PHYSFS_Stat *stat)
 * \brief Get various information about a directory or a file.
 *
 * Obtain various information about a file or directory from the meta data.
 *
 * This function will never follow symbolic links. If you haven't enabled
 *  symlinks with PHYSFS_permitSymbolicLinks(), stat'ing a symlink will be
 *  treated like stat'ing a non-existant file. If symlinks are enabled,
 *  stat'ing a symlink will give you information on the link itself and not
 *  what it points to.
 *
 *    \param fname filename to check, in platform-indepedent notation.
 *    \param stat pointer to structure to fill in with data about (fname).
 *   \return non-zero on success, zero on failure. On failure, (stat)'s
 *           contents are undefined.
 *
 * \sa PHYSFS_Stat
 */
PHYSFS_DECL int PHYSFS_stat(const char *fname, PHYSFS_Stat *stat);


/**
 * \fn void PHYSFS_utf8FromUtf16(const PHYSFS_uint16 *src, char *dst, PHYSFS_uint64 len)
 * \brief Convert a UTF-16 string to a UTF-8 string.
 *
 * \warning This function will not report an error if there are invalid UTF-16
 *          sequences in the source string. It will replace them with a '?'
 *          character and continue on.
 *
 * UTF-16 strings are 16-bits per character (except some chars, which are
 *  32-bits): \c TCHAR on Windows, when building with Unicode support. Modern
 *  Windows releases use UTF-16. Windows releases before 2000 used TCHAR, but
 *  only handled UCS-2. UTF-16 _is_ UCS-2, except for the characters that
 *  are 4 bytes, which aren't representable in UCS-2 at all anyhow. If you
 *  aren't sure, you should be using UTF-16 at this point on Windows.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is double the size of the source buffer.
 *  UTF-8 never uses more than 32-bits per character, so while it may shrink
 *  a UTF-16 string, it may also expand it.
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UTF-8
 *  sequence at the end. If the buffer length is 0, this function does nothing.
 *
 *   \param src Null-terminated source string in UTF-16 format.
 *   \param dst Buffer to store converted UTF-8 string.
 *   \param len Size, in bytes, of destination buffer.
 */
PHYSFS_DECL void PHYSFS_utf8FromUtf16(const PHYSFS_uint16 *src, char *dst,
                                      PHYSFS_uint64 len);

/**
 * \fn PHYSFS_utf8ToUtf16(const char *src, PHYSFS_uint16 *dst, PHYSFS_uint64 len)
 * \brief Convert a UTF-8 string to a UTF-16 string.
 *
 * \warning This function will not report an error if there are invalid UTF-8
 *          sequences in the source string. It will replace them with a '?'
 *          character and continue on.
 *
 * UTF-16 strings are 16-bits per character (except some chars, which are
 *  32-bits): \c TCHAR on Windows, when building with Unicode support. Modern
 *  Windows releases use UTF-16. Windows releases before 2000 used TCHAR, but
 *  only handled UCS-2. UTF-16 _is_ UCS-2, except for the characters that
 *  are 4 bytes, which aren't representable in UCS-2 at all anyhow. If you
 *  aren't sure, you should be using UTF-16 at this point on Windows.
 *
 * To ensure that the destination buffer is large enough for the conversion,
 *  please allocate a buffer that is double the size of the source buffer.
 *  UTF-8 uses from one to four bytes per character, but UTF-16 always uses
 *  two to four, so an entirely low-ASCII string will double in size! The
 *  UTF-16 characters that would take four bytes also take four bytes in UTF-8,
 *  so you don't need to allocate 4x the space just in case: double will do.
 *
 * Strings that don't fit in the destination buffer will be truncated, but
 *  will always be null-terminated and never have an incomplete UTF-16
 *  surrogate pair at the end. If the buffer length is 0, this function does
 *  nothing.
 *
 *   \param src Null-terminated source string in UTF-8 format.
 *   \param dst Buffer to store converted UTF-16 string.
 *   \param len Size, in bytes, of destination buffer.
 *
 * \sa PHYSFS_utf8ToUtf16
 */
PHYSFS_DECL void PHYSFS_utf8ToUtf16(const char *src, PHYSFS_uint16 *dst,
                                    PHYSFS_uint64 len);


/**
 * \fn PHYSFS_sint64 PHYSFS_readBytes(PHYSFS_File *handle, void *buffer, PHYSFS_uint64 len)
 * \brief Read bytes from a PhysicsFS filehandle
 *
 * The file must be opened for reading.
 *
 *   \param handle handle returned from PHYSFS_openRead().
 *   \param buffer buffer of at least (len) bytes to store read data into.
 *   \param len number of bytes being read from (handle).
 *  \return number of bytes read. This may be less than (len); this does not
 *          signify an error, necessarily (a short read may mean EOF).
 *          PHYSFS_getLastErrorCode() can shed light on the reason this might
 *          be < (len), as can PHYSFS_eof(). -1 if complete failure.
 *
 * \sa PHYSFS_eof
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_readBytes(PHYSFS_File *handle, void *buffer,
                                           PHYSFS_uint64 len);

/**
 * \fn PHYSFS_sint64 PHYSFS_writeBytes(PHYSFS_File *handle, const void *buffer, PHYSFS_uint64 len)
 * \brief Write data to a PhysicsFS filehandle
 *
 * The file must be opened for writing.
 *
 * Please note that while (len) is an unsigned 64-bit integer, you are limited
 *  to 63 bits (9223372036854775807 bytes), so we can return a negative value
 *  on error. If length is greater than 0x7FFFFFFFFFFFFFFF, this function will
 *  immediately fail. For systems without a 64-bit datatype, you are limited
 *  to 31 bits (0x7FFFFFFF, or 2147483647 bytes). We trust most things won't
 *  need to do multiple gigabytes of i/o in one call anyhow, but why limit
 *  things?
 *
 *   \param handle retval from PHYSFS_openWrite() or PHYSFS_openAppend().
 *   \param buffer buffer of (len) bytes to write to (handle).
 *   \param len number of bytes being written to (handle).
 *  \return number of bytes written. This may be less than (len); in the case
 *          of an error, the system may try to write as many bytes as possible,
 *          so an incomplete write might occur. PHYSFS_getLastErrorCode() can
 *          shed light on the reason this might be < (len). -1 if complete
 *          failure.
 */
PHYSFS_DECL PHYSFS_sint64 PHYSFS_writeBytes(PHYSFS_File *handle,
                                            const void *buffer,
                                            PHYSFS_uint64 len);


/**
 * \struct PHYSFS_Io
 * \brief An abstract i/o interface.
 *
 * \warning This is advanced, hardcore stuff. You don't need this unless you
 *          really know what you're doing. Most apps will not need this.
 *
 * Historically, PhysicsFS provided access to the physical filesystem and
 *  archives within that filesystem. However, sometimes you need more power
 *  than this. Perhaps you need to provide an archive that is entirely
 *  contained in RAM, or you need to bridge some other file i/o API to
 *  PhysicsFS, or you need to translate the bits (perhaps you have a
 *  a standard .zip file that's encrypted, and you need to decrypt on the fly
 *  for the unsuspecting zip archiver).
 *
 * A PHYSFS_Io is the interface that Archivers use to get archive data.
 *  Historically, this has mapped to file i/o to the physical filesystem, but
 *  as of PhysicsFS 2.1, applications can provide their own i/o implementations
 *  at runtime.
 *
 * This interface isn't necessarily a good universal fit for i/o. There are a
 *  few requirements of note:
 *
 *  - They only do blocking i/o (at least, for now).
 *  - They need to be able to duplicate. If you have a file handle from
 *    fopen(), you need to be able to create a unique clone of it (so we
 *    have two handles to the same file that can both seek/read/etc without
 *    stepping on each other).
 *  - They need to know the size of their entire data set.
 *  - They need to be able to seek and rewind on demand.
 *
 * ...in short, you're probably not going to write an HTTP implementation.
 *
 * Thread safety: PHYSFS_Io implementations are not guaranteed to be thread
 *  safe in themselves. Under the hood where PhysicsFS uses them, the library
 *  provides its own locks. If you plan to use them directly from separate
 *  threads, you should either use mutexes to protect them, or don't use the
 *  same PHYSFS_Io from two threads at the same time.
 *
 * \sa PHYSFS_mountIo
 */
typedef struct PHYSFS_Io
{
    /**
     * \brief Binary compatibility information.
     *
     * This must be set to zero at this time. Future versions of this
     *  struct will increment this field, so we know what a given
     *  implementation supports. We'll presumably keep supporting older
     *  versions as we offer new features, though.
     */
    PHYSFS_uint32 version;

    /**
     * \brief Instance data for this struct.
     *
     * Each instance has a pointer associated with it that can be used to
     *  store anything it likes. This pointer is per-instance of the stream,
     *  so presumably it will change when calling duplicate(). This can be
     *  deallocated during the destroy() method.
     */
    void *opaque;

    /**
     * \brief Read more data.
     *
     * Read (len) bytes from the interface, at the current i/o position, and
     *  store them in (buffer). The current i/o position should move ahead
     *  by the number of bytes successfully read.
     *
     * You don't have to implement this; set it to NULL if not implemented.
     *  This will only be used if the file is opened for reading. If set to
     *  NULL, a default implementation that immediately reports failure will
     *  be used.
     *
     *   \param io The i/o instance to read from.
     *   \param buf The buffer to store data into. It must be at least
     *                 (len) bytes long and can't be NULL.
     *   \param len The number of bytes to read from the interface.
     *  \return number of bytes read from file, 0 on EOF, -1 if complete
     *          failure.
     */
    PHYSFS_sint64 (*read)(struct PHYSFS_Io *io, void *buf, PHYSFS_uint64 len);

    /**
     * \brief Write more data.
     *
     * Write (len) bytes from (buffer) to the interface at the current i/o
     *  position. The current i/o position should move ahead by the number of
     *  bytes successfully written.
     *
     * You don't have to implement this; set it to NULL if not implemented.
     *  This will only be used if the file is opened for writing. If set to
     *  NULL, a default implementation that immediately reports failure will
     *  be used.
     *
     * You are allowed to buffer; a write can succeed here and then later
     *  fail when flushing. Note that PHYSFS_setBuffer() may be operating a
     *  level above your i/o, so you should usually not implement your
     *  own buffering routines.
     *
     *   \param io The i/o instance to write to.
     *   \param buffer The buffer to read data from. It must be at least
     *                 (len) bytes long and can't be NULL.
     *   \param len The number of bytes to read from (buffer).
     *  \return number of bytes written to file, -1 if complete failure.
     */
    PHYSFS_sint64 (*write)(struct PHYSFS_Io *io, const void *buffer,
                           PHYSFS_uint64 len);

    /**
     * \brief Move i/o position to a given byte offset from start.
     *
     * This method moves the i/o position, so the next read/write will
     *  be of the byte at (offset) offset. Seeks past the end of file should
     *  be treated as an error condition.
     *
     *   \param io The i/o instance to seek.
     *   \param offset The new byte offset for the i/o position.
     *  \return non-zero on success, zero on error.
     */
    int (*seek)(struct PHYSFS_Io *io, PHYSFS_uint64 offset);

    /**
     * \brief Report current i/o position.
     *
     * Return bytes offset, or -1 if you aren't able to determine. A failure
     *  will almost certainly be fatal to further use of this stream, so you
     *  may not leave this unimplemented.
     *
     *   \param io The i/o instance to query.
     *  \return The current byte offset for the i/o position, -1 if unknown.
     */
    PHYSFS_sint64 (*tell)(struct PHYSFS_Io *io);

    /**
     * \brief Determine size of the i/o instance's dataset.
     *
     * Return number of bytes available in the file, or -1 if you
     *  aren't able to determine. A failure will almost certainly be fatal
     *  to further use of this stream, so you may not leave this unimplemented.
     *
     *   \param io The i/o instance to query.
     *  \return Total size, in bytes, of the dataset.
     */
    PHYSFS_sint64 (*length)(struct PHYSFS_Io *io);

    /**
     * \brief Duplicate this i/o instance.
     *
     * This needs to result in a full copy of this PHYSFS_Io, that can live
     *  completely independently. The copy needs to be able to perform all
     *  its operations without altering the original, including either object
     *  being destroyed separately (so, for example: they can't share a file
     *  handle; they each need their own).
     *
     * If you can't duplicate a handle, it's legal to return NULL, but you
     *  almost certainly need this functionality if you want to use this to
     *  PHYSFS_Io to back an archive.
     *
     *   \param io The i/o instance to duplicate.
     *  \return A new value for a stream's (opaque) field, or NULL on error.
     */
    struct PHYSFS_Io *(*duplicate)(struct PHYSFS_Io *io);

    /**
     * \brief Flush resources to media, or wherever.
     *
     * This is the chance to report failure for writes that had claimed
     *  success earlier, but still had a chance to actually fail. This method
     *  can be NULL if flushing isn't necessary.
     *
     * This function may be called before destroy(), as it can report failure
     *  and destroy() can not. It may be called at other times, too.
     *
     *   \param io The i/o instance to flush.
     *  \return Zero on error, non-zero on success.
     */
    int (*flush)(struct PHYSFS_Io *io);

    /**
     * \brief Cleanup and deallocate i/o instance.
     *
     * Free associated resources, including (opaque) if applicable.
     *
     * This function must always succeed: as such, it returns void. The
     *  system may call your flush() method before this. You may report
     *  failure there if necessary. This method may still be called if
     *  flush() fails, in which case you'll have to abandon unflushed data
     *  and other failing conditions and clean up.
     *
     * Once this method is called for a given instance, the system will assume
     *  it is unsafe to touch that instance again and will discard any
     *  references to it.
     *
     *   \param s The i/o instance to destroy.
     */
    void (*destroy)(struct PHYSFS_Io *io);
} PHYSFS_Io;


/**
 * \fn int PHYSFS_mountIo(PHYSFS_Io *io, const char *newDir, const char *mountPoint, int appendToPath)
 * \brief Add an archive, built on a PHYSFS_Io, to the search path.
 *
 * \warning Unless you have some special, low-level need, you should be using
 *          PHYSFS_mount() instead of this.
 *
 * This function operates just like PHYSFS_mount(), but takes a PHYSFS_Io
 *  instead of a pathname. Behind the scenes, PHYSFS_mount() calls this
 *  function with a physical-filesystem-based PHYSFS_Io.
 *
 * (newDir) must be a unique string to identify this archive. It is used
 *  to optimize archiver selection (if you name it XXXXX.zip, we might try
 *  the ZIP archiver first, for example, or directly choose an archiver that
 *  can only trust the data is valid by filename extension). It doesn't
 *  need to refer to a real file at all. If the filename extension isn't
 *  helpful, the system will try every archiver until one works or none
 *  of them do. This filename must be unique, as the system won't allow you
 *  to have two archives with the same name.
 *
 * (io) must remain until the archive is unmounted. When the archive is
 *  unmounted, the system will call (io)->destroy(io), which will give you
 *  a chance to free your resources.
 *
 * If this function fails, (io)->destroy(io) is not called.
 *
 *   \param io i/o instance for archive to add to the path.
 *   \param newDir Filename that can represent this stream.
 *   \param mountPoint Location in the interpolated tree that this archive
 *                     will be "mounted", in platform-independent notation.
 *                     NULL or "" is equivalent to "/".
 *   \param appendToPath nonzero to append to search path, zero to prepend.
 *  \return nonzero if added to path, zero on failure (bogus archive, stream
 *                   i/o issue, etc). Use PHYSFS_getLastErrorCode() to obtain
 *                   the specific error.
 *
 * \sa PHYSFS_unmount
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_getMountPoint
 */
PHYSFS_DECL int PHYSFS_mountIo(PHYSFS_Io *io, const char *newDir,
                               const char *mountPoint, int appendToPath);


/**
 * \fn int PHYSFS_mountMemory(const void *buf, PHYSFS_uint64 len, void (*del)(void *), const char *newDir, const char *mountPoint, int appendToPath)
 * \brief Add an archive, contained in a memory buffer, to the search path.
 *
 * \warning Unless you have some special, low-level need, you should be using
 *          PHYSFS_mount() instead of this.
 *
 * This function operates just like PHYSFS_mount(), but takes a memory buffer
 *  instead of a pathname. This buffer contains all the data of the archive,
 *  and is used instead of a real file in the physical filesystem.
 *
 * (newDir) must be a unique string to identify this archive. It is used
 *  to optimize archiver selection (if you name it XXXXX.zip, we might try
 *  the ZIP archiver first, for example, or directly choose an archiver that
 *  can only trust the data is valid by filename extension). It doesn't
 *  need to refer to a real file at all. If the filename extension isn't
 *  helpful, the system will try every archiver until one works or none
 *  of them do. This filename must be unique, as the system won't allow you
 *  to have two archives with the same name.
 *
 * (ptr) must remain until the archive is unmounted. When the archive is
 *  unmounted, the system will call (del)(ptr), which will notify you that
 *  the system is done with the buffer, and give you a chance to free your
 *  resources. (del) can be NULL, in which case the system will make no
 *  attempt to free the buffer.
 *
 * If this function fails, (del) is not called.
 *
 *   \param buf Address of the memory buffer containing the archive data.
 *   \param len Size of memory buffer, in bytes.
 *   \param del A callback that triggers upon unmount. Can be NULL.
 *   \param newDir Filename that can represent this stream.
 *   \param mountPoint Location in the interpolated tree that this archive
 *                     will be "mounted", in platform-independent notation.
 *                     NULL or "" is equivalent to "/".
 *   \param appendToPath nonzero to append to search path, zero to prepend.
 *  \return nonzero if added to path, zero on failure (bogus archive, etc).
 *          Use PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_unmount
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_getMountPoint
 */
PHYSFS_DECL int PHYSFS_mountMemory(const void *buf, PHYSFS_uint64 len,
                                   void (*del)(void *), const char *newDir,
                                   const char *mountPoint, int appendToPath);


/**
 * \fn int PHYSFS_mountHandle(PHYSFS_File *file, const char *newDir, const char *mountPoint, int appendToPath)
 * \brief Add an archive, contained in a PHYSFS_File handle, to the search path.
 *
 * \warning Unless you have some special, low-level need, you should be using
 *          PHYSFS_mount() instead of this.
 *
 * \warning Archives-in-archives may be very slow! While a PHYSFS_File can
 *          seek even when the data is compressed, it may do so by rewinding
 *          to the start and decompressing everything before the seek point.
 *          Normal archive usage may do a lot of seeking behind the scenes.
 *          As such, you might find normal archive usage extremely painful
 *          if mounted this way. Plan accordingly: if you, say, have a
 *          self-extracting .zip file, and want to mount something in it,
 *          compress the contents of the inner archive and make sure the outer
 *          .zip file doesn't compress the inner archive too.
 *
 * This function operates just like PHYSFS_mount(), but takes a PHYSFS_File
 *  handle instead of a pathname. This handle contains all the data of the
 *  archive, and is used instead of a real file in the physical filesystem.
 *  The PHYSFS_File may be backed by a real file in the physical filesystem,
 *  but isn't necessarily. The most popular use for this is likely to mount
 *  archives stored inside other archives.
 *
 * (newDir) must be a unique string to identify this archive. It is used
 *  to optimize archiver selection (if you name it XXXXX.zip, we might try
 *  the ZIP archiver first, for example, or directly choose an archiver that
 *  can only trust the data is valid by filename extension). It doesn't
 *  need to refer to a real file at all. If the filename extension isn't
 *  helpful, the system will try every archiver until one works or none
 *  of them do. This filename must be unique, as the system won't allow you
 *  to have two archives with the same name.
 *
 * (file) must remain until the archive is unmounted. When the archive is
 *  unmounted, the system will call PHYSFS_close(file). If you need this
 *  handle to survive, you will have to wrap this in a PHYSFS_Io and use
 *  PHYSFS_mountIo() instead.
 *
 * If this function fails, PHYSFS_close(file) is not called.
 *
 *   \param file The PHYSFS_File handle containing archive data.
 *   \param newDir Filename that can represent this stream.
 *   \param mountPoint Location in the interpolated tree that this archive
 *                     will be "mounted", in platform-independent notation.
 *                     NULL or "" is equivalent to "/".
 *   \param appendToPath nonzero to append to search path, zero to prepend.
 *  \return nonzero if added to path, zero on failure (bogus archive, etc).
 *          Use PHYSFS_getLastErrorCode() to obtain the specific error.
 *
 * \sa PHYSFS_unmount
 * \sa PHYSFS_getSearchPath
 * \sa PHYSFS_getMountPoint
 */
PHYSFS_DECL int PHYSFS_mountHandle(PHYSFS_File *file, const char *newDir,
                                   const char *mountPoint, int appendToPath);


/**
 * \enum PHYSFS_ErrorCode
 * \brief Values that represent specific causes of failure.
 *
 * Most of the time, you should only concern yourself with whether a given
 *  operation failed or not, but there may be occasions where you plan to
 *  handle a specific failure case gracefully, so we provide specific error
 *  codes.
 *
 * Most of these errors are a little vague, and most aren't things you can
 *  fix...if there's a permission error, for example, all you can really do
 *  is pass that information on to the user and let them figure out how to
 *  handle it. In most these cases, your program should only care that it
 *  failed to accomplish its goals, and not care specifically why.
 *
 * \sa PHYSFS_getLastErrorCode
 * \sa PHYSFS_getErrorByCode
 */
typedef enum PHYSFS_ErrorCode
{
    PHYSFS_ERR_OK,               /**< Success; no error.                    */
    PHYSFS_ERR_OTHER_ERROR,      /**< Error not otherwise covered here.     */
    PHYSFS_ERR_OUT_OF_MEMORY,    /**< Memory allocation failed.             */
    PHYSFS_ERR_NOT_INITIALIZED,  /**< PhysicsFS is not initialized.         */
    PHYSFS_ERR_IS_INITIALIZED,   /**< PhysicsFS is already initialized.     */
    PHYSFS_ERR_ARGV0_IS_NULL,    /**< Needed argv[0], but it is NULL.       */
    PHYSFS_ERR_UNSUPPORTED,      /**< Operation or feature unsupported.     */
    PHYSFS_ERR_PAST_EOF,         /**< Attempted to access past end of file. */
    PHYSFS_ERR_FILES_STILL_OPEN, /**< Files still open.                     */
    PHYSFS_ERR_INVALID_ARGUMENT, /**< Bad parameter passed to an function.  */
    PHYSFS_ERR_NOT_MOUNTED,      /**< Requested archive/dir not mounted.    */
    PHYSFS_ERR_NOT_FOUND,        /**< File (or whatever) not found.         */
    PHYSFS_ERR_SYMLINK_FORBIDDEN,/**< Symlink seen when not permitted.      */
    PHYSFS_ERR_NO_WRITE_DIR,     /**< No write dir has been specified.      */
    PHYSFS_ERR_OPEN_FOR_READING, /**< Wrote to a file opened for reading.   */
    PHYSFS_ERR_OPEN_FOR_WRITING, /**< Read from a file opened for writing.  */
    PHYSFS_ERR_NOT_A_FILE,       /**< Needed a file, got a directory (etc). */
    PHYSFS_ERR_READ_ONLY,        /**< Wrote to a read-only filesystem.      */
    PHYSFS_ERR_CORRUPT,          /**< Corrupted data encountered.           */
    PHYSFS_ERR_SYMLINK_LOOP,     /**< Infinite symbolic link loop.          */
    PHYSFS_ERR_IO,               /**< i/o error (hardware failure, etc).    */
    PHYSFS_ERR_PERMISSION,       /**< Permission denied.                    */
    PHYSFS_ERR_NO_SPACE,         /**< No space (disk full, over quota, etc) */
    PHYSFS_ERR_BAD_FILENAME,     /**< Filename is bogus/insecure.           */
    PHYSFS_ERR_BUSY,             /**< Tried to modify a file the OS needs.  */
    PHYSFS_ERR_DIR_NOT_EMPTY,    /**< Tried to delete dir with files in it. */
    PHYSFS_ERR_OS_ERROR,         /**< Unspecified OS-level error.           */
    PHYSFS_ERR_DUPLICATE,        /**< Duplicate entry.                      */
    PHYSFS_ERR_BAD_PASSWORD,     /**< Bad password.                         */
    PHYSFS_ERR_APP_CALLBACK      /**< Application callback reported error.  */
} PHYSFS_ErrorCode;


/**
 * \fn PHYSFS_ErrorCode PHYSFS_getLastErrorCode(void)
 * \brief Get machine-readable error information.
 *
 * Get the last PhysicsFS error message as an integer value. This will return
 *  PHYSFS_ERR_OK if there's been no error since the last call to this
 *  function. Each thread has a unique error state associated with it, but
 *  each time a new error message is set, it will overwrite the previous one
 *  associated with that thread. It is safe to call this function at anytime,
 *  even before PHYSFS_init().
 *
 * PHYSFS_getLastError() and PHYSFS_getLastErrorCode() both reset the same
 *  thread-specific error state. Calling one will wipe out the other's
 *  data. If you need both, call PHYSFS_getLastErrorCode(), then pass that
 *  value to PHYSFS_getErrorByCode().
 *
 * Generally, applications should only concern themselves with whether a
 *  given function failed; however, if you require more specifics, you can
 *  try this function to glean information, if there's some specific problem
 *  you're expecting and plan to handle. But with most things that involve
 *  file systems, the best course of action is usually to give up, report the
 *  problem to the user, and let them figure out what should be done about it.
 *  For that, you might prefer PHYSFS_getErrorByCode() instead.
 *
 *   \return Enumeration value that represents last reported error.
 *
 * \sa PHYSFS_getErrorByCode
 */
PHYSFS_DECL PHYSFS_ErrorCode PHYSFS_getLastErrorCode(void);


/**
 * \fn const char *PHYSFS_getErrorByCode(PHYSFS_ErrorCode code)
 * \brief Get human-readable description string for a given error code.
 *
 * Get a static string, in UTF-8 format, that represents an English
 *  description of a given error code.
 *
 * This string is guaranteed to never change (although we may add new strings
 *  for new error codes in later versions of PhysicsFS), so you can use it
 *  for keying a localization dictionary.
 *
 * It is safe to call this function at anytime, even before PHYSFS_init().
 *
 * These strings are meant to be passed on directly to the user.
 *  Generally, applications should only concern themselves with whether a
 *  given function failed, but not care about the specifics much.
 *
 * Do not attempt to free the returned strings; they are read-only and you
 *  don't own their memory pages.
 *
 *   \param code Error code to convert to a string.
 *   \return READ ONLY string of requested error message, NULL if this
 *           is not a valid PhysicsFS error code. Always check for NULL if
 *           you might be looking up an error code that didn't exist in an
 *           earlier version of PhysicsFS.
 *
 * \sa PHYSFS_getLastErrorCode
 */
PHYSFS_DECL const char *PHYSFS_getErrorByCode(PHYSFS_ErrorCode code);

/**
 * \fn void PHYSFS_setErrorCode(PHYSFS_ErrorCode code)
 * \brief Set the current thread's error code.
 *
 * This lets you set the value that will be returned by the next call to
 *  PHYSFS_getLastErrorCode(). This will replace any existing error code,
 *  whether set by your application or internally by PhysicsFS.
 *
 * Error codes are stored per-thread; what you set here will not be
 *  accessible to another thread.
 *
 * Any call into PhysicsFS may change the current error code, so any code you
 *  set here is somewhat fragile, and thus you shouldn't build any serious
 *  error reporting framework on this function. The primary goal of this
 *  function is to allow PHYSFS_Io implementations to set the error state,
 *  which generally will be passed back to your application when PhysicsFS
 *  makes a PHYSFS_Io call that fails internally.
 *
 * This function doesn't care if the error code is a value known to PhysicsFS
 *  or not (but PHYSFS_getErrorByCode() will return NULL for unknown values).
 *  The value will be reported unmolested by PHYSFS_getLastErrorCode().
 *
 *   \param code Error code to become the current thread's new error state.
 *
 * \sa PHYSFS_getLastErrorCode
 * \sa PHYSFS_getErrorByCode
 */
PHYSFS_DECL void PHYSFS_setErrorCode(PHYSFS_ErrorCode code);


/**
 * \fn const char *PHYSFS_getPrefDir(const char *org, const char *app)
 * \brief Get the user-and-app-specific path where files can be written.
 *
 * Helper function.
 *
 * Get the "pref dir". This is meant to be where users can write personal
 *  files (preferences and save games, etc) that are specific to your
 *  application. This directory is unique per user, per application.
 *
 * This function will decide the appropriate location in the native filesystem,
 *  create the directory if necessary, and return a string in
 *  platform-dependent notation, suitable for passing to PHYSFS_setWriteDir().
 *
 * On Windows, this might look like:
 *  "C:\\Users\\bob\\AppData\\Roaming\\My Company\\My Program Name"
 *
 * On Linux, this might look like:
 *  "/home/bob/.local/share/My Program Name"
 *
 * On Mac OS X, this might look like:
 *  "/Users/bob/Library/Application Support/My Program Name"
 *
 * (etc.)
 *
 * You should probably use the pref dir for your write dir, and also put it
 *  near the beginning of your search path. Older versions of PhysicsFS
 *  offered only PHYSFS_getUserDir() and left you to figure out where the
 *  files should go under that tree. This finds the correct location
 *  for whatever platform, which not only changes between operating systems,
 *  but also versions of the same operating system.
 *
 * You specify the name of your organization (if it's not a real organization,
 *  your name or an Internet domain you own might do) and the name of your
 *  application. These should be proper names.
 *
 * Both the (org) and (app) strings may become part of a directory name, so
 *  please follow these rules:
 *
 *    - Try to use the same org string (including case-sensitivity) for
 *      all your applications that use this function.
 *    - Always use a unique app string for each one, and make sure it never
 *      changes for an app once you've decided on it.
 *    - Unicode characters are legal, as long as it's UTF-8 encoded, but...
 *    - ...only use letters, numbers, and spaces. Avoid punctuation like
 *      "Game Name 2: Bad Guy's Revenge!" ... "Game Name 2" is sufficient.
 *
 * The pointer returned by this function remains valid until you call this
 *  function again, or call PHYSFS_deinit(). This is not necessarily a fast
 *  call, though, so you should call this once at startup and copy the string
 *  if you need it.
 *
 * You should assume the path returned by this function is the only safe
 *  place to write files (and that PHYSFS_getUserDir() and PHYSFS_getBaseDir(),
 *  while they might be writable, or even parents of the returned path, aren't
 *  where you should be writing things).
 *
 *   \param org The name of your organization.
 *   \param app The name of your application.
 *  \return READ ONLY string of user dir in platform-dependent notation. NULL
 *          if there's a problem (creating directory failed, etc).
 *
 * \sa PHYSFS_getBaseDir
 * \sa PHYSFS_getUserDir
 */
PHYSFS_DECL const char *PHYSFS_getPrefDir(const char *org, const char *app);


/**
 * \struct PHYSFS_Archiver
 * \brief Abstract interface to provide support for user-defined archives.
 *
 * \warning This is advanced, hardcore stuff. You don't need this unless you
 *          really know what you're doing. Most apps will not need this.
 *
 * Historically, PhysicsFS provided a means to mount various archive file
 *  formats, and physical directories in the native filesystem. However,
 *  applications have been limited to the file formats provided by the
 *  library. This interface allows an application to provide their own
 *  archive file types.
 *
 * Conceptually, a PHYSFS_Archiver provides directory entries, while
 *  PHYSFS_Io provides data streams for those directory entries. The most
 *  obvious use of PHYSFS_Archiver is to provide support for an archive
 *  file type that isn't provided by PhysicsFS directly: perhaps some
 *  proprietary format that only your application needs to understand.
 *
 * Internally, all the built-in archive support uses this interface, so the
 *  best examples for building a PHYSFS_Archiver is the source code to
 *  PhysicsFS itself.
 *
 * An archiver is added to the system with PHYSFS_registerArchiver(), and then
 *  it will be available for use automatically with PHYSFS_mount(); if a
 *  given archive can be handled with your archiver, it will be given control
 *  as appropriate.
 *
 * These methods deal with dir handles. You have one instance of your
 *  archiver, and it generates a unique, opaque handle for each opened
 *  archive in its openArchive() method. Since the lifetime of an Archiver
 *  (not an archive) is generally the entire lifetime of the process, and it's
 *  assumed to be a singleton, we do not provide any instance data for the
 *  archiver itself; the app can just use some static variables if necessary.
 *
 * Symlinks should always be followed (except in stat()); PhysicsFS will
 *  use the stat() method to check for symlinks and make a judgement on
 *  whether to continue to call other methods based on that.
 *
 * Archivers, when necessary, should set the PhysicsFS error state with
 *  PHYSFS_setErrorCode() before returning. PhysicsFS will pass these errors
 *  back to the application unmolested in most cases.
 *
 * Thread safety: PHYSFS_Archiver implementations are not guaranteed to be
 *  thread safe in themselves. PhysicsFS provides thread safety when it calls
 *  into a given archiver inside the library, but it does not promise that
 *  using the same PHYSFS_File from two threads at once is thread-safe; as
 *  such, your PHYSFS_Archiver can assume that locking is handled for you
 *  so long as the PHYSFS_Io you return from PHYSFS_open* doesn't change any
 *  of your Archiver state, as the PHYSFS_Io won't be as aggressively
 *  protected.
 *
 * \sa PHYSFS_registerArchiver
 * \sa PHYSFS_deregisterArchiver
 * \sa PHYSFS_supportedArchiveTypes
 */
typedef struct PHYSFS_Archiver
{
    /**
     * \brief Binary compatibility information.
     *
     * This must be set to zero at this time. Future versions of this
     *  struct will increment this field, so we know what a given
     *  implementation supports. We'll presumably keep supporting older
     *  versions as we offer new features, though.
     */
    PHYSFS_uint32 version;

    /**
     * \brief Basic info about this archiver.
     *
     * This is used to identify your archive, and is returned in
     *  PHYSFS_supportedArchiveTypes().
     */
    PHYSFS_ArchiveInfo info;

    /**
     * \brief Open an archive provided by (io).
     *
     * This is where resources are allocated and data is parsed when mounting
     *  an archive.
     * (name) is a filename associated with (io), but doesn't necessarily
     *  map to anything, let alone a real filename. This possibly-
     *  meaningless name is in platform-dependent notation.
     * (forWrite) is non-zero if this is to be used for
     *  the write directory, and zero if this is to be used for an
     *  element of the search path.
     * (claimed) should be set to 1 if this is definitely an archive your
     *  archiver implementation can handle, even if it fails. We use to
     *  decide if we should stop trying other archivers if you fail to open
     *  it. For example: the .zip archiver will set this to 1 for something
     *  that's got a .zip file signature, even if it failed because the file
     *  was also truncated. No sense in trying other archivers here, we
     *  already tried to handle it with the appropriate implementation!.
     * Return NULL on failure and set (claimed) appropriately. If no archiver
     *  opened the archive or set (claimed), PHYSFS_mount() will report
     *  PHYSFS_ERR_UNSUPPORTED. Otherwise, it will report the error from the
     *  archiver that claimed the data through (claimed).
     * Return non-NULL on success. The pointer returned will be
     *  passed as the "opaque" parameter for later calls.
     */
    void *(*openArchive)(PHYSFS_Io *io, const char *name,
                         int forWrite, int *claimed);

    /**
     * \brief List all files in (dirname).
     *
     * Each file is passed to (cb), where a copy is made if appropriate, so
     *  you can dispose of it upon return from the callback. (dirname) is in
     *  platform-independent notation.
     * If you have a failure, call PHYSFS_SetErrorCode() with whatever code
     *  seem appropriate and return PHYSFS_ENUM_ERROR.
     * If the callback returns PHYSFS_ENUM_ERROR, please call
     *  PHYSFS_SetErrorCode(PHYSFS_ERR_APP_CALLBACK) and then return
     *  PHYSFS_ENUM_ERROR as well. Don't call the callback again in any
     *  circumstances.
     * If the callback returns PHYSFS_ENUM_STOP, stop enumerating and return
     *  PHYSFS_ENUM_STOP as well. Don't call the callback again in any
     *  circumstances. Don't set an error code in this case.
     * Callbacks are only supposed to return a value from
     *  PHYSFS_EnumerateCallbackResult. Any other result has undefined
     *  behavior.
     * As long as the callback returned PHYSFS_ENUM_OK and you haven't
     *  experienced any errors of your own, keep enumerating until you're done
     *  and then return PHYSFS_ENUM_OK without setting an error code.
     *
     * \warning PHYSFS_enumerate returns zero or non-zero (success or failure),
     *          so be aware this function pointer returns different values!
     */
    PHYSFS_EnumerateCallbackResult (*enumerate)(void *opaque,
                     const char *dirname, PHYSFS_EnumerateCallback cb,
                     const char *origdir, void *callbackdata);

    /**
     * \brief Open a file in this archive for reading.
     *
     * This filename, (fnm), is in platform-independent notation.
     * Fail if the file does not exist.
     * Returns NULL on failure, and calls PHYSFS_setErrorCode().
     *  Returns non-NULL on success. The pointer returned will be
     *  passed as the "opaque" parameter for later file calls.
     */
    PHYSFS_Io *(*openRead)(void *opaque, const char *fnm);

    /**
     * \brief Open a file in this archive for writing.
     *
     * If the file does not exist, it should be created. If it exists,
     *  it should be truncated to zero bytes. The writing offset should
     *  be the start of the file.
     * If the archive is read-only, this operation should fail.
     * This filename is in platform-independent notation.
     * Returns NULL on failure, and calls PHYSFS_setErrorCode().
     *  Returns non-NULL on success. The pointer returned will be
     *  passed as the "opaque" parameter for later file calls.
     */
    PHYSFS_Io *(*openWrite)(void *opaque, const char *filename);

    /**
     * \brief Open a file in this archive for appending.
     *
     * If the file does not exist, it should be created. The writing
     *  offset should be the end of the file.
     * If the archive is read-only, this operation should fail.
     * This filename is in platform-independent notation.
     * Returns NULL on failure, and calls PHYSFS_setErrorCode().
     *  Returns non-NULL on success. The pointer returned will be
     *  passed as the "opaque" parameter for later file calls.
     */
    PHYSFS_Io *(*openAppend)(void *opaque, const char *filename);

    /**
     * \brief Delete a file or directory in the archive.
     *
     * This same call is used for both files and directories; there is not a
     *  separate rmdir() call. Directories are only meant to be removed if
     *  they are empty.
     * If the archive is read-only, this operation should fail.
     *
     * Return non-zero on success, zero on failure.
     * This filename is in platform-independent notation.
     * On failure, call PHYSFS_setErrorCode().
     */
    int (*remove)(void *opaque, const char *filename);

    /**
     * \brief Create a directory in the archive.
     *
     * If the application is trying to make multiple dirs, PhysicsFS
     *  will split them up into multiple calls before passing them to
     *  your driver.
     * If the archive is read-only, this operation should fail.
     * Return non-zero on success, zero on failure.
     *  This filename is in platform-independent notation.
     * On failure, call PHYSFS_setErrorCode().
     */
    int (*mkdir)(void *opaque, const char *filename);

    /**
     * \brief Obtain basic file metadata.
     *
     * On success, fill in all the fields in (stat), using
     *  reasonable defaults for fields that apply to your archive.
     *
     * Returns non-zero on success, zero on failure.
     * This filename is in platform-independent notation.
     * On failure, call PHYSFS_setErrorCode().
     */
    int (*stat)(void *opaque, const char *fn, PHYSFS_Stat *stat);

    /**
     * \brief Destruct a previously-opened archive.
     *
     * Close this archive, and free any associated memory,
     *  including the original PHYSFS_Io and (opaque) itself, if
     *  applicable. Implementation can assume that it won't be called if
     *  there are still files open from this archive.
     */
    void (*closeArchive)(void *opaque);
} PHYSFS_Archiver;

/**
 * \fn int PHYSFS_registerArchiver(const PHYSFS_Archiver *archiver)
 * \brief Add a new archiver to the system.
 *
 * \warning This is advanced, hardcore stuff. You don't need this unless you
 *          really know what you're doing. Most apps will not need this.
 *
 * If you want to provide your own archiver (for example, a custom archive
 *  file format, or some virtual thing you want to make look like a filesystem
 *  that you can access through the usual PhysicsFS APIs), this is where you
 *  start. Once an archiver is successfully registered, then you can use
 *  PHYSFS_mount() to add archives that your archiver supports to the
 *  search path, or perhaps use it as the write dir. Internally, PhysicsFS
 *  uses this function to register its own built-in archivers, like .zip
 *  support, etc.
 *
 * You may not have two archivers that handle the same extension. If you are
 *  going to have a clash, you can deregister the other archiver (including
 *  built-in ones) with PHYSFS_deregisterArchiver().
 *
 * The data in (archiver) is copied; you may free this pointer when this
 *  function returns.
 *
 * Once this function returns successfully, PhysicsFS will be able to support
 *  archives of this type until you deregister the archiver again.
 *
 *   \param archiver The archiver to register.
 *  \return Zero on error, non-zero on success.
 *
 * \sa PHYSFS_Archiver
 * \sa PHYSFS_deregisterArchiver
 */
PHYSFS_DECL int PHYSFS_registerArchiver(const PHYSFS_Archiver *archiver);

/**
 * \fn int PHYSFS_deregisterArchiver(const char *ext)
 * \brief Remove an archiver from the system.
 *
 * If for some reason, you only need your previously-registered archiver to
 *  live for a portion of your app's lifetime, you can remove it from the
 *  system once you're done with it through this function.
 *
 * This fails if there are any archives still open that use this archiver.
 *
 * This function can also remove internally-supplied archivers, like .zip
 *  support or whatnot. This could be useful in some situations, like
 *  disabling support for them outright or overriding them with your own
 *  implementation. Once an internal archiver is disabled like this,
 *  PhysicsFS provides no mechanism to recover them, short of calling
 *  PHYSFS_deinit() and PHYSFS_init() again.
 *
 * PHYSFS_deinit() will automatically deregister all archivers, so you don't
 *  need to explicitly deregister yours if you otherwise shut down cleanly.
 *
 *   \param ext Filename extension that the archiver handles.
 *  \return Zero on error, non-zero on success.
 *
 * \sa PHYSFS_Archiver
 * \sa PHYSFS_registerArchiver
 */
PHYSFS_DECL int PHYSFS_deregisterArchiver(const char *ext);


/* Everything above this line is part of the PhysicsFS 2.1 API. */


/**
 * \fn int PHYSFS_setRoot(const char *archive, const char *subdir)
 * \brief Make a subdirectory of an archive its root directory.
 *
 * This lets you narrow down the accessible files in a specific archive. For
 *  example, if you have x.zip with a file in y/z.txt, mounted to /a, if you
 *  call PHYSFS_setRoot("x.zip", "/y"), then the call
 *  PHYSFS_openRead("/a/z.txt") will succeed.
 *
 * You can change an archive's root at any time, altering the interpolated
 *  file tree (depending on where paths shift, a different archive may be
 *  providing various files). If you set the root to NULL or "/", the
 *  archive will be treated as if no special root was set (as if the archive
 *  was just mounted normally).
 *
 * Changing the root only affects future operations on pathnames; a file
 *  that was opened from a path that changed due to a setRoot will not be
 *  affected.
 *
 * Setting a new root is not limited to archives in the search path; you may
 *  set one on the write dir, too, which might be useful if you have files
 *  open for write and thus can't change the write dir at the moment.
 *
 * It is not an error to set a subdirectory that does not exist to be the
 *  root of an archive; however, no files will be visible in this case. If
 *  the missing directories end up getting created (a mkdir to the physical
 *  filesystem, etc) then this will be reflected in the interpolated tree.
 *
 *    \param archive dir/archive on which to change root.
 *    \param subdir new subdirectory to make the root of this archive.
 *   \return nonzero on success, zero on failure. Use
 *           PHYSFS_getLastErrorCode() to obtain the specific error.
 */
PHYSFS_DECL int PHYSFS_setRoot(const char *archive, const char *subdir);


/* Everything above this line is part of the PhysicsFS 3.1 API. */


#ifdef __cplusplus
}
#endif

#endif  /* !defined _INCLUDE_PHYSFS_H_ */

/* end of physfs.h ... */

