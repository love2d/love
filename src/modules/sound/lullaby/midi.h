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
#define WIN32_LEAN_AND_MEAN
#define USE_WINDOWS_DWORD
#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0400
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#ifndef USE_WINDOWS_DWORD
#define USE_WINDOWS_DWORD
#endif
#include <windows.h>
#include <mmsystem.h>
#else
#define FALSE 0
#define TRUE 1
#endif

// LOVE
#include "common/Data.h"
#include "common/int.h"
#include "sound/Decoder.h"
#include "thread/threads.h"

#include <string>
#include <vector>

namespace love
{
namespace sound
{
namespace lullaby
{


void I_InitMusicWin32();
void I_ShutdownMusicWin32();

//extern float relative_volume;

//extern float timidity_mastervolume;


// A device that provides a WinMM-like MIDI streaming interface -------------

#ifndef _WIN32
struct MIDIHDR
{
	BYTE *lpData;
	DWORD dwBufferLength;
	DWORD dwBytesRecorded;
	MIDIHDR *lpNext;
};

enum
{
	MOD_MIDIPORT = 1,
	MOD_SYNTH,
	MOD_SQSYNTH,
	MOD_FMSYNTH,
	MOD_MAPPER,
	MOD_WAVETABLE,
	MOD_SWSYNTH
};

typedef BYTE *LPSTR;

#define MEVT_TEMPO			((BYTE)1)
#define MEVT_NOP			((BYTE)2)
#define MEVT_LONGMSG		((BYTE)128)

#define MEVT_EVENTTYPE(x)	((BYTE)((x) >> 24))
#define MEVT_EVENTPARM(x)   ((x) & 0xffffff)

#define MOM_DONE			969
#else
// w32api does not define these
#ifndef MOD_WAVETABLE
#define MOD_WAVETABLE   6
#define MOD_SWSYNTH     7
#endif
#endif

class MIDIStreamer;

class MIDIDevice
{
public:
	MIDIDevice();
	virtual ~MIDIDevice();

	virtual int Open(void(*callback)(unsigned int, void *, DWORD, DWORD), void *userdata) = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() const = 0;
	virtual int GetTechnology() const = 0;
	virtual int SetTempo(int tempo) = 0;
	virtual int SetTimeDiv(int timediv) = 0;
	virtual int StreamOut(MIDIHDR *data) = 0;
	virtual int StreamOutSync(MIDIHDR *data) = 0;
	virtual int Resume() = 0;
	virtual void Stop() = 0;
	virtual int PrepareHeader(MIDIHDR *data);
	virtual int UnprepareHeader(MIDIHDR *data);
	virtual bool FakeVolume();
	virtual bool Pause(bool paused) = 0;
	virtual bool NeedThreadedCallback();
	virtual void PrecacheInstruments(const WORD *instruments, int count);
	virtual bool Preprocess(MIDIStreamer *song, bool looping);
	virtual std::string GetStats();
};

// WinMM implementation of a MIDI output device -----------------------------

#ifdef _WIN32
class WinMIDIDevice : public MIDIDevice
{
public:
	WinMIDIDevice(int dev_id);
	~WinMIDIDevice();
	int Open(void(*callback)(unsigned int, void *, DWORD, DWORD), void *userdata);
	void Close();
	bool IsOpen() const;
	int GetTechnology() const;
	int SetTempo(int tempo);
	int SetTimeDiv(int timediv);
	int StreamOut(MIDIHDR *data);
	int StreamOutSync(MIDIHDR *data);
	int Resume();
	void Stop();
	int PrepareHeader(MIDIHDR *data);
	int UnprepareHeader(MIDIHDR *data);
	bool FakeVolume();
	bool NeedThreadedCallback();
	bool Pause(bool paused);
	void PrecacheInstruments(const WORD *instruments, int count);

protected:
	static void CALLBACK CallbackFunc(HMIDIOUT, UINT, DWORD_PTR, DWORD, DWORD);

