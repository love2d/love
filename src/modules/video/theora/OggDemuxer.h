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

#ifndef LOVE_VIDEO_THEORA_OGGDEMUXER_H
#define LOVE_VIDEO_THEORA_OGGDEMUXER_H

// STL
#include <functional>

// LOVE
#include "filesystem/File.h"

// OGG
#include <ogg/ogg.h>

namespace love
{
namespace video
{
namespace theora
{

class OggDemuxer
{
public:
	enum StreamType
	{
		TYPE_THEORA,
		TYPE_UNKNOWN,
	};

	OggDemuxer(love::filesystem::File *file);
	~OggDemuxer();

	StreamType findStream();
	bool readPacket(ogg_packet &packet, bool mustSucceed = false);
	void resync();
	bool isEos() const;
	const std::string &getFilename() const;
	bool seek(ogg_packet &packet, double target, std::function<double(int64)> getTime);

private:
	StrongRef<love::filesystem::File> file;

	ogg_sync_state sync;
	ogg_stream_state stream;
	ogg_page page;

	bool streamInited;
	int videoSerial;
	bool eos;

	bool readPage(bool erroreof = false);
	StreamType determineType();
}; // OggDemuxer

} // theora
} // video
} // love

#endif // LOVE_VIDEO_THEORA_OGGDEMUXER_H
