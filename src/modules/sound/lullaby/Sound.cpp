/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#include "Sound.h"

#include "ModPlugDecoder.h"
#include "Mpg123Decoder.h"
#include "VorbisDecoder.h"
#include "FLACDecoder.h"

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
		Mpg123Decoder::quit();
	}

	const char * Sound::getName() const
	{
		return "love.sound.lullaby";
	}

	sound::Decoder * Sound::newDecoder(love::filesystem::File * file, int bufferSize, int sampleRate)
	{
		Data * data = file->read();
		std::string ext = file->getExtension();
		
		sound::Decoder * decoder = 0;

		// Find a suitable decoder here, and return it.
		if(ModPlugDecoder::accepts(ext))
			decoder = new ModPlugDecoder(data, ext, bufferSize, sampleRate);
		else if (Mpg123Decoder::accepts(ext))
			decoder = new Mpg123Decoder(data, ext, bufferSize, sampleRate);
		else if (VorbisDecoder::accepts(ext))
			decoder = new VorbisDecoder(data, ext, bufferSize, sampleRate);
		/*else if (FLACDecoder::accepts(ext))
			decoder = new FLACDecoder(data, ext, bufferSize, sampleRate);*/

		// else if(OtherDecoder::accept(ext))

		data->release();

		return decoder;
	}

} // lullaby
} // sound
} // love
