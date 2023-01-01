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

#ifndef LOVE_SOUND_LULLABY_FLAC_DECODER_H
#define LOVE_SOUND_LULLABY_FLAC_DECODER_H

// LOVE
#include "common/Data.h"
#include "sound/Decoder.h"

#include "dr_flac/dr_flac.h"
#include <string.h>

namespace love
{
namespace sound
{
namespace lullaby
{

class FLACDecoder : public Decoder
{
public:
	FLACDecoder(Data *data, int bufferSize);
	~FLACDecoder();

	static bool accepts(const std::string &ext);
	love::sound::Decoder *clone();
	int decode();
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	int getSampleRate() const;
	double getDuration();

private:
	drflac *flac;
}; // Decoder

} // lullaby
} // sound
} // love

#endif // LOVE_SOUND_LULLABY_FLAC_DECODER_H
