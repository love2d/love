/*
** music_softsynth_mididevice.cpp
** Common base class for software synthesis MIDI devices.
**
**---------------------------------------------------------------------------
** Copyright 2008-2010 Randy Heit
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

#include "midi.h"
#include <assert.h>

namespace love
{
namespace sound
{
namespace lullaby
{


// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

bool synth_watch = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// SoftSynthMIDIDevice Constructor
//
//==========================================================================

SoftSynthMIDIDevice::SoftSynthMIDIDevice()
{
	Tempo = 0;
	Division = 0;
	Events = NULL;
	Started = false;
	SampleRate = 44100;
}

//==========================================================================
//
// SoftSynthMIDIDevice Destructor
//
//==========================================================================

SoftSynthMIDIDevice::~SoftSynthMIDIDevice()
{
	Close();
}

//==========================================================================
//
// SoftSynthMIDIDevice :: OpenStream
//
//==========================================================================

int SoftSynthMIDIDevice::OpenStream(int chunks, int flags, void (*callback)(unsigned int, void *, uint32, uint32), void *userdata)
{
	int chunksize = (SampleRate / chunks) * 4;
	// Stream = GSnd->CreateStream(FillStream, chunksize, SoundStream::Float | flags, SampleRate, this);
	// if (Stream == NULL)
	// {
	// 	return 2;
	// }

	Callback = callback;
	CallbackData = userdata;
	Tempo = 500000;
	Division = 100;
	CalcTickRate();
	return 0;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: Close
//
//==========================================================================

void SoftSynthMIDIDevice::Close()
{
	// if (Stream != NULL)
	// {
	// 	delete Stream;
	// 	Stream = NULL;
	// }
	Started = false;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: IsOpen
//
//==========================================================================

bool SoftSynthMIDIDevice::IsOpen() const
{
	// return Stream != NULL;
    return true;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: GetTechnology
//
//==========================================================================

int SoftSynthMIDIDevice::GetTechnology() const
{
	return MOD_SWSYNTH;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: SetTempo
//
//==========================================================================

int SoftSynthMIDIDevice::SetTempo(int tempo)
{
	Tempo = tempo;
	CalcTickRate();
	return 0;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: SetTimeDiv
//
//==========================================================================

int SoftSynthMIDIDevice::SetTimeDiv(int timediv)
{
	Division = timediv;
	CalcTickRate();
	return 0;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: CalcTickRate
//
// Tempo is the number of microseconds per quarter note.
// Division is the number of ticks per quarter note.
//
//==========================================================================

void SoftSynthMIDIDevice::CalcTickRate()
{
	SamplesPerTick = SampleRate / (1000000.0 / Tempo) / Division;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: Resume
//
//==========================================================================

int SoftSynthMIDIDevice::Resume()
{
	if (!Started)
	{
                if (false)//(Stream->Play(true, 1))
		{
			Started = true;
			return 0;
		}
		return 1;
	}
	return 0;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: Stop
//
//==========================================================================

void SoftSynthMIDIDevice::Stop()
{
	if (Started)
	{
            // Stream->Stop();
		Started = false;
	}
}

//==========================================================================
//
// SoftSynthMIDIDevice :: StreamOutSync
//
// This version is called from the main game thread and needs to
// synchronize with the player thread.
//
//==========================================================================

int SoftSynthMIDIDevice::StreamOutSync(MIDIHDR *header)
{
    {
        love::thread::Lock l(mutex);
	StreamOut(header);
    }
	return 0;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: StreamOut
//
// This version is called from the player thread so does not need to
// arbitrate for access to the Events pointer.
//
//==========================================================================

int SoftSynthMIDIDevice::StreamOut(MIDIHDR *header)
{
	header->lpNext = NULL;
	if (Events == NULL)
	{
		Events = header;
		NextTickIn = SamplesPerTick * *(uint32 *)header->lpData;
		Position = 0;
	}
	else
	{
		MIDIHDR **p;

		for (p = &Events; *p != NULL; p = &(*p)->lpNext)
		{ }
		*p = header;
	}
	return 0;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: Pause
//
//==========================================================================

bool SoftSynthMIDIDevice::Pause(bool paused)
{
    // if (Stream != NULL)
    // {
    // 	return Stream->SetPaused(paused);
    // }
	return true;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: PlayTick
//
// event[0] = delta time
// event[1] = unused
// event[2] = event
//
//==========================================================================

int SoftSynthMIDIDevice::PlayTick()
{
	uint32 delay = 0;

	while (delay == 0 && Events != NULL)
	{
		uint32 *event = (uint32 *)(Events->lpData + Position);
		if (MEVT_EVENTTYPE(event[2]) == MEVT_TEMPO)
		{
			SetTempo(MEVT_EVENTPARM(event[2]));
		}
		else if (MEVT_EVENTTYPE(event[2]) == MEVT_LONGMSG)
		{
			HandleLongEvent((uint8 *)&event[3], MEVT_EVENTPARM(event[2]));
		}
		else if (MEVT_EVENTTYPE(event[2]) == 0)
		{ // Short MIDI event
			int status = event[2] & 0xff;
			int parm1 = (event[2] >> 8) & 0x7f;
			int parm2 = (event[2] >> 16) & 0x7f;
			HandleEvent(status, parm1, parm2);
		}

		// Advance to next event.
		if (event[2] < 0x80000000)
		{ // Short message
			Position += 12;
		}
		else
		{ // Long message
			Position += 12 + ((MEVT_EVENTPARM(event[2]) + 3) & ~3);
		}

		// Did we use up this buffer?
		if (Position >= Events->dwBytesRecorded)
		{
			Events = Events->lpNext;
			Position = 0;

			if (Callback != NULL)
			{
				Callback(MOM_DONE, CallbackData, 0, 0);
			}
		}

		if (Events == NULL)
		{ // No more events. Just return something to keep the song playing
		  // while we wait for more to be submitted.
			return int(Division);
		}

		delay = *(uint32 *)(Events->lpData + Position);
	}
	return delay;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: ServiceStream
//
//==========================================================================

bool SoftSynthMIDIDevice::ServiceStream (void *buff, int numbytes)
{
	float *samples = (float *)buff;
	float *samples1;
	int numsamples = numbytes / sizeof(float) / 2;
	bool prev_ended = false;
	bool res = true;

	samples1 = samples;
	memset(buff, 0, numbytes);

        {
            love::thread::Lock l(mutex);

            while (Events != NULL && numsamples > 0)
            {
                    double ticky = NextTickIn;
                    int tick_in = int(NextTickIn);
                    int samplesleft = MIN(numsamples, tick_in);

                    if (samplesleft > 0)
                    {
                            ComputeOutput(samples1, samplesleft);
                            assert(NextTickIn == ticky);
                            NextTickIn -= samplesleft;
                            assert(NextTickIn >= 0);
                            numsamples -= samplesleft;
                            samples1 += samplesleft * 2;
                    }

                    if (NextTickIn < 1)
                    {
                            int next = PlayTick();
                            assert(next >= 0);
                            if (next == 0)
                            { // end of song
                                    if (numsamples > 0)
                                    {
                                            ComputeOutput(samples1, numsamples);
                                    }
                                    res = false;
                                    break;
                            }
                            else
                            {
                                    NextTickIn += SamplesPerTick * next;
                                    assert(NextTickIn >= 0);
                            }
                    }
            }

            if (Events == NULL)
            {
                    res = false;
            }
	}
	return res;
}

//==========================================================================
//
// SoftSynthMIDIDevice :: FillStream								static
//
//==========================================================================

bool SoftSynthMIDIDevice::FillStream(Data *data, void *buff, int len, void *userdata)
{
	SoftSynthMIDIDevice *device = (SoftSynthMIDIDevice *)userdata;
	return device->ServiceStream(buff, len);
}

}
}
}
