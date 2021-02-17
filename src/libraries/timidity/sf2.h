#include <vector>

typedef uint16_t SFGenerator;

struct SFRange
{
	uint8_t Lo;
	uint8_t Hi;
};

struct SFPreset
{
	char Name[21];
	uint8_t LoadOrder:7;
	uint8_t bHasGlobalZone:1;
	uint16_t Program;
	uint16_t Bank;
	uint16_t BagIndex;
	/* Don't care about library, genre, and morphology */
};

struct SFBag
{
	uint16_t GenIndex;
//	uint16_t ModIndex;		// If I am feeling ambitious, I might add support for modulators some day.
	SFRange KeyRange;
	SFRange VelRange;
	int Target;			// Either an instrument or sample index
};

struct SFInst
{
	char Name[21];
	uint8_t Pad:7;
	uint8_t bHasGlobalZone:1;
	uint16_t BagIndex;
};

struct SFSample
{
	float *InMemoryData;
	uint32_t Start;
	uint32_t End;
	uint32_t StartLoop;
	uint32_t EndLoop;
	uint32_t SampleRate;
	uint8_t  OriginalPitch;
	int8_t PitchCorrection;
	uint16_t  SampleLink;
	uint16_t  SampleType;
	char  Name[21];
};

// Sample type bit fields (all but ROM are mutually exclusive)
enum
{
	SFST_Mono = 1,
	SFST_Right = 2,
	SFST_Left = 4,
	SFST_Linked = 8,	/* SF2.04 defines this bit but not its function */
	SFST_Bad = 16384,	/* Used internally */
	SFST_ROM = 32768
};

// Generator definitions

struct SFGenList
{
	SFGenerator Oper;
	union
	{
		SFRange Range;
		int16_t Amount;
		uint16_t uAmount;
	};
};

enum
{
	GEN_startAddrsOffset,
	GEN_endAddrsOffset,
	GEN_startloopAddrsOffset,
	GEN_endloopAddrsOffset,
	GEN_startAddrsCoarseOffset,
	GEN_modLfoToPitch,
	GEN_vibLfoToPitch,
	GEN_modEnvToPitch,
	GEN_initialFilterFC,
	GEN_initialFilterQ,
	GEN_modLfoToFilterFc,
	GEN_modEnvToFilterFc,
	GEN_endAddrsCoarseOffset,
	GEN_modLfoToVolume,
	GEN_unused1,
	GEN_chorusEffectsSend,
	GEN_reverbEffectsSend,
	GEN_pan,
	GEN_unused2,
	GEN_unused3,
	GEN_unused4,
	GEN_delayModLFO,
	GEN_freqModLFO,
	GEN_delayVibLFO,
	GEN_freqVibLFO,
	GEN_delayModEnv,
	GEN_attackModEnv,
	GEN_holdModEnv,
	GEN_decayModEnv,
	GEN_sustainModEnv,
	GEN_releaseModEnv,
	GEN_keynumToModEnvHold,
	GEN_keynumToModEnvDecay,
	GEN_delayVolEnv,
	GEN_attackVolEnv,
	GEN_holdVolEnv,
	GEN_decayVolEnv,
	GEN_sustainVolEnv,
	GEN_releaseVolEnv,
	GEN_keynumToVolEnvHold,
	GEN_keynumToVolEnvDecay,
	GEN_instrument,
	GEN_reserved1,
	GEN_keyRange,
	GEN_velRange,
	GEN_startloopAddrsCoarseOffset,
	GEN_keynum,
	GEN_velocity,
	GEN_initialAttenuation,
	GEN_reserved2,
	GEN_endloopAddrsCoarseOffset,
	GEN_coarseTune,
	GEN_fineTune,
	GEN_sampleID,
	GEN_sampleModes,
	GEN_reserved3,
	GEN_scaleTuning,
	GEN_exclusiveClass,
	GEN_overridingRootKey,

	GEN_NumGenerators
};

// Modulator definitions

struct SFModulator
{
	uint16_t Index:7;
	uint16_t CC:1;
	uint16_t Dir:1;			/* 0 = min->max, 1 = max->min */
	uint16_t Polarity:1;	/* 0 = unipolar, 1 = bipolar */
	uint16_t Type:6;
};

struct SFModList
{
	SFModulator SrcOper;
	SFGenerator DestOper;
	int16_t		Amount;
	SFModulator AmtSrcOper;
	uint16_t		Transform;
};

// Modulator sources when CC is 0

enum
{
	SFMod_One = 0,			// Pseudo-controller that always has the value 1
	SFMod_NoteVelocity = 2,
	SFMod_KeyNumber = 3,
	SFMod_PolyPressure = 10,
	SFMod_ChannelPressure = 13,
	SFMod_PitchWheel = 14,
	SFMod_PitchSens = 16,
	SFMod_Link = 127
};

// Modulator types

