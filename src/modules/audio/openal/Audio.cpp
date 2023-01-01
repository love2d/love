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

#include "Audio.h"
#include "common/delay.h"
#include "RecordingDevice.h"
#include "sound/Decoder.h"

#include <cstdlib>
#include <iostream>

#ifdef LOVE_IOS
#include "common/ios.h"
#endif

namespace love
{
namespace audio
{
namespace openal
{

Audio::PoolThread::PoolThread(Pool *pool)
	: pool(pool)
	, finish(false)
{
	threadName = "AudioPool";
}

Audio::PoolThread::~PoolThread()
{
}


void Audio::PoolThread::threadFunction()
{
	while (true)
	{
		{
			thread::Lock lock(mutex);
			if (finish)
			{
				return;
			}
		}

		pool->update();
		sleep(5);
	}
}

void Audio::PoolThread::setFinish()
{
	thread::Lock lock(mutex);
	finish = true;
}

ALenum Audio::getFormat(int bitDepth, int channels)
{
	if (bitDepth != 8 && bitDepth != 16)
		return AL_NONE;

	if (channels == 1)
		return bitDepth == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	else if (channels == 2)
		return bitDepth == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
#ifdef AL_EXT_MCFORMATS
	else if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
	{
		if (channels == 6)
			return bitDepth == 8 ? AL_FORMAT_51CHN8 : AL_FORMAT_51CHN16;
		else if (channels == 8)
			return bitDepth == 8 ? AL_FORMAT_71CHN8 : AL_FORMAT_71CHN16;
	}
#endif
	return AL_NONE;
}

Audio::Audio()
	: device(nullptr)
	, context(nullptr)
	, pool(nullptr)
	, poolThread(nullptr)
	, distanceModel(DISTANCE_INVERSE_CLAMPED)
{
	// Before opening new device, check if recording
	// is requested.
	if (getRequestRecordingPermission())
	{
		if (!hasRecordingPermission())
			// Request recording permission on some OSes.
			requestRecordingPermission();
	}

	{
#if defined(LOVE_LINUX)
		// Temporarly block signals, as the thread inherits this mask
		love::thread::ScopedDisableSignals disableSignals;
#endif

		// Passing null for default device.
		device = alcOpenDevice(nullptr);

		if (device == nullptr)
			throw love::Exception("Could not open device.");

#ifdef ALC_EXT_EFX
		ALint attribs[4] = { ALC_MAX_AUXILIARY_SENDS, MAX_SOURCE_EFFECTS, 0, 0 };
#else
		ALint *attribs = nullptr;
#endif

		context = alcCreateContext(device, attribs);

		if (context == nullptr)
			throw love::Exception("Could not create context.");

		if (!alcMakeContextCurrent(context) || alcGetError(device) != ALC_NO_ERROR)
			throw love::Exception("Could not make context current.");
	}

#ifdef ALC_EXT_EFX
	initializeEFX();

	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &MAX_SOURCE_EFFECTS);

	alGetError();
	if (alGenAuxiliaryEffectSlots)
	{
		for (int i = 0; i < MAX_SCENE_EFFECTS; i++)
		{
			ALuint slot;
			alGenAuxiliaryEffectSlots(1, &slot);
			if (alGetError() == AL_NO_ERROR)
				slotlist.push(slot);
			else
			{
				MAX_SCENE_EFFECTS = i;
				break;
			}
		}
	}
	else
		MAX_SCENE_EFFECTS = MAX_SOURCE_EFFECTS = 0;
#else
	MAX_SCENE_EFFECTS = MAX_SOURCE_EFFECTS = 0;
#endif

	try
	{
		pool = new Pool();
	}
	catch (love::Exception &)
	{
		for (auto c : capture)
			delete c;

#ifdef ALC_EXT_EFX
		if (alDeleteAuxiliaryEffectSlots)
		{
			while (!slotlist.empty())
			{
				alDeleteAuxiliaryEffectSlots(1, &slotlist.top());
				slotlist.pop();
			}
		}
#endif

		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		alcCloseDevice(device);
		throw;
	}

