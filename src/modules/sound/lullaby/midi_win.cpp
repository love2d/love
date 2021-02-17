/*
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

#ifdef _WIN32

// HEADER FILES ------------------------------------------------------------

#include "midi.h"

#ifndef __GNUC__
#include <mmdeviceapi.h>
#endif

namespace love
{
namespace sound
{
namespace lullaby
{


// MACROS ------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static bool IgnoreMIDIVolume(UINT id);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

extern bool snd_midiprecache = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// WinMIDIDevice Contructor
//
//==========================================================================

WinMIDIDevice::WinMIDIDevice(int dev_id)
{
	DeviceID = MAX<DWORD>(dev_id, 0);
	MidiOut = 0;
}

//==========================================================================
//
// WinMIDIDevice Destructor
//
//==========================================================================

WinMIDIDevice::~WinMIDIDevice()
{
	Close();
}

//==========================================================================
//
// WinMIDIDevice :: Open
//
//==========================================================================

int WinMIDIDevice::Open(void(*callback)(UINT, void *, DWORD, DWORD), void *userdata)
{
	MMRESULT err;

	Callback = callback;
	CallbackData = userdata;
	if (MidiOut == NULL)
	{
		err = midiStreamOpen(&MidiOut, &DeviceID, 1, (DWORD_PTR)CallbackFunc, (DWORD_PTR)this, CALLBACK_FUNCTION);

		if (err == MMSYSERR_NOERROR)
		{
			if (IgnoreMIDIVolume(DeviceID))
			{
				VolumeWorks = false;
			}
			else
			{
				// Set master volume to full, if the device allows it on this interface.
				VolumeWorks = (MMSYSERR_NOERROR == midiOutGetVolume((HMIDIOUT)MidiOut, &SavedVolume));
				if (VolumeWorks)
				{
					VolumeWorks &= (MMSYSERR_NOERROR == midiOutSetVolume((HMIDIOUT)MidiOut, 0xffffffff));
				}
			}
		}
	}
	return 0;
}

//==========================================================================
//
// WinMIDIDevice :: Close
//
//==========================================================================

void WinMIDIDevice::Close()
{
	if (MidiOut != NULL)
	{
		midiStreamClose(MidiOut);
		MidiOut = NULL;
	}
}

//==========================================================================
//
// WinMIDIDevice :: IsOpen
//
//==========================================================================

bool WinMIDIDevice::IsOpen() const
{
	return MidiOut != NULL;
}

//==========================================================================
//
// WinMIDIDevice :: GetTechnology
//
//==========================================================================

int WinMIDIDevice::GetTechnology() const
{
	MIDIOUTCAPS caps;

	if (MMSYSERR_NOERROR == midiOutGetDevCaps(DeviceID, &caps, sizeof(caps)))
	{
		return caps.wTechnology;
	}
	return -1;
}

//==========================================================================
//
// WinMIDIDevice :: SetTempo
//
//==========================================================================

int WinMIDIDevice::SetTempo(int tempo)
{
	MIDIPROPTEMPO data = { sizeof(MIDIPROPTEMPO), (DWORD)tempo };
	return midiStreamProperty(MidiOut, (LPBYTE)&data, MIDIPROP_SET | MIDIPROP_TEMPO);
}

//==========================================================================
//
// WinMIDIDevice :: SetTimeDiv
//
//==========================================================================

int WinMIDIDevice::SetTimeDiv(int timediv)
{
	MIDIPROPTIMEDIV data = { sizeof(MIDIPROPTIMEDIV), (DWORD)timediv };
	return midiStreamProperty(MidiOut, (LPBYTE)&data, MIDIPROP_SET | MIDIPROP_TIMEDIV);
}

//==========================================================================
//
// WinMIDIDevice :: Resume
//
//==========================================================================

int WinMIDIDevice::Resume()
{
	return midiStreamRestart(MidiOut);
}

//==========================================================================
//
// WinMIDIDevice :: Stop
//
//==========================================================================

void WinMIDIDevice::Stop()
{
	midiStreamStop(MidiOut);
	midiOutReset((HMIDIOUT)MidiOut);
	if (VolumeWorks)
	{
		midiOutSetVolume((HMIDIOUT)MidiOut, SavedVolume);
	}
}

//==========================================================================
//
// WinMIDIDevice :: PrecacheInstruments
//
// Each entry is packed as follows:
//   Bits 0- 6: Instrument number
//   Bits 7-13: Bank number
//   Bit    14: Select drum set if 1, tone bank if 0
//
// My old GUS PnP needed the instruments to be preloaded, or it would miss
// some notes the first time through a song. I doubt any modern
// hardware has this problem, but since I'd already written the code for
// ZDoom 1.22 and below, I'm resurrecting it now for completeness, since I'm
// using preloading for the internal Timidity.
//
// NOTETOSELF: Why did I never notice the midiOutCache(Drum)Patches calls
// before now? Should I switch to them? This code worked on my GUS, but
// using the APIs intended for caching might be better.
//
//==========================================================================

void WinMIDIDevice::PrecacheInstruments(const WORD *instruments, int count)
{
	// Setting snd_midiprecache to false disables this precaching, since it
	// does involve sleeping for more than a miniscule amount of time.
	if (!snd_midiprecache)
	{
		return;
	}
	BYTE bank[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int i, chan;

	for (i = 0, chan = 0; i < count; ++i)
	{
		int instr = instruments[i] & 127;
		int banknum = (instruments[i] >> 7) & 127;
		int percussion = instruments[i] >> 14;

		if (percussion)
		{
			if (bank[9] != banknum)
			{
				midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_CTRLCHANGE | 9 | (0 << 8) | (banknum << 16));
				bank[9] = banknum;
			}
			midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_NOTEON | 9 | ((instruments[i] & 0x7f) << 8) | (1 << 16));
		}
		else
		{ // Melodic
			if (bank[chan] != banknum)
			{
				midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_CTRLCHANGE | 9 | (0 << 8) | (banknum << 16));
				bank[chan] = banknum;
			}
			midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_PRGMCHANGE | chan | (instruments[i] << 8));
			midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_NOTEON | chan | (60 << 8) | (1 << 16));
			if (++chan == 9)
			{ // Skip the percussion channel
				chan = 10;
			}
		}
		// Once we've got an instrument playing on each melodic channel, sleep to give
		// the driver time to load the instruments. Also do this for the final batch
		// of instruments.
		if (chan == 16 || i == count - 1)
		{
			Sleep(250);
			for (chan = 15; chan-- != 0;)
			{
				// Turn all notes off
				midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_CTRLCHANGE | chan | (123 << 8));
			}
			// And now chan is back at 0, ready to start the cycle over.
		}
	}
	// Make sure all channels are set back to bank 0.
	for (i = 0; i < 16; ++i)
	{
		if (bank[i] != 0)
		{
			midiOutShortMsg((HMIDIOUT)MidiOut, MIDI_CTRLCHANGE | 9 | (0 << 8) | (0 << 16));
		}
	}
}

//==========================================================================
//
// WinMIDIDevice :: Pause
//
// Some docs claim pause is unreliable and can cause the stream to stop
// functioning entirely. Truth or fiction?
//
//==========================================================================

bool WinMIDIDevice::Pause(bool paused)
{
    if (paused)
    {
        midiStreamPause(MidiOut);
    }
    else
    {
        midiStreamRestart(MidiOut);
    }
	return false;
}

//==========================================================================
//
// WinMIDIDevice :: StreamOut
//
//==========================================================================

int WinMIDIDevice::StreamOut(MIDIHDR *header)
{
	return midiStreamOut(MidiOut, header, sizeof(MIDIHDR));
}

//==========================================================================
//
// WinMIDIDevice :: StreamOutSync
//
//==========================================================================

int WinMIDIDevice::StreamOutSync(MIDIHDR *header)
{
	return midiStreamOut(MidiOut, header, sizeof(MIDIHDR));
}

//==========================================================================
//
// WinMIDIDevice :: PrepareHeader
//
//==========================================================================

int WinMIDIDevice::PrepareHeader(MIDIHDR *header)
{
	return midiOutPrepareHeader((HMIDIOUT)MidiOut, header, sizeof(MIDIHDR));
}

//==========================================================================
//
// WinMIDIDevice :: UnprepareHeader
//
//==========================================================================

int WinMIDIDevice::UnprepareHeader(MIDIHDR *header)
{
	return midiOutUnprepareHeader((HMIDIOUT)MidiOut, header, sizeof(MIDIHDR));
}

//==========================================================================
//
// WinMIDIDevice :: FakeVolume
//
// Because there are too many MIDI devices out there that don't support
// global volume changes, fake the volume for all of them.
//
//==========================================================================

bool WinMIDIDevice::FakeVolume()
{
	return true;
}

//==========================================================================
//
// WinMIDIDevice :: NeedThreadedCallback
//
// When using the MM system, the callback can't yet touch the buffer, so
// the real processing needs to happen in a different thread.
//
//==========================================================================

bool WinMIDIDevice::NeedThreadedCallback()
{
	return true;
}

//==========================================================================
//
// WinMIDIDevice :: CallbackFunc									static
//
//==========================================================================

void CALLBACK WinMIDIDevice::CallbackFunc(HMIDIOUT hOut, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	WinMIDIDevice *self = (WinMIDIDevice *)dwInstance;
	if (self->Callback != NULL)
	{
		self->Callback(uMsg, self->CallbackData, dwParam1, dwParam2);
	}
}

//==========================================================================
//
// IgnoreMIDIVolume
//
// Should we ignore this MIDI device's volume control even if it works?
//
// Under Windows Vista and up, when using the standard "Microsoft GS
// Wavetable Synth", midiOutSetVolume() will affect the application's audio
// session volume rather than the volume for just the MIDI stream. At first,
// I thought I could get around this by enumerating the streams in the
// audio session to find the MIDI device's stream to set its volume
// manually, but there doesn't appear to be any way to enumerate the
// individual streams in a session. Consequently, we'll just assume the MIDI
// device gets created at full volume like we want. (Actual volume changes
// are done by sending MIDI channel volume messages to the stream, not
// through midiOutSetVolume().)
//
// This is using VC++'s __uuidof extension instead of the the CLSID_ and
// IID_ definitions because I couldn't figure out why it wasn't finding them
// when linking, and __uuidof circumvents that problem. I'd also be
// surprised if w32api includes any WASAPI stuff any time soon, so it's no
// big loss making this VC++-specific for the time being
//
//==========================================================================

static bool IgnoreMIDIVolume(UINT id)
{
#ifndef __GNUC__
	MIDIOUTCAPS caps;

	if (MMSYSERR_NOERROR == midiOutGetDevCaps(id, &caps, sizeof(caps)))
	{
		// The Microsoft GS Wavetable Synth advertises itself as MOD_SWSYNTH with a VOLUME control.
		// If the one we're using doesn't match that, we don't need to bother checking the name.
		if (caps.wTechnology == MOD_SWSYNTH && (caps.dwSupport & MIDICAPS_VOLUME))
		{
			if (strncmp(caps.szPname, "Microsoft GS", 12) == 0)
			{
				IMMDeviceEnumerator *enumerator;

				// Now try to create an IMMDeviceEnumerator interface. If it succeeds,
				// we know we're using the new audio stack introduced with Vista and
				// should ignore this MIDI device's volume control.
				if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
					__uuidof(IMMDeviceEnumerator), (void**)&enumerator))
					&& enumerator != NULL)
				{
					enumerator->Release();
					return true;
				}
			}
		}
	}
#endif
	return false;
}

}
}
}
#endif