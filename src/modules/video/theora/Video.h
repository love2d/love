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

#ifndef LOVE_VIDEO_THEORA_VIDEO_H
#define LOVE_VIDEO_THEORA_VIDEO_H

// STL
#include <vector>

// LOVE
#include "filesystem/File.h"
#include "video/Video.h"
#include "thread/threads.h"
#include "VideoStream.h"

namespace love
{
namespace video
{
namespace theora
{

class Worker;

class Video : public love::video::Video
{
public:
	Video();
	virtual ~Video();

	// Implements Module
	virtual const char *getName() const;

	VideoStream *newVideoStream(love::filesystem::File* file);

private:
	Worker *workerThread;
}; // Video

class Worker : public love::thread::Threadable
{
public:
	Worker();
	virtual ~Worker();

	// Implements Threadable
	void threadFunction();

	void addStream(VideoStream *stream);
	// Frees itself!
	void stop();

private:

	std::vector<StrongRef<VideoStream>> streams;

	love::thread::MutexRef mutex;
	love::thread::ConditionalRef cond;

	bool stopping;
}; // Worker

} // theora
} // video
} // love

#endif // LOVE_VIDEO_THEORA_VIDEO_H