	poolThread = new PoolThread(pool);
	poolThread->start();
	
#ifdef LOVE_IOS
	love::ios::initAudioSessionInterruptionHandler();
#endif

#ifdef LOVE_ANDROID
	bool hasPauseDeviceExt = alcIsExtensionPresent(device, "ALC_SOFT_pause_device") == ALC_TRUE;
	alcDevicePauseSOFT = hasPauseDeviceExt
		? (LPALCDEVICEPAUSESOFT) alcGetProcAddress(device, "alcDevicePauseSOFT")
		: nullptr;
	alcDeviceResumeSOFT = hasPauseDeviceExt
		? (LPALCDEVICERESUMESOFT) alcGetProcAddress(device, "alcDeviceResumeSOFT")
		: nullptr;
#endif
}

Audio::~Audio()
{
#ifdef LOVE_IOS
	love::ios::destroyAudioSessionInterruptionHandler();
#endif
	poolThread->setFinish();
	poolThread->wait();

	delete poolThread;
	delete pool;

	for (auto c : capture)
		delete c;

#ifdef ALC_EXT_EFX
	for (auto e : effectmap)
	{
		delete e.second.effect;
		slotlist.push(e.second.slot);
	}

	if (alDeleteAuxiliaryEffectSlots)
	{
		while (!slotlist.empty())
		{
			alDeleteAuxiliaryEffectSlots(1, &slotlist.top());
			slotlist.pop();
		}
	}
#endif
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

const char *Audio::getName() const
{
	return "love.audio.openal";
}

love::audio::Source *Audio::newSource(love::sound::Decoder *decoder)
{
	return new Source(pool, decoder);
}

love::audio::Source *Audio::newSource(love::sound::SoundData *soundData)
{
	return new Source(pool, soundData);
}

love::audio::Source *Audio::newSource(int sampleRate, int bitDepth, int channels, int buffers)
{
	return new Source(pool, sampleRate, bitDepth, channels, buffers);
}

int Audio::getActiveSourceCount() const
{
	return pool->getActiveSourceCount();
}

int Audio::getMaxSources() const
{
	return pool->getMaxSources();
}

bool Audio::play(love::audio::Source *source)
{
	return source->play();
}

bool Audio::play(const std::vector<love::audio::Source*> &sources)
{
	return Source::play(sources);
}

void Audio::stop(love::audio::Source *source)
{
	source->stop();
}

void Audio::stop(const std::vector<love::audio::Source*> &sources)
{
	return Source::stop(sources);
}

void Audio::stop()
{
	return Source::stop(pool);
}

void Audio::pause(love::audio::Source *source)
{
	source->pause();
}

void Audio::pause(const std::vector<love::audio::Source*> &sources)
{
	return Source::pause(sources);
}

std::vector<love::audio::Source*> Audio::pause()
{
	return Source::pause(pool);
}

void Audio::pauseContext()
{
#ifdef LOVE_ANDROID
	if (alcDevicePauseSOFT)
		alcDevicePauseSOFT(device);
	else
	{
		// This is extremely rare case since we're using OpenAL-soft
		// in Android and the ALC_SOFT_pause_device has been supported
		// since 1.16
		for (auto &src: pausedSources)
			src->release();
		pausedSources = pause();
		for (auto &src: pausedSources)
			src->retain();
	}
#else
	alcMakeContextCurrent(nullptr);
#endif
}

void Audio::resumeContext()
{
#ifdef LOVE_ANDROID
	if (alcDeviceResumeSOFT)
		alcDeviceResumeSOFT(device);
	else
	{
		// Again, this is rare case
		play(pausedSources);
		for (auto &src: pausedSources)
			src->release();
		pausedSources.resize(0);
	}
#else
	if (context && alcGetCurrentContext() != context)
		alcMakeContextCurrent(context);
#endif
}

void Audio::setVolume(float volume)
{
	alListenerf(AL_GAIN, volume);
}

float Audio::getVolume() const
{
	ALfloat volume;
	alGetListenerf(AL_GAIN, &volume);
	return volume;
}

void Audio::getPosition(float *v) const
{
	alGetListenerfv(AL_POSITION, v);
}

void Audio::setPosition(float *v)
{
	alListenerfv(AL_POSITION, v);
}

void Audio::getOrientation(float *v) const
{
	alGetListenerfv(AL_ORIENTATION, v);
}

void Audio::setOrientation(float *v)
{
	alListenerfv(AL_ORIENTATION, v);
}

void Audio::getVelocity(float *v) const
{
	alGetListenerfv(AL_VELOCITY, v);
}

void Audio::setVelocity(float *v)
{
	alListenerfv(AL_VELOCITY, v);
}

void Audio::setDopplerScale(float scale)
{
	if (scale >= 0.0f)
		alDopplerFactor(scale);
}

float Audio::getDopplerScale() const
{
	return alGetFloat(AL_DOPPLER_FACTOR);
}
/*
void Audio::setMeter(float scale)
{
	if (scale >= 0.0f)
	{
		metersPerUnit = scale;
#ifdef ALC_EXT_EFX
		alListenerf(AL_METERS_PER_UNIT, scale);
#endif
	}
}

float Audio::getMeter() const
{
	return metersPerUnit;
}
*/
Audio::DistanceModel Audio::getDistanceModel() const
{
	return distanceModel;
}

void Audio::setDistanceModel(DistanceModel distanceModel)
{
	this->distanceModel = distanceModel;

	switch (distanceModel)
	{
	case DISTANCE_NONE:
		alDistanceModel(AL_NONE);
		break;

	case DISTANCE_INVERSE:
		alDistanceModel(AL_INVERSE_DISTANCE);
		break;

	case DISTANCE_INVERSE_CLAMPED:
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		break;

	case DISTANCE_LINEAR:
		alDistanceModel(AL_LINEAR_DISTANCE);
		break;

	case DISTANCE_LINEAR_CLAMPED:
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		break;

	case DISTANCE_EXPONENT:
		alDistanceModel(AL_EXPONENT_DISTANCE);
		break;

	case DISTANCE_EXPONENT_CLAMPED:
		alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
		break;

	default:
		break;
	}
}

const std::vector<love::audio::RecordingDevice*> &Audio::getRecordingDevices()
{
	std::vector<std::string> devnames;
	std::vector<love::audio::RecordingDevice*> devices;

	// If recording permission is not granted, inform user about it
	// and return empty list.
	if (!hasRecordingPermission() && getRequestRecordingPermission())
	{
		showRecordingPermissionMissingDialog();
		capture.clear();
		return capture;
	}

	std::string defaultname(alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));

	//no device name obtained from AL, fallback to reading from device
	if (defaultname.length() == 0)
	{
		//use some safe basic parameters - 8 kHz, 8 bits, 1 channel
		ALCdevice *defaultdevice = alcCaptureOpenDevice(NULL, 8000, AL_FORMAT_MONO8, 1024);
		if (alGetError() == AL_NO_ERROR)
		{
			defaultname = alcGetString(defaultdevice, ALC_CAPTURE_DEVICE_SPECIFIER);
			alcCaptureCloseDevice(defaultdevice);
		}
		else
		{
			//failed to open default recording device - bail, return empty list
			capture.clear();
			return capture;
		}
	}

	devnames.reserve(capture.size());
	devnames.push_back(defaultname);

	//find devices name list
	const ALCchar *devstr = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	size_t offset = 0;
	while (true)
	{
		if (devstr[offset] == '\0')
			break;
		std::string str((ALCchar*)&devstr[offset]);
		if (str != defaultname)
			devnames.push_back(str);
		offset += str.length() + 1;
	}

	devices.reserve(devnames.size());
	//build ordered list of devices
	for (int i = 0; i < (int) devnames.size(); i++)
	{
		devices.push_back(nullptr);
		auto d = devices.end() - 1;

		for (auto c : capture)
			if (devnames[i] == c->getName())
				*d = c;

		if (*d == nullptr)
			*d = new RecordingDevice(devnames[i].c_str());
		else
			(*d)->retain();
	}

	for (auto c : capture)
		c->release();
	capture.clear();
	capture.reserve(devices.size());

	//this needs to be executed in specific order
	for (unsigned int i = 0; i < devnames.size(); i++)
		capture.push_back(devices[i]);

	return capture;
}

bool Audio::setEffect(const char *name, std::map<Effect::Parameter, float> &params)
{
	Effect *effect;
	ALuint slot;

	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
	{
		//new effect needed but no more slots
		if (effectmap.size() >= (unsigned int)MAX_SCENE_EFFECTS)
			return false;

		effect = new Effect();
		slot = slotlist.top();
		slotlist.pop();

		effectmap[name] = {effect, slot};
	}
	else
	{
		effect = iter->second.effect;
		slot = iter->second.slot;
	}

	bool result = effect->setParams(params);

#ifdef ALC_EXT_EFX
	if (alAuxiliaryEffectSloti)
	{
		if (result)
		{
			auto iter = params.find(Effect::EFFECT_VOLUME);
			if (iter != params.end())
				alAuxiliaryEffectSlotf(slot, AL_EFFECTSLOT_GAIN, iter->second);
			alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect->getEffect());
		}
		else
			alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
		alGetError();
	}
#endif

