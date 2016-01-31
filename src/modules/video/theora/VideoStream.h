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

#ifndef LOVE_VIDEO_THEORA_VIDEOSTREAM_H
#define LOVE_VIDEO_THEORA_VIDEOSTREAM_H

#include "video/VideoStream.h"

// LOVE
#include "common/int.h"
#include "filesystem/File.h"
#include "thread/threads.h"

// OGG/Theora
#include <ogg/ogg.h>
#include <theora/codec.h>
#include <theora/theoradec.h>

namespace love
{
namespace video
{
namespace theora
{

class VideoStream : public love::video::VideoStream
{
public:
	VideoStream(love::filesystem::File *file);
	~VideoStream();

	const void *getFrontBuffer() const;
	size_t getSize() const;
	void fillBackBuffer();
	bool swapBuffers();

	int getWidth() const;
	int getHeight() const;
	const std::string &getFilename() const;
	void setSync(FrameSync *frameSync);

	bool isPlaying() const;

	void threadedFillBackBuffer(double dt);

private:
	StrongRef<love::filesystem::File> file;

	bool headerParsed;
	bool streamInited;
	int videoSerial;
	ogg_sync_state sync;
	ogg_stream_state stream;
	ogg_page page;
	ogg_packet packet;

	th_info videoInfo;
	th_dec_ctx *decoder;

	Frame *frontBuffer;
	Frame *backBuffer;
	unsigned int yPlaneXOffset;
	unsigned int cPlaneXOffset;
	unsigned int yPlaneYOffset;
	unsigned int cPlaneYOffset;

	love::thread::MutexRef bufferMutex;
	bool frameReady;

	double lastFrame;
	double nextFrame;
	bool eos;
	unsigned int lagCounter;

	void readPage();
	bool readPacket(bool mustSucceed = false); // true if eos
	void parseHeader();
	void rewind();
	void seekDecoder(double target);
}; // VideoStream

} // theora
} // video
} // love

#endif // LOVE_VIDEO_THEORA_VIDEOSTREAM_H