	HMIDISTRM MidiOut;
	UINT DeviceID;
	DWORD SavedVolume;
	bool VolumeWorks;

	void(*Callback)(unsigned int, void *, DWORD, DWORD);
	void *CallbackData;
};
#endif

enum EMidiDevice
{
	MDEV_DEFAULT = -1,
	MDEV_MMAPI = 0,
	MDEV_OPL = 1,
	MDEV_SNDSYS = 2,
	MDEV_TIMIDITY = 3,
	MDEV_FLUIDSYNTH = 4,
	MDEV_GUS = 5,
	MDEV_WILDMIDI = 6,
};

class MIDIStreamer : public Decoder
{
public:
	MIDIStreamer(Data *data, int bufferSize);
	virtual ~MIDIStreamer();

	virtual int decode() = 0;
	bool seek(double s);
	bool rewind();
	bool isSeekable();
	int getChannelCount() const;
	int getBitDepth() const;
	int getSampleRate() const;
	double getDuration();
    bool isFinished();
    void start();
    void pause();
    void resume();
    void stop();

    // other functions
	void MusicVolumeChanged();
	void TimidityVolumeChanged();
	bool IsMIDI() const;
	bool IsValid() const;
	bool SetSubsong(int subsong);
	void Update();
	std::string GetStats();

protected:

	void OutputVolume(DWORD volume);
	int FillBuffer(int buffer_num, int max_events, DWORD max_time);
	int FillStopBuffer(int buffer_num);
	DWORD *WriteStopNotes(DWORD *events);
	int ServiceEvent();
	int VolumeControllerChange(int channel, int volume);
	int ClampLoopCount(int loopcount);
	void SetTempo(int new_tempo);
	static EMidiDevice SelectMIDIDevice(EMidiDevice devtype);
	MIDIDevice *CreateMIDIDevice(EMidiDevice devtype) const;

	static void Callback(unsigned int uMsg, void *userdata, DWORD dwParam1, DWORD dwParam2);

	// Virtuals for subclasses to override
	virtual void StartPlayback();
	virtual void CheckCaps(int tech);
	virtual void DoInitialSetup() = 0;
	virtual void DoRestart() = 0;
	virtual bool CheckDone() = 0;
	virtual void Precache();
	virtual bool SetMIDISubsong(int subsong);
	virtual DWORD *MakeEvents(DWORD *events, DWORD *max_event_p, DWORD max_time) = 0;

	enum
	{
		MAX_EVENTS = 128
	};

	enum
	{
		SONG_MORE,
		SONG_DONE,
		SONG_ERROR
	};

#ifdef _WIN32
	static DWORD WINAPI PlayerProc(LPVOID lpParameter);
	DWORD PlayerLoop();

	HANDLE PlayerThread;
	HANDLE ExitEvent;
	HANDLE BufferDoneEvent;
#endif

	MIDIDevice *MIDI;
	DWORD Events[2][MAX_EVENTS * 3];
	MIDIHDR Buffer[2];
	int BufferNum;
	int EndQueued;
	bool VolumeChanged;
	bool Restarting;
	bool InitialPlayback;
	DWORD NewVolume;
	int Division;
	int Tempo;
	int InitialTempo;
	BYTE ChannelVolumes[16];
	DWORD Volume;
	EMidiDevice DeviceType;
	bool CallbackIsThreaded;
	int LoopLimit;
	std::string DumpFilename;
	std::string Args;

	enum EState
	{
		STATE_Stopped,
		STATE_Playing,
		STATE_Paused
	} m_Status;
    bool m_Looping;

    love::thread::MutexRef mutex;
};

// MIDI file played with a MIDI stream --------------------------------------

class MIDISong2 : public MIDIStreamer
{
public:
    MIDISong2(Data *data, int bufferSize);
	~MIDISong2();

    // Decoder impl
    static bool accepts(const std::string &ext);

    love::sound::Decoder *clone();
    int decode();

protected:
	MIDISong2(const MIDISong2 *original, const char *filename, EMidiDevice type);	// file dump constructor

