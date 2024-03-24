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

#ifndef LOVE_SOUND_LULLABY_MP3_DECODER_H
#define LOVE_SOUND_LULLABY_MP3_DECODER_H

// LOVE
#include "common/Stream.h"
#include "sound/Decoder.h"

// dr_mp3
#include "dr/dr_mp3.h"

#include <vector>

namespace love
{
namespace sound
{
namespace lullaby
{

class MP3Decoder: public love::sound::Decoder
{
public:

	MP3Decoder(Stream *stream, int bufsize);
	virtual ~MP3Decoder();

	love::sound::Decoder *clone() override;
	int decode() override;
	bool seek(double s) override;
	bool rewind() override;
	bool isSeekable() override;
	int getChannelCount() const override;
	int getBitDepth() const override;
	double getDuration() override;

private:
	static size_t onRead(void *pUserData, void *pBufferOut, size_t bytesToRead);
	static drmp3_bool32 onSeek(void *pUserData, int offset, drmp3_seek_origin origin);

	// MP3 handle
	drmp3 mp3;
	// Used for fast seeking
	std::vector<drmp3_seek_point> seekTable;
	// Position of first MP3 frame found
	int64 offset;

	double duration;
}; // MP3Decoder

} // lullaby
} // sound
} // love

#endif
