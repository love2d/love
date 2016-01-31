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

#ifndef LOVE_SOUND_LULLABY_FLAC_DECODER_H
#define LOVE_SOUND_LULLABY_FLAC_DECODER_H

#if 0

// LOVE
#include "common/Data.h"
#include "Decoder.h"

#include <FLAC++/decoder.h>
#include <string.h>

namespace love
{
namespace sound
{
namespace lullaby
{

class FLACDecoder : public Decoder, public FLAC::Decoder::Stream
{
public:
	FLACDecoder(Data *data, const std::string &ext, int bufferSize);
	~FLACDecoder();

	static bool accepts(const std::string &ext);
	love::sound::Decoder *clone();
	int decode();
	bool seek(float s);
	bool rewind();
	bool isSeekable();
	int getChannels() const;
	int getBitDepth() const;
	int getSampleRate() const;
	double getDuration();

	//needed for FLAC
	FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *bytes);
	FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 offset);
	FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64 *offset);
	FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *length);
	bool eof_callback();
	FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
	void metadata_callback(const FLAC__StreamMetadata *metadata);
	void error_callback(FLAC__StreamDecoderErrorStatus status);

private:
	int pos;
}; // Decoder

} // lullaby
} // sound
} // love

#endif // 0

#endif // LOVE_SOUND_LULLABY_FLAC_DECODER_H
