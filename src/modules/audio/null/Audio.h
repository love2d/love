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

#ifndef LOVE_AUDIO_NULL_AUDIO_H
#define LOVE_AUDIO_NULL_AUDIO_H

// LOVE
#include "audio/Audio.h"

#include "Source.h"

namespace love
{
namespace audio
{
namespace null
{

class Audio : public love::audio::Audio
{
public:

	Audio();
	virtual ~Audio();

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
	float volume;
	DistanceModel distanceModel;

}; // Audio

} // null
} // audio
} // love

#endif // LOVE_AUDIO_NULL_AUDIO_H
