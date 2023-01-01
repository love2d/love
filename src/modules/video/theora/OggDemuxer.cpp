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

#include "OggDemuxer.h"

namespace love
{
namespace video
{
namespace theora
{

OggDemuxer::OggDemuxer(love::filesystem::File *file)
	: file(file)
	, streamInited(false)
	, videoSerial(0)
	, eos(false)
{
	ogg_sync_init(&sync);
}

OggDemuxer::~OggDemuxer()
{
	if (streamInited)
		ogg_stream_clear(&stream);
	ogg_sync_clear(&sync);
}

bool OggDemuxer::readPage(bool erroreof)
{
	char *syncBuffer = nullptr;
	while (ogg_sync_pageout(&sync, &page) != 1)
	{
		if (syncBuffer && !streamInited && ogg_stream_check(&stream))
			throw love::Exception("Invalid stream");

		syncBuffer = ogg_sync_buffer(&sync, 8192);
		size_t read = file->read(syncBuffer, 8192);
		if (read == 0 && erroreof)
			return false;

		ogg_sync_wrote(&sync, read);
	}

	return true;
}

bool OggDemuxer::readPacket(ogg_packet &packet, bool mustSucceed)
{
	if (!streamInited)
		throw love::Exception("Reading from OggDemuxer before initialization (engine bug)");

	while (ogg_stream_packetout(&stream, &packet) != 1)
	{
		do
		{
			// We need to read another page, but there is none, we're at the end
			if (ogg_page_serialno(&page) == videoSerial && ogg_page_eos(&page) && !mustSucceed)
				return eos = true;

			readPage();
		} while (ogg_page_serialno(&page) != videoSerial);

		ogg_stream_pagein(&stream, &page);
	}

	return eos = false;
}

void OggDemuxer::resync()
{
	ogg_sync_reset(&sync);
	ogg_sync_pageseek(&sync, &page);
	ogg_stream_reset(&stream);
}

bool OggDemuxer::isEos() const
{
	return eos;
}

const std::string &OggDemuxer::getFilename() const
{
	return file->getFilename();
}

OggDemuxer::StreamType OggDemuxer::determineType()
{
	ogg_packet packet;
	if (ogg_stream_packetpeek(&stream, &packet) != 1)
		return TYPE_UNKNOWN;

	// Theora
	// See https://www.theora.org/doc/Theora.pdf section 6.1
	if (packet.bytes >= 7) {
		uint8_t headerType = packet.packet[0];
		if (headerType & 0x80 && std::strncmp((const char*) packet.packet+1, "theora", 6) == 0)
			return TYPE_THEORA;
	}

	return TYPE_UNKNOWN;
}

OggDemuxer::StreamType OggDemuxer::findStream()
{
	if (streamInited)
	{
		eos = false;
		streamInited = false;
		file->seek(0);
		ogg_stream_clear(&stream);
		ogg_sync_reset(&sync);
	}

	while (true)
	{
		if (!readPage(true))
			return TYPE_UNKNOWN;

		// If this page isn't at the start of a stream, we've seen all streams
		if (!ogg_page_bos(&page))
			break;

		videoSerial = ogg_page_serialno(&page);
		ogg_stream_init(&stream, videoSerial);
		ogg_stream_pagein(&stream, &page);
		streamInited = true;

		StreamType type = determineType();
		switch(type)
		{
		case TYPE_THEORA:
			return type;
		default:
			break;
		}

		ogg_stream_clear(&stream);
		streamInited = false;
	}

	if (streamInited)
	{
		streamInited = false;
		ogg_stream_clear(&stream);
	}

	ogg_sync_reset(&sync);

	return TYPE_UNKNOWN;
}

bool OggDemuxer::seek(ogg_packet &packet, double target, std::function<double(int64)> getTime)
{
	static const double rewindThreshold = 0.01;

	eos = false;

	if (target < rewindThreshold)
	{
		file->seek(0);
		resync();
		readPacket(packet, true);
		return true;
	}

	double low = 0;
	double high = file->getSize();

	// If we know our current position, we can drastically decrease the search area
	if (packet.granulepos != -1)
	{
		double currentTime = getTime(packet.granulepos);
		if (currentTime < target)
			low = file->tell();
		else if (currentTime > target)
			high = file->tell();
	}

	while (high-low > rewindThreshold)
	{
		// Determine our next binary search position
		double pos = (high+low)/2;
		file->seek(pos);

		// Break sync
		resync();

		// Read a page
		readPage();
		readPacket(packet, false);
		if (isEos())
		{
			// EOS, so we're definitely past our target (or the target is past
			// the end)
			high = pos;
			eos = false;

			// And a workaround for single-page files:
			if (high < rewindThreshold)
			{
				file->seek(0);
				resync();
				readPacket(packet, true);
			}
			else
				continue;
		}

		// Now search all packets in this page
		int result = -1;
		for (int i = 0; i < ogg_page_packets(&page); ++i)
		{
			if (i > 0)
				readPacket(packet, true);

			// Determine if this is the right place
			double curTime = getTime(packet.granulepos);
			double nextTime = getTime(packet.granulepos+1);

			if (curTime == -1)
				continue; // Invalid granule position (magic?)
			else if (curTime <= target && nextTime > target)
			{
				// the current frame should be displaying right now
				result = 0;
				break;
			}
			else if (curTime > target)
			{
				// No need to check the other packets, they're all past
				// this one
				result = 1;
				break;
			}
		}

		// The sign of result determines the direction
		if (result == 0)
			break;
		else if (result < 0)
			low = pos;
		else
			high = pos;
	}

	return true;
}

} // theora
} // video
} // love
