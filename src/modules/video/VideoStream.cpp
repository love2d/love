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

#include "VideoStream.h"

using love::thread::Lock;

namespace love
{
namespace video
{

love::Type VideoStream::type("VideoStream", &Stream::type);

void VideoStream::setSync(VideoStream::FrameSync *frameSync)
{
	this->frameSync = frameSync;
}

VideoStream::FrameSync *VideoStream::getSync() const
{
	return frameSync;
}

void VideoStream::play()
{
	frameSync->play();
}

void VideoStream::pause()
{
	frameSync->pause();
}

void VideoStream::seek(double offset)
{
	frameSync->seek(offset);
}

double VideoStream::tell() const
{
	return frameSync->tell();
}

bool VideoStream::isPlaying() const
{
	return frameSync->isPlaying();
}

VideoStream::Frame::Frame()
	: yplane(nullptr)
	, cbplane(nullptr)
	, crplane(nullptr)
{
}

VideoStream::Frame::~Frame()
{
	delete[] yplane;
	delete[] cbplane;
	delete[] crplane;
}

void VideoStream::FrameSync::copyState(const VideoStream::FrameSync *other)
{
	seek(other->tell());
	if (other->isPlaying())
		play();
	else
		pause();
}

double VideoStream::FrameSync::tell() const
{
	return getPosition();
}

VideoStream::DeltaSync::DeltaSync()
	: playing(false)
	, position(0)
	, speed(1)
{
}

VideoStream::DeltaSync::~DeltaSync()
{
}

double VideoStream::DeltaSync::getPosition() const
{
	return position;
}

void VideoStream::DeltaSync::update(double dt)
{
	Lock l(mutex);
	if (playing)
		position += dt*speed;
}

void VideoStream::DeltaSync::play()
{
	playing = true;
}

void VideoStream::DeltaSync::pause()
{
	playing = false;
}

void VideoStream::DeltaSync::seek(double time)
{
	Lock l(mutex);
	position = time;
}

bool VideoStream::DeltaSync::isPlaying() const
{
	return playing;
}

VideoStream::SourceSync::SourceSync(love::audio::Source *source)
	: source(source)
{
}

double VideoStream::SourceSync::getPosition() const
{
	return source->tell(love::audio::Source::UNIT_SECONDS);
}

void VideoStream::SourceSync::play()
{
	source->play();
}

void VideoStream::SourceSync::pause()
{
	source->pause();
}

void VideoStream::SourceSync::seek(double time)
{
	source->seek(time, love::audio::Source::UNIT_SECONDS);
}

bool VideoStream::SourceSync::isPlaying() const
{
	return source->isPlaying();
}

} // video
} // love
