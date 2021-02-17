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

#include "midi.h"

namespace love
{
namespace sound
{
namespace lullaby
{


// MACROS ------------------------------------------------------------------

#define MAX_TIME	(1000000/10)	// Send out 1/10 of a sec of events at a time.

#define EXPORT_LOOP_LIMIT	30		// Maximum number of times to loop when exporting a MIDI file.
// (for songs with loop controller events)

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// extern float snd_musicvolume;
// extern int snd_mididevice;

#ifdef _WIN32
	// extern UINT mididevice;
#endif

extern char MIDI_EventLengths[7];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const BYTE StaticMIDIhead[] =
{
	'M', 'T', 'h', 'd', 0, 0, 0, 6,
	0, 0, // format 0: only one track
	0, 1, // yes, there is really only one track
	0, 0, // divisions (filled in)
	'M', 'T', 'r', 'k', 0, 0, 0, 0,
	// The first event sets the tempo (filled in)
	0, 255, 81, 3, 0, 0, 0
};

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// MIDIStreamer Constructor
//
//==========================================================================

MIDIStreamer::MIDIStreamer(Data *data, int bufferSize)
	: Decoder(data, bufferSize),
#ifdef _WIN32
	PlayerThread(0), ExitEvent(0), BufferDoneEvent(0),
#endif
    MIDI(0), Division(0), InitialTempo(500000), EndQueued(0)
{
#ifdef _WIN32
	BufferDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (BufferDoneEvent == NULL)
	{
		throw love::Exception("Could not create buffer done event for MIDI playback");
	}
	ExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ExitEvent == NULL)
	{
		throw love::Exception("Could not create exit event for MIDI playback");
		return;
	}
#endif

