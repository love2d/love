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

#ifndef LOVE_SOUND_LULLABY_DECODER_H
#define LOVE_SOUND_LULLABY_DECODER_H

// LOVE
#include "sound/Decoder.h"
#include "filesystem/File.h"

#include <string>

namespace love
{
namespace sound
{
namespace lullaby
{

class Decoder : public love::sound::Decoder
{
public:

	Decoder(Data *data, const std::string &ext, int bufferSize);

	virtual ~Decoder();

	// Implement some of love::sound::Decoder, but allow subclasses
	// to override them.
	virtual void *getBuffer() const;
	virtual int getSize() const;
	virtual int getSampleRate() const;
	virtual bool isFinished();

protected:

	// The encoded data. This should be replaced with buffered file
	// reads in the future.
	StrongRef<Data> data;

	// File extension.
	std::string ext;

	// When the decoder decodes data incrementally, it writes
	// this many bytes at a time (at most).
	int bufferSize;

	// The desired frequency of the samples. 44100, 22050, or 11025.
	int sampleRate;

	// Holds internal memory.
	void *buffer;

	// Set this to true when eof has been reached.
	bool eof;
}; // Decoder

} // lullaby
} // sound
} // love

#endif // LOVE_SOUND_LULLABY_DECODER_H
