/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#ifndef LOVE_VIDEO_VIDEO_H
#define LOVE_VIDEO_VIDEO_H

// LOVE
#include "common/Module.h"
#include "common/Stream.h"
#include "filesystem/File.h"

#include "VideoStream.h"

namespace love
{
namespace video
{

class Video : public Module
{
public:
	virtual ~Video() {}

	// Implements Module
	virtual ModuleType getModuleType() const { return M_VIDEO; }

	/**
	 * Create a VideoStream representing video frames
	 **/
	virtual VideoStream *newVideoStream(love::filesystem::File *file) = 0;
}; // Video

} // video
} // love

#endif // LOVE_VIDEO_VIDEO_H
