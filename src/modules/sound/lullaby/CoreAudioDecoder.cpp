/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#ifdef LOVE_SUPPORT_COREAUDIO

// LOVE
#include "CoreAudioDecoder.h"
#include "common/Exception.h"

// C++
#include <vector>

namespace love
{
namespace sound
{
namespace lullaby
{

// Callbacks
static OSStatus readFunc(void *inClientData, SInt64 inPosition, UInt32 requestCount, void *buffer, UInt32 *actualCount)
{
	auto stream = (Stream *) inClientData;
	int64 readbytes = stream->read(buffer, requestCount);

	*actualCount = (UInt32) readbytes;
	return readbytes > 0 ? noErr : kAudioFilePositionError;
}

static SInt64 getSizeFunc(void *inClientData)
{
	auto stream = (Stream *) inClientData;
	return stream->getSize();
}

CoreAudioDecoder::CoreAudioDecoder(Stream *stream, int bufferSize)
	: Decoder(stream, bufferSize)
	, audioFile(nullptr)
	, extAudioFile(nullptr)
	, inputInfo()
	, outputInfo()
	, duration(-2.0)
{
	try
	{
		OSStatus err = noErr;

		// Open the file represented by the Stream.
		err = AudioFileOpenWithCallbacks(stream, readFunc, nullptr, getSizeFunc, nullptr, kAudioFileMP3Type, &audioFile);
		if (err != noErr)
			throw love::Exception("Could not open audio file for decoding with CoreAudio.");

		// We want to use the Extended AudioFile API.
		err = ExtAudioFileWrapAudioFileID(audioFile, false, &extAudioFile);

		if (err != noErr)
			throw love::Exception("Could not open audio file for decoding with CoreAudio.");

		// Get the format of the audio data.
		UInt32 propertySize = sizeof(inputInfo);
		err = ExtAudioFileGetProperty(extAudioFile, kExtAudioFileProperty_FileDataFormat, &propertySize, &inputInfo);

		if (err != noErr)
			throw love::Exception("Could not determine CoreAudio file format.");

		// Set the output format to 16 bit signed integer (native-endian) data.
		// Keep the channel count and sample rate of the source format.
		outputInfo.mSampleRate = inputInfo.mSampleRate;
		outputInfo.mChannelsPerFrame = inputInfo.mChannelsPerFrame;

		int bytes = (inputInfo.mBitsPerChannel == 8) ? 1 : 2;

		outputInfo.mFormatID = kAudioFormatLinearPCM;
		outputInfo.mBitsPerChannel = bytes * 8;
		outputInfo.mBytesPerFrame = bytes * outputInfo.mChannelsPerFrame;
		outputInfo.mFramesPerPacket = 1;
		outputInfo.mBytesPerPacket = bytes * outputInfo.mChannelsPerFrame;
		outputInfo.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;

		// unsigned 8-bit or signed 16-bit integer PCM data.
		if (outputInfo.mBitsPerChannel == 16)
			outputInfo.mFormatFlags |= kAudioFormatFlagIsSignedInteger;

		// Set the desired output format.
		propertySize = sizeof(outputInfo);
		err = ExtAudioFileSetProperty(extAudioFile, kExtAudioFileProperty_ClientDataFormat, propertySize, &outputInfo);

		if (err != noErr)
			throw love::Exception("Could not set CoreAudio decoder properties.");
	}
	catch (love::Exception &)
	{
		closeAudioFile();
		throw;
	}

	sampleRate = (int) outputInfo.mSampleRate;
}

CoreAudioDecoder::~CoreAudioDecoder()
{
	closeAudioFile();
}

void CoreAudioDecoder::closeAudioFile()
{
	if (extAudioFile != nullptr)
		ExtAudioFileDispose(extAudioFile);
	else if (audioFile != nullptr)
		AudioFileClose(audioFile);

	extAudioFile = nullptr;
	audioFile = nullptr;
}

love::sound::Decoder *CoreAudioDecoder::clone()
{
	StrongRef<Stream> s(stream->clone(), Acquire::NORETAIN);
	return new CoreAudioDecoder(s, bufferSize);
}

int CoreAudioDecoder::decode()
{
	int size = 0;

	while (size < bufferSize)
	{
		AudioBufferList dataBuffer;
		dataBuffer.mNumberBuffers = 1;
		dataBuffer.mBuffers[0].mDataByteSize = bufferSize - size;
		dataBuffer.mBuffers[0].mData = (char *) buffer + size;
		dataBuffer.mBuffers[0].mNumberChannels = outputInfo.mChannelsPerFrame;

		UInt32 frames = (bufferSize - size) / outputInfo.mBytesPerFrame;

		if (ExtAudioFileRead(extAudioFile, &frames, &dataBuffer) != noErr)
			return size;

		if (frames == 0)
		{
			eof = true;
			break;
		}

		size += frames * outputInfo.mBytesPerFrame;
	}

	return size;
}

bool CoreAudioDecoder::seek(double s)
{
	OSStatus err = ExtAudioFileSeek(extAudioFile, (SInt64) (s * inputInfo.mSampleRate));

	if (err == noErr)
	{
		eof = false;
		return true;
	}

	return false;
}

bool CoreAudioDecoder::rewind()
{
	OSStatus err = ExtAudioFileSeek(extAudioFile, 0);

	if (err == noErr)
	{
		eof = false;
		return true;
	}

	return false;
}

bool CoreAudioDecoder::isSeekable()
{
	return true;
}

int CoreAudioDecoder::getChannelCount() const
{
	return outputInfo.mChannelsPerFrame;
}

int CoreAudioDecoder::getBitDepth() const
{
	return outputInfo.mBitsPerChannel;
}

double CoreAudioDecoder::getDuration()
{
	// Only calculate the duration if we haven't done so already.
	if (duration == -2.0)
	{
		SInt64 samples = 0;
		UInt32 psize = (UInt32) sizeof(samples);

		OSStatus err = ExtAudioFileGetProperty(extAudioFile, kExtAudioFileProperty_FileLengthFrames, &psize, &samples);

		if (err == noErr)
			duration = (double) samples / (double) sampleRate;
		else
			duration = -1.0;
	}

	return duration;
}

} // lullaby
} // sound
} // love

#endif // LOVE_SUPPORT_COREAUDIO
