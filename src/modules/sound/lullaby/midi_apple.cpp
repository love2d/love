#ifdef __APPLE__
#define LOVE_SUPPORT_COREAUDIO
#endif

#ifdef LOVE_SUPPORT_COREAUDIO

// HEADER FILES ------------------------------------------------------------

#include "midi.h"

#include <CoreServices/CoreServices.h>      /* ComponentDescription */
#include <AvailabilityMacros.h>

namespace love
{
namespace sound
{
namespace lullaby
{

static OSStatus GetSequenceLength(MusicSequence sequence, MusicTimeStamp *_sequenceLength)
{
    // http://lists.apple.com/archives/Coreaudio-api/2003/Jul/msg00370.html
    // figure out sequence length
    UInt32 ntracks, i;
    MusicTimeStamp sequenceLength = 0;
    OSStatus err;

    err = MusicSequenceGetTrackCount(sequence, &ntracks);
    if (err != noErr)
        return err;

    for (i = 0; i < ntracks; ++i)
    {
        MusicTrack track;
        MusicTimeStamp tracklen = 0;
        UInt32 tracklenlen = sizeof (tracklen);

        err = MusicSequenceGetIndTrack(sequence, i, &track);
        if (err != noErr)
            return err;

        err = MusicTrackGetProperty(track, kSequenceTrackProperty_TrackLength,
                                    &tracklen, &tracklenlen);
        if (err != noErr)
            return err;

        if (sequenceLength < tracklen)
            sequenceLength = tracklen;
    }

    *_sequenceLength = sequenceLength;

    return noErr;
}

/* we're looking for the sequence output audiounit. */
static OSStatus GetSequenceAudioUnit(MusicSequence sequence, AudioUnit *aunit)
{
    AUGraph graph;
    UInt32 nodecount, i;
    OSStatus err;

    err = MusicSequenceGetAUGraph(sequence, &graph);
    if (err != noErr)
        return err;

    err = AUGraphGetNodeCount(graph, &nodecount);
    if (err != noErr)
        return err;

    for (i = 0; i < nodecount; i++) {
        AUNode node;

        if (AUGraphGetIndNode(graph, i, &node) != noErr)
            continue;  /* better luck next time. */
        {
        # if !defined(AUDIO_UNIT_VERSION) || ((AUDIO_UNIT_VERSION + 0) < 1060)
         /* AUGraphAddNode () is changed to take an AudioComponentDescription*
          * desc parameter instead of a ComponentDescription* in the 10.6 SDK.
          * AudioComponentDescription is in 10.6 or newer, but it is actually
          * the same as struct ComponentDescription with 20 bytes of size and
          * the same offsets of all members, therefore, is binary compatible. */
        #   define AudioComponentDescription ComponentDescription
        # endif
            AudioComponentDescription desc;
            if (AUGraphNodeInfo(graph, node, &desc, aunit) != noErr)
                continue;
            else if (desc.componentType != kAudioUnitType_Output)
                continue;
            else if (desc.componentSubType != kAudioUnitSubType_DefaultOutput)
                continue;
        }

        return noErr;  /* found it! */
    }

    return kAUGraphErr_NodeNotFound;
}

AppleMIDIStreamer::AppleMIDIStreamer(Data *data, int bufferSize)
	: Decoder(data, bufferSize),
	LatchedVolume(128), IsLooping(true)
{
    if (NewMusicPlayer(&Player) != noErr)
	{
	    throw love::Exception("Could not create music player");
	}
    if (NewMusicSequence(&Sequence) != noErr)
	{
		throw love::Exception("Could not create music sequence");
	}
	
	DataRef = CFDataCreate(NULL, (const UInt8 *) data->getData(), data->getSize());
    if (DataRef == NULL)
	{
		throw love::Exception("Could not create MIDI data");
	}
	
    CFRetain(DataRef);
	
	if (MusicSequenceFileLoadData(Sequence, DataRef, kMusicSequenceFile_MIDIType, 0) != noErr)
	{
		throw love::Exception("Could not load MIDI data");
	}

    if (GetSequenceLength(Sequence, &EndTime) != noErr)
	{
		throw love::Exception("Could not get MIDI sequence length");
	}
	
    if (MusicPlayerSetSequence(Player, Sequence) != noErr)
	{
		throw love::Exception("Could not set MIDI sequence");
	}
}

AppleMIDIStreamer::~AppleMIDIStreamer()
{
	stop();
	
	if (Sequence != NULL)
	{
		DisposeMusicSequence(Sequence);
		Sequence = NULL;
	}
	if (Player != NULL)
	{
		DisposeMusicPlayer(Player);
		Player = NULL;
	}
	if (DataRef != NULL)
	{
		CFRelease(DataRef);
	}
}

void AppleMIDIStreamer::start()
{
    stop();
	
	MusicPlayerPreroll(Player);
    MusicPlayerSetTime(Player, 0);
    MusicPlayerStart(Player);

    GetSequenceAudioUnit(Sequence, &AudioUnit);

	setVolume(LatchedVolume);
}

void AppleMIDIStreamer::setVolume(int volume)
{
	if (AudioUnit != NULL)
	{
	    LatchedVolume = volume;
        const float floatvol = ((float) volume) / 128.0;
        AudioUnitSetParameter(AudioUnit, kHALOutputParam_Volume,
                              kAudioUnitScope_Global, 0, floatvol, 0);
	}
}

void AppleMIDIStreamer::pause()
{
}

void AppleMIDIStreamer::resume()
{
}

void AppleMIDIStreamer::stop()
{
    if (Player == NULL)
	{
	    return;
	}
	MusicPlayerStop(Player);
}

bool AppleMIDIStreamer::seek(double s)
{
    if (Player == NULL)
	{
	    return false;
	}
    return MusicPlayerSetTime(Player, (MusicTimeStamp)s);
}

bool AppleMIDIStreamer::rewind()
{
    stop();
	return seek(0);
}

bool AppleMIDIStreamer::isSeekable()
{
    return true;
}

int AppleMIDIStreamer::getChannelCount() const
{
    return 2;
}

int AppleMIDIStreamer::getBitDepth() const
{
    return 16;
}

int AppleMIDIStreamer::getSampleRate() const
{
    return 44100;
}

double AppleMIDIStreamer::getDuration()
{
    // dummy value so OpenAL doesn't stop the source spontaneously
    return 1024 * 1024;
}

bool AppleMIDIStreamer::isFinished()
{
	if (Player == NULL)
	{
		return true;
	}
	
	MusicTimeStamp currentTime = 0;
	MusicPlayerGetTime(Player, &currentTime);
	return currentTime >= EndTime || currentTime < kMusicTimeStamp_EndOfTrack;
}

bool AppleMIDIStreamer::accepts(const std::string &ext)
{
	static const std::string supported[] =
	{
		"mid"
	};

	for (int i = 0; !(supported[i].empty()); i++)
	{
		if (supported[i].compare(ext) == 0)
			return true;
	}

	return false;
}

love::sound::Decoder *AppleMIDIStreamer::clone()
{
    return new AppleMIDIStreamer(data.get(), bufferSize);
}

int AppleMIDIStreamer::decode()
{
    int read = bufferSize;

    if ((int)read < bufferSize)
        eof = true;
	
	if (IsLooping && isFinished())
	{
		seek(0);
	}

    return (int)read;
}

}
}
}
#endif
