AC_DEFUN([ACLOVE_DEP_FREETYPE2], [
	PKG_CHECK_MODULES([freetype2], [freetype2], [], [LOVE_MSG_ERROR([FreeType2])])])

AC_DEFUN([ACLOVE_DEP_OPENAL], [
	PKG_CHECK_MODULES([openal], [openal], [], [LOVE_MSG_ERROR([OpenAL])])])

AC_DEFUN([ACLOVE_DEP_LIBMODPLUG], [
	PKG_CHECK_MODULES([libmodplug], [libmodplug], [], [LOVE_MSG_ERROR([libmodplug])])])

AC_DEFUN([ACLOVE_DEP_VORBISFILE], [
	PKG_CHECK_MODULES([vorbisfile], [vorbisfile], [], [LOVE_MSG_ERROR([libvorbis and libvorbisfile])])])

AC_DEFUN([ACLOVE_DEP_ZLIB], [
	PKG_CHECK_MODULES([zlib], [zlib], [], [LOVE_MSG_ERROR([zlib])])])

AC_DEFUN([ACLOVE_DEP_THEORA], [
	PKG_CHECK_MODULES([theora], [theoradec], [], [LOVE_MSG_ERROR([libtheora])])])

AC_DEFUN([ACLOVE_DEP_LIBM], [
	AC_SEARCH_LIBS([sqrt], [m], [], [LOVE_MSG_ERROR([the C math library])])])

AC_DEFUN([ACLOVE_DEP_SDL2], [
	aclove_sdl2_found=no
	AM_PATH_SDL2([], [aclove_sdl2_found=yes], [])
	AS_VAR_IF([aclove_sdl2_found], [no], [LOVE_MSG_ERROR([SDL 2])], [])])

AC_DEFUN([ACLOVE_DEP_PTHREAD], [
	AC_SEARCH_LIBS([pthread_create], [pthread], [], [LOVE_MSG_ERROR([the POSIX threads library])])])

# does not use pkg-config because of the FILE_OFFSET_BITS.. bit
AC_DEFUN([ACLOVE_DEP_MPG123], [
	AC_SEARCH_LIBS([mpg123_open_feed], [mpg123], [],
		[LOVE_MSG_ERROR([libmpg123])])
	AC_SEARCH_LIBS([mpg123_seek_64], [mpg123],
		AC_SUBST([FILE_OFFSET],[-D_FILE_OFFSET_BITS=64]),
		AC_SUBST([FILE_OFFSET],[]))])

AC_DEFUN([ACLOVE_DEP_GME], [
	AC_SEARCH_LIBS([gme_open_data], [gme], [], [LOVE_MSG_ERROR([gme])])
	AC_DEFINE([LOVE_SUPPORT_GME], [], [Enable gme])
	AC_CHECK_HEADER([gme/gme.h], [includes="$includes -I/usr/include/gme"], [])])

# For enet
AC_DEFUN([ACLOVE_SOCKLEN_T], [
	AC_CHECK_TYPE([socklen_t], [AC_DEFINE([HAS_SOCKLEN_T], [1], [Define if socklen_t exists.] )], ,
		#include <sys/types.h>
		#include <sys/socket.h>
	)])

# The lua detection is quite annoying because of distro differences and our version selection code
AC_DEFUN([ACLOVE_DEP_LUA_PKGCONFIG], [
	PKG_CHECK_MODULES([lua], [${with_lua}${with_luaversion}], [aclove_lua_found=yes],
		[PKG_CHECK_MODULES([lua], [${with_lua}${with_clean_luaversion}], [aclove_lua_found=yes],
			[PKG_CHECK_MODULES([lua], [${with_lua}], [aclove_lua_found=yes], [])])])

	AS_VAR_IF([aclove_lua_found], [yes],
		[
			aclove_luaheaders_found=yes
			AC_MSG_CHECKING([for library containing lua_call])
			AC_MSG_RESULT([${lua_LIBS}])
		], [])])

AC_DEFUN([ACLOVE_DEP_LUA_FALLBACK], [
	AC_MSG_WARN([Could not find pkg-config definition for ${with_lua}${with_luaversion} or ${with_lua}${with_clean_luaversion}${with_lua}, falling back to manual detection])
	AC_SEARCH_LIBS([lua_call], ["${with_lua}${with_luaversion}" "${with_lua}"], [aclove_lua_found=yes],
		[LOVE_MSG_ERROR([$with_lua])])

	AC_CHECK_HEADER(["${with_lua}${with_luaversion}/lua.h"], [aclove_luaheaders_found=yes includes="$includes -I/usr/include/${with_lua}${with_luaversion}"], [])
	AC_CHECK_HEADER(["${with_lua}/lua.h"], [aclove_luaheaders_found=yes includes="$includes -I/usr/include/${with_lua}"], [])])

# First, try pkg-config, then fall back to manual detection if not available.
AC_DEFUN([ACLOVE_DEP_LUA], [ dnl Requires with-lua and with-luaversion
	aclove_lua_found=no
	aclove_luaheaders_found=no

	ACLOVE_DEP_LUA_PKGCONFIG
	AS_VAR_IF([aclove_lua_found], [no], [ACLOVE_DEP_LUA_FALLBACK], [])
	AS_VAR_IF([aclove_luaheaders_found], [no],
		[AC_MSG_WARN([Could not locate lua headers for ${with_lua}${with_luaversion} or ${with_lua}, you probably need to specify them with CPPFLAGS])], [])])