	return result;
}

bool Audio::unsetEffect(const char *name)
{
	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
		return false;

	Effect *effect = iter->second.effect;
	ALuint slot = iter->second.slot;

#ifdef ALC_EXT_EFX
	if (alAuxiliaryEffectSloti)
		alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
#endif

	delete effect;
	effectmap.erase(iter);
	slotlist.push(slot);
	return true;
}

bool Audio::getEffect(const char *name, std::map<Effect::Parameter, float> &params)
{
	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
		return false;

	params = iter->second.effect->getParams();

	return true;
}

bool Audio::getActiveEffects(std::vector<std::string> &list) const
{
	if (effectmap.empty())
		return false;

	list.reserve(effectmap.size());
	for (auto i : effectmap)
		list.push_back(i.first);

	return true;
}

int Audio::getMaxSceneEffects() const
{
	return MAX_SCENE_EFFECTS;
}

int Audio::getMaxSourceEffects() const
{
	return MAX_SOURCE_EFFECTS;
}

bool Audio::isEFXsupported() const
{
#ifdef ALC_EXT_EFX
	return (alGenEffects != nullptr);
#else
	return false;
#endif
}

bool Audio::getEffectID(const char *name, ALuint &id)
{
	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
		return false;

	id = iter->second.slot;
	return true;
}

#ifdef ALC_EXT_EFX
LPALGENEFFECTS alGenEffects = nullptr;
LPALDELETEEFFECTS alDeleteEffects = nullptr;
LPALISEFFECT alIsEffect = nullptr;
LPALEFFECTI alEffecti = nullptr;
LPALEFFECTIV alEffectiv = nullptr;
LPALEFFECTF alEffectf = nullptr;
LPALEFFECTFV alEffectfv = nullptr;
LPALGETEFFECTI alGetEffecti = nullptr;
LPALGETEFFECTIV alGetEffectiv = nullptr;
LPALGETEFFECTF alGetEffectf = nullptr;
LPALGETEFFECTFV alGetEffectfv = nullptr;
LPALGENFILTERS alGenFilters = nullptr;
LPALDELETEFILTERS alDeleteFilters = nullptr;
LPALISFILTER alIsFilter = nullptr;
LPALFILTERI alFilteri = nullptr;
LPALFILTERIV alFilteriv = nullptr;
LPALFILTERF alFilterf = nullptr;
LPALFILTERFV alFilterfv = nullptr;
LPALGETFILTERI alGetFilteri = nullptr;
LPALGETFILTERIV alGetFilteriv = nullptr;
LPALGETFILTERF alGetFilterf = nullptr;
LPALGETFILTERFV alGetFilterfv = nullptr;
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;
#endif

void Audio::initializeEFX()
{
#ifdef ALC_EXT_EFX
	if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_FALSE)
		return;

	alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
	alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
	alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
	alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
	alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
	alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
	alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
	alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
	alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
	alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
	alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
	alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
	alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
	alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
	alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
	alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
	alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
	alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
	alGetFilteri = (LPALGETFILTERI)alGetProcAddress("alGetFilteri");
	alGetFilteriv = (LPALGETFILTERIV)alGetProcAddress("alGetFilteriv");
	alGetFilterf = (LPALGETFILTERF)alGetProcAddress("alGetFilterf");
	alGetFilterfv = (LPALGETFILTERFV)alGetProcAddress("alGetFilterfv");
	alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
	alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
	alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
	alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
	alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
	alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
	alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
	alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
	alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

	//failed to initialize functions, revert to nullptr
	if (!alGenEffects || !alDeleteEffects || !alIsEffect ||
		!alGenFilters || !alDeleteFilters || !alIsFilter ||
		!alGenAuxiliaryEffectSlots || !alDeleteAuxiliaryEffectSlots || !alIsAuxiliaryEffectSlot ||
		!alEffecti || !alEffectiv || !alEffectf || !alEffectfv ||
		!alGetEffecti || !alGetEffectiv || !alGetEffectf || !alGetEffectfv ||
		!alFilteri || !alFilteriv || !alFilterf || !alFilterfv ||
		!alGetFilteri || !alGetFilteriv || !alGetFilterf || !alGetFilterfv ||
		!alAuxiliaryEffectSloti || !alAuxiliaryEffectSlotiv || !alAuxiliaryEffectSlotf || !alAuxiliaryEffectSlotfv ||
		!alGetAuxiliaryEffectSloti || !alGetAuxiliaryEffectSlotiv || !alGetAuxiliaryEffectSlotf || !alGetAuxiliaryEffectSlotfv)
	{
		alGenEffects = nullptr; alDeleteEffects = nullptr; alIsEffect = nullptr;
		alEffecti = nullptr; alEffectiv = nullptr; alEffectf = nullptr; alEffectfv = nullptr;
		alGetEffecti = nullptr; alGetEffectiv = nullptr; alGetEffectf = nullptr; alGetEffectfv = nullptr;
		alGenFilters = nullptr; alDeleteFilters = nullptr; alIsFilter = nullptr;
		alFilteri = nullptr; alFilteriv = nullptr; alFilterf = nullptr; alFilterfv = nullptr;
		alGetFilteri = nullptr; alGetFilteriv = nullptr; alGetFilterf = nullptr; alGetFilterfv = nullptr;
		alGenAuxiliaryEffectSlots = nullptr; alDeleteAuxiliaryEffectSlots = nullptr; alIsAuxiliaryEffectSlot = nullptr;
		alAuxiliaryEffectSloti = nullptr; alAuxiliaryEffectSlotiv = nullptr;
		alAuxiliaryEffectSlotf = nullptr; alAuxiliaryEffectSlotfv = nullptr;
		alGetAuxiliaryEffectSloti = nullptr; alGetAuxiliaryEffectSlotiv = nullptr;
		alGetAuxiliaryEffectSlotf = nullptr; alGetAuxiliaryEffectSlotfv = nullptr;
	}

#endif
}

} // openal
} // audio
} // love
