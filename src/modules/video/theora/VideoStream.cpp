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

// STL
#include <iostream>

// LOVE
#include "VideoStream.h"

using love::filesystem::File;

namespace love
{
namespace video
{
namespace theora
{

VideoStream::VideoStream(love::filesystem::File *file)
	: file(file)
	, headerParsed(false)
	, streamInited(false)
	, videoSerial(0)
	, decoder(nullptr)
	, frameReady(false)
	, lastFrame(0)
	, nextFrame(0)
	, eos(false)
	, lagCounter(0)
{
	ogg_sync_init(&sync);
	th_info_init(&videoInfo);

	frontBuffer = new Frame();
	backBuffer = new Frame();

	try
	{
		parseHeader();
	}
	catch (love::Exception &ex)
	{
		delete backBuffer;
		delete frontBuffer;
		th_info_clear(&videoInfo);
		ogg_sync_clear(&sync);
		throw ex;
	}

	frameSync = new DeltaSync();
	frameSync->release();
}

VideoStream::~VideoStream()
{
	if (decoder)
		th_decode_free(decoder);

	th_info_clear(&videoInfo);
	if (headerParsed)
		ogg_stream_clear(&stream);

	ogg_sync_clear(&sync);

	delete frontBuffer;
	delete backBuffer;
}

int VideoStream::getWidth() const
{
	if (headerParsed)
		return videoInfo.pic_width;
	else
		return 0;
}

int VideoStream::getHeight() const
{
	if (headerParsed)
		return videoInfo.pic_height;
	else
		return 0;
}

const std::string &VideoStream::getFilename() const
{
	return file->getFilename();
}

void VideoStream::setSync(FrameSync *frameSync)
{
	love::thread::Lock l(bufferMutex);
	this->frameSync = frameSync;
}

const void *VideoStream::getFrontBuffer() const
{
	return frontBuffer;
}

size_t VideoStream::getSize() const
{
	return sizeof(Frame);
}

bool VideoStream::isPlaying() const
{
	return frameSync->isPlaying() && !eos;
}

void VideoStream::readPage()
{
	char *syncBuffer = nullptr;
	while (ogg_sync_pageout(&sync, &page) != 1)
	{
		if (syncBuffer && !headerParsed && ogg_stream_check(&stream))
			throw love::Exception("Invalid stream");

		syncBuffer = ogg_sync_buffer(&sync, 8192);
		size_t read = file->read(syncBuffer, 8192);
		ogg_sync_wrote(&sync, read);
	}
}

bool VideoStream::readPacket(bool mustSucceed)
{
	if (!streamInited)
	{
		readPage();
		videoSerial = ogg_page_serialno(&page);
		ogg_stream_init(&stream, videoSerial);
		streamInited = true;
		ogg_stream_pagein(&stream, &page);
	}

	while (ogg_stream_packetout(&stream, &packet) != 1)
	{
		// We need to read another page, but there is none, we're at the end
		if (ogg_page_eos(&page) && !mustSucceed)
			return eos = true;

		do
		{
			readPage();
		} while (ogg_page_serialno(&page) != videoSerial);

		ogg_stream_pagein(&stream, &page);
	}

	return false;
}

template<typename T>
inline void scaleFormat(th_pixel_fmt fmt, T &x, T &y)
{
	switch(fmt)
	{
	case TH_PF_420:
		y /= 2;
	case TH_PF_422:
		x /= 2;
		break;
	default:
		break;
	}
}

void VideoStream::parseHeader()
{
	if (headerParsed)
		return;

	th_comment comment;
	th_setup_info *setupInfo = nullptr;
	th_comment_init(&comment);
	int ret;

	do
	{
		readPacket();
		ret = th_decode_headerin(&videoInfo, &comment, &setupInfo, &packet);
		if (ret == TH_ENOTFORMAT)
		{
			ogg_stream_clear(&stream);
			streamInited = false;
		}
	} while(ret < 0 && !ogg_page_eos(&page));

	if (ret < 0)
	{
		th_comment_clear(&comment);
		throw love::Exception("Could not find header");
	}

	while (ret > 0)
	{
		readPacket();
		ret = th_decode_headerin(&videoInfo, &comment, &setupInfo, &packet);
	}

	th_comment_clear(&comment);

	decoder = th_decode_alloc(&videoInfo, setupInfo);
	th_setup_free(setupInfo);

	Frame *buffers[2] = {backBuffer, frontBuffer};

	yPlaneXOffset = cPlaneXOffset = videoInfo.pic_x;
	yPlaneYOffset = cPlaneYOffset = videoInfo.pic_y;

	scaleFormat(videoInfo.pixel_fmt, cPlaneXOffset, cPlaneYOffset);

	for (int i = 0; i < 2; i++)
	{
		buffers[i]->cw = buffers[i]->yw = videoInfo.pic_width;
		buffers[i]->ch = buffers[i]->yh = videoInfo.pic_height;

		scaleFormat(videoInfo.pixel_fmt, buffers[i]->cw, buffers[i]->ch);

		buffers[i]->yplane = new unsigned char[buffers[i]->yw * buffers[i]->yh];
		buffers[i]->cbplane = new unsigned char[buffers[i]->cw * buffers[i]->ch];
		buffers[i]->crplane = new unsigned char[buffers[i]->cw * buffers[i]->ch];

		memset(buffers[i]->yplane, 16, buffers[i]->yw * buffers[i]->yh);
		memset(buffers[i]->cbplane, 128, buffers[i]->cw * buffers[i]->ch);
		memset(buffers[i]->crplane, 128, buffers[i]->cw * buffers[i]->ch);
	}

	headerParsed = true;
	th_decode_packetin(decoder, &packet, nullptr);
}

// Arbitrary seeking isn't supported yet, but rewinding is
void VideoStream::rewind()
{
	// Seek our data stream back to the start
	file->seek(0);

	// Break our sync, and discard the rest of the page
	ogg_sync_reset(&sync);
	ogg_sync_pageseek(&sync, &page);

	// Read our first page/packet from the stream again
	readPacket(true);

	// Now tell theora we're at frame 1 (not 0!)
	int64 granPos = 1;
	th_decode_ctl(decoder, TH_DECCTL_SET_GRANPOS, &granPos, sizeof(granPos));

	// Force a redraw, since this will always be less than the sync's position
	lastFrame = nextFrame = -1;
	eos = false;
}

void VideoStream::seekDecoder(double target)
{
	if (target < 0.01)
	{
		rewind();
		return;
	}

	double low = 0;
	double high = file->getSize();

	while (high-low > 0.0001)
	{
		// Determine our next binary search position
		double pos = (high+low)/2;
		file->seek(pos);

		// Break sync
		ogg_sync_reset(&sync);
		ogg_sync_pageseek(&sync, &page);

		// Read a packet
		readPacket(false);
		if (eos)
			return;

		// Determine if this is the right place
		double curTime = th_granule_time(decoder, packet.granulepos);
		double nextTime = th_granule_time(decoder, packet.granulepos+1);

		if (curTime == -1)
			continue; // Invalid granule position (magic?)
		else if (curTime <= target && nextTime > target)
			break; // the current frame should be displaying right now
		else if (curTime > target)
			high = pos;
		else
			low = pos;
	}

	// Now update theora and our decoder on this new position of ours
	lastFrame = nextFrame = -1;
	eos = false;
	th_decode_ctl(decoder, TH_DECCTL_SET_GRANPOS, &packet.granulepos, sizeof(packet.granulepos));
}

void VideoStream::threadedFillBackBuffer(double dt)
{
	// Synchronize
	frameSync->update(dt);
	double position = frameSync->getPosition();

	// Seeking backwards
	if (position < lastFrame)
		seekDecoder(position);

	// If we're at the end of the stream, or if we're displaying the right frame
	// stop here
	if (eos || position < nextFrame)
		return;

	th_ycbcr_buffer bufferinfo;
	th_decode_ycbcr_out(decoder, bufferinfo);

	ogg_int64_t granulePosition;
	do
	{
		if (readPacket())
			return;
	} while (th_decode_packetin(decoder, &packet, &granulePosition) != 0);
	lastFrame = nextFrame;
	nextFrame = th_granule_time(decoder, granulePosition);

	{
		// Don't swap whilst we're writing to the backbuffer
		love::thread::Lock l(bufferMutex);
		frameReady = false;
	}

	for (int y = 0; y < backBuffer->yh; ++y)
	{
		memcpy(backBuffer->yplane+backBuffer->yw*y,
				bufferinfo[0].data+
					bufferinfo[0].stride*(y+yPlaneYOffset)+yPlaneXOffset,
				backBuffer->yw);
	}

	for (int y = 0; y < backBuffer->ch; ++y)
	{
		memcpy(backBuffer->cbplane+backBuffer->cw*y,
				bufferinfo[1].data+
					bufferinfo[1].stride*(y+cPlaneYOffset)+cPlaneXOffset,
				backBuffer->cw);
	}

	for (int y = 0; y < backBuffer->ch; ++y)
	{
		memcpy(backBuffer->crplane+backBuffer->cw*y,
				bufferinfo[2].data+
					bufferinfo[2].stride*(y+cPlaneYOffset)+cPlaneXOffset,
				backBuffer->cw);
	}

	// Seeking forwards:
	// If we're still not on the right frame, either we're lagging or we're seeking
	// After 5 frames, go for a seek. This is not ideal.. but what is
	if (position > nextFrame)
	{
		if (++lagCounter > 5)
			seek(position);
	}
	else
		lagCounter = 0;

	love::thread::Lock l(bufferMutex);
	frameReady = true;
}

void VideoStream::fillBackBuffer()
{
	// Done in worker thread
}

bool VideoStream::swapBuffers()
{
	if (eos)
		return false;

	love::thread::Lock l(bufferMutex);
	if (!frameReady)
		return false;
	frameReady = false;

	Frame *temp = frontBuffer;
	frontBuffer = backBuffer;
	backBuffer = temp;

	return true;
}

} // theora
} // video
} // love
