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

#ifndef LOVE_AUDIO_OPENAL_AUDIO_H
#define LOVE_AUDIO_OPENAL_AUDIO_H

// STD
#include <queue>
#include <map>
#include <cmath>

// LOVE
#include "audio/Audio.h"
#include "common/config.h"
#include "sound/SoundData.h"

#include "Source.h"
#include "Pool.h"
#include "thread/threads.h"

// OpenAL
#ifdef LOVE_APPLE_USE_FRAMEWORKS // Frameworks have different include paths.
#ifdef LOVE_IOS
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <OpenAL-Soft/alc.h>
#include <OpenAL-Soft/al.h>
#endif
#else
#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>
#endif

namespace love
{
namespace audio
{
namespace openal
{

class Audio : public love::audio::Audio
{
public:

	Audio();
	~Audio();

	// Implements Module.
	const char *getName() const;

	// Implements Audio.
	love::audio::Source *newSource(love::sound::Decoder *decoder);
	love::audio::Source *newSource(love::sound::SoundData *soundData);
	int getSourceCount() const;
	int getMaxSources() const;
	bool play(love::audio::Source *source);
	void stop(love::audio::Source *source);
	void stop();
	void pause(love::audio::Source *source);
	void pause();
	void resume(love::audio::Source *source);
	void resume();
	void rewind(love::audio::Source *source);
	void rewind();
	void setVolume(float volume);
	float getVolume() const;

	void getPosition(float *v) const;
	void setPosition(float *v);
	void getOrientation(float *v) const;
	void setOrientation(float *v);
	void getVelocity(float *v) const;
	void setVelocity(float *v);

	void setDopplerScale(float scale);
	float getDopplerScale() const;

	void record();
	love::sound::SoundData *getRecordedData();
	love::sound::SoundData *stopRecording(bool returnData);
	bool canRecord();

	DistanceModel getDistanceModel() const;
	void setDistanceModel(DistanceModel distanceModel);

private:

	// The OpenAL device.
	ALCdevice *device;

	// The OpenAL capture device (microphone).
	ALCdevice *capture;

	// The OpenAL context.
	ALCcontext *context;

	// The Pool.
	Pool *pool;

	class PoolThread: public thread::Threadable
	{
	protected:
		Pool *pool;

		// Set this to true when the thread should finish.
		// Main thread will write to this value, and PoolThread
		// will read from it.
		volatile bool finish;

		// finish lock
		love::thread::MutexRef mutex;

	public:
		PoolThread(Pool *pool);
		virtual ~PoolThread();
		void setFinish();
		void threadFunction();
	};

	PoolThread *poolThread;

	DistanceModel distanceModel;

}; // Audio

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_AUDIO_H
