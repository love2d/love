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

#ifndef LOVE_AUDIO_OPENAL_EFFECTS_H
#define LOVE_AUDIO_OPENAL_EFFECTS_H

#include "common/config.h"

// OpenAL
#ifdef LOVE_APPLE_USE_FRAMEWORKS // Frameworks have different include paths.
#ifdef LOVE_IOS
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <OpenAL-Soft/alc.h>
#include <OpenAL-Soft/al.h>
#include <OpenAL-Soft/alext.h>
#endif
#else
#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>
#endif

#include <vector>
#include <map>

#include "audio/Effect.h"
#include "Audio.h"

#ifndef AL_EFFECT_NULL
#define AL_EFFECT_NULL (0)
#endif

#ifndef AL_EFFECTSLOT_NULL
#define AL_EFFECTSLOT_NULL (0)
#endif

namespace love
{
namespace audio
{
namespace openal
{

class Effect : public love::audio::Effect
{
public:
	Effect();
	Effect(const Effect &s);
	virtual ~Effect();
	virtual Effect *clone();
	ALuint getEffect() const;
	virtual bool setParams(const std::map<Parameter, float> &params);
	virtual const std::map<Parameter, float> &getParams() const;

private:
	bool generateEffect();
	void deleteEffect();
	float getValue(Parameter in, float def) const;
	int getValue(Parameter in, int def) const;

	ALuint effect = AL_EFFECT_NULL;
	std::map<Parameter, float> params;
	//static std::map<Phoneme, ALint> phonemeMap;
};

} //openal
} //audio
} //love

#endif //LOVE_AUDIO_OPENAL_EFFECTS_H
