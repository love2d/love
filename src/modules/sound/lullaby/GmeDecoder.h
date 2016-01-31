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

#ifndef LOVE_SOUND_LULLABY_GME_DECODER_H
#define LOVE_SOUND_LULLABY_GME_DECODER_H

#ifdef LOVE_SUPPORT_GME

// LOVE
#include "common/Data.h"
#include "Decoder.h"

#ifdef LOVE_APPLE_USE_FRAMEWORKS
#include <Game_Music_Emu/gme.h>
#else
#include <gme.h>
#endif

namespace love
{
namespace sound
{
namespace lullaby
{

class GmeDecoder : public Decoder
{
public:

	GmeDecoder(Data *data, const std::string &ext, int bufferSize);
	virtual ~GmeDecoder();

	static bool accepts(const std::string &ext);

	love::sound::Decoder *clone();
	int decode();
	bool seek(float s);
	bool rewind();
	bool isSeekable();
	int getChannels() const;
	int getBitDepth() const;
	double getDuration();

private:
	Music_Emu *emu;
	int num_tracks;
	int cur_track;
}; // Decoder

} // lullaby
} // sound
} // love

#endif // LOVE_SUPPORT_GME

#endif // LOVE_SOUND_LULLABY_GME_DECODER_H
