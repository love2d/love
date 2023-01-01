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

#ifdef LOVE_SUPPORT_GME

#include "common/Exception.h"
#include "GmeDecoder.h"

namespace love
{
namespace sound
{
namespace lullaby
{

GmeDecoder::GmeDecoder(Data *data, int bufferSize)
	: Decoder(data, bufferSize)
	, emu(0)
	, num_tracks(0)
	, cur_track(0)
{
	void *d = data->getData();
	int s = data->getSize();

	if (gme_open_data(d, s, &emu, sampleRate) != 0)
		throw love::Exception("Could not open game music file");

	num_tracks = gme_track_count(emu);

	try
	{
		if (num_tracks <= 0)
			throw love::Exception("Game music file has no tracks");

		if (gme_start_track(emu, cur_track) != 0)
			throw love::Exception("Could not start game music playback");
	}
	catch (love::Exception &)
	{
		gme_delete(emu);
		throw;
	}
}

GmeDecoder::~GmeDecoder()
{
	if (emu)
		gme_delete(emu);
}

bool GmeDecoder::accepts(const std::string &ext)
{
	static const std::string supported[] =
	{
		"ay", "gbs", "gym", "hes", "kss", "nsf",
		"nsfe", "sap", "spc", "vgm", "vgz", ""
	};

	for (int i = 0; !(supported[i].empty()); i++)
	{
		if (supported[i].compare(ext) == 0)
			return true;
	}

	return false;
}

love::sound::Decoder *GmeDecoder::clone()
{
	return new GmeDecoder(data.get(), bufferSize);
}

int GmeDecoder::decode()
{
	short *sbuf = static_cast<short*>(buffer);
	int size = bufferSize / sizeof(short);

	if (gme_play(emu, size, sbuf) != 0)
		throw love::Exception("Error while decoding game music");

	if (!eof && gme_track_ended(emu))
	{
		// Start the next track if this one ended.
		if (cur_track < num_tracks - 1)
			gme_start_track(emu, ++cur_track);
		else
			eof = true;
	}

	return bufferSize;
}

bool GmeDecoder::seek(double s)
{
	return gme_seek(emu, static_cast<long>(s * 1000.0)) != 0;
}

bool GmeDecoder::rewind()
{
	// If we're in the first track, rewind.
	if (cur_track == 0)
		return gme_seek(emu, 0) == 0;
	else
	{
		// Otherwise, start from the first track again.
		cur_track = 0;
		return gme_start_track(emu, cur_track) == 0;
	}
}

bool GmeDecoder::isSeekable()
{
	return true;
}

int GmeDecoder::getChannelCount() const
{
	return 2;
}

int GmeDecoder::getBitDepth() const
{
	return 16;
}

double GmeDecoder::getDuration()
{
	return -1;
}

} // lullaby
} // sound
} // love

#endif // LOVE_SUPPORT_GME
