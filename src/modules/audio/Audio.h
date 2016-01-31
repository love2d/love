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
 * 1. The origin of this software must not be misrepresented = 0; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_AUDIO_AUDIO_H
#define LOVE_AUDIO_AUDIO_H

#include "common/Module.h"
#include "common/StringMap.h"
#include "Source.h"

namespace love
{

namespace sound
{

class Decoder;
class SoundData;

} // sound

namespace audio
{

/**
 * The Audio module is responsible for playing back raw sound samples.
 **/
class Audio : public Module
{
public:

	/**
	 * Attenuation by distance.
	 */
	enum DistanceModel
	{
		DISTANCE_NONE,
		DISTANCE_INVERSE,
		DISTANCE_INVERSE_CLAMPED,
		DISTANCE_LINEAR,
		DISTANCE_LINEAR_CLAMPED,
		DISTANCE_EXPONENT,
		DISTANCE_EXPONENT_CLAMPED,
		DISTANCE_MAX_ENUM
	};

	static bool getConstant(const char *in, DistanceModel &out);
	static bool getConstant(DistanceModel in, const char  *&out);

	virtual ~Audio() {}

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_AUDIO; }

	virtual Source *newSource(love::sound::Decoder *decoder) = 0;
	virtual Source *newSource(love::sound::SoundData *soundData) = 0;

	/**
	 * Gets the current number of simultaneous playing sources.
	 * @return The current number of simultaneous playing sources.
	 **/
	virtual int getSourceCount() const = 0;

	/**
	 * Gets the maximum supported number of simultaneous playing sources.
	 * @return The maximum supported number of simultaneous playing sources.
	 **/
	virtual int getMaxSources() const = 0;

	/**
	 * Play the specified Source.
	 * @param source The Source to play.
	 **/
	virtual bool play(Source *source) = 0;

	/**
	 * Stops playback on the specified source.
	 * @param source The source on which to stop the playback.
	 **/
	virtual void stop(Source *source) = 0;

	/**
	 * Stops all playing audio.
	 **/
	virtual void stop() = 0;

	/**
	 * Pauses playback on the specified source.
	 * @param source The source on which to pause the playback.
	 **/
	virtual void pause(Source *source) = 0;

	/**
	 * Pauses all audio.
	 **/
	virtual void pause() = 0;

	/**
	 * Resumes playback on the specified source.
	 * @param source The source on which to resume the playback.
	 **/
	virtual void resume(Source *source) = 0;

	/**
	 * Resumes all audio.
	 **/
	virtual void resume() = 0;

	/**
	 * Rewinds the specified source. Whatever is playing on this
	 * source gets rewound to the start.
	 * @param source The source to rewind.
	 **/
	virtual void rewind(Source *source) = 0;

	/**
	 * Rewinds all playing audio.
	 **/
	virtual void rewind() = 0;

	/**
	 * Sets the master volume, where 0.0f is min (off) and 1.0f is max.
	 * @param volume The new master volume.
	 **/
	virtual void setVolume(float volume) = 0;

	/**
	 * Gets the master volume.
	 * @return The current master volume.
	 **/
	virtual float getVolume() const = 0;

	/**
	 * Gets the position of the listener.
	 * @param v A float array of size 3 containing (x,y,z) in that order.
	 **/
	virtual void getPosition(float *v) const = 0;

	/**
	 * Sets the position of the listener.
	 * @param v A float array of size 3 containing [x,y,z] in that order.
	 **/
	virtual void setPosition(float *v) = 0;

	/**
	 * Gets the orientation of the listener.
	 * @param v A float array of size 6 containing [x,y,z] for the forward
	 * vector, followed by [x,y,z] for the up vector.
	 **/
	virtual void getOrientation(float *v) const = 0;

	/**
	 * Sets the orientation of the listener.
	 * @param v A float array of size 6 containing [x,y,z] for the forward
	 * vector, followed by [x,y,z] for the up vector.
	 **/
	virtual void setOrientation(float *v) = 0;

	/**
	 * Gets the velocity of the listener.
	 * @param v A float array of size 3 containing [x,y,z] in that order.
	 **/
	virtual void getVelocity(float *v) const = 0;

	/**
	 * Sets the velocity of the listener.
	 * @param v A float array of size 3 containing [x,y,z] in that order.
	 **/
	virtual void setVelocity(float *v) = 0;

	virtual void setDopplerScale(float scale) = 0;
	virtual float getDopplerScale() const = 0;

	/**
	 * Begins recording audio input from the microphone.
	 **/
	virtual void record() = 0;

	/**
	 * Gets a section of recorded audio.
	 * Per OpenAL, the measurement begins from the start of the
	 * audio data in memory, which is after the last time this function
	 * was called. If this function has not been called yet this recording
	 * session, it just grabs from the beginning.
	 * @return All the recorded SoundData thus far.
	 **/
	virtual love::sound::SoundData *getRecordedData() = 0;

	/**
	 * Stops recording and, if passed true, returns all the recorded audio
	 * not already gotten by getRecordedData.
	 * @param returnData Whether to return recorded audio.
	 * @return if returnData, all the recorded audio yet to be gotten,
	 * otherwise NULL.
	 **/
	virtual love::sound::SoundData *stopRecording(bool returnData) = 0;

	/**
	 * Checks whether LOVE is able to record audio input.
	 * @return hasMic Whether LOVE has a microphone enabled.
	 **/
	virtual bool canRecord() = 0;

	/**
	 * Gets the distance model used for attenuation.
	 * @return Distance model.
	 */
	virtual DistanceModel getDistanceModel() const = 0;

	/**
	 * Sets the distance model used for attenuation.
	 * @param distanceModel Distance model.
	 */
	virtual void setDistanceModel(DistanceModel distanceModel) = 0;

private:

	static StringMap<DistanceModel, DISTANCE_MAX_ENUM>::Entry distanceModelEntries[];
	static StringMap<DistanceModel, DISTANCE_MAX_ENUM> distanceModels;
}; // Audio

} // audio
} // love

#endif // LOVE_AUDIO_AUDIO_H
