/**
 * Copyright (c) 2006-2012 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_CONFIG_H
#define LOVE_CONFIG_H

// Platform stuff.
#if defined(WIN32) || defined(_WIN32)
#	define LOVE_WINDOWS 1
#endif
#if defined(linux) || defined(__linux) || defined(__linux__)
#	define LOVE_LINUX 1
#endif
#if defined(__APPLE__)
#	define LOVE_MACOSX 1
#endif
#if defined(macintosh)
#	define LOVE_MACOS 1
#endif

// Endianness.
#if defined(__i386__) || defined(__i386)
#	define LOVE_LITTLE_ENDIAN 1
#endif
#if defined(__ppc__) || defined(__ppc) || defined(__powerpc__) || defined(__powerpc)
#	define LOVE_BIG_ENDIAN 1
#endif

// Warnings.
#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

// Preferably, and ironically, this macro should go unused.
#ifndef LOVE_UNUSED
#	define LOVE_UNUSED(x) (void)sizeof(x)
#endif

#ifndef LOVE_BUILD
#	define LOVE_BUILD
#	define LOVE_BUILD_STANDALONE
#	define LOVE_BUILD_EXE
//#	define LOVE_BUILD_DLL
#endif

// DLL-stuff.
#ifdef LOVE_WINDOWS
#	define LOVE_EXPORT __declspec(dllexport)
#else
#	define LOVE_EXPORT
#endif

#if defined(LOVE_WINDOWS)
#	define LOVE_LEGENDARY_UTF8_ARGV_HACK
#	define LOVE_LEGENDARY_CONSOLE_IO_HACK
#	define NOMINMAX
#endif

#if defined(LOVE_MACOSX)
#	define LOVE_LEGENDARY_LIBSTDCXX_HACK
#endif

// Autotools config.h
#ifdef HAVE_CONFIG_H
#	include <../config.h>
#	undef VERSION
#	ifdef WORDS_BIGENDIAN
#		undef LOVE_LITTLE_ENDIAN
#		define LOVE_BIG_ENDIAN 1
#	else
#		undef LOVE_BIG_ENDIAN
#		define LOVE_LITTLE_ENDIAN 1
#	endif
#endif

#endif // LOVE_CONFIG_H
