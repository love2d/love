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

#ifndef LOVE_AUDIO_SOURCE_H
#define LOVE_AUDIO_SOURCE_H

// LOVE
#include "common/Object.h"
#include "common/StringMap.h"
#include "Filter.h"

#include <vector>

namespace love
{
namespace audio
{

class Source : public Object
{
public:

	static love::Type type;

	enum Type
	{
		TYPE_STATIC,
		TYPE_STREAM,
		TYPE_QUEUE,
		TYPE_MAX_ENUM
	};

	enum Unit
	{
		UNIT_SECONDS,
		UNIT_SAMPLES,
		UNIT_MAX_ENUM
	};

	Source(Type type);
	virtual ~Source();

	virtual Source *clone() = 0;

	virtual bool play() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual bool isPlaying() const = 0;
	virtual bool isFinished() const = 0;
	virtual bool update() = 0;

	virtual void setPitch(float pitch) = 0;
	virtual float getPitch() const = 0;

	virtual void setVolume(float volume) = 0;
	virtual float getVolume() const = 0;

	virtual void seek(double offset, Unit unit) = 0;
	virtual double tell(Unit unit) = 0;
	virtual double getDuration(Unit unit) = 0;

	// all float * v must be of size 3
	virtual void setPosition(float *v) = 0;
	virtual void getPosition(float *v) const = 0;
	virtual void setVelocity(float *v) = 0;
	virtual void getVelocity(float *v) const = 0;
	virtual void setDirection(float *v) = 0;
	virtual void getDirection(float *v) const = 0;

	virtual void setCone(float innerAngle, float outerAngle, float outerVolume, float outerHighGain) = 0;
	virtual void getCone(float &innerAngle, float &outerAngle, float &outerVolume, float &outerHighGain) const = 0;

	virtual void setRelative(bool enable) = 0;
	virtual bool isRelative() const = 0;

	virtual void setLooping(bool looping) = 0;
	virtual bool isLooping() const = 0;

	virtual void setMinVolume(float volume) = 0;
	virtual float getMinVolume() const = 0;
	virtual void setMaxVolume(float volume) = 0;
	virtual float getMaxVolume() const = 0;

	virtual void setReferenceDistance(float distance) = 0;
	virtual float getReferenceDistance() const = 0;
	virtual void setRolloffFactor(float factor) = 0;
	virtual float getRolloffFactor() const = 0;
	virtual void setMaxDistance(float distance) = 0;
	virtual float getMaxDistance() const = 0;
	virtual void setAirAbsorptionFactor(float factor) = 0;
	virtual float getAirAbsorptionFactor() const = 0;

	virtual int getChannelCount() const = 0;

	virtual bool setFilter(const std::map<Filter::Parameter, float> &params) = 0;
	virtual bool setFilter() = 0;
	virtual bool getFilter(std::map<Filter::Parameter, float> &params) = 0;

	virtual bool setEffect(const char *effect) = 0;
	virtual bool setEffect(const char *effect, const std::map<Filter::Parameter, float> &params) = 0;
	virtual bool unsetEffect(const char *effect) = 0;
	virtual bool getEffect(const char *effect, std::map<Filter::Parameter, float> &params) = 0;
	virtual bool getActiveEffects(std::vector<std::string> &list) const = 0;

	virtual int getFreeBufferCount() const = 0;
	virtual bool queue(void *data, size_t length, int dataSampleRate, int dataBitDepth, int dataChannels) = 0;

	virtual Type getType() const;

	static bool getConstant(const char *in, Type &out);
	static bool getConstant(Type in, const char  *&out);
	static std::vector<std::string> getConstants(Type);
	static bool getConstant(const char *in, Unit &out);
	static bool getConstant(Unit in, const char  *&out);
	static std::vector<std::string> getConstants(Unit);

protected:

	Type sourceType;

private:

	static StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<Type, TYPE_MAX_ENUM> types;
	static StringMap<Unit, UNIT_MAX_ENUM>::Entry unitEntries[];
	static StringMap<Unit, UNIT_MAX_ENUM> units;

}; // Source

} // audio
} // love

#endif // LOVE_AUDIO_SOURCE_H
