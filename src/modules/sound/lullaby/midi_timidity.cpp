/*
** music_timidity_mididevice.cpp
** Provides access to TiMidity as a generic MIDI device.
**
**---------------------------------------------------------------------------
** Copyright 2008 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

// HEADER FILES ------------------------------------------------------------

#define LOVE_SUPPORT_TIMIDITY

#ifdef LOVE_SUPPORT_TIMIDITY
#include "midi.h"
#include "libraries/timidity/timidity.h"

namespace love
{
namespace sound
{
namespace lullaby
{

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

struct FmtChunk
{
	uint32 ChunkID;
	uint32 ChunkLen;
	uint16 FormatTag;
	uint16 Channels;
	uint32 SamplesPerSec;
	uint32 AvgBytesPerSec;
	uint16 BlockAlign;
	uint16 BitsPerSample;
	uint16 ExtensionSize;
	uint16 ValidBitsPerSample;
	uint32 ChannelMask;
	uint32 SubFormatA;
	uint16 SubFormatB;
	uint16 SubFormatC;
	uint8  SubFormatD[8];
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// TimidityMIDIDevice Constructor
//
//==========================================================================

TimidityMIDIDevice::TimidityMIDIDevice(const char *args)
{
	Renderer = NULL;
	Renderer = new Timidity::Renderer((float)SampleRate, args);
}

//==========================================================================
//
// TimidityMIDIDevice Destructor
//
//==========================================================================

TimidityMIDIDevice::~TimidityMIDIDevice()
{
	Close();
	if (Renderer != NULL)
	{
		delete Renderer;
	}
}

//==========================================================================
//
// TimidityMIDIDevice :: Open
//
// Returns 0 on success.
//
//==========================================================================

int TimidityMIDIDevice::Open(void (*callback)(unsigned int, void *, uint32, uint32), void *userdata)
{
	int ret = OpenStream(2, 0, callback, userdata);
	if (ret == 0)
	{
		Renderer->Reset();
	}
	return ret;
}

//==========================================================================
//
// TimidityMIDIDevice :: PrecacheInstruments
//
// Each entry is packed as follows:
//   Bits 0- 6: Instrument number
//   Bits 7-13: Bank number
//   Bit    14: Select drum set if 1, tone bank if 0
//
//==========================================================================

void TimidityMIDIDevice::PrecacheInstruments(const uint16 *instruments, int count)
{
	for (int i = 0; i < count; ++i)
	{
		Renderer->MarkInstrument((instruments[i] >> 7) & 127, instruments[i] >> 14, instruments[i] & 127);
	}
	Renderer->load_missing_instruments();
}

//==========================================================================
//
// TimidityMIDIDevice :: HandleEvent
//
//==========================================================================

void TimidityMIDIDevice::HandleEvent(int status, int parm1, int parm2)
{
	Renderer->HandleEvent(status, parm1, parm2);
}

//==========================================================================
//
// TimidityMIDIDevice :: HandleLongEvent
//
//==========================================================================

void TimidityMIDIDevice::HandleLongEvent(const uint8 *data, int len)
{
	Renderer->HandleLongMessage(data, len);
}

//==========================================================================
//
// TimidityMIDIDevice :: ComputeOutput
//
//==========================================================================

void TimidityMIDIDevice::ComputeOutput(float *buffer, int len)
{
	Renderer->ComputeOutput(buffer, len);
}

//==========================================================================
//
// TimidityWaveWriterMIDIDevice Constructor
//
//==========================================================================

TimidityWaveWriterMIDIDevice::TimidityWaveWriterMIDIDevice(const char *filename, int rate)
	:TimidityMIDIDevice(NULL)
{
	File = fopen(filename, "wb");
	if (File != NULL)
	{ // Write wave header
		uint32 work[3];
		FmtChunk fmt;

		work[0] = MAKE_ID('R','I','F','F');
		work[1] = 0;								// filled in later
		work[2] = MAKE_ID('W','A','V','E');
		if (3 != fwrite(work, 4, 3, File)) goto fail;

		fmt.ChunkID = MAKE_ID('f','m','t',' ');
		fmt.ChunkLen = uint32(sizeof(fmt) - 8);
		fmt.FormatTag = 0xFFFE;		// WAVE_FORMAT_EXTENSIBLE
		fmt.Channels = 2;
		fmt.SamplesPerSec = (int)Renderer->rate;
		fmt.AvgBytesPerSec = (int)Renderer->rate * 8;
		fmt.BlockAlign = 8;
		fmt.BitsPerSample = 32;
		fmt.ExtensionSize = 2 + 4 + 16;
		fmt.ValidBitsPerSample = 32;
		fmt.ChannelMask = 3;
		fmt.SubFormatA = 0x00000003;	// Set subformat to KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
		fmt.SubFormatB = 0x0000;
		fmt.SubFormatC = 0x0010;
		fmt.SubFormatD[0] = 0x80;
		fmt.SubFormatD[1] = 0x00;
		fmt.SubFormatD[2] = 0x00;
		fmt.SubFormatD[3] = 0xaa;
		fmt.SubFormatD[4] = 0x00;
		fmt.SubFormatD[5] = 0x38;
		fmt.SubFormatD[6] = 0x9b;
		fmt.SubFormatD[7] = 0x71;
		if (1 != fwrite(&fmt, sizeof(fmt), 1, File)) goto fail;

		work[0] = MAKE_ID('d','a','t','a');
		work[1] = 0;								// filled in later
		if (2 != fwrite(work, 4, 2, File)) goto fail;

		return;
fail:
		printf("Failed to write %s: %s\n", filename, strerror(errno));
		fclose(File);
		File = NULL;
	}
}

//==========================================================================
//
// TimidityWaveWriterMIDIDevice Destructor
//
//==========================================================================

TimidityWaveWriterMIDIDevice::~TimidityWaveWriterMIDIDevice()
{
	if (File != NULL)
	{
		long pos = ftell(File);
		uint32 size;
                printf("pos %lu\n", pos);

		// data chunk size
		size = uint32(pos - 8);
		if (0 == fseek(File, 4, SEEK_SET))
		{
			if (1 == fwrite(&size, 4, 1, File))
			{
				size = uint32(pos - 12 - sizeof(FmtChunk) - 8);
				if (0 == fseek(File, 4 + sizeof(FmtChunk) + 4, SEEK_CUR))
				{
                                    printf("seek %lu\n", pos);
					if (1 == fwrite(&size, 4, 1, File))
					{
                                    fflush(File);
                                    printf("write\n");
						fclose(File);
                                    printf("close\n");
						return;
					}
				}
			}
		}
		printf("Could not finish writing wave file: %s\n", strerror(errno));
		fclose(File);
	}
}

//==========================================================================
//
// TimidityWaveWriterMIDIDevice :: Resume
//
//==========================================================================

int TimidityWaveWriterMIDIDevice::Resume()
{
	float writebuffer[4096];

	while (ServiceStream(writebuffer, sizeof(writebuffer)))
	{
		if (fwrite(writebuffer, sizeof(writebuffer), 1, File) != 1)
		{
			printf("Could not write entire wave file: %s\n", strerror(errno));
			return 1;
		}
	}
	return 0;
}

//==========================================================================
//
// TimidityWaveWriterMIDIDevice Stop
//
//==========================================================================

void TimidityWaveWriterMIDIDevice::Stop()
{
}

}
}
}
#endif
