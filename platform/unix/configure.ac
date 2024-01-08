AC_INIT([love], [12.0])
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
m4_include([platform/unix/cpp14.m4])
m4_include([platform/unix/deps.m4])

includes=

AC_DEFUN([LOVE_MSG_ERROR],
		 [AC_MSG_ERROR([LÖVE needs "$1"[,] please install "$1" with development files and try again])])

# C++17 support in cpp14.m4
ACLOVE_CPP17_TEST

# Add -fvisibility=hidden and -fvisibility-inlines-hidden
CFLAGS="-fvisibility=hidden $CFLAGS"
CXXFLAGS="-fvisibility=hidden -fvisibility-inlines-hidden $CXXFLAGS"

# Allow people on OSX to use autotools, they need their platform files
AC_ARG_ENABLE([osx],
			  AC_HELP_STRING([--enable-osx], [Compile platform-specific files for OSX]), [], [enable_osx=no])
AS_VAR_IF([enable_osx], [no], [], #else
		  ac_cv_search_glLoadIdentity="-framework OpenGL"
		  AC_SUBST([LDFLAGS], ["${LDFLAGS} -framework CoreFoundation -framework Cocoa"])
		  AC_SUBST([CPPFLAGS], ["${CPPFLAGS} -I../platform/macosx"]))

# stb_image sse2 override (https://github.com/nothings/stb/issues/280)
AC_ARG_ENABLE([stbi-sse2-override],
			  AC_HELP_STRING([--enable-stbi-sse2-override], [Force stb_image SSE2 support]), [], [enable_stbi_sse2_override=no])
AS_VAR_IF([enable_stbi_sse2_override], [no], [], #else
		  AC_SUBST([CPPFLAGS], ["${CPPFLAGS} -DLOVE_STBI_SSE2_OVERRIDE"]))

# --with-lua and --with-luaversion
AC_ARG_WITH([lua], [AS_HELP_STRING([--with-lua], [Select the lua implementation])],
	[], [with_lua=luajit])
AC_ARG_WITH([luaversion], [AS_HELP_STRING([--with-luaversion], [Select the lua version])],
	[], [with_luaversion=5.1])

with_clean_luaversion=`printf ${with_luaversion} | sed 's/\.//g'`

# Generated sources for enabling/disabling modules
m4_include([configure-modules-pre.ac])

# Dependencies we always use
ACLOVE_DEP_LUA
ACLOVE_DEP_SDL2
ACLOVE_DEP_LIBM
ACLOVE_DEP_ZLIB
ACLOVE_DEP_PTHREAD

# Conditional dependencies
AS_VAR_IF([enable_module_audio], [yes], [ACLOVE_DEP_OPENAL], [])
AS_VAR_IF([enable_module_font], [yes], [
	ACLOVE_DEP_FREETYPE2
	ACLOVE_DEP_HARFBUZZ
], [])
AS_VAR_IF([enable_module_sound], [yes], [
	ACLOVE_DEP_LIBMODPLUG
	ACLOVE_DEP_VORBISFILE
], [])
AS_VAR_IF([enable_module_video], [yes], [ACLOVE_DEP_THEORA], [])

# Add flags for optional libraries
AC_ARG_ENABLE([library-enet], [  --disable-library-enet    Turn off library enet], [], [enable_library_enet=yes])
AC_ARG_ENABLE([library-luasocket], [  --disable-library-luasocket    Turn off library luasocket], [], [enable_library_luasocket=yes])
AC_ARG_ENABLE([library-lua53], [  --disable-library-lua53    Turn off library lua53 (lua 5.3 backports, required by love.data)], [], [enable_library_lua53=yes])
AC_ARG_ENABLE([library-luahttps], [  --disable-library-luahttps    Turn off library luahttps], [], [enable_library_luahttps=yes])

# Select the libraries we need to build, based on the selected modules
AS_VAR_IF([enable_module_filesystem], [yes], [enable_library_physfs=yes], [enable_library_physfs=no])
AS_VAR_IF([enable_module_physics], [yes], [enable_library_box2d=yes], [enable_library_box2d=no])
AS_VAR_IF([enable_module_image], [yes], [enable_library_ddsparse=yes], [enable_library_ddsparse=no])
AS_VAR_IF([enable_module_graphics], [yes], [enable_library_glad=yes], [enable_library_glad=no])
AS_VAR_IF([enable_module_graphics], [yes], [enable_library_spirv_cross=yes], [enable_library_spirv_cross=no])
AS_VAR_IF([enable_module_graphics], [yes], [enable_library_glslang=yes], [enable_library_glslang=no])
AS_VAR_IF([enable_module_image], [yes], [enable_library_lodepng=yes], [enable_library_lodepng=no])
AS_VAR_IF([enable_module_data], [yes], [enable_library_lua53=yes], [])
AS_VAR_IF([enable_module_math], [yes], [enable_library_lz4=yes], [enable_library_lz4=no])
AS_VAR_IF([enable_module_math], [yes], [enable_library_noise1234=yes], [enable_library_noise1234=no])
AS_VAR_IF([enable_module_image], [yes], [enable_library_stb=yes], [enable_library_stb=no])
AS_VAR_IF([enable_module_image], [yes], [enable_library_tinyexr=yes], [enable_library_tinyexr=no])
AS_VAR_IF([enable_module_sound], [yes], [enable_library_Wuff=yes], [enable_library_Wuff=no])
AS_VAR_IF([enable_module_graphics], [yes], [enable_library_xxHash=yes], [enable_library_xxHash=no])

# Utf8 is required by both graphics and font, so enable if either is enabled
enable_library_utf8=no
AS_VAR_IF([enable_module_graphics], [yes], [enable_library_utf8=yes], [])
AS_VAR_IF([enable_module_font], [yes], [enable_library_utf8=yes], [])

# libenet check for socklen_t
ACLOVE_SOCKLEN_T

# Generated sources for enabling/disabling modules
m4_include([configure-modules-post.ac])

# Optionally build the executable (default on)
AC_ARG_ENABLE([exe],
			  AC_HELP_STRING([--disable-exe], [Disable building of executable launcher]), [], [enable_exe=yes])

AS_VAR_IF([enable_exe], [no], [], #else
	  AC_DEFINE([LOVE_BUILD_EXE], [], [Skip building launcher]))

AM_CONDITIONAL([LOVE_BUILD_EXE], [test "x$enable_exe" != xno])
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
