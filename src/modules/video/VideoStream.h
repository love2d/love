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

#ifndef LOVE_VIDEO_VIDEOSTREAM_H
#define LOVE_VIDEO_VIDEOSTREAM_H

// LOVE
#include "common/Stream.h"
#include "audio/Source.h"
#include "thread/threads.h"

namespace love
{
namespace video
{

class VideoStream : public Stream
{
public:

	static love::Type type;

	virtual ~VideoStream() {}

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	virtual const std::string &getFilename() const = 0;

	// Playback api
	virtual void play();
	virtual void pause();
	virtual void seek(double offset);
	virtual double tell() const;
	virtual bool isPlaying() const;

	class FrameSync;
	class DeltaSync;

	// The stream now owns the sync, do not reuse or free
	virtual void setSync(FrameSync *frameSync);
	virtual FrameSync *getSync() const;

	// Data structures
	struct Frame
	{
		Frame();
		~Frame();

		int yw, yh;
		unsigned char *yplane;

		int cw, ch;
		unsigned char *cbplane;
		unsigned char *crplane;
	};

	class FrameSync : public Object
	{
	public:
		virtual double getPosition() const = 0;
		virtual void update(double /*dt*/) {}
		virtual ~FrameSync() {}

		void copyState(const FrameSync *other);

		// Playback api
		virtual void play() = 0;
		virtual void pause() = 0;
		virtual void seek(double offset) = 0;
		virtual double tell() const;
		virtual bool isPlaying() const = 0;
	};

	class DeltaSync : public FrameSync
	{
	public:
		DeltaSync();
		~DeltaSync();

		virtual double getPosition() const override;
		virtual void update(double dt) override;

		virtual void play() override;
		virtual void pause() override;
		virtual void seek(double time) override;
		virtual bool isPlaying() const override;

	private:
		bool playing;
		double position;
		double speed;
		love::thread::MutexRef mutex;
	};

	class SourceSync : public FrameSync
	{
	public:
		SourceSync(love::audio::Source *source);

		virtual double getPosition() const override;
		virtual void play() override;
		virtual void pause() override;
		virtual void seek(double time) override;
		virtual bool isPlaying() const override;

	private:
		StrongRef<love::audio::Source> source;
	};

protected:
	StrongRef<FrameSync> frameSync;
};

} // video
} // love

#endif // LOVE_VIDEO_VIDEOSTREAM_H
