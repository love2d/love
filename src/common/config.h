/**
* Copyright (c) 2006-2009 LOVE Development Team
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

// Warnings.
#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

// Build.
#define LOVE_BUILD_EXE 1
#define LOVE_BUILD_DLL 0

// DLL-stuff.
#ifdef LOVE_WINDOWS
#	define LOVE_EXPORT __declspec(dllexport)
#else
#	define LOVE_EXPORT
#endif

#endif // LOVE_CONFIG_H
