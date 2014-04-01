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
AC_PROG_OBJCXX
AC_C_BIGENDIAN
AC_LANG([C++])

includes=

AC_DEFUN([ACLOVE_CXX_FLAG_TEST], # WARNING: NOT REENTRANT
		 [aclove_cxx_flag_test_save_cflags="$CXXFLAGS"
		  CXXFLAGS="$1"
		  AC_MSG_CHECKING([whether $CXX supports flag $1])
		  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
			  [AC_MSG_RESULT([yes])]; $2,
			  [AC_MSG_RESULT([no]); $3])
		  CXXFLAGS="$aclove_cxx_flag_test_save_cflags"])

AC_DEFUN([LOVE_MSG_ERROR],
		 [AC_MSG_ERROR([LÖVE needs "$1"[,] please install "$1" with development files and try again])])

# C++11 support
cxx11name="no"
ACLOVE_CXX_FLAG_TEST([-std=c++0x], cxx11name="c++0x", [])
ACLOVE_CXX_FLAG_TEST([-std=c++11], cxx11name="c++11", [])
AS_VAR_IF([cxx11name], [no], AC_MSG_ERROR([LÖVE needs a C++ compiler with C++11 support]), CXXFLAGS="$CXXFLAGS -std=$cxx11name")

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

# pkg-config libraries
AM_PATH_SDL2([], [], [LOVE_MSG_ERROR([SDL 2])])
PKG_CHECK_MODULES([lua], [${with_lua}${with_luaversion}], [lua_found=yes],
				  [PKG_CHECK_MODULES([lua], [${with_lua}], [lua_found=yes], [lua_found=no])])
PKG_CHECK_MODULES([freetype2], [freetype2], [], [LOVE_MSG_ERROR([FreeType2])])
PKG_CHECK_MODULES([openal], [openal], [], [LOVE_MSG_ERROR([OpenAL])])
PKG_CHECK_MODULES([devil], [IL], [], [LOVE_MSG_ERROR([DevIL])])
PKG_CHECK_MODULES([libmodplug], [libmodplug], [], [LOVE_MSG_ERROR([libmodplug])])
PKG_CHECK_MODULES([vorbisfile], [vorbisfile], [], [LOVE_MSG_ERROR([libvorbis and libvorbisfile])])

# Other libraries
AC_SEARCH_LIBS([sqrt], [m], [], [LOVE_MSG_ERROR([the C math library])])
AC_SEARCH_LIBS([PHYSFS_init], [physfs], [], [LOVE_MSG_ERROR([PhysicsFS])])
AC_SEARCH_LIBS([glLoadIdentity], [GL], [], [LOVE_MSG_ERROR([OpenGL])])

# Lua, treated seperately because of --with-lua
AS_VAR_IF([with_luaversion], [5.2], [luatest=lua_version], [luatest=lua_pcall]) # use lua_version for 5.2
AS_VAR_IF([lua_found], [yes],
		  #if
		  [
		   luaheaders_found=yes
		   AC_MSG_CHECKING([for library containing ${luatest}])
		   AC_MSG_RESULT([${lua_LIBS}])],
		  #else
		  [
		   AC_MSG_WARN([Could not find pkg-config definition for ${with_lua}${with_luaversion} or ${with_lua}, falling back to manual detection])
		   AC_SEARCH_LIBS([$luatest], ["${with_lua}${with_luaversion}" "${with_lua}"], [],
						  [LOVE_MSG_ERROR([$with_lua])])
		   luaheaders_found=no
		   AC_CHECK_HEADER(["${with_lua}${with_luaversion}/lua.h"], [luaheaders_found=yes includes="$includes -I/usr/include/${with_lua}${with_luaversion}"], [])
		   AC_CHECK_HEADER(["${with_lua}/lua.h"], [luaheaders_found=yes includes="$includes -I/usr/include/${with_lua}"], [])])
AS_VAR_IF([luaheaders_found], [yes], [], #else
		  [
		   AC_MSG_WARN([Could not locate lua headers for ${with_lua}${with_luaversion} or ${with_lua}, you probably need to specify them with CPPFLAGS])])

# mpg123, treated seperately because it can be disabled (default on)
# also not pkg-config because of the FILE_OFFSET_BITS.. bit
AC_ARG_ENABLE([mpg123], AC_HELP_STRING([--disable-mpg123], [Disable mp3 support, for patent-free builds]), [], [enable_mpg123=yes])
AS_VAR_IF([enable_mpg123], [no],
	  AC_DEFINE([LOVE_NOMPG123], [], [Build without mpg123]),
	  # else
	  AC_SEARCH_LIBS([mpg123_open_feed], [mpg123], [],
					 [LOVE_MSG_ERROR([libmpg123])])
	  AC_SEARCH_LIBS([mpg123_seek_64], [mpg123],
					 AC_SUBST([FILE_OFFSET],[-D_FILE_OFFSET_BITS=64]),
					 AC_SUBST([FILE_OFFSET],[])))

# GME, treated seperately because it can be enabled (default off)
AC_ARG_ENABLE([gme], AC_HELP_STRING([--enable-gme], [Enable GME support, for more chiptuney goodness]), [], [enable_gme=no])
AS_VAR_IF([enable_gme], [yes],
	  AC_SEARCH_LIBS([gme_open_data], [gme], [], [LOVE_MSG_ERROR([gme])])
	  AC_DEFINE([LOVE_SUPPORT_GME], [], [Enable gme])
	  AC_CHECK_HEADER([gme/gme.h], [includes="$includes -I/usr/include/gme"], []))

# libenet check for socklen_t
AC_CHECK_TYPE([socklen_t], [AC_DEFINE([HAS_SOCKLEN_T], [1], [Define if socklen_t exists.] )], , 
              #include <sys/types.h>
              #include <sys/socket.h>
)

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

m4_include([configure-modules.ac])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	platform/unix/debian/control
	platform/unix/debian/changelog
	platform/unix/debian/rules
])
AC_OUTPUT

chmod 755 platform/unix/debian/rules
