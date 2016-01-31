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

#include "Source.h"

namespace love
{
namespace audio
{
namespace null
{

Source::Source()
	: love::audio::Source(Source::TYPE_STATIC)
{
}

Source::~Source()
{
}

love::audio::Source *Source::clone()
{
	this->retain();
	return this;
}

bool Source::play()
{
	return false;
}

void Source::stop()
{
}

void Source::pause()
{
}

void Source::resume()
{
}

void Source::rewind()
{
}

bool Source::isStopped() const
{
	return true;
}

bool Source::isPaused() const
{
	return false;
}

bool Source::isFinished() const
{
	return true;
}

bool Source::update()
{
	return false;
}

void Source::setPitch(float pitch)
{
	this->pitch = pitch;
}

float Source::getPitch() const
{
	return pitch;
}

void Source::setVolume(float volume)
{
	this->volume = volume;
}

float Source::getVolume() const
{
	return volume;
}

void Source::seek(float, Source::Unit)
{
}

float Source::tell(Source::Unit)
{
	return 0.0f;
}

double Source::getDuration(Unit)
{
	return -1.0f;
}

void Source::setPosition(float *)
{
}

void Source::getPosition(float *) const
{
}

void Source::setVelocity(float *)
{
}

void Source::getVelocity(float *) const
{
}

void Source::setDirection(float *)
{
}

void Source::getDirection(float *) const
{
}

void Source::setCone(float innerAngle, float outerAngle, float outerVolume)
{
	coneInnerAngle = innerAngle;
	coneOuterAngle = outerAngle;
	coneOuterVolume = outerVolume;
}

void Source::getCone(float &innerAngle, float &outerAngle, float &outerVolume) const
{
	innerAngle = coneInnerAngle;
	outerAngle = coneOuterAngle;
	outerVolume = coneOuterVolume;
}

void Source::setRelative(bool enable)
{
	relative = enable;
}

bool Source::isRelative() const
{
	return relative;
}

void Source::setLooping(bool looping)
{
	this->looping = looping;
}

bool Source::isLooping() const
{
	return looping;
}

void Source::setMinVolume(float volume)
{
	this->minVolume = volume;
}

float Source::getMinVolume() const
{
	return this->minVolume;
}

void Source::setMaxVolume(float volume)
{
	this->maxVolume = volume;
}

float Source::getMaxVolume() const
{
	return this->maxVolume;
}

void Source::setReferenceDistance(float distance)
{
	this->referenceDistance = distance;
}

float Source::getReferenceDistance() const
{
	return this->referenceDistance;
}

void Source::setRolloffFactor(float factor)
{
	this->rolloffFactor = factor;
}

float Source::getRolloffFactor() const
{
	return this->rolloffFactor;
}

void Source::setMaxDistance(float distance)
{
	this->maxDistance = distance;
}

float Source::getMaxDistance() const
{
	return this->maxDistance;
}

int Source::getChannels() const
{
	return 2;
}

} // null
} // audio
} // love
