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

#ifndef LOVE_AUDIO_OPENAL_AUDIO_H
#define LOVE_AUDIO_OPENAL_AUDIO_H

// STD
#include <queue>
#include <map>
#include <vector>
#include <stack>
#include <cmath>

// LOVE
#include "audio/Audio.h"
#include "audio/RecordingDevice.h"
#include "audio/Filter.h"
#include "common/config.h"
#include "sound/SoundData.h"

#include "Source.h"
#include "Effect.h"
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

	/**
	 * Gets the OpenAL format identifier based on number of
	 * channels and bits.
	 * @param channels.
	 * @param bitDepth Either 8-bit samples, or 16-bit samples.
	 * @return One of AL_FORMAT_*, or AL_NONE if unsupported format.
	 **/
	static ALenum getFormat(int bitDepth, int channels);

	// Implements Module.
	const char *getName() const;

	// Implements Audio.
	love::audio::Source *newSource(love::sound::Decoder *decoder);
	love::audio::Source *newSource(love::sound::SoundData *soundData);
	love::audio::Source *newSource(int sampleRate, int bitDepth, int channels, int buffers);
	int getActiveSourceCount() const;
	int getMaxSources() const;
	bool play(love::audio::Source *source);
	bool play(const std::vector<love::audio::Source*> &sources);
	void stop(love::audio::Source *source);
	void stop(const std::vector<love::audio::Source*> &sources);
	void stop();
	void pause(love::audio::Source *source);
	void pause(const std::vector<love::audio::Source*> &sources);
	std::vector<love::audio::Source*> pause();
	void pauseContext();
	void resumeContext();
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
	//void setMeter(float scale);
	//float getMeter() const;

	const std::vector<love::audio::RecordingDevice*> &getRecordingDevices();

	DistanceModel getDistanceModel() const;
	void setDistanceModel(DistanceModel distanceModel);

	bool setEffect(const char *name, std::map<Effect::Parameter, float> &params);
	bool unsetEffect(const char *name);
	bool getEffect(const char *name, std::map<Effect::Parameter, float> &params);
	bool getActiveEffects(std::vector<std::string> &list) const;
	int getMaxSceneEffects() const;
	int getMaxSourceEffects() const;
	bool isEFXsupported() const;

	bool getEffectID(const char *name, ALuint &id);

private:
	void initializeEFX();
	// The OpenAL device.
	ALCdevice *device;

	// The OpenAL capture devices.
	std::vector<love::audio::RecordingDevice*> capture;

	// The OpenAL context.
	ALCcontext *context;

	// The OpenAL effects
	struct EffectMapStorage
	{
		Effect *effect;
		ALuint slot;
	};
	std::map<std::string, struct EffectMapStorage> effectmap;
	std::stack<ALuint> slotlist;
	int MAX_SCENE_EFFECTS = 64;
	int MAX_SOURCE_EFFECTS = 64;

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
	//float metersPerUnit = 1.0;

#ifdef LOVE_ANDROID
#	ifndef ALC_SOFT_pause_device
	typedef void (ALC_APIENTRY*LPALCDEVICEPAUSESOFT)(ALCdevice *device);
	typedef void (ALC_APIENTRY*LPALCDEVICERESUMESOFT)(ALCdevice *device);
#	endif
	LPALCDEVICEPAUSESOFT alcDevicePauseSOFT;
	LPALCDEVICERESUMESOFT alcDeviceResumeSOFT;
	std::vector<love::audio::Source*> pausedSources;
#endif
}; // Audio

#ifdef ALC_EXT_EFX
 // Effect objects
extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALISEFFECT alIsEffect;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTIV alEffectiv;
extern LPALEFFECTF alEffectf;
extern LPALEFFECTFV alEffectfv;
extern LPALGETEFFECTI alGetEffecti;
extern LPALGETEFFECTIV alGetEffectiv;
extern LPALGETEFFECTF alGetEffectf;
extern LPALGETEFFECTFV alGetEffectfv;

//Filter objects
extern LPALGENFILTERS alGenFilters;
extern LPALDELETEFILTERS alDeleteFilters;
extern LPALISFILTER alIsFilter;
extern LPALFILTERI alFilteri;
extern LPALFILTERIV alFilteriv;
extern LPALFILTERF alFilterf;
extern LPALFILTERFV alFilterfv;
extern LPALGETFILTERI alGetFilteri;
extern LPALGETFILTERIV alGetFilteriv;
extern LPALGETFILTERF alGetFilterf;
extern LPALGETFILTERFV alGetFilterfv;

// Auxiliary slot object
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
#endif

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_AUDIO_H
