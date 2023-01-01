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

#ifndef LOVE_AUDIO_AUDIO_H
#define LOVE_AUDIO_AUDIO_H

// STL
#include <vector>

// LOVE
#include "common/Module.h"
#include "common/StringMap.h"
#include "Source.h"
#include "Effect.h"
#include "RecordingDevice.h"

namespace love
{

namespace sound
{

class Decoder;
class SoundData;

} // sound

namespace audio
{

/*
 * In some platforms (notably Android), recording from mic
 * requires user permission. This function sets whetever to
 * request the permission later or not.
 */
void setRequestRecordingPermission(bool rec);

/*
 * Gets whetever recording permission will be requested.
 */
bool getRequestRecordingPermission();

/*
 * Gets whetever recording permission is granted.
 */
bool hasRecordingPermission();

/*
 * Request recording permission. This is blocking function.
 */
void requestRecordingPermission();

/*
 * In case recording permission is not granted, this
 * function shows the dialog about the recording permission.
 */
void showRecordingPermissionMissingDialog();

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
	static std::vector<std::string> getConstants(DistanceModel);

	virtual ~Audio() {}

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_AUDIO; }

	virtual Source *newSource(love::sound::Decoder *decoder) = 0;
	virtual Source *newSource(love::sound::SoundData *soundData) = 0;
	virtual Source *newSource(int sampleRate, int bitDepth, int channels, int buffers) = 0;

	/**
	 * Gets the current number of simultaneous playing sources.
	 * @return The current number of simultaneous playing sources.
	 **/
	virtual int getActiveSourceCount() const = 0;

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
	 * Play the specified Sources.
	 * @param sources The Sources to play.
	 **/
	virtual bool play(const std::vector<Source*> &sources) = 0;

	/**
	 * Stops playback on the specified source.
	 * @param source The source on which to stop the playback.
	 **/
	virtual void stop(Source *source) = 0;

	/**
	 * Stops playback on the specified sources.
	 * @param sources The sources on which to stop the playback.
	 **/
	virtual void stop(const std::vector<Source*> &sources) = 0;

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
	 * Pauses playback on the specified sources.
	 * @param sources The sources on which to pause the playback.
	 **/
	virtual void pause(const std::vector<Source*> &sources) = 0;

	/**
	 * Pauses all audio.
	 **/
	virtual std::vector<Source*> pause() = 0;

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
	//virtual void setMeter(float scale) = 0;
	//virtual float getMeter() const = 0;
	/**
	 * @return Reference to a vector of pointers to recording devices. May be empty.
	 **/
	virtual const std::vector<RecordingDevice*> &getRecordingDevices() = 0;

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

	/**
	 * Sets scene EFX effect.
	 * @param name Effect name to use.
	 * @param fxparams Effect description table.
	 * @return true if successful, false otherwise.
	 */
	virtual bool setEffect(const char *name, std::map<Effect::Parameter, float> &params) = 0;

	/**
	 * Removes scene EFX effect.
	 * @param name Effect name to clear.
	 * @return true if successful, false otherwise.
	 */
	virtual bool unsetEffect(const char *name) = 0;

	/**
	 * Gets scene EFX effect.
	 * @param name Effect name to get data from.
	 * @param fxparams Effect description table.
	 * @return true if effect was present, false otherwise.
	 */
	virtual bool getEffect(const char *name, std::map<Effect::Parameter, float> &params) = 0;

	/**
	 * Gets list of EFX effect names.
	 * @param list List of EFX names to fill.
	 * @return true if effect was present, false otherwise.
	 */
	virtual bool getActiveEffects(std::vector<std::string> &list) const = 0;

	/**
	 * Gets maximum number of scene EFX effects.
	 * @return number of effects.
	 */
	virtual int getMaxSceneEffects() const = 0;

	/**
	 * Gets maximum number of source EFX effects.
	 * @return number of effects.
	 */
	virtual int getMaxSourceEffects() const = 0;

	/**
	 * Gets EFX (or analog) availability.
	 * @return true if supported.
	 */
	virtual bool isEFXsupported() const = 0;

	/**
	 * Sets whether audio from other apps mixes with love.audio or is muted,
	 * on supported platforms.
	 **/
	static bool setMixWithSystem(bool mix);

	/**
	 * Pause/resume audio context
	 */
	virtual void pauseContext() = 0;
	virtual void resumeContext() = 0;

private:

	static StringMap<DistanceModel, DISTANCE_MAX_ENUM>::Entry distanceModelEntries[];
	static StringMap<DistanceModel, DISTANCE_MAX_ENUM> distanceModels;
}; // Audio

} // audio
} // love

#endif // LOVE_AUDIO_AUDIO_H
