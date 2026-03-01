/**
 * Copyright (c) 2006-2026 LOVE Development Team
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

#include "RecordingDevice.h"
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

	// Implements Audio.
	love::audio::Source *newSource(love::sound::Decoder *decoder) override;
	love::audio::Source *newSource(love::sound::SoundData *soundData) override;
	love::audio::Source *newSource(int sampleRate, int bitDepth, int channels, int buffers) override;
	int getActiveSourceCount() const override;
	int getMaxSources() const override;
	bool play(love::audio::Source *source) override;
	bool play(const std::vector<love::audio::Source*> &sources) override;
	void stop(love::audio::Source *source) override;
	void stop(const std::vector<love::audio::Source*> &sources) override;
	void stop() override;
	void pause(love::audio::Source *source) override;
	void pause(const std::vector<love::audio::Source*> &sources) override;
	std::vector<love::audio::Source*> pause() override;
	void setVolume(float volume) override;
	float getVolume() const override;

	void getPosition(float *v) const override;
	void setPosition(float *v) override;
	void getOrientation(float *v) const override;
	void setOrientation(float *v) override;
	void getVelocity(float *v) const override;
	void setVelocity(float *v) override;

	void setDopplerScale(float scale) override;
	float getDopplerScale() const override;
	//void setMeter(float scale);
	//float getMeter() const;

	const std::vector<love::audio::RecordingDevice*> &getRecordingDevices() override;

	DistanceModel getDistanceModel() const override;
	void setDistanceModel(DistanceModel distanceModel) override;

	bool setEffect(const char *, std::map<Effect::Parameter, float> &params) override;
	bool unsetEffect(const char *) override;
	bool getEffect(const char *, std::map<Effect::Parameter, float> &params) override;
	bool getActiveEffects(std::vector<std::string> &list) const override;
	int getMaxSceneEffects() const override;
	int getMaxSourceEffects() const override;
	bool isEFXsupported() const override;

	bool setOutputSpatialization(bool enable, const char *filter = nullptr) override;
	bool getOutputSpatialization(const char *&filter) const override;
	void getOutputSpatializationFilters(std::vector<std::string> &list) const override;

	void pauseContext() override;
	void resumeContext() override;

	std::string getPlaybackDevice() override;
	void getPlaybackDevices(std::vector<std::string> &list) override;

private:
	float volume;
	DistanceModel distanceModel;
	std::vector<love::audio::RecordingDevice*> capture;

}; // Audio

} // null
} // audio
} // love

#endif // LOVE_AUDIO_NULL_AUDIO_H
