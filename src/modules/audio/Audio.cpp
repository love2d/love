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
#include "common/config.h"

#if defined(LOVE_IOS)
#include "common/ios.h"
#elif defined(LOVE_ANDROID)
#include "common/android.h"
#endif

namespace love
{
namespace audio
{

static bool requestRecPermission = false;

void setRequestRecordingPermission(bool rec)
{
	requestRecPermission = rec;
}

bool getRequestRecordingPermission()
{
	return requestRecPermission;
}

bool hasRecordingPermission()
{
#if defined(LOVE_ANDROID)
	return love::android::hasRecordingPermission();
#else
	// Always available(?)
	return true;
#endif
}

void requestRecordingPermission()
{
#ifdef LOVE_ANDROID
	love::android::requestRecordingPermission();
#endif
}

void showRecordingPermissionMissingDialog()
{
#ifdef LOVE_ANDROID
	love::android::showRecordingPermissionMissingDialog();
#endif
}

bool Audio::setMixWithSystem(bool mix)
{
#ifdef LOVE_IOS
	return love::ios::setAudioMixWithOthers(mix);
#else
	LOVE_UNUSED(mix);
	return false;
#endif
}

StringMap<Audio::DistanceModel, Audio::DISTANCE_MAX_ENUM>::Entry Audio::distanceModelEntries[] =
{
	{"none", Audio::DISTANCE_NONE},
	{"inverse", Audio::DISTANCE_INVERSE},
	{"inverseclamped", Audio::DISTANCE_INVERSE_CLAMPED},
	{"linear", Audio::DISTANCE_LINEAR},
	{"linearclamped", Audio::DISTANCE_LINEAR_CLAMPED},
	{"exponent", Audio::DISTANCE_EXPONENT},
	{"exponentclamped", Audio::DISTANCE_EXPONENT_CLAMPED}
};

StringMap<Audio::DistanceModel, Audio::DISTANCE_MAX_ENUM> Audio::distanceModels(Audio::distanceModelEntries, sizeof(Audio::distanceModelEntries));

bool Audio::getConstant(const char *in, DistanceModel &out)
{
	return distanceModels.find(in, out);
}

bool Audio::getConstant(DistanceModel in, const char  *&out)
{
	return distanceModels.find(in, out);
}

std::vector<std::string> Audio::getConstants(DistanceModel)
{
	return distanceModels.getNames();
}

} // audio
} // love