    m_Looping = false;
}

//==========================================================================
//
// MIDIStreamer Destructor
//
//==========================================================================

MIDIStreamer::~MIDIStreamer()
{
	stop();
#ifdef _WIN32
	if (ExitEvent != NULL)
	{
		CloseHandle(ExitEvent);
	}
	if (BufferDoneEvent != NULL)
	{
		CloseHandle(BufferDoneEvent);
	}
#endif
	if (MIDI != NULL)
	{
		delete MIDI;
	}
}

//==========================================================================
//
// MIDIStreamer :: IsValid
//
//==========================================================================

bool MIDIStreamer::IsValid() const
{
#ifdef _WIN32
	return ExitEvent != NULL && Division != 0;
#else
	return Division != 0;
#endif
}

//==========================================================================
//
// MIDIStreamer :: CheckCaps
//
// Called immediately after the device is opened in case a subclass should
// want to alter its behavior depending on which device it got.
//
//==========================================================================

void MIDIStreamer::CheckCaps(int tech)
{
}

//==========================================================================
//
// MIDIStreamer :: SelectMIDIDevice									static
//
// Select the MIDI device to play on
//
//==========================================================================

EMidiDevice MIDIStreamer::SelectMIDIDevice(EMidiDevice device)
{
	/* MIDI are played as:
	- OPL:
	- if explicitly selected by $mididevice
	- when snd_mididevice  is -3 and no midi device is set for the song

	- Timidity:
	- if explicitly selected by $mididevice
	- when snd_mididevice  is -2 and no midi device is set for the song

	- Sound System:
	- if explicitly selected by $mididevice
	- when snd_mididevice  is -1 and no midi device is set for the song
	- as fallback when both OPL and Timidity failed unless snd_mididevice is >= 0

	- MMAPI (Win32 only):
	- if explicitly selected by $mididevice (non-Win32 redirects this to Sound System)
	- when snd_mididevice  is >= 0 and no midi device is set for the song
	- as fallback when both OPL and Timidity failed and snd_mididevice is >= 0
	*/

	// Choose the type of MIDI device we want.
	if (device != MDEV_DEFAULT)
	{
		return device;
	}

	return MDEV_MMAPI;
}

//==========================================================================
//
// MIDIStreamer :: CreateMIDIDevice
//
//==========================================================================

MIDIDevice *MIDIStreamer::CreateMIDIDevice(EMidiDevice devtype) const
{
    int mididevice = 1;

	switch (devtype)
	{
	case MDEV_MMAPI:
#ifdef _WIN32
		return new WinMIDIDevice(mididevice);
#endif
		assert(0);
		// Intentional fall-through for non-Windows systems.

	default:
		return NULL;
	}
}

void MIDIStreamer::start()
{
	DWORD tid;
    EMidiDevice devtype;

    love::thread::Lock l(mutex);

    if (MIDI == NULL)
    {
	    m_Status = STATE_Stopped;
	    EndQueued = 0;
	    VolumeChanged = false;
	    Restarting = true;
	    InitialPlayback = true;

        DeviceType = MDEV_MMAPI;

        devtype = SelectMIDIDevice(DeviceType);

        MIDI = CreateMIDIDevice(devtype);

#ifndef _WIN32
        assert(MIDI == NULL || MIDI->NeedThreadedCallback() == false);
#endif

        if (MIDI == NULL || 0 != MIDI->Open(Callback, this))
        {
            throw love::Exception("Could not open MIDI out device");
            if (MIDI != NULL)
            {
                delete MIDI;
                MIDI = NULL;
            }
            return;
        }

        SetMIDISubsong(0);
        CheckCaps(MIDI->GetTechnology());

        if (MIDI->Preprocess(this, m_Looping))
        {
            StartPlayback();
            if (MIDI == NULL)
            { // The MIDI file had no content and has been automatically closed.
                return;
            }
        }
    }

	if (0 != MIDI->Resume())
	{
		printf("Starting MIDI playback failed\n");
		stop();
	}
	else
	{
#ifdef _WIN32
		if (PlayerThread == NULL && MIDI->NeedThreadedCallback())
		{
			PlayerThread = CreateThread(NULL, 0, PlayerProc, this, 0, &tid);
			if (PlayerThread == NULL)
			{
				printf("Creating MIDI thread failed\n");
				stop();
			}
			else
			{
                if (m_Status == STATE_Stopped)
                {
                    m_Status = STATE_Playing;
                }
			}
		}
		else
#endif
		{
            if (m_Status == STATE_Stopped)
            {
                m_Status = STATE_Playing;
            }
		}
	}
}

//==========================================================================
//
// MIDIStreamer :: StartPlayback
//
//==========================================================================

void MIDIStreamer::StartPlayback()
{
    love::thread::Lock l(mutex);

	Precache();
	LoopLimit = 0;

	// Set time division and tempo.
	if (0 != MIDI->SetTimeDiv(Division) ||
		0 != MIDI->SetTempo(Tempo = InitialTempo))
	{
		printf("Setting MIDI stream speed failed\n");
		MIDI->Close();
		return;
	}

	MusicVolumeChanged();	// set volume to current music's properties
	OutputVolume(Volume);

#ifdef _WIN32
	ResetEvent(ExitEvent);
	ResetEvent(BufferDoneEvent);
#endif

	// Fill the initial buffers for the song.
	BufferNum = 0;
	do
	{
		int res = FillBuffer(BufferNum, MAX_EVENTS, MAX_TIME);
		if (res == SONG_MORE)
		{
			if (0 != MIDI->StreamOutSync(&Buffer[BufferNum]))
			{
				printf("Initial midiStreamOut failed\n");
				stop();
				return;
			}
			BufferNum ^= 1;
		}
		else if (res == SONG_DONE)
		{
			// Do not play super short songs that can't fill the initial two buffers.
			stop();
			return;
		}
		else
		{
			stop();
			return;
		}
	} while (BufferNum != 0);
}

//==========================================================================
//
// MIDIStreamer :: Pause
//
// "Pauses" the song by setting it to zero volume and filling subsequent
// buffers with NOPs until the song is unpaused. A MIDI device that
// supports real pauses will return true from its Pause() method.
//
//==========================================================================

void MIDIStreamer::pause()
{
	if (m_Status == STATE_Playing)
    {
        m_Status = STATE_Paused;
        love::thread::Lock l(mutex);
		if (!MIDI->Pause(true))
		{
			OutputVolume(0);
		}
	}
}

//==========================================================================
//
// MIDIStreamer :: Resume
//
// "Unpauses" a song by restoring the volume and letting subsequent
// buffers store real MIDI events again.
//
//==========================================================================

void MIDIStreamer::resume()
{
	if (m_Status == STATE_Paused)
	{
        love::thread::Lock l(mutex);
		if (!MIDI->Pause(false))
		{
			OutputVolume(Volume);
		}
		m_Status = STATE_Playing;
	}
}

//==========================================================================
//
// MIDIStreamer :: Stop
//
// Stops playback and closes the player thread and MIDI device.
//
//==========================================================================

void MIDIStreamer::stop()
{
	EndQueued = 4;
#ifdef _WIN32
	if (PlayerThread != NULL)
	{
		SetEvent(ExitEvent);
		WaitForSingleObject(PlayerThread, INFINITE);
		CloseHandle(PlayerThread);
		PlayerThread = NULL;
	}
#endif

    love::thread::Lock l(mutex);
	if (MIDI != NULL && MIDI->IsOpen())
	{
		MIDI->Stop();
		MIDI->UnprepareHeader(&Buffer[0]);
		MIDI->UnprepareHeader(&Buffer[1]);
		MIDI->Close();
	}
	if (MIDI != NULL)
	{
		delete MIDI;
		MIDI = NULL;
	}
	m_Status = STATE_Stopped;
}

bool MIDIStreamer::isFinished()
{
	if (m_Status != STATE_Stopped && (MIDI == NULL || (EndQueued != 0 && EndQueued < 4)))
	{
		stop();
	}
	if (m_Status != STATE_Stopped && !MIDI->IsOpen())
	{
		stop();
	}
	return m_Status == STATE_Stopped;
}


//==========================================================================
//
// MIDIStreamer :: MusicVolumeChanged
//
// WinMM MIDI doesn't go through the sound system, so the normal volume
// changing procedure doesn't work for it.
//
//==========================================================================

void MIDIStreamer::MusicVolumeChanged()
{
	if (MIDI != NULL && MIDI->FakeVolume())
	{
        float snd_musicvolume = 1;
        float relative_volume = 1;
		float realvolume = clamp<float>(snd_musicvolume * relative_volume, 0.f, 1.f);
		Volume = clamp<DWORD>((DWORD)(realvolume * 65535.f), 0, 65535);
	}
	else
	{
		Volume = 0xFFFF;
	}
	if (m_Status == STATE_Playing)
	{
		OutputVolume(Volume);
	}
}


//==========================================================================
//
// MIDIStreamer :: OutputVolume
//
// Signals the buffer filler to send volume change events on all channels.
//
//==========================================================================

void MIDIStreamer::OutputVolume(DWORD volume)
{
	if (MIDI != NULL && MIDI->FakeVolume())
	{
		NewVolume = volume;
		VolumeChanged = true;
	}
}

//==========================================================================
//
// MIDIStreamer :: VolumeControllerChange
//
// Some devices don't support master volume
// (e.g. the Audigy's software MIDI synth--but not its two hardware ones),
// so assume none of them do and scale channel volumes manually.
//
//==========================================================================

int MIDIStreamer::VolumeControllerChange(int channel, int volume)
{
	ChannelVolumes[channel] = volume;
	// If loops are limited, we can assume we're exporting this MIDI file,
	// so we should not adjust the volume level.
	return LoopLimit != 0 ? volume : ((volume + 1) * Volume) >> 16;
}

//==========================================================================
//
// MIDIStreamer :: Callback											Static
//
// Signals the BufferDoneEvent to prepare the next buffer. The buffer is not
// prepared in the callback directly, because it's generally still in use by
// the MIDI streamer when this callback is executed.
//
//==========================================================================

void MIDIStreamer::Callback(unsigned int uMsg, void *userdata, DWORD dwParam1, DWORD dwParam2)
{
	MIDIStreamer *self = (MIDIStreamer *)userdata;

	if (self->EndQueued >= 4)
	{
		return;
	}
	if (uMsg == MOM_DONE)
	{
#ifdef _WIN32
		if (self->PlayerThread != NULL)
		{
			SetEvent(self->BufferDoneEvent);
		}
		else
#endif
		{
			self->ServiceEvent();
		}
	}
}

//==========================================================================
//
// MIDIStreamer :: Update
//
// Called periodically to see if the player thread is still alive. If it
// isn't, stop playback now.
//
//==========================================================================

void MIDIStreamer::Update()
{
#ifdef _WIN32
	// If the PlayerThread is signalled, then it's dead.
	if (PlayerThread != NULL &&
		WaitForSingleObject(PlayerThread, 0) == WAIT_OBJECT_0)
	{
		static const char *const MMErrorCodes[] =
		{
			"No error",
			"Unspecified error",
			"Device ID out of range",
			"Driver failed enable",
			"Device already allocated",
			"Device handle is invalid",
			"No device driver present",
			"Memory allocation error",
			"Function isn't supported",
			"Error value out of range",
			"Invalid flag passed",
			"Invalid parameter passed",
			"Handle being used simultaneously on another thread",
			"Specified alias not found",
			"Bad registry database",
			"Registry key not found",
			"Registry read error",
			"Registry write error",
			"Registry delete error",
			"Registry value not found",
			"Driver does not call DriverCallback",
			"More data to be returned",
		};
		static const char *const MidiErrorCodes[] =
		{
			"MIDI header not prepared",
			"MIDI still playing something",
			"MIDI no configured instruments",
			"MIDI hardware is still busy",
			"MIDI port no longer connected",
			"MIDI invalid MIF",
			"MIDI operation unsupported with open mode",
			"MIDI through device 'eating' a message",
		};
		DWORD code = 0xABADCAFE;
		GetExitCodeThread(PlayerThread, &code);
		CloseHandle(PlayerThread);
		PlayerThread = NULL;
		printf("MIDI playback failure: ");
		if (code < countof(MMErrorCodes))
		{
			printf("%s\n", MMErrorCodes[code]);
		}
		else if (code >= MIDIERR_BASE && code < MIDIERR_BASE + countof(MidiErrorCodes))
		{
			printf("%s\n", MidiErrorCodes[code - MIDIERR_BASE]);
		}
		else
		{
			printf("%08x\n", code);
		}
		stop();
	}
#endif
}

//==========================================================================
//
// MIDIStreamer :: PlayerProc										Static
//
// Entry point for the player thread.
//
//==========================================================================

#ifdef _WIN32
DWORD WINAPI MIDIStreamer::PlayerProc(LPVOID lpParameter)
{
	return ((MIDIStreamer *)lpParameter)->PlayerLoop();
}
#endif

//==========================================================================
//
// MIDIStreamer :: PlayerLoop
//
// Services MIDI playback events.
//
//==========================================================================

#ifdef _WIN32
DWORD MIDIStreamer::PlayerLoop()
{
	HANDLE events[2] = { BufferDoneEvent, ExitEvent };
	int res;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	for (;;)
	{
		switch (WaitForMultipleObjects(2, events, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0:
			if (0 != (res = ServiceEvent()))
			{
				return res;
			}
			break;

		case WAIT_OBJECT_0 + 1:
			return 0;

		default:
			// Should not happen.
			return MMSYSERR_ERROR;
		}
	}
}
#endif

//==========================================================================
//
// MIDIStreamer :: ServiceEvent
//
// Fills the buffer that just finished playing with new events and appends
// it to the MIDI stream queue. Stops the song if playback is over. Returns
// non-zero if a problem occured and playback should stop.
//
//==========================================================================

int MIDIStreamer::ServiceEvent()
{
	int res;

    love::thread::Lock l(mutex);

    if (MIDI == NULL)
    {
        return 1;
    }

	if (EndQueued == 2)
	{
		return 0;
	}
	if (0 != (res = MIDI->UnprepareHeader(&Buffer[BufferNum])))
	{
		return res;
	}
fill:
	if (EndQueued == 1)
	{
		res = FillStopBuffer(BufferNum);
		if ((res & 3) != SONG_ERROR)
		{
			EndQueued = 2;
		}
	}
	else
	{
		res = FillBuffer(BufferNum, MAX_EVENTS, MAX_TIME);
	}
	switch (res & 3)
	{
	case SONG_MORE:
		if ((MIDI->NeedThreadedCallback() && 0 != (res = MIDI->StreamOutSync(&Buffer[BufferNum]))) ||
			(!MIDI->NeedThreadedCallback() && 0 != (res = MIDI->StreamOut(&Buffer[BufferNum]))))
		{
			return res;
		}
		else
		{
			BufferNum ^= 1;
		}
		break;

	case SONG_DONE:
		if (m_Looping)
		{
			Restarting = true;
			goto fill;
		}
		EndQueued = 1;
		break;

	default:
		return res >> 2;
	}
	return 0;
}

//==========================================================================
//
// MIDIStreamer :: FillBuffer
//
// Copies MIDI events from the MIDI file and puts them into a MIDI stream
// buffer. Filling the buffer stops when the song end is encountered, the
// buffer space is used up, or the maximum time for a buffer is hit.
//
// Can return:
// - SONG_MORE if the buffer was prepared with data.
// - SONG_DONE if the song's end was reached.
//             The buffer will never have data in this case.
// - SONG_ERROR if there was a problem preparing the buffer.
//
//==========================================================================

int MIDIStreamer::FillBuffer(int buffer_num, int max_events, DWORD max_time)
{
	if (!Restarting && CheckDone())
	{
		return SONG_DONE;
	}

	int i;
	DWORD *events = Events[buffer_num], *max_event_p;
	DWORD tot_time = 0;
	DWORD time = 0;

	// The final event is for a NOP to hold the delay from the last event.
	max_event_p = events + (max_events - 1) * 3;

	if (InitialPlayback)
	{
		InitialPlayback = false;
		// Send the full master volume SysEx message.
		events[0] = 0;								// dwDeltaTime
		events[1] = 0;								// dwStreamID
		events[2] = (MEVT_LONGMSG << 24) | 8;		// dwEvent
		events[3] = MAKE_ID(0xf0, 0x7f, 0x7f, 0x04);	// dwParms[0]
		events[4] = MAKE_ID(0x01, 0x7f, 0x7f, 0xf7);	// dwParms[1]
		events += 5;
		DoInitialSetup();
	}

	// If the volume has changed, stick those events at the start of this buffer.
	if (VolumeChanged && (m_Status != STATE_Paused || NewVolume == 0))
	{
		VolumeChanged = false;
		for (i = 0; i < 16; ++i)
		{
			BYTE courseVol = (BYTE)(((ChannelVolumes[i] + 1) * NewVolume) >> 16);
			events[0] = 0;				// dwDeltaTime
			events[1] = 0;				// dwStreamID
			events[2] = MIDI_CTRLCHANGE | i | (7 << 8) | (courseVol << 16);
			events += 3;
		}
	}

	// Play nothing while paused.
	if (m_Status == STATE_Paused)
	{
		// Be more responsive when unpausing by only playing each buffer
		// for a third of the maximum time.
		events[0] = MAX<DWORD>(1, (max_time / 3) * Division / Tempo);
		events[1] = 0;
		events[2] = MEVT_NOP << 24;
		events += 3;
	}
	else
	{
		if (Restarting)
		{
			Restarting = false;
			// Reset the tempo to the inital value.
			events[0] = 0;									// dwDeltaTime
			events[1] = 0;									// dwStreamID
			events[2] = (MEVT_TEMPO << 24) | InitialTempo;	// dwEvent
			events += 3;
			// Stop all notes in case any were left hanging.
			events = WriteStopNotes(events);
			DoRestart();
		}
		events = MakeEvents(events, max_event_p, max_time);
	}
	memset(&Buffer[buffer_num], 0, sizeof(MIDIHDR));
	Buffer[buffer_num].lpData = (LPSTR)Events[buffer_num];
	Buffer[buffer_num].dwBufferLength = DWORD((LPSTR)events - Buffer[buffer_num].lpData);
	Buffer[buffer_num].dwBytesRecorded = Buffer[buffer_num].dwBufferLength;
	if (0 != (i = MIDI->PrepareHeader(&Buffer[buffer_num])))
	{
		return SONG_ERROR | (i << 2);
	}
	return SONG_MORE;
}

//==========================================================================
//
// MIDIStreamer :: FillStopBuffer
//
// Fills a MIDI buffer with events to stop all channels.
//
//==========================================================================

int MIDIStreamer::FillStopBuffer(int buffer_num)
{
	DWORD *events = Events[buffer_num];
	int i;

	events = WriteStopNotes(events);

	// wait some tics, just so that this buffer takes some time
	events[0] = 500;
	events[1] = 0;
	events[2] = MEVT_NOP << 24;
	events += 3;

	memset(&Buffer[buffer_num], 0, sizeof(MIDIHDR));
	Buffer[buffer_num].lpData = (LPSTR)Events[buffer_num];
	Buffer[buffer_num].dwBufferLength = DWORD((LPSTR)events - Buffer[buffer_num].lpData);
	Buffer[buffer_num].dwBytesRecorded = Buffer[buffer_num].dwBufferLength;
	if (0 != (i = MIDI->PrepareHeader(&Buffer[buffer_num])))
	{
		return SONG_ERROR | (i << 2);
	}
	return SONG_MORE;
}

//==========================================================================
//
// MIDIStreamer :: WriteStopNotes
//
// Generates MIDI events to stop all notes and reset controllers on
// every channel.
//
//==========================================================================

DWORD *MIDIStreamer::WriteStopNotes(DWORD *events)
{
	for (int i = 0; i < 16; ++i)
	{
		events[0] = 0;				// dwDeltaTime
		events[1] = 0;				// dwStreamID
		events[2] = MIDI_CTRLCHANGE | i | (123 << 8);	// All notes off
		events[3] = 0;
		events[4] = 0;
		events[5] = MIDI_CTRLCHANGE | i | (121 << 8);	// Reset controllers
		events += 6;
	}
	return events;
}

//==========================================================================
//
// MIDIStreamer :: Precache
//
// Generates a list of instruments this song uses and passes them to the
// MIDI device for precaching. The default implementation here pretends to
// play the song and watches for program change events on normal channels
// and note on events on channel 10.
//
//==========================================================================

void MIDIStreamer::Precache()
{
	BYTE found_instruments[256] = { 0, };
	BYTE found_banks[256] = { 0, };
	bool multiple_banks = false;

	LoopLimit = 1;
	DoRestart();
	found_banks[0] = true;		// Bank 0 is always used.
	found_banks[128] = true;

	// Simulate playback to pick out used instruments.
	while (!CheckDone())
	{
		DWORD *event_end = MakeEvents(Events[0], &Events[0][MAX_EVENTS * 3], 1000000 * 600);
		for (DWORD *event = Events[0]; event < event_end;)
		{
			if (MEVT_EVENTTYPE(event[2]) == 0)
			{
				int command = (event[2] & 0x70);
				int channel = (event[2] & 0x0f);
				int data1 = (event[2] >> 8) & 0x7f;
				int data2 = (event[2] >> 16) & 0x7f;

				if (channel != 9 && command == (MIDI_PRGMCHANGE & 0x70))
				{
					found_instruments[data1] = true;
				}
				else if (channel == 9 && command == (MIDI_PRGMCHANGE & 0x70) && data1 != 0)
				{ // On a percussion channel, program change also serves as bank select.
					multiple_banks = true;
					found_banks[data1 | 128] = true;
				}
				else if (channel == 9 && command == (MIDI_NOTEON & 0x70) && data2 != 0)
				{
					found_instruments[data1 | 128] = true;
				}
				else if (command == (MIDI_CTRLCHANGE & 0x70) && data1 == 0 && data2 != 0)
				{
					multiple_banks = true;
					if (channel == 9)
					{
						found_banks[data2 | 128] = true;
					}
					else
					{
						found_banks[data2] = true;
					}
				}
			}
			// Advance to next event
			if (event[2] < 0x80000000)
			{ // short message
				event += 3;
			}
			else
			{ // long message
				event += 3 + ((MEVT_EVENTPARM(event[2]) + 3) >> 2);
			}
		}
	}
	DoRestart();

	// Now pack everything into a contiguous region for the PrecacheInstruments call().
	std::vector<WORD> packed;

	for (int i = 0; i < 256; ++i)
	{
		if (found_instruments[i])
		{
			WORD packnum = (i & 127) | ((i & 128) << 7);
			if (!multiple_banks)
			{
				packed.emplace_back(packnum);
			}
			else
			{ // In order to avoid having to multiplex tracks in a type 1 file,
				// precache every used instrument in every used bank, even if not
				// all combinations are actually used.
				for (int j = 0; j < 128; ++j)
				{
					if (found_banks[j + (i & 128)])
					{
						packed.emplace_back(packnum | (j << 7));
					}
				}
			}
		}
	}
	MIDI->PrecacheInstruments(&packed[0], packed.size());
}

//==========================================================================
//
// MIDIStreamer :: SetTempo
//
// Sets the tempo from a track's initial meta events. Later tempo changes
// create MEVT_TEMPO events instead.
//
//==========================================================================

void MIDIStreamer::SetTempo(int new_tempo)
{
	InitialTempo = new_tempo;
	if (NULL != MIDI && 0 == MIDI->SetTempo(new_tempo))
	{
		Tempo = new_tempo;
	}
}


//==========================================================================
//
// MIDIStreamer :: ClampLoopCount
//
// We use the XMIDI interpretation of loop count here, where 1 means it
// plays that section once (in other words, no loop) rather than the EMIDI
// interpretation where 1 means to loop it once.
//
// If LoopLimit is 1, we limit all loops, since this pass over the song is
// used to determine instruments for precaching.
//
// If LoopLimit is higher, we only limit infinite loops, since this song is
// being exported.
//
//==========================================================================

int MIDIStreamer::ClampLoopCount(int loopcount)
{
	if (LoopLimit == 0)
	{
		return loopcount;
	}
	if (LoopLimit == 1)
	{
		return 1;
	}
	if (loopcount == 0)
	{
		return LoopLimit;
	}
	return loopcount;
}

//==========================================================================
//
// MIDIStreamer :: GetStats
//
//==========================================================================

std::string MIDIStreamer::GetStats()
{
	if (MIDI == NULL)
	{
		return "No MIDI device in use.";
	}
	return MIDI->GetStats();
}

//==========================================================================
//
// MIDIStreamer :: SetSubsong
//
// Selects which subsong to play in an already-playing file. This is public.
//
//==========================================================================

bool MIDIStreamer::SetSubsong(int subsong)
{
	if (SetMIDISubsong(subsong))
	{
		stop();
		start();
		return true;
	}
	return false;
}

//==========================================================================
//
// MIDIStreamer :: SetMIDISubsong
//
// Selects which subsong to play. This is private.
//
//==========================================================================

bool MIDIStreamer::SetMIDISubsong(int subsong)
{
	return subsong == 0;
}

//==========================================================================
//
// MIDIDevice stubs.
//
//==========================================================================

MIDIDevice::MIDIDevice()
{
}

MIDIDevice::~MIDIDevice()
{
}

//==========================================================================
//
// MIDIDevice :: PrecacheInstruments
//
// The MIDIStreamer calls this method between device open and the first
// buffered stream with a list of instruments known to be used by the song.
// If the device can benefit from preloading the instruments, it can do so
// now.
//
// Each entry is packed as follows:
//   Bits 0- 6: Instrument number
//   Bits 7-13: Bank number
//   Bit    14: Select drum set if 1, tone bank if 0
//
//==========================================================================

void MIDIDevice::PrecacheInstruments(const WORD *instruments, int count)
{
}

//==========================================================================
//
// MIDIDevice :: Preprocess
//
// Gives the MIDI device a chance to do some processing with the song before
// it starts playing it. Returns true if MIDIStreamer should perform its
// standard playback startup sequence.
//
//==========================================================================

bool MIDIDevice::Preprocess(MIDIStreamer *song, bool looping)
{
	return true;
}

//==========================================================================
//
// MIDIDevice :: PrepareHeader
//
// Wrapper for MCI's midiOutPrepareHeader.
//
//==========================================================================

int MIDIDevice::PrepareHeader(MIDIHDR *header)
{
	return 0;
}

//==========================================================================
//
// MIDIDevice :: UnprepareHeader
//
// Wrapper for MCI's midiOutUnprepareHeader.
//
//==========================================================================

int MIDIDevice::UnprepareHeader(MIDIHDR *header)
{
	return 0;
}

//==========================================================================
//
// MIDIDevice :: FakeVolume
//
// Since most implementations render as a normal stream, their volume is
// controlled through the GSnd interface, not here.
//
//==========================================================================

bool MIDIDevice::FakeVolume()
{
	return false;
}

//==========================================================================
//
// MIDIDevice :: NeedThreadedCallabck
//
// Most implementations can service the callback directly rather than using
// a separate thread.
//
//==========================================================================

bool MIDIDevice::NeedThreadedCallback()
{
	return false;
}

//==========================================================================
//
// MIDIDevice :: GetStats
//
//==========================================================================

std::string MIDIDevice::GetStats()
{
	return "This MIDI device does not have any stats.";
}


bool MIDIStreamer::seek(double s)
{
    return true;
}

bool MIDIStreamer::rewind()
{
    stop();
    Restarting = true;
    return seek(0);
}

bool MIDIStreamer::isSeekable()
{
    return true;
}

int MIDIStreamer::getChannelCount() const
{
    return 2;
}

int MIDIStreamer::getBitDepth() const
{
    return 16;
}

int MIDIStreamer::getSampleRate() const
{
    return 44100;
}

double MIDIStreamer::getDuration()
{
    // dummy value so OpenAL doesn't stop the source spontaneously
    return 1024 * 1024;
}

}
}
}