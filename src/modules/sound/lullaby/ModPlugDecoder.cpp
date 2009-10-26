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

#include "ModPlugDecoder.h"

#include <set>
#include <common/Exception.h>

namespace love
{
namespace sound
{
namespace lullaby
{
	ModPlugDecoder::ModPlugDecoder(Data * data, const std::string & ext, int bufferSize, int sampleRate)
		: Decoder(data, ext, bufferSize, sampleRate), plug(0)
	{

		// Set some ModPlug settings.
		settings.mFlags = MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION;
		settings.mChannels = 2;
		settings.mBits = 16;
		settings.mFrequency = sampleRate;
		settings.mResamplingMode = MODPLUG_RESAMPLE_LINEAR;
		settings.mLoopCount = 0;
		ModPlug_SetSettings(&settings);

		// Load the module.
		plug = ModPlug_Load(data->getData(), data->getSize());

		ModPlug_SetMasterVolume(plug, 512);

		if(plug == 0)
			throw love::Exception("Could not load file with ModPlug.");
	}

	ModPlugDecoder::~ModPlugDecoder()
	{
		if(plug != 0)
			ModPlug_Unload(plug);
	}

	bool ModPlugDecoder::accepts(const std::string & ext)
	{
		static const std::string supported[] = {
			"it", "xm", "mod", "wav", ""
		};

		for(int i = 0; !(supported[i].empty()); i++)
		{
			if(supported[i].compare(ext) == 0)
				return true;
		}

		return false;
	}

	love::sound::Decoder * ModPlugDecoder::clone()
	{
		return new ModPlugDecoder(data, ext, bufferSize, settings.mFrequency);
	}

	int ModPlugDecoder::decode()
	{
		int r =  ModPlug_Read(plug, buffer, bufferSize);

		if(r == 0)
			eof = true;

		return r;
	}

	bool ModPlugDecoder::seek(float s)
	{
		ModPlug_Seek(plug, (int)(s*1000.0f));
		return true;
	}

	bool ModPlugDecoder::rewind()
	{
		// Let's reload.
		ModPlug_Unload(plug);
		plug = ModPlug_Load(data->getData(), data->getSize());
		eof = false;
		return (plug != 0);
	}

	bool ModPlugDecoder::isSeekable()
	{
		return true;
	}

	int ModPlugDecoder::getChannels() const
	{
		return 2;
	}

	int ModPlugDecoder::getBits() const
	{
		return 16;
	}

} // lullaby
} // sound
} // love
