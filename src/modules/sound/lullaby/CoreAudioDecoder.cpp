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

#ifdef LOVE_SUPPORT_COREAUDIO

// LOVE
#include "CoreAudioDecoder.h"

// C++
#include <vector>

namespace love
{
namespace sound
{
namespace lullaby
{

// Callbacks
namespace
{
OSStatus readFunc(void *inClientData, SInt64 inPosition, UInt32 requestCount, void *buffer, UInt32 *actualCount)
{
	Data *data = (Data *) inClientData;
	SInt64 bytesLeft = data->getSize() - inPosition;

	if (bytesLeft > 0)
	{
		UInt32 actualSize = bytesLeft >= requestCount ? requestCount : (UInt32) bytesLeft;
		memcpy(buffer, (char *) data->getData() + inPosition, actualSize);
		*actualCount = actualSize;
	}
	else
	{
		*actualCount = 0;
		return kAudioFilePositionError;
	}

	return noErr;
}

SInt64 getSizeFunc(void *inClientData)
{
	Data *data = (Data *) inClientData;
	return data->getSize();
}
} // anonymous namespace

CoreAudioDecoder::CoreAudioDecoder(Data *data, const std::string &ext, int bufferSize)
	: Decoder(data, ext, bufferSize)
	, audioFile(nullptr)
	, extAudioFile(nullptr)
	, inputInfo()
	, outputInfo()
	, duration(-2.0)
{
	try
	{
		OSStatus err = noErr;

		// Open the file represented by the Data.
		err = AudioFileOpenWithCallbacks(data, readFunc, nullptr, getSizeFunc, nullptr, kAudioFileMP3Type, &audioFile);
		if (err != noErr)
			throw love::Exception("Could open audio file for decoding.");

		// We want to use the Extended AudioFile API.
		err = ExtAudioFileWrapAudioFileID(audioFile, false, &extAudioFile);

		if (err != noErr)
			throw love::Exception("Could open audio file for decoding.");

		// Get the format of the audio data.
		UInt32 propertySize = sizeof(inputInfo);
		err = ExtAudioFileGetProperty(extAudioFile, kExtAudioFileProperty_FileDataFormat, &propertySize, &inputInfo);

		if (err != noErr)
			throw love::Exception("Could not determine file format.");

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
			throw love::Exception("Could not set decoder properties.");
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

bool CoreAudioDecoder::accepts(const std::string &ext)
{
	UInt32 size = 0;
	std::vector<UInt32> types;

	// Get the size in bytes of the type array we're about to get.
	OSStatus err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, sizeof(UInt32), nullptr, &size);
	if (err != noErr)
		return false;

	types.resize(size / sizeof(UInt32));

	// Get an array of supported types.
	err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ReadableTypes, 0, nullptr, &size, &types[0]);
	if (err != noErr)
		return false;

	// Turn the extension string into a CFStringRef.
	CFStringRef extstr = CFStringCreateWithCString(nullptr, ext.c_str(), kCFStringEncodingUTF8);

	CFArrayRef exts = nullptr;
	size = sizeof(CFArrayRef);

	for (UInt32 type : types)
	{
		// Get the extension strings for the type.
		err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof(UInt32), &type, &size, &exts);
		if (err != noErr)
			continue;

		// A type can have more than one extension string.
		for (CFIndex i = 0; i < CFArrayGetCount(exts); i++)
		{
			CFStringRef value = (CFStringRef) CFArrayGetValueAtIndex(exts, i);

			if (CFStringCompare(extstr, value, 0) == kCFCompareEqualTo)
			{
				CFRelease(extstr);
				CFRelease(exts);
				return true;
			}
		}

		CFRelease(exts);
	}

	CFRelease(extstr);
	return false;
}

love::sound::Decoder *CoreAudioDecoder::clone()
{
	return new CoreAudioDecoder(data.get(), ext, bufferSize);
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

bool CoreAudioDecoder::seek(float s)
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

int CoreAudioDecoder::getChannels() const
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
