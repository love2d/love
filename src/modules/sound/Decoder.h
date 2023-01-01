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

#ifndef LOVE_SOUND_DECODER_H
#define LOVE_SOUND_DECODER_H

// LOVE
#include "common/Object.h"
#include "filesystem/File.h"

#include <string>

namespace love
{
namespace sound
{
/**
 * Decoder objects are responsible for decoding audio files. They maintain
 * an interal buffer into which they write raw decoded audio data.
 **/
class Decoder : public Object
{
public:

	static love::Type type;

	Decoder(Data *data, int bufferSize);
	virtual ~Decoder();

	/**
	 * Indicates how many bytes of raw data should be generated at each
	 * call to Decode.
	 **/
	static const int DEFAULT_BUFFER_SIZE = 16384;

	/**
	 * Indicates the quality of the sound.
	 **/
	static const int DEFAULT_SAMPLE_RATE = 44100;

	/**
	 * Default is stereo.
	 **/
	static const int DEFAULT_CHANNELS = 2;

	/**
	 * 16 bit audio is the default.
	 **/
	static const int DEFAULT_BIT_DEPTH = 16;

	/**
	 * Creates a deep of itself. The sound stream can (and should) be
	 * rewound, and does not have to be at the same place.
	 * @return A new Decoder object.
	 **/
	virtual Decoder *clone() = 0;

	/**
	 * Decodes the next chunk of the music stream, this will usually be
	 * bufferSize amount of bytes, unless EOF occurs. Zero or negative values
	 * indicate EOF or errors.
	 * @return The number of bytes actually decoded.
	 **/
	virtual int decode() = 0;

	/**
	 * Gets the size of the buffer (NOT the size of the entire stream).
	 * @return The size of the buffer.
	 **/
	virtual int getSize() const;

	/**
	 * Gets a pointer to the actual data. The contents of this buffer will
	 * change with each call to decode, so the client must copy the data.
	 * @return A buffer to raw sound data.
	 **/
	virtual void *getBuffer() const;

	/**
	 * Seeks to the specified position, if possible.
	 * @param s The position in the stream in seconds.
	 * @return True if success, false on fail/unsupported.
	 **/
	virtual bool seek(double s) = 0;

	/**
	 * Rewinds the stream to the start.
	 * @return True if success, false on fail/unsupported.
	 **/
	virtual bool rewind() = 0;

	/**
	 * Checks whether a stream is seekable.
	 * @return True if seekable, false otherwise.
	 **/
	virtual bool isSeekable() = 0;

	/**
	 * Checks whether a stream has more data to decode or not. Use
	 * rewind to start the stream again.
	 * @return False if there is more data, true on EOF.
	 **/
	virtual bool isFinished();

	/**
	 * Gets the number of channels in a stream. Supported values are 1 (mono) or 2 (stereo).
	 * @return Either 1 for mono, 2 for stereo, or 0 on errors.
	 **/
	virtual int getChannelCount() const = 0;

	/**
	 * Gets the number of bits per sample. Supported values are 8 or 16.
	 * @return Either 8, 16, or 0 if unsupported.
	 **/
	virtual int getBitDepth() const = 0;

	/**
	 * Gets the sample rate for the Decoder, that is, samples per second.
	 * @return The sample rate, eg. 44100.
	 **/
	virtual int getSampleRate() const;

	/**
	 * Gets the estimated total duration of the stream. in seconds. May return
	 * -1 if the duration cannot be determined.
	 **/
	virtual double getDuration() = 0;

protected:

	// The encoded data. This should be replaced with buffered file
	// reads in the future.
	StrongRef<Data> data;

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

} // sound
} // love

#endif // LOVE_SOUND_DECODER_H
