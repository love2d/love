/* GF1 Patch definition: */
enum
{
	HEADER_SIZE					= 12,
	ID_SIZE						= 10,
	DESC_SIZE					= 60,
	RESERVED_SIZE				= 40,
	PATCH_HEADER_RESERVED_SIZE	= 36,
	LAYER_RESERVED_SIZE			= 40,
	PATCH_DATA_RESERVED_SIZE	= 36,
	INST_NAME_SIZE				= 16,
	ENVELOPES					= 6,
	MAX_LAYERS					= 4
};
#define GF1_HEADER_TEXT			"GF1PATCH110"

#ifdef _MSC_VER
#pragma pack(push, 1)
#define GCC_PACKED
#else
#define GCC_PACKED __attribute__((__packed__))
#endif

struct GF1PatchHeader
{
	char Header[HEADER_SIZE];
	char GravisID[ID_SIZE];		/* Id = "ID#000002" */
	char Description[DESC_SIZE];
	uint8_t Instruments;
	uint8_t Voices;
	uint8_t Channels;
	uint16_t WaveForms;
	uint16_t MasterVolume;
	uint32_t DataSize;
	uint8_t Reserved[PATCH_HEADER_RESERVED_SIZE];
} GCC_PACKED;

struct GF1InstrumentData
{
	uint16_t Instrument;
	char InstrumentName[INST_NAME_SIZE];
	int  InstrumentSize;
	uint8_t Layers;
	uint8_t Reserved[RESERVED_SIZE];
} GCC_PACKED;

struct GF1LayerData
{
	uint8_t LayerDuplicate;
	uint8_t Layer;
	int  LayerSize;
	uint8_t Samples;
	uint8_t Reserved[LAYER_RESERVED_SIZE];
} GCC_PACKED;

struct GF1PatchData
{
	char  WaveName[7];
	uint8_t  Fractions;
	int   WaveSize;
	int   StartLoop;
	int   EndLoop;
	uint16_t  SampleRate;
	int   LowFrequency;
	int   HighFrequency;
	int   RootFrequency;
	int16_t Tune;
	uint8_t  Balance;
	uint8_t  EnvelopeRate[ENVELOPES];
	uint8_t  EnvelopeOffset[ENVELOPES];
	uint8_t  TremoloSweep;
	uint8_t  TremoloRate;
	uint8_t  TremoloDepth;
	uint8_t  VibratoSweep;
	uint8_t  VibratoRate;
	uint8_t  VibratoDepth;
	uint8_t  Modes;
	int16_t ScaleFrequency;
	uint16_t  ScaleFactor;		/* From 0 to 2048 or 0 to 2 */
	uint8_t  Reserved[PATCH_DATA_RESERVED_SIZE];
} GCC_PACKED;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef GCC_PACKED
