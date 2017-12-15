#ifndef _INCL_PHYSFS_PLATFORMS
#define _INCL_PHYSFS_PLATFORMS

#ifndef __PHYSICSFS_INTERNAL__
#error Do not include this header from your applications.
#endif

/*
 * These only define the platforms to determine which files in the platforms
 *  directory should be compiled. For example, technically BeOS can be called
 *  a "unix" system, but since it doesn't use unix.c, we don't define
 *  PHYSFS_PLATFORM_UNIX on that system.
 */

#if (defined __HAIKU__)
#  define PHYSFS_PLATFORM_HAIKU 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif ((defined __BEOS__) || (defined __beos__))
#  error BeOS support was dropped since PhysicsFS 2.1. Sorry. Try Haiku!
#elif (defined _WIN32_WCE) || (defined _WIN64_WCE)
#  error PocketPC support was dropped since PhysicsFS 2.1. Sorry. Try WinRT!
#elif (defined(_MSC_VER) && (_MSC_VER >= 1700) && !_USING_V110_SDK71_)	/* _MSC_VER==1700 for MSVC 2012 */
#  include <winapifamily.h>
#  define PHYSFS_PLATFORM_WINDOWS 1
#  if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#    define PHYSFS_NO_CDROM_SUPPORT 1
#    define PHYSFS_PLATFORM_WINRT 1
#  endif
#elif (((defined _WIN32) || (defined _WIN64)) && (!defined __CYGWIN__))
#  define PHYSFS_PLATFORM_WINDOWS 1
#elif defined(__OS2__) || defined(OS2)
#  define PHYSFS_PLATFORM_OS2 1
#elif ((defined __MACH__) && (defined __APPLE__))
/* To check if iOS or not, we need to include this file */
#  include <TargetConditionals.h>
#  if ((TARGET_IPHONE_SIMULATOR) || (TARGET_OS_IPHONE))
#     define PHYSFS_NO_CDROM_SUPPORT 1
#  endif
#  define PHYSFS_PLATFORM_APPLE 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(macintosh)
#  error Classic Mac OS support was dropped from PhysicsFS 2.0. Move to OS X.
#elif defined(ANDROID)
#  define PHYSFS_PLATFORM_LINUX 1
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#  define PHYSFS_NO_CDROM_SUPPORT 1
#elif defined(__linux)
#  define PHYSFS_PLATFORM_LINUX 1
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(__sun) || defined(sun)
#  define PHYSFS_PLATFORM_SOLARIS 1
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
#  define PHYSFS_PLATFORM_FREEBSD 1
#  define PHYSFS_PLATFORM_BSD 1
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__)
#  define PHYSFS_PLATFORM_BSD 1
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(__EMSCRIPTEN__)
#  define PHYSFS_NO_CDROM_SUPPORT 1
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(__QNX__)
#  define PHYSFS_PLATFORM_QNX 1
#  define PHYSFS_PLATFORM_POSIX 1
#elif defined(unix) || defined(__unix__)
#  define PHYSFS_PLATFORM_UNIX 1
#  define PHYSFS_PLATFORM_POSIX 1
#else
#  error Unknown platform.
#endif

#endif  /* include-once blocker. */

