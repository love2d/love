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

#include "common/config.h"

#include <algorithm>
#include <sstream>

#include "Sound.h"

#include "ModPlugDecoder.h"
#include "VorbisDecoder.h"
#include "GmeDecoder.h"
#include "WaveDecoder.h"
#include "FLACDecoder.h"

#ifndef LOVE_NOMPG123
#	include "Mpg123Decoder.h"
#endif // LOVE_NOMPG123

#ifdef LOVE_SUPPORT_COREAUDIO
#	include "CoreAudioDecoder.h"
#endif

struct DecoderImpl
{
	love::sound::Decoder *(*create)(love::filesystem::FileData *data, int bufferSize);
	bool (*accepts)(const std::string& ext);
};

template<typename DecoderType>
DecoderImpl DecoderImplFor()
{
	DecoderImpl decoderImpl;
	decoderImpl.create = [](love::filesystem::FileData *data, int bufferSize) -> love::sound::Decoder*
	{
		return new DecoderType(data, bufferSize);
	};
	decoderImpl.accepts = [](const std::string& ext) -> bool
	{
		return DecoderType::accepts(ext);
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
{
}

Sound::~Sound()
{
#ifndef LOVE_NOMPG123
	Mpg123Decoder::quit();
#endif // LOVE_NOMPG123
}

const char *Sound::getName() const
{
	return "love.sound.lullaby";
}

sound::Decoder *Sound::newDecoder(love::filesystem::FileData *data, int bufferSize)
{
	std::string ext = data->getExtension();
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

	std::vector<DecoderImpl> possibleDecoders = {
#ifndef LOVE_NO_MODPLUG
		DecoderImplFor<ModPlugDecoder>(),
#endif // LOVE_NO_MODPLUG
#ifndef LOVE_NOMPG123
		DecoderImplFor<Mpg123Decoder>(),
#endif // LOVE_NOMPG123
		DecoderImplFor<VorbisDecoder>(),
#ifdef LOVE_SUPPORT_GME
		DecoderImplFor<GmeDecoder>(),
#endif // LOVE_SUPPORT_GME
#ifdef LOVE_SUPPORT_COREAUDIO
		DecoderImplFor<CoreAudioDecoder>(),
#endif
		DecoderImplFor<WaveDecoder>(),
		DecoderImplFor<FLACDecoder>(),
		// DecoderImplFor<OtherDecoder>(),
	};

	// First find a matching decoder based on extension
	for (DecoderImpl &possibleDecoder : possibleDecoders)
	{
		if (possibleDecoder.accepts(ext))
			return possibleDecoder.create(data, bufferSize);
	}

	// If that fails, start probing instead
	std::stringstream decodingErrors;
	decodingErrors << "Failed to determine file type:\n";
	for (DecoderImpl &possibleDecoder : possibleDecoders)
	{
		try
		{
			sound::Decoder *decoder = possibleDecoder.create(data, bufferSize);
			return decoder;
		}
		catch (love::Exception &e)
		{
			decodingErrors << e.what() << '\n';
		}
	}

	// Probing failed too, bail with the accumulated errors
	throw love::Exception(decodingErrors.str().c_str());

	// Unreachable, but here to prevent (possible) warnings
	return nullptr;
}

} // lullaby
} // sound
} // love
