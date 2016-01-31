/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
# define LOVE_LINUX 1
#endif
#if defined(__ANDROID__)
#  define LOVE_ANDROID 1
#endif
#if defined(__APPLE__)
#	include <TargetConditionals.h>
#	if TARGET_OS_IPHONE
#		define LOVE_IOS 1
#	elif TARGET_OS_MAC
#		define LOVE_MACOSX 1
#	endif
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
// I know it's not linux, but it seems most "linux-only" code is bsd-compatible
#	define LOVE_LINUX 1
#endif

// Endianness.
#if defined(__ppc__) || defined(__ppc) || defined(__powerpc__) || defined(__powerpc)
#	define LOVE_BIG_ENDIAN 1
#else
#	define LOVE_LITTLE_ENDIAN 1
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
#	define LOVE_LEGENDARY_CONSOLE_IO_HACK
#	define NOMINMAX
#endif

#if defined(LOVE_MACOSX) || defined(LOVE_IOS)
#	define LOVE_LEGENDARY_APP_ARGV_HACK
#endif

#if defined(LOVE_ANDROID) || defined(LOVE_IOS)
#	define LOVE_LEGENDARY_ACCELEROMETER_AS_JOYSTICK_HACK
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
#else
#	define LOVE_ENABLE_AUDIO
#	define LOVE_ENABLE_AUDIO_NULL
#	define LOVE_ENABLE_AUDIO_OPENAL
#	define LOVE_ENABLE_BOX2D
#	define LOVE_ENABLE_DDSPARSE
#	define LOVE_ENABLE_ENET
#	define LOVE_ENABLE_EVENT
#	define LOVE_ENABLE_EVENT_SDL
#	define LOVE_ENABLE_FILESYSTEM
#	define LOVE_ENABLE_FILESYSTEM_PHYSFS
#	define LOVE_ENABLE_FONT
#	define LOVE_ENABLE_FONT_FREETYPE
#	define LOVE_ENABLE_GRAPHICS
#	define LOVE_ENABLE_GRAPHICS_OPENGL
#	define LOVE_ENABLE_IMAGE
#	define LOVE_ENABLE_IMAGE_MAGPIE
#	define LOVE_ENABLE_JOYSTICK
#	define LOVE_ENABLE_JOYSTICK_SDL
#	define LOVE_ENABLE_KEYBOARD
#	define LOVE_ENABLE_KEYBOARD_SDL
#	define LOVE_ENABLE_LOVE
#	define LOVE_ENABLE_LUASOCKET
#	define LOVE_ENABLE_LUAUTF8
#	define LOVE_ENABLE_MATH
#	define LOVE_ENABLE_MOUSE
#	define LOVE_ENABLE_MOUSE_SDL
#	define LOVE_ENABLE_NOISE1234
#	define LOVE_ENABLE_PHYSICS
#	define LOVE_ENABLE_PHYSICS_BOX2D
#	define LOVE_ENABLE_SOUND
#	define LOVE_ENABLE_SOUND_LULLABY
#	define LOVE_ENABLE_SYSTEM
#	define LOVE_ENABLE_SYSTEM_SDL
#	define LOVE_ENABLE_THREAD
#	define LOVE_ENABLE_THREAD_SDL
#	define LOVE_ENABLE_TIMER
#	define LOVE_ENABLE_TIMER_SDL
#	define LOVE_ENABLE_TOUCH
#	define LOVE_ENABLE_TOUCH_SDL
#	define LOVE_ENABLE_UTF8
#	define LOVE_ENABLE_VIDEO
#	define LOVE_ENABLE_VIDEO_THEORA
#	define LOVE_ENABLE_WINDOW
#	define LOVE_ENABLE_WINDOW_SDL
#	define LOVE_ENABLE_WUFF
#endif

// Check we have a sane configuration
#if !defined(LOVE_WINDOWS) && !defined(LOVE_LINUX) && !defined(LOVE_IOS) && !defined(LOVE_MACOSX)
#	error Could not detect target platform
#endif
#if !defined(LOVE_LITTLE_ENDIAN) && !defined(LOVE_BIG_ENDIAN)
#	error Could not detect endianness
#endif

#endif // LOVE_CONFIG_H
