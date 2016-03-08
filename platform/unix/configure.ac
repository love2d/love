AC_INIT([love], [HEAD])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([platform/unix])
AC_CONFIG_MACRO_DIR([platform/unix/m4])
AC_CONFIG_SRCDIR([src/love.cpp])
AM_INIT_AUTOMAKE([foreign -Wall foreign tar-ustar silent-rules])
AM_SILENT_RULES
AC_PREFIX_DEFAULT([/usr])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([disable-static])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_SED
AC_PROG_MKDIR_P
AC_PROG_OBJCXX
PKG_PROG_PKG_CONFIG
AC_C_BIGENDIAN
AC_LANG([C++])

dnl Workaround for old aclocal versions
m4_include([platform/unix/cpp11.m4])
m4_include([platform/unix/deps.m4])

includes=

AC_DEFUN([LOVE_MSG_ERROR],
		 [AC_MSG_ERROR([LÖVE needs "$1"[,] please install "$1" with development files and try again])])

# C++11 support in cpp11.m4
ACLOVE_CPP11_TEST

# Allow people on OSX to use autotools, they need their platform files
AC_ARG_ENABLE([osx],
			  AC_HELP_STRING([--enable-osx], [Compile platform-specific files for OSX]), [], [enable_osx=no])
AS_VAR_IF([enable_osx], [no], [], #else
		  ac_cv_search_glLoadIdentity="-framework OpenGL"
		  AC_SUBST([LDFLAGS], ["${LDFLAGS} -framework CoreFoundation -framework Cocoa"])
		  AC_SUBST([CPPFLAGS], ["${CPPFLAGS} -I../platform/macosx"]))

# --with-lua and --with-luaversion
AC_ARG_WITH([lua], [AS_HELP_STRING([--with-lua], [Select the lua implementation])],
	[], [with_lua=luajit])
AC_ARG_WITH([luaversion], [AS_HELP_STRING([--with-luaversion], [Select the lua version])],
	[], [with_luaversion=5.1])

with_clean_luaversion=`printf ${with_luaversion} | sed 's/\.//g'`

# Generated sources for enabling/disabling modules
m4_include([configure-modules.ac])

# Other features that can be enabled/disabled
AC_ARG_ENABLE([mpg123], AC_HELP_STRING([--disable-mpg123], [Disable mp3 support, for patent-free builds]), [], [enable_mpg123=yes])
AC_ARG_ENABLE([gme], AC_HELP_STRING([--enable-gme], [Enable GME support, for more chiptuney goodness]), [], [enable_gme=no])

ACLOVE_DEP_LUA
ACLOVE_DEP_SDL2
ACLOVE_DEP_LIBM
ACLOVE_DEP_ZLIB

AS_VAR_IF([enable_audio_openal], [true], [ACLOVE_DEP_OPENAL], [])
AS_VAR_IF([enable_filesystem_physfs], [true], [ACLOVE_DEP_PHYSFS], [])
AS_VAR_IF([enable_font_freetype], [true], [ACLOVE_DEP_FREETYPE2], [])
AS_VAR_IF([enable_sound_lullaby], [true], [
	ACLOVE_DEP_LIBMODPLUG
	ACLOVE_DEP_VORBISFILE
], [enable_mpg123=no])
AS_VAR_IF([enable_video_theora], [true], [ACLOVE_DEP_THEORA], [])
AS_VAR_IF([enable_gme], [yes], [ACLOVE_DEP_GME], [])
AS_VAR_IF([enable_mpg123], [no],
	  AC_DEFINE([LOVE_NOMPG123], [], [Build without mpg123]),
	  [ACLOVE_DEP_MPG123])

# GME, treated seperately because it can be enabled (default off)

# libenet check for socklen_t
ACLOVE_SOCKLEN_T

# Optionally build the executable (default on)
AC_ARG_ENABLE([exe],
			  AC_HELP_STRING([--disable-exe], [Disable building of executable launcher]), [], [enable_exe=yes])

AS_VAR_IF([enable_exe], [no], [], #else
	  AC_DEFINE([LOVE_BUILD_EXE], [], [Skip building launcher]))

AM_CONDITIONAL([LOVE_BUILD_EXE], [test "x$enable_exe" != xno])
AM_CONDITIONAL([LOVE_NOMPG123], [test "x$enable_mpg123" = xno])
AM_CONDITIONAL([LOVE_TARGET_OSX], [test "x$enable_osx" != xno])

# Automatic script file rebuilding
AC_CHECK_PROGS([LUA_EXECUTABLE], ["${with_lua}${with_luaversion}" "${with_lua}" "lua"], [:])
AS_VAR_IF([LUA_EXECUTABLE], [:],
		  [AC_MSG_WARN([Did not find a lua executable, you may need to update the scripts manually if you change them])], [])

# Set our includes as automake variable
AC_SUBST([LOVE_INCLUDES], ["$includes"])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	platform/unix/debian/control
	platform/unix/debian/changelog
	platform/unix/debian/rules
])
AC_OUTPUT

chmod 755 platform/unix/debian/rules
