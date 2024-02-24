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

#include "common/config.h"

#include <algorithm>
#include <sstream>

#include "Sound.h"

#include "ModPlugDecoder.h"
#include "VorbisDecoder.h"
#include "WaveDecoder.h"
#include "FLACDecoder.h"
#include "MP3Decoder.h"

#ifdef LOVE_SUPPORT_COREAUDIO
#	include "CoreAudioDecoder.h"
#endif

struct DecoderImpl
{
	love::sound::Decoder *(*create)(love::Stream *stream, int bufferSize);
};

template<typename DecoderType>
DecoderImpl DecoderImplFor()
{
	DecoderImpl decoderImpl;
	decoderImpl.create = [](love::Stream *stream, int bufferSize) -> love::sound::Decoder*
	{
		return new DecoderType(stream, bufferSize);
	};
	return decoderImpl;
}

namespace love
{
namespace sound
{
namespace lullaby
{

Sound::Sound()
	: love::sound::Sound("love.sound.lullaby")
{
}

Sound::~Sound()
{
}

sound::Decoder *Sound::newDecoder(Stream *stream, int bufferSize)
{
	std::vector<DecoderImpl> possibleDecoders = {
		DecoderImplFor<WaveDecoder>(),
		DecoderImplFor<FLACDecoder>(),
		DecoderImplFor<VorbisDecoder>(),
#ifdef LOVE_SUPPORT_COREAUDIO
		DecoderImplFor<CoreAudioDecoder>(),
#endif
		DecoderImplFor<MP3Decoder>(),
#ifndef LOVE_NO_MODPLUG
		DecoderImplFor<ModPlugDecoder>(), // Last because it doesn't work well with Streams.
#endif
	};

	std::stringstream decodingErrors;
	decodingErrors << "Failed to determine file type:\n";
	for (DecoderImpl &possibleDecoder : possibleDecoders)
	{
		try
		{
			stream->seek(0);
			sound::Decoder *decoder = possibleDecoder.create(stream, bufferSize);
			return decoder;
		}
		catch (love::Exception &e)
		{
			decodingErrors << e.what() << '\n';
		}
	}

	std::string errors = decodingErrors.str();
	throw love::Exception("No suitable audio decoders found.\n%s", errors.c_str());

	// Unreachable, but here to prevent (possible) warnings
	return nullptr;
}

} // lullaby
} // sound
} // love
