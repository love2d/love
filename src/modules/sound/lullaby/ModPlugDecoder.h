/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#ifndef LOVE_SOUND_LULLABY_MODPLUG_DECODER_H
#define LOVE_SOUND_LULLABY_MODPLUG_DECODER_H

#include "common/config.h"

#ifndef LOVE_NO_MODPLUG

// LOVE
#include "common/Stream.h"
#include "sound/Decoder.h"

// libmodplug
#if defined(LOVE_ANDROID) || defined(LOVE_IOS)
#include <modplug.h>
#else
#include <libmodplug/modplug.h>
#endif

namespace love
{
namespace sound
{
namespace lullaby
{

class ModPlugDecoder : public Decoder
{
public:

	ModPlugDecoder(Stream *stream, int bufferSize);
	virtual ~ModPlugDecoder();

	love::sound::Decoder *clone() override;
	int decode() override;
	bool seek(double s) override;
	bool rewind() override;
	bool isSeekable() override;
	int getChannelCount() const override;
	int getBitDepth() const override;
	double getDuration() override;

private:

	StrongRef<Data> data;

	ModPlugFile *plug;
	ModPlug_Settings settings;

	double duration;

}; // ModPlugDecoder

} // lullaby
} // sound
} // love

#endif // LOVE_NO_MODPLUG

#endif // LOVE_SOUND_LULLABY_MODPLUG_DECODER_H
