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

// STD
#include <string>

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

// Build.
#define LOVE_BUILD_EXE 1
#define LOVE_BUILD_DLL 0

// Version stuff.
const int LOVE_VERSION = 060;
const int LOVE_VERSION_COMPATIBILITY[] =  { 0 };
const std::string LOVE_VERSION_STR = "0.6.0";
const std::string LOVE_VERSION_CODENAME = "Jiggly Juice";
const std::string LOVE_VERSION_FULL_STR = std::string("LOVE ") + LOVE_VERSION_STR + std::string(" (") + LOVE_VERSION_CODENAME + std::string(")");

// DLL-stuff.
#ifdef LOVE_WINDOWS
#	ifndef DECLSPEC
#		define DECLSPEC __declspec(dllexport)
#	endif
#else
#	ifndef DECLSPEC
#		define DECLSPEC
#	endif
#endif

#endif // LOVE_CONFIG_H
