/**
 * Copyright (c) 2006-2022 LOVE Development Team
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
	int (*probe)(love::Stream *stream);
	int probeScore;
};

static bool compareProbeScore(const DecoderImpl& a, const DecoderImpl& b)
{
	return a.probeScore > b.probeScore;
}

template<typename DecoderType>
DecoderImpl DecoderImplFor()
{
	DecoderImpl decoderImpl;
	decoderImpl.create = [](love::Stream *stream, int bufferSize) -> love::sound::Decoder*
	{
		return new DecoderType(stream, bufferSize);
	};
	decoderImpl.probe = [](love::Stream* stream)
	{
		return DecoderType::probe(stream);
	};
	// Short description of probe score:
	// Probe score indicates how likely is a file is in certain format. If the
	// score is 0 then this particular decoder factory is skipped. Otherwise,
	// decoder with highest probe score is used first then decoder with lower
	// score.
	// There's no standarized value for the probe value (except 0) but if a file
	// is "very likely" on certain format, it's safe to return 100. If the file
	// is "unlikely" to be certain format but determining such thing requires
	// more complicated parsing, it's better to return 1.
	decoderImpl.probeScore = 0;
	return decoderImpl;
}

namespace love
{
namespace sound
{
namespace lullaby
{

Sound::Sound()
{
}

Sound::~Sound()
{
}

const char *Sound::getName() const
{
	return "love.sound.lullaby";
}

sound::Decoder *Sound::newDecoder(Stream *stream, int bufferSize)
{
	std::vector<DecoderImpl> possibleActiveDecoders, possibleDecoders = {
		DecoderImplFor<MP3Decoder>(),
		DecoderImplFor<VorbisDecoder>(),
#ifdef LOVE_SUPPORT_COREAUDIO
		DecoderImplFor<CoreAudioDecoder>(),
#endif
		DecoderImplFor<WaveDecoder>(),
		DecoderImplFor<FLACDecoder>(),
#ifndef LOVE_NO_MODPLUG
		DecoderImplFor<ModPlugDecoder>(), // Last because it doesn't work well with Streams.
#endif
	};

	// Probe decoders
	for (DecoderImpl& possibleDecoder : possibleDecoders)
	{
		stream->seek(0);
		possibleDecoder.probeScore = possibleDecoder.probe(stream);

		if (possibleDecoder.probeScore > 0)
			possibleActiveDecoders.push_back(possibleDecoder);
	}
	std::sort(possibleActiveDecoders.begin(), possibleActiveDecoders.end(), compareProbeScore);

	// Load
	std::stringstream decodingErrors;
	for (DecoderImpl &possibleDecoder : possibleActiveDecoders)
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
