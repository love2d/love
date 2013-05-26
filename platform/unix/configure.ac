AC_INIT([love], [HEAD])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([platform/unix])
AC_CONFIG_MACRO_DIR([platform/unix/m4])
AC_CONFIG_SRCDIR([src/love.cpp])
AM_INIT_AUTOMAKE([foreign -Wall foreign tar-ustar silent-rules])
AM_SILENT_RULES
AC_PREFIX_DEFAULT([/usr])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_SED
AC_PROG_MKDIR_P
AC_C_BIGENDIAN

includes=

# Libraries
AC_SEARCH_LIBS([sqrt], [m], [], AC_MSG_ERROR([Can''t LÖVE without C math library]))
AC_SEARCH_LIBS([SDL_Init], [SDL], [], AC_MSG_ERROR([Can''t LÖVE without SDL]))
AC_SEARCH_LIBS([glLoadIdentity], [GL], [], AC_MSG_ERROR([Can''t LÖVE without OpenGL]))
AC_SEARCH_LIBS([alSourcePlay], [openal], [], AC_MSG_ERROR([Can''t LÖVE without OpenAL]))
AC_SEARCH_LIBS([ilInit], [IL], [], AC_MSG_ERROR([Can''t LÖVE without DevIL]))
AC_SEARCH_LIBS([FT_Load_Glyph], [freetype], [], AC_MSG_ERROR([Can''t LÖVE without FreeType]))
AC_SEARCH_LIBS([PHYSFS_init], [physfs], [], AC_MSG_ERROR([Can''t LÖVE without PhysicsFS]))
AC_SEARCH_LIBS([ModPlug_Load], [modplug], [], AC_MSG_ERROR([Can''t LÖVE without ModPlug]))
AC_SEARCH_LIBS([ov_open], [vorbisfile], [], AC_MSG_ERROR([Can''t LÖVE without VorbisFile]))

# Includes
AC_CHECK_HEADER([SDL/SDL.h], [includes="$includes -I/usr/include/SDL"])
AC_CHECK_HEADER([freetype2/freetype/config/ftheader.h], [includes="$includes -I/usr/include/freetype2"])

# Lua, treated seperately because of --with-lua
AC_ARG_WITH([lua], [AS_HELP_STRING([--with-lua], [Select the lua implementation])],
			[], [with_lua=lua])

AC_SEARCH_LIBS([lua_pcall], ["$with_lua" "${with_lua}5.1"], [],
			   AC_MSG_ERROR([Can''t LÖVE without $with_lua]))
AC_CHECK_HEADER(["${with_lua}/lua.h"], [includes="$includes -I/usr/include/${with_lua}"], [])
AC_CHECK_HEADER(["${with_lua}5.1/lua.h"], [includes="$includes -I/usr/include/${with_lua}5.1"], [])

# mpg123, treated seperately because it can be disabled (default on)
AC_ARG_ENABLE([mpg123], AC_HELP_STRING([--disable-mpg123], [Disable mp3 support, for patent-free builds]), [], [enable_mpg123=yes])
AS_VAR_IF([enable_mpg123], [no],
	  AC_DEFINE([LOVE_NOMPG123], [], [Build without mpg123]),
	  # else
	  AC_SEARCH_LIBS([mpg123_open_feed], [mpg123], [],
					 AC_MSG_ERROR([Can''t LÖVE without Mpg123]))
	  AC_SEARCH_LIBS([mpg123_seek_64], [mpg123],
					 AC_SUBST([FILE_OFFSET],[-D_FILE_OFFSET_BITS=64]),
					 AC_SUBST([FILE_OFFSET],[])))

# GME, treated seperately because it can be enabled (default off)
AC_ARG_ENABLE([gme], AC_HELP_STRING([--enable-gme], [Enable GME support, for more chiptuney goodness]), [], [enable_gme=no])
AS_VAR_IF([enable_gme], [yes],
	  AC_SEARCH_LIBS([gme_open_data], [gme], [], AC_MSG_ERROR([Can''t LÖVE without gme]))
	  AC_DEFINE([LOVE_SUPPORT_GME], [], [Enable gme])
	  AC_CHECK_HEADER([gme/gme.h], [includes="$includes -I/usr/include/gme"], []))

# Optionally build the executable (default on)
AC_ARG_ENABLE([exe],
			  AC_HELP_STRING([--disable-exe], [Disable building of executable launcher]), [], [enable_exe=yes])

AS_VAR_IF([enable_exe], [no], [], #else
	  AC_DEFINE([LOVE_BUILD_EXE], [], [Don''t build launcher]))

AM_CONDITIONAL([LOVE_BUILD_EXE], [test "x$enable_exe" != xno])

# Set our includes as automake variable
AC_SUBST([LOVE_INCLUDES], ["$includes"])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
