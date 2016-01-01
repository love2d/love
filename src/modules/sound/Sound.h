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

#ifndef LOVE_SOUND_SOUND_H
#define LOVE_SOUND_SOUND_H

// LOVE
#include "common/Module.h"
#include "filesystem/File.h"

#include "SoundData.h"
#include "Decoder.h"

namespace love
{
namespace sound
{

/**
 * The Sound module is responsible for decoding sound data. It is
 * not responsible for playing it.
 **/
class Sound : public Module
{

public:

	virtual ~Sound();

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_SOUND; }

	/**
	 * Creates new SoundData from a decoder. Fully expands the
	 * encoded sound data into raw sound data. Not recommended
	 * on large (long-duration) files.
	 * @param decoder The file to decode the data from.
	 * @return A SoundData object, or zero if the file type couldn't be handled.
	 **/
	SoundData *newSoundData(Decoder *decoder);

	/**
	 * Creates a new SoundData with the specified number of samples and format.
	 * @param samples The number of samples.
	 * @param sampleRate Number of samples per second.
	 * @param bitDepth Bits per sample (8 or 16).
	 * @param channels Either 1 for mono, or 2 for stereo.
	 * @return A new SoundData object, or zero in case of errors.
	 **/
	SoundData *newSoundData(int samples, int sampleRate, int bitDepth, int channels);

	/**
	 * Attempts to find a decoder for the encoded sound data in the
	 * specified file.
	 * @param file The file with encoded sound data.
	 * @param bufferSize The size of each decoded chunk.
	 * @return A Decoder object on success, or zero if no decoder could be found.
	 **/
	virtual Decoder *newDecoder(filesystem::FileData *file, int bufferSize) = 0;

}; // Sound

} // sound
} // love

#endif // LOVE_SOUND_SOUND_H
