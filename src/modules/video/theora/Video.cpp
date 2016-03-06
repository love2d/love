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

// STL
#include <vector>

// LOVE
#include "Video.h"
#include "common/delay.h"
#include "timer/Timer.h"

namespace love
{
namespace video
{
namespace theora
{

Video::Video()
{
	workerThread = new Worker();
	workerThread->start();
}

Video::~Video()
{
	delete workerThread;
}

VideoStream *Video::newVideoStream(love::filesystem::File *file)
{
	VideoStream *stream = new VideoStream(file);
	workerThread->addStream(stream);
	return stream;
}

const char *Video::getName() const
{
	return "love.video.theora";
}

Worker::Worker()
	: stopping(false)
{
	threadName = "VideoWorker";
}

Worker::~Worker()
{
	stop();
}

void Worker::addStream(VideoStream *stream)
{
	love::thread::Lock l(mutex);
	streams.push_back(stream);
	cond->broadcast();
}

void Worker::stop()
{
	{
		love::thread::Lock l(mutex);
		stopping = true;
		cond->broadcast();
	}

	owner->wait();
}

void Worker::threadFunction()
{
	double lastFrame = love::timer::Timer::getTime();

	while (true)
	{
		love::sleep(2);

		love::thread::Lock l(mutex);

		while (!stopping && streams.empty())
		{
			cond->wait(mutex);
			lastFrame = love::timer::Timer::getTime();
		}

		if (stopping)
			return;

		double curFrame = love::timer::Timer::getTime();
		double dt = curFrame-lastFrame;
		lastFrame = curFrame;

		for (auto it = streams.begin(); it != streams.end(); ++it)
		{
			VideoStream *stream = *it;
			if (stream->getReferenceCount() == 1)
			{
				// We're the only ones left
				streams.erase(it);
				break;
			}

			stream->threadedFillBackBuffer(dt);
		}
	}
}

} // theora
} // video
} // love
