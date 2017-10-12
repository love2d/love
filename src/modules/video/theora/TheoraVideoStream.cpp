/**
 * Copyright (c) 2006-2017 LOVE Development Team
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
#include "TheoraVideoStream.h"

using love::filesystem::File;

namespace love
{
namespace video
{
namespace theora
{

TheoraVideoStream::TheoraVideoStream(love::filesystem::File *file)
	: demuxer(file)
	, headerParsed(false)
	, decoder(nullptr)
	, frameReady(false)
	, lastFrame(0)
	, nextFrame(0)
	, lagCounter(0)
{
	if (demuxer.findStream() != OggDemuxer::TYPE_THEORA)
		throw love::Exception("Invalid video file, video is not theora");

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
		throw ex;
	}

	frameSync.set(new DeltaSync(), Acquire::NORETAIN);
}

TheoraVideoStream::~TheoraVideoStream()
{
	if (decoder)
		th_decode_free(decoder);

	th_info_clear(&videoInfo);

	delete frontBuffer;
	delete backBuffer;
}

int TheoraVideoStream::getWidth() const
{
	if (headerParsed)
		return videoInfo.pic_width;
	else
		return 0;
}

int TheoraVideoStream::getHeight() const
{
	if (headerParsed)
		return videoInfo.pic_height;
	else
		return 0;
}

const std::string &TheoraVideoStream::getFilename() const
{
	return demuxer.getFilename();
}

void TheoraVideoStream::setSync(FrameSync *frameSync)
{
	love::thread::Lock l(bufferMutex);
	this->frameSync = frameSync;
}

const void *TheoraVideoStream::getFrontBuffer() const
{
	return frontBuffer;
}

size_t TheoraVideoStream::getSize() const
{
	return sizeof(Frame);
}

bool TheoraVideoStream::isPlaying() const
{
	return frameSync->isPlaying() && !demuxer.isEos();
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

void TheoraVideoStream::parseHeader()
{
	if (headerParsed)
		return;

	th_comment comment;
	th_setup_info *setupInfo = nullptr;
	th_comment_init(&comment);
	int ret;

	demuxer.readPacket(packet);
	ret = th_decode_headerin(&videoInfo, &comment, &setupInfo, &packet);

	if (ret < 0)
	{
		th_comment_clear(&comment);
		throw love::Exception("Could not find header");
	}

	while (ret > 0)
	{
		demuxer.readPacket(packet);
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

void TheoraVideoStream::seekDecoder(double target)
{
	bool success = demuxer.seek(packet, target, [this](int64 granulepos) {
		return th_granule_time(decoder, granulepos);
	});

	if (!success)
		return;

	// Now update theora and our decoder on this new position of ours
	lastFrame = nextFrame = -1;
	th_decode_ctl(decoder, TH_DECCTL_SET_GRANPOS, &packet.granulepos, sizeof(packet.granulepos));
}

void TheoraVideoStream::threadedFillBackBuffer(double dt)
{
	// Synchronize
	frameSync->update(dt);
	double position = frameSync->getPosition();

	// Seeking backwards
	if (position < lastFrame)
		seekDecoder(position);

	// If we're at the end of the stream, or if we're displaying the right frame
	// stop here
	if (demuxer.isEos() || position < nextFrame)
		return;

	th_ycbcr_buffer bufferinfo;
	th_decode_ycbcr_out(decoder, bufferinfo);

	ogg_int64_t granulePosition;
	do
	{
		if (demuxer.readPacket(packet))
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
			seekDecoder(position);
	}
	else
		lagCounter = 0;

	love::thread::Lock l(bufferMutex);
	frameReady = true;
}

void TheoraVideoStream::fillBackBuffer()
{
	// Done in worker thread
}

bool TheoraVideoStream::swapBuffers()
{
	if (demuxer.isEos())
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
