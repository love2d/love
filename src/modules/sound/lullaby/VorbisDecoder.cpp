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

#include "VorbisDecoder.h"

#include <string.h>
#include "common/config.h"
#include "common/Exception.h"

namespace love
{
namespace sound
{
namespace lullaby
{

/**
 * CALLBACK FUNCTIONS
 **/
static int vorbisClose(void *	/* ptr to the data that the vorbis files need*/)
{
	// Does nothing (handled elsewhere)
	return 1;
}

static size_t vorbisRead(void *ptr	/* ptr to the data that the vorbis files need*/,
				  size_t byteSize	/* how big a byte is*/,
				  size_t sizeToRead	/* How much we can read*/,
				  void *datasource	/* this is a pointer to the data we passed into ov_open_callbacks (our SOggFile struct*/)
{
	size_t				spaceToEOF;			// How much more we can read till we hit the EOF marker
	size_t				actualSizeToRead;	// How much data we are actually going to read from memory
	SOggFile			 *vorbisData;			// Our vorbis data, for the typecast

	// Get the data in the right format
	vorbisData = (SOggFile *)datasource;

	// Calculate how much we need to read.  This can be sizeToRead*byteSize or less depending on how near the EOF marker we are
	spaceToEOF = vorbisData->dataSize - vorbisData->dataRead;
	if ((sizeToRead*byteSize) < spaceToEOF)
		actualSizeToRead = (sizeToRead*byteSize);
	else
		actualSizeToRead = spaceToEOF;

	// A simple copy of the data from memory to the datastruct that the vorbis libs will use
	if (actualSizeToRead)
	{
		// Copy the data from the start of the file PLUS how much we have already read in
		memcpy(ptr, (const char *)vorbisData->dataPtr + vorbisData->dataRead, actualSizeToRead);
		// Increase by how much we have read by
		vorbisData->dataRead += (actualSizeToRead);
	}

	// Return how much we read (in the same way fread would)
	return actualSizeToRead;
}

static int vorbisSeek(void *datasource	/* ptr to the data that the vorbis files need*/,
			   ogg_int64_t offset	/*offset from the point we wish to seek to*/,
			   int whence			/*where we want to seek to*/)
{
	int         spaceToEOF;   // How much more we can read till we hit the EOF marker
	ogg_int64_t actualOffset; // How much we can actually offset it by
	SOggFile   *vorbisData;   // The data we passed in (for the typecast)

	// Get the data in the right format
	vorbisData = (SOggFile *) datasource;

	// Goto where we wish to seek to
	switch (whence)
	{
	case SEEK_SET: // Seek to the start of the data file
		// Make sure we are not going to the end of the file
		if (vorbisData->dataSize >= offset)
			actualOffset = offset;
		else
			actualOffset = vorbisData->dataSize;
		// Set where we now are
		vorbisData->dataRead = (int)actualOffset;
		break;
	case SEEK_CUR: // Seek from where we are
		// Make sure we dont go past the end
		spaceToEOF = vorbisData->dataSize - vorbisData->dataRead;
		if (offset < spaceToEOF)
			actualOffset = (offset);
		else
			actualOffset = spaceToEOF;
		// Seek from our currrent location
		vorbisData->dataRead += (int)actualOffset;
		break;
	case SEEK_END: // Seek from the end of the file
		if (offset < 0)
			vorbisData->dataRead = vorbisData->dataSize + offset;
		else
			vorbisData->dataRead = vorbisData->dataSize;
		break;
	default:
		break;
	};

	return 0;
}

static long vorbisTell(void *datasource	/* ptr to the data that the vorbis files need*/)
{
	SOggFile *vorbisData;
	vorbisData = (SOggFile *) datasource;
	return vorbisData->dataRead;
}
/**
 * END CALLBACK FUNCTIONS
 **/

VorbisDecoder::VorbisDecoder(Data *data, const std::string &ext, int bufferSize)
	: Decoder(data, ext, bufferSize)
	, duration(-2.0)
{
	// Initialize callbacks
	vorbisCallbacks.close_func = vorbisClose;
	vorbisCallbacks.seek_func  = vorbisSeek;
	vorbisCallbacks.read_func  = vorbisRead;
	vorbisCallbacks.tell_func  = vorbisTell;

	// Check endianness
#ifdef LOVE_BIG_ENDIAN
	endian = 1;
#else
	endian = 0;
#endif

	// Initialize OGG file
	oggFile.dataPtr = (const char *) data->getData();
	oggFile.dataSize = (int) data->getSize();
	oggFile.dataRead = 0;

	// Open Vorbis handle
	if (ov_open_callbacks(&oggFile, &handle, NULL, 0, vorbisCallbacks) < 0)
		throw love::Exception("Could not read Ogg bitstream");

	// Get info and comments
	vorbisInfo = ov_info(&handle, -1);
	vorbisComment = ov_comment(&handle, -1);
}

VorbisDecoder::~VorbisDecoder()
{
	ov_clear(&handle);
}

bool VorbisDecoder::accepts(const std::string &ext)
{
	static const std::string supported[] =
	{
		"ogg", "oga", "ogv", ""
	};

	for (int i = 0; !(supported[i].empty()); i++)
	{
		if (supported[i].compare(ext) == 0)
			return true;
	}

	return false;
}

love::sound::Decoder *VorbisDecoder::clone()
{
	return new VorbisDecoder(data.get(), ext, bufferSize);
}

int VorbisDecoder::decode()
{
	int size = 0;

	while (size < bufferSize)
	{
		long result = ov_read(&handle, (char *) buffer + size, bufferSize - size, endian, (getBitDepth() == 16 ? 2 : 1), 1, 0);

		if (result == OV_HOLE)
			continue;
		else if (result <= OV_EREAD)
			return -1;
		else if (result == 0)
		{
			eof = true;
			break;
		}
		else if (result > 0)
			size += result;
	}

	return size;
}

bool VorbisDecoder::seek(float s)
{
	int result = 0;

	// Avoid ov_time_seek (which calls ov_pcm_seek) when seeking to 0, to avoid
	// a bug in libvorbis <= 1.3.4 when seeking to PCM 0 in multiplexed streams.
	if (s <= 0.000001)
		result = ov_raw_seek(&handle, 0);
	else
		result = ov_time_seek(&handle, s);

	if (result == 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool VorbisDecoder::rewind()
{
	// Avoid ov_time_seek to avoid a bug in libvorbis <= 1.3.4 when seeking to
	// PCM 0 in multiplexed streams.
	int result = ov_raw_seek(&handle, 0);

	if (result == 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool VorbisDecoder::isSeekable()
{
	long result = ov_seekable(&handle);
	return (result != 0);
}

int VorbisDecoder::getChannels() const
{
	return vorbisInfo->channels;
}

int VorbisDecoder::getBitDepth() const
{
	return 16;
}

int VorbisDecoder::getSampleRate() const
{
	return (int) vorbisInfo->rate;
}

double VorbisDecoder::getDuration()
{
	// Only calculate the duration if we haven't done so already.
	if (duration == -2.0)
	{
		duration = ov_time_total(&handle, -1);

		if (duration == OV_EINVAL || duration < 0.0)
			duration = -1.0;
	}

	return duration;
}

} // lullaby
} // sound
} // love