enum
{
	SFModType_Linear,
	SFModType_Concave,		// log(fabs(value)/(max value)^2)
	SFModType_Convex,
	SFModType_Switch
};

// Modulator transforms

enum
{
	SFModTrans_Linear = 0,
	SFModTrans_Abs = 2
};

// All possible generators in a single structure

struct SFGenComposite
{
	union
	{
		SFRange keyRange;	// For normal use
		struct				// For intermediate percussion use
		{
			uint8_t drumset;
			uint8_t key;
		};
	};
	SFRange velRange;
	union
	{
		uint16_t instrument;	// At preset level
		uint16_t sampleID;		// At instrument level
	};
	int16_t modLfoToPitch;
	int16_t vibLfoToPitch;
	int16_t modEnvToPitch;
	int16_t initialFilterFc;
	int16_t initialFilterQ;
	int16_t modLfoToFilterFc;
	int16_t modEnvToFilterFc;
	int16_t modLfoToVolume;
	int16_t chorusEffectsSend;
	int16_t reverbEffectsSend;
	int16_t pan;
	int16_t delayModLFO;
	int16_t freqModLFO;
	int16_t delayVibLFO;
	int16_t freqVibLFO;
	int16_t delayModEnv;
	int16_t attackModEnv;
	int16_t holdModEnv;
	int16_t decayModEnv;
	int16_t sustainModEnv;
	int16_t releaseModEnv;
	int16_t keynumToModEnvHold;
	int16_t keynumToModEnvDecay;
	int16_t delayVolEnv;
	int16_t attackVolEnv;
	int16_t holdVolEnv;
	int16_t decayVolEnv;
	int16_t sustainVolEnv;
	int16_t releaseVolEnv;
	int16_t keynumToVolEnvHold;
	int16_t keynumToVolEnvDecay;
	int16_t initialAttenuation;
	int16_t coarseTune;
	int16_t fineTune;
	int16_t scaleTuning;

	// The following are only for instruments:
	int16_t startAddrsOffset,		startAddrsCoarseOffset;
	int16_t endAddrsOffset,		endAddrsCoarseOffset;
	int16_t startLoopAddrsOffset,	startLoopAddrsCoarseOffset;
	int16_t endLoopAddrsOffset,	endLoopAddrsCoarseOffset;
	int16_t keynum;
	int16_t velocity;
	uint16_t  sampleModes;
	int16_t exclusiveClass;
	int16_t overridingRootKey;
};

// Intermediate percussion representation

struct SFPerc
{
	SFPreset *Preset;
	SFGenComposite Generators;
	uint8_t LoadOrder;
};

// Container for all parameters from a SoundFont file

struct SFFile : public Timidity::FontFile
{
    SFFile(std::string filename);
	~SFFile();
	Timidity::Instrument *LoadInstrument(struct Timidity::Renderer *song, int drum, int bank, int program);
	Timidity::Instrument *LoadInstrumentOrder(struct Timidity::Renderer *song, int order, int drum, int bank, int program);
	void		 SetOrder(int order, int drum, int bank, int program);
	void		 SetAllOrders(int order);

	bool		 FinalStructureTest();
	void		 CheckBags();
	void		 CheckZones(int start, int stop, bool instr);
	void		 TranslatePercussions();
	void		 TranslatePercussionPreset(SFPreset *preset);
	void		 TranslatePercussionPresetZone(SFPreset *preset, SFBag *zone);

	void		 SetInstrumentGenerators(SFGenComposite *composite, int start, int stop);
	void		 AddPresetGenerators(SFGenComposite *composite, int start, int stop, SFPreset *preset);
	void		 AddPresetGenerators(SFGenComposite *composite, int start, int stop, bool gen_set[GEN_NumGenerators]);

	Timidity::Instrument *LoadPercussion(Timidity::Renderer *song, SFPerc *perc);
	Timidity::Instrument *LoadPreset(Timidity::Renderer *song, SFPreset *preset);
	void		 LoadSample(SFSample *sample);
	void		 ApplyGeneratorsToRegion(SFGenComposite *gen, SFSample *sfsamp, Timidity::Renderer *song, Timidity::Sample *sp);

	SFPreset	*Presets;
	SFBag		*PresetBags;
	SFGenList	*PresetGenerators;
	SFInst		*Instruments;
	SFBag		*InstrBags;
	SFGenList	*InstrGenerators;
	SFSample	*Samples;
        std::vector<SFPerc> Percussion;
	int			 MinorVersion;
	uint32_t		 SampleDataOffset;
	uint32_t		 SampleDataLSBOffset;
	uint32_t		 SizeSampleData;
	uint32_t		 SizeSampleDataLSB;
	int			 NumPresets;
	int			 NumPresetBags;
	int			 NumPresetGenerators;
	int			 NumInstruments;
	int			 NumInstrBags;
	int			 NumInstrGenerators;
	int			 NumSamples;
};

SFFile *ReadSF2(const char *filename, FILE *f);