	void CheckCaps(int tech);
	void DoInitialSetup();
	void DoRestart();
	bool CheckDone();
	DWORD *MakeEvents(DWORD *events, DWORD *max_events_p, DWORD max_time);
	void AdvanceTracks(DWORD time);

	struct TrackInfo;

	void ProcessInitialMetaEvents();
	DWORD *SendCommand(DWORD *event, TrackInfo *track, DWORD delay, ptrdiff_t room, bool &sysex_noroom);
	TrackInfo *FindNextDue();

	BYTE *MusHeader;
	int SongLen;
	TrackInfo *Tracks;
	TrackInfo *TrackDue;
	int NumTracks;
	int Format;
	WORD DesignationMask;
};

// HMI file played with a MIDI stream ---------------------------------------

struct AutoNoteOff
{
	DWORD Delay;
	BYTE Channel, Key;
};
// Sorry, std::priority_queue, but I want to be able to modify the contents of the heap.
class NoteOffQueue : public std::vector<AutoNoteOff>
{
public:
	void AddNoteOff(DWORD delay, BYTE channel, BYTE key);
	void AdvanceTime(DWORD time);
	bool Pop(AutoNoteOff &item);

protected:
	void Heapify();

	unsigned int Parent(unsigned int i) const { return (i + 1u) / 2u - 1u; }
	unsigned int Left(unsigned int i) const { return (i + 1u) * 2u - 1u; }
	unsigned int Right(unsigned int i) const { return (i + 1u) * 2u; }
};

//==========================================================================
//
// MIN
//
// Returns the minimum of a and b.
//==========================================================================

#ifdef MIN
#undef MIN
#endif

template<class T>
inline
const T MIN(const T a, const T b)
{
	return a < b ? a : b;
}

//==========================================================================
//
// MAX
//
// Returns the maximum of a and b.
//==========================================================================

#ifdef MAX
#undef MAX
#endif

template<class T>
inline
const T MAX(const T a, const T b)
{
	return a > b ? a : b;
}

//==========================================================================
//
// clamp
//
// Clamps in to the range [min,max].
//==========================================================================

template<class T>
inline
T clamp(const T in, const T min, const T max)
{
	return in <= min ? min : in >= max ? max : in;
}

template <typename T, size_t N>
char(&_ArraySizeHelper(T(&array)[N]))[N];

#define countof( array ) (sizeof( _ArraySizeHelper( array ) ))

#ifndef LOVE_BIG_ENDIAN
#define MAKE_ID(a,b,c,d)	((DWORD)((a)|((b)<<8)|((c)<<16)|((d)<<24)))
#else
#define MAKE_ID(a,b,c,d)	((DWORD)((d)|((c)<<8)|((b)<<16)|((a)<<24)))
#endif

#define MIDI_SYSEX		((BYTE)0xF0)		 // SysEx begin
#define MIDI_SYSEXEND	((BYTE)0xF7)		 // SysEx end
#define MIDI_META		((BYTE)0xFF)		 // Meta event begin
#define MIDI_META_TEMPO ((BYTE)0x51)
#define MIDI_META_EOT	((BYTE)0x2F)		 // End-of-track
#define MIDI_META_SSPEC	((BYTE)0x7F)		 // System-specific event

#define MIDI_NOTEOFF	((BYTE)0x80)		 // + note + velocity
#define MIDI_NOTEON 	((BYTE)0x90)		 // + note + velocity
#define MIDI_POLYPRESS	((BYTE)0xA0)		 // + pressure (2 bytes)
#define MIDI_CTRLCHANGE ((BYTE)0xB0)		 // + ctrlr + value
#define MIDI_PRGMCHANGE ((BYTE)0xC0)		 // + new patch
#define MIDI_CHANPRESS	((BYTE)0xD0)		 // + pressure (1 byte)
#define MIDI_PITCHBEND	((BYTE)0xE0)		 // + pitch bend (2 bytes)

//extern float snd_musicvolume;
}
}
}