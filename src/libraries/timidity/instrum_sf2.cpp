#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <cassert>

#include "common.h"
#include "timidity.h"
#include "sf2.h"

using namespace Timidity;

#define cindex(identifier)	(uint8_t)(((size_t)&((SFGenComposite *)1)->identifier - 1) / 2)

class CIOErr {};
class CBadForm {};
class CBadVer {};

struct ListHandler
{
	uint32_t ID;
	void (*Parser)(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
};

enum
{
	GENF_InstrOnly	= 1,	// Only valid at intstrument level
	GENF_PresetOnly	= 2,	// Only valid at preset level
	GENF_Range		= 4,	// Value is a range,
	GENF_Index		= 8,	// Value is an index (aka unsigned)
	GENF_32768_Ok	= 16,	// -32768 is a valid value
};
struct GenDef
{
	short Min;
	short Max;
	uint8_t StructIndex;
	uint8_t Flags;
};

static const GenDef GenDefs[] =
{
	/*  0 */ {		  0,	32767, cindex(startAddrsOffset),			GENF_InstrOnly },
	/*  1 */ {   -32768,		0, cindex(endAddrsOffset),				GENF_InstrOnly },
	/*  2 */ {   -32768,	32767, cindex(startLoopAddrsOffset),		GENF_InstrOnly },
	/*  3 */ {   -32768,	32767, cindex(endLoopAddrsOffset),			GENF_InstrOnly },
	/*  4 */ {		  0,	32767, cindex(startAddrsCoarseOffset),		GENF_InstrOnly },
	/*  5 */ {   -12000,	12000, cindex(modLfoToPitch),				0 },
	/*  6 */ {   -12000,	12000, cindex(vibLfoToPitch),				0 },
	/*  7 */ {   -12000,	12000, cindex(modEnvToPitch),				0 },
	/*  8 */ {	   1500,	13500, cindex(initialFilterFc),				0 },
	/*  9 */ {		  0,	  960, cindex(initialFilterQ),				0 },
	/* 10 */ {   -12000,	12000, cindex(modLfoToFilterFc),			0 },
	/* 11 */ {   -12000,	12000, cindex(modEnvToFilterFc),			0 },
	/* 12 */ {   -32768,		0, cindex(endAddrsCoarseOffset),		0 },
	/* 13 */ {	   -960,	  960, cindex(modLfoToVolume),				0 },
	/* 14 */ {		  0,		0, 255 /* unused1 */,					0 },
	/* 15 */ {		  0,	 1000, cindex(chorusEffectsSend),			0 },
	/* 16 */ {		  0,	 1000, cindex(reverbEffectsSend),			0 },
	/* 17 */ {	   -500,	  500, cindex(pan),							0 },
	/* 18 */ {		  0,		0, 255 /* unused2 */,					0 },
	/* 19 */ {		  0,		0, 255 /* unused3 */,					0 },
	/* 20 */ {		  0,		0, 255 /* unused4 */,					0 },
	/* 21 */ {   -12000,	 5000, cindex(delayModLFO),					GENF_32768_Ok },
	/* 22 */ {   -16000,	 4500, cindex(freqModLFO),					0 },
	/* 23 */ {   -12000,	 5000, cindex(delayVibLFO),					GENF_32768_Ok },
	/* 24 */ {   -16000,	 4500, cindex(freqVibLFO),					0 },
	/* 25 */ {   -12000,	 5000, cindex(delayModEnv),					GENF_32768_Ok },
	/* 26 */ {   -12000,	 8000, cindex(attackModEnv),				GENF_32768_Ok },
	/* 27 */ {   -12000,	 5000, cindex(holdModEnv),					GENF_32768_Ok },
	/* 28 */ {   -12000,	 8000, cindex(decayModEnv),					0 },
	/* 29 */ {		  0,	 1000, cindex(sustainModEnv),				0 },
	/* 30 */ {   -12000,	 8000, cindex(releaseModEnv),				0 },
	/* 31 */ {	  -1200,	 1200, cindex(keynumToModEnvHold),			0 },
	/* 32 */ {	  -1200,	 1200, cindex(keynumToModEnvDecay),			0 },
	/* 33 */ {   -12000,	 5000, cindex(delayVolEnv),					GENF_32768_Ok },
	/* 34 */ {   -12000,	 8000, cindex(attackVolEnv),				GENF_32768_Ok },
	/* 35 */ {   -12000,	 5000, cindex(holdVolEnv),					GENF_32768_Ok },
	/* 36 */ {   -12000,	 5000, cindex(decayVolEnv),					0 },
	/* 37 */ {		  0,	 1440, cindex(sustainVolEnv),				0 },
	/* 38 */ {   -12000,	 8000, cindex(releaseVolEnv),				0 },
	/* 39 */ {	  -1200,	 1200, cindex(keynumToVolEnvHold),			0 },
	/* 40 */ {	  -1200,	 1200, cindex(keynumToVolEnvDecay),			0 },
	/* 41 */ {   -32768,	32767, 255 /* instrument */,				GENF_Index | GENF_PresetOnly },
	/* 42 */ {		  0,		0, 255 /* reserved1 */,					0 },
	/* 43 */ {		  0,	  127, 255 /* keyRange */,					GENF_Range },
	/* 44 */ {		  0,	  127, 255 /* velRange */,					GENF_Range },
	/* 45 */ {   -32768,	32767, cindex(startLoopAddrsCoarseOffset),	GENF_InstrOnly },
	/* 46 */ {		  0,	  127, cindex(keynum),						GENF_InstrOnly },
	/* 47 */ {		  1,	  127, cindex(velocity),					GENF_InstrOnly },
	/* 48 */ {		  0,	 1440, cindex(initialAttenuation),			0 },
	/* 49 */ {		  0,		0, 255 /* reserved2 */,					0 },
	/* 50 */ {   -32768,  32767, cindex(endLoopAddrsCoarseOffset),	GENF_InstrOnly },
	/* 51 */ {	   -120,	  120, cindex(coarseTune),					0 },
	/* 52 */ {	    -99,	   99, cindex(fineTune),					0 },
	/* 53 */ {   -32768,	32767, 255 /* sampleID */,					GENF_Index | GENF_InstrOnly },
	/* 54 */ {   -32768,	32767, cindex(sampleModes),					GENF_InstrOnly },
	/* 55 */ {		  0,		0, 255 /* reserved3 */,					0 },
	/* 56 */ {		  0,	 1200, cindex(scaleTuning),					0 },
	/* 57 */ {		  1,	  127, cindex(exclusiveClass),				GENF_InstrOnly },
	/* 58 */ {		  0,	  127, cindex(overridingRootKey),			GENF_InstrOnly },
};

static const SFGenComposite DefaultGenerators =
{
	{ { 0, 127 } },	// keyRange
	{ 0, 127 },		// velRange
	{ 0 },			// instrument/sampleID
	0,				// modLfoToPitch
	0,				// vibLfoToPitch
	0,				// modEnvToPitch
	13500,			// initialFilterFc
	0,				// initialFilterQ
	0,				// modLfoToFilterFc
	0,				// modEnvToFilterFc
	0,				// modLfoToVolume
	0,				// chorusEffectsSend
	0,				// reverbEffectsSend
	0,				// pan
	-12000,			// delayModLFO
	0,				// freqModLFO
	-12000,			// delayVibLFO
	0,				// freqVibLFO
	-12000,			// delayModEnv
	-12000,			// attackModEnv
	-12000,			// holdModEnv
	-12000,			// decayModEnv
	0,				// sustainModEnv
	-12000,			// releaseModEnv
	0,				// keynumToModEnvHold
	0,				// keynumToModEnvDecay
	-12000,			// delayVolEnv
	-12000,			// attackVolEnv
	-12000,			// holdVolEnv
	-12000,			// decayVolEnv
	0,				// sustainVolEnv
	-12000,			// releaseVolEnv
	0,				// keynumToVolEnvHold
	0,				// keynumToVolEnvDecay
	0,				// initialAttenuation
	0,				// coarseTune
	0,				// fineTune
	100,			// scaleTuning

	0, 0,			// startAddrs(Coarse)Offset
	0, 0,			// endAddrs(Coarse)Offset
	0, 0,			// startLoop(Coarse)Offset
	0, 0,			// endLoop(Coarse)Offset
	-1,				// keynum
	-1,				// velocity
	0,				// sampleModes
	0,				// exclusiveClass
	-1				// overridingRootKey
};

static void ParseIfil(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseSmpl(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseSm24(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParsePhdr(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseBag(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseMod(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseGen(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseInst(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);
static void ParseShdr(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen);

ListHandler INFOHandlers[] =
{
	{ ID_ifil, ParseIfil },
	{ 0, 0 }
};

ListHandler SdtaHandlers[] =
{
	{ ID_smpl, ParseSmpl },
	{ ID_sm24, ParseSm24 },
	{ 0, 0 }
};

ListHandler PdtaHandlers[] =
{
	{ ID_phdr, ParsePhdr },
	{ ID_pbag, ParseBag },
	{ ID_pmod, ParseMod },
	{ ID_pgen, ParseGen },
	{ ID_inst, ParseInst },
	{ ID_ibag, ParseBag },
	{ ID_imod, ParseMod },
	{ ID_igen, ParseGen },
	{ ID_shdr, ParseShdr },
	{ 0, 0 }
};

static double timecent_to_sec(int16_t timecent)
{
	if (timecent == -32768)
		return 0;
	return pow(2.0, timecent / 1200.0);
}

static int32_t to_offset(int offset)
{
	return (int32_t)offset << (7+15);
}

static int32_t calc_rate(Renderer *song, int diff, double sec)
{
	double rate;

	if(sec < 0.006)
		sec = 0.006;
	if(diff == 0)
		diff = 255;
	diff <<= (7+15);
	rate = ((double)diff / song->rate) * song->control_ratio / sec;
	return (int32_t)rate;
}


static inline uint32_t read_id(FILE *f)
{
	uint32_t id;
        int res;
	if (fread(&id, 4, 1, f) != 1)
	{
		throw CIOErr();
	}
	return id;
}

static inline int read_byte(FILE *f)
{
	uint8_t x;
	if (fread(&x, 1, 1, f) != 1)
	{
		throw CIOErr();
	}
	return x;
}

static inline int read_char(FILE *f)
{
	int8_t x;
	if (fread(&x, 1, 1, f) != 1)
	{
		throw CIOErr();
	}
	return x;
}

static inline int read_uword(FILE *f)
{
	uint16_t x;
	if (fread(&x, 2, 1, f) != 1)
	{
		throw CIOErr();
	}
	return x;
}

static inline int read_sword(FILE *f)
{
	int16_t x;
	if (fread(&x, 2, 1, f) != 1)
	{
		throw CIOErr();
	}
	return x;
}

static inline uint32_t read_dword(FILE *f)
{
	uint32_t x;
	if (fread(&x, 4, 1, f) != 1)
	{
		throw CIOErr();
	}
	return x;
}

static inline void read_name(FILE *f, char name[21])
{
    if (fread(name, 20, 1, f) != 1)
	{
		throw CIOErr();
	}
	name[20] = 0;
}

static inline void skip_chunk(FILE *f, uint32_t len)
{
	// RIFF, like IFF, adds an extra pad byte to the end of
	// odd-sized chunks so that new chunks are always on even
	// byte boundaries.
    if (fseek(f, len + (len & 1), SEEK_CUR) != 0)
	{
		throw CIOErr();
	}
}

static void check_list(FILE *f, uint32_t id, uint32_t filelen, uint32_t &chunklen)
{
	if (read_id(f) != ID_LIST)
	{
		throw CBadForm();
	}
	chunklen = read_dword(f);
	if (chunklen + 8 > filelen)
	{
		throw CBadForm();
	}
	if (read_id(f) != id)
	{
		throw CBadForm();
	}
}

static void ParseIfil(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	uint16_t major, minor;

	if (chunklen != 4)
	{
		throw CBadForm();
	}

	major = read_uword(f);
	minor = read_uword(f);

	if (major != 2)
	{
		throw CBadVer();
	}
	sf2->MinorVersion = minor;
}

static void ParseLIST(SFFile *sf2, FILE *f, uint32_t chunklen, ListHandler *handlers)
{
	ListHandler *handler;
	uint32_t id;
	uint32_t len;

	chunklen -= 4;
	while (chunklen > 0)
	{
		id = read_id(f);
		len = read_dword(f);

		if (len + 8 > chunklen)
		{
			throw CBadForm();
		}
		chunklen -= len + (len & 1) + 8;

		for (handler = handlers; handler->ID != 0; ++handler)
		{
			if (handler->ID == id && handler->Parser != NULL)
			{
				handler->Parser(sf2, f, id, len);
				break;
			}
		}
		if (handler->ID == 0)
		{
			// Skip unknown chunks
			skip_chunk(f, len);
		}
	}
}

static void ParseINFO(SFFile *sf2, FILE *f, uint32_t chunklen)
{
	sf2->MinorVersion = -1;

	ParseLIST(sf2, f, chunklen, INFOHandlers);

	if (sf2->MinorVersion < 0)
	{ // The ifil chunk must be present.
		throw CBadForm();
	}
}

static void ParseSdta(SFFile *sf2, FILE *f, uint32_t chunklen)
{
	ParseLIST(sf2, f, chunklen, SdtaHandlers);
	if (sf2->SampleDataOffset == 0)
	{
		throw CBadForm();
	}
	// Section 6.2, page 20: It is not clear if the extra pad byte for an
	// odd chunk is supposed to be included in the chunk length field.
	if (sf2->SizeSampleDataLSB != sf2->SizeSampleData &&
		sf2->SizeSampleDataLSB != sf2->SizeSampleData + (sf2->SizeSampleData & 1))
	{
		sf2->SampleDataLSBOffset = 0;
		sf2->SizeSampleDataLSB = 0;
	}
}

static void ParseSmpl(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	// Only use the first smpl chunk. (Or should we reject files with more than one?)
	if (sf2->SampleDataOffset == 0)
	{
		if (chunklen & 1)
		{ // Chunk must be an even number of bytes.
			throw CBadForm();
		}
		sf2->SampleDataOffset = ftell(f);
		sf2->SizeSampleData = chunklen >> 1;
	}
	skip_chunk(f, chunklen);
}

static void ParseSm24(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	// The sm24 chunk is ignored if the file version is < 2.04
	if (sf2->MinorVersion >= 4)
	{
		// Only use the first sm24 chunk. (Or should we reject files with more than one?)
		if (sf2->SampleDataLSBOffset == 0)
		{
			sf2->SampleDataLSBOffset = ftell(f);
			sf2->SizeSampleDataLSB = chunklen;
		}
	}
	skip_chunk(f, chunklen);
}

static void ParsePdta(SFFile *sf2, FILE *f, uint32_t chunklen)
{
	ParseLIST(sf2, f, chunklen, PdtaHandlers);
}

static void ParsePhdr(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	SFPreset *preset;

	// Section 7.2, page 22:
	//		If the PHDR sub-chunk is missing, or contains fewer than two records,
	//		or its size is not a multiple of 38 bytes, the file should be rejected
	//		as structurally unsound.
	if (chunklen < 38*2 || chunklen % 38 != 0)
	{
		throw CBadForm();
	}

	sf2->NumPresets = chunklen / 38;
	sf2->Presets = new SFPreset[sf2->NumPresets];
	preset = sf2->Presets;

	for (int i = sf2->NumPresets; i != 0; --i, ++preset)
	{
		read_name(f, preset->Name);
		preset->Program = read_uword(f);
		preset->Bank = read_uword(f);
		preset->BagIndex = read_uword(f);
		skip_chunk(f, 4*3);	// Skip library, genre, and morphology

		// Section 7.2, page 22:
		//		The preset bag indices will be monotonically increasing with
		//		increasing preset headers.
		if (preset != sf2->Presets)
		{
			if (preset->BagIndex < (preset - 1)->BagIndex)
			{
				throw CBadForm();
			}
		}
	}
}

static void ParseBag(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	SFBag *bags, *bag;
	uint16_t prev_mod = 0;
	int numbags;
	int i;

	// Section 7.3, page 22:
	//		It is always a multiple of four bytes in length, and contains one
	//		record for each preset zone plus one record for a terminal zone. ...
	//		If the PBAG sub-chunk is missing, or its size is not a multiple of
	//		four bytes, the file should be rejected as structurally unsound.
	// Section 7.7: IBAG is the same, but substitute "instrument" for "preset".
	if (chunklen & 3)
	{
		throw CBadForm();
	}

	numbags = chunklen >> 2;

	if (chunkid == ID_pbag)
	{
		if (numbags != sf2->Presets[sf2->NumPresets - 1].BagIndex + 1)
		{
			throw CBadForm();
		}
		sf2->PresetBags = bags = new SFBag[numbags];
		sf2->NumPresetBags = numbags;
	}
	else
	{
		assert(chunkid == ID_ibag);
		if (numbags != sf2->Instruments[sf2->NumInstruments - 1].BagIndex + 1)
		{
			throw CBadForm();
		}
		sf2->InstrBags = bags = new SFBag[numbags];
		sf2->NumInstrBags = numbags;
	}

	for (bag = bags, i = numbags; i != 0; --i, ++bag)
	{
		bag->GenIndex = read_uword(f);
		uint16_t mod = read_uword(f);
		// Section 7.3, page 22:
		//		If the generator or modulator indices are non-monotonic or do not
		//		match the size of the respective PGEN or PMOD sub-chunks, the file
		//		is structurally defective and should be rejected at load time.
		if (bag != bags)
		{
			if (bag->GenIndex < (bag - 1)->GenIndex || mod < prev_mod)
			{
				throw CBadForm();
			}
		}
		prev_mod = mod;
		bag->KeyRange.Lo = bag->VelRange.Lo = 0;
		bag->KeyRange.Hi = bag->VelRange.Hi = 127;
		bag->Target = -1;
	}
}

static void ParseMod(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	// Section 7.4, page 23:
	//		It [the PMOD sub-chunk] is always a multiple of ten bytes in length,
	//		and contains zero or more modulators plus a terminal record
	if (chunklen % 10 != 0)
	{
		throw CBadForm();
	}
	// We've checked the length, now ignore the chunk.
	skip_chunk(f, chunklen);
}

static void ParseGen(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	SFGenList *gens, *gen;
	int numgens;
	int i;

	// Section 7.5, page 24:
	//		If the PGEN sub-chunk is missing, or its size is not a multiple of
	//		four bytes, the file should be rejected as structurally unsound.
	if (chunklen & 3)
	{
		throw CBadForm();
	}
	numgens = chunklen >> 2;

	if (chunkid == ID_pgen)
	{
		// Section 7.3, page 22:
		//		the size of the PGEN sub-chunk in bytes will be equal to four
		//		times the terminal preset’s wGenNdx plus four.
		if (numgens != sf2->PresetBags[sf2->NumPresetBags - 1].GenIndex + 1)
		{
			throw CBadForm();
		}
		sf2->PresetGenerators = gens = new SFGenList[numgens];
		sf2->NumPresetGenerators = numgens;
	}
	else
	{
		assert(chunkid == ID_igen);
		if (numgens != sf2->InstrBags[sf2->NumInstrBags - 1].GenIndex + 1)
		{
			throw CBadForm();
		}
		sf2->InstrGenerators = gens = new SFGenList[numgens];
		sf2->NumInstrGenerators = numgens;
	}

	for (i = numgens, gen = gens; i != 0; --i, ++gen)
	{
		gen->Oper = read_uword(f);
		gen->uAmount = read_uword(f);
#ifdef __BIG_ENDIAN__
		if (gen->Oper == GEN_keyRange || gen->Oper == GEN_velRange)
		{
			// Reswap range generators
			gen->uAmount = LittleShort(gen->uAmount);
		}
#endif
	}
}

static void ParseInst(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	int i;
	SFInst *inst;

	// Section 7.6, page 25:
	//		If the INST sub-chunk is missing, contains fewer than two records, or its
	//		size is not a multiple of 22 bytes, the file should be rejected as
	//		structurally unsound.
	if (chunklen < 22*2 || chunklen % 22 != 0)
	{
		throw CBadForm();
	}

	sf2->NumInstruments = chunklen / 22;
	sf2->Instruments = inst = new SFInst[sf2->NumInstruments];
	for (i = sf2->NumInstruments; i != 0; --i, ++inst)
	{
		read_name(f, inst->Name);
		inst->BagIndex = read_uword(f);

		// Section 7.6, page 25:
		//		If the instrument bag indices are non-monotonic or if the terminal
		//		instrument’s wInstBagNdx does not match the IBAG sub-chunk size, the
		//		file is structurally defective and should be rejected at load time.
		if (inst != sf2->Instruments)
		{
			if (inst->BagIndex < (inst - 1)->BagIndex)
			{
				throw CBadForm();
			}
		}
	}
}

static void ParseShdr(SFFile *sf2, FILE *f, uint32_t chunkid, uint32_t chunklen)
{
	int i;
	SFSample *sample;

	// Section 7.10, page 29:
	// If the SHDR sub-chunk is missing, or its is size is not a multiple of 46
	// bytes the file should be rejected as structurally unsound.
	if (chunklen % 46 != 0)
	{
		throw CBadForm();
	}

	sf2->NumSamples = chunklen / 46;
	sf2->Samples = sample = new SFSample[sf2->NumSamples];
	for (i = sf2->NumSamples; i != 0; --i, ++sample)
	{
		sample->InMemoryData = NULL;
		read_name(f, sample->Name);
		sample->Start = read_dword(f);
		sample->End = read_dword(f);
		sample->StartLoop = read_dword(f);
		sample->EndLoop = read_dword(f);
		sample->SampleRate = read_dword(f);
		sample->OriginalPitch = read_byte(f);
		sample->PitchCorrection = read_char(f);
		sample->SampleLink = read_uword(f);
		sample->SampleType = read_uword(f);

		if (sample->SampleRate == 0)
		{
			// Section 7.10, page 29:
			// A value of zero is illegal. If an illegal or impractical value is
			// encountered, the nearest practical value should be used.
			sample->SampleRate = 400;
		}
		if (sample->OriginalPitch > 127)
		{
			// Section 7.10, page 29:
			// For unpitched sounds, a conventional value of 255 should be used
			// Values between 128 and 254 are illegal. Whenever an illegal value
			// or a value of 255 is encountered, the value 60 should be used.
			sample->OriginalPitch = 60;
		}

		// Clamp sample positions to the available sample data.
		sample->Start = MIN(sample->Start, sf2->SizeSampleData - 1);
		sample->End = MIN(sample->End, sf2->SizeSampleData - 1);
		sample->StartLoop = MIN(sample->StartLoop, sf2->SizeSampleData - 1);
		sample->EndLoop = MIN(sample->EndLoop, sf2->SizeSampleData - 1);

		if (sample->Start >= sample->End)
		{
			sample->SampleType |= SFST_Bad;
		}
	}
}


SFFile *ReadSF2(const char *filename, FILE *f)
{
	SFFile *sf2 = NULL;
	uint32_t filelen;
	uint32_t chunklen;

        printf("Reading SF2 %s\n", filename);

	try
	{
		// Read RIFF sfbk header
		if (read_id(f) != ID_RIFF)
		{
			return NULL;
		}
		filelen = read_dword(f);
		if (read_id(f) != ID_sfbk)
		{
			return NULL;
		}
		filelen -= 4;

		// First chunk must be an INFO LIST
		check_list(f, ID_INFO, filelen, chunklen);

		sf2 = new SFFile(filename);

		ParseINFO(sf2, f, chunklen);
		filelen -= chunklen + 8;

		// Second chunk must be a sdta LIST
		check_list(f, ID_sdta, filelen, chunklen);
		ParseSdta(sf2, f, chunklen);

		// Third chunk must be a pdta LIST
		check_list(f, ID_pdta, filelen, chunklen);
		ParsePdta(sf2, f, chunklen);

		// There should be no more chunks. If there are, we'll just ignore them rather than reject the file.
		if (!sf2->FinalStructureTest())
		{
			throw CBadForm();
		}

		sf2->CheckBags();
		sf2->TranslatePercussions();

		return sf2;
	}
	catch (CIOErr)
	{
		printf("Error reading %s: %s\n", filename, strerror(errno));
	}
	catch (CBadForm)
	{
		printf("%s is corrupted.\n", filename);
	}
	catch (CBadVer)
	{
		printf("%s is not a SoundFont version 2 file.\n", filename);
	}
	if (sf2 != NULL)
	{
		delete sf2;
	}
	return NULL;
}

SFFile::SFFile(std::string filename)
: FontFile(filename)
{
	Presets = NULL;
	PresetBags = NULL;
	PresetGenerators = NULL;
	Instruments = NULL;
	InstrBags = NULL;
	InstrGenerators = NULL;
	Samples = NULL;
	MinorVersion = 0;
	SampleDataOffset = 0;
	SampleDataLSBOffset = 0;
	SizeSampleData = 0;
	SizeSampleDataLSB = 0;
	NumPresets = 0;
	NumPresetBags = 0;
	NumPresetGenerators = 0;
	NumInstruments = 0;
	NumInstrBags = 0;
	NumInstrGenerators = 0;
	NumSamples = 0;
}

SFFile::~SFFile()
{
	if (Presets != NULL)
	{
		delete[] Presets;
	}
	if (PresetBags != NULL)
	{
		delete[] PresetBags;
	}
	if (PresetGenerators != NULL)
	{
		delete[] PresetGenerators;
	}
	if (Instruments != NULL)
	{
		delete[] Instruments;
	}
	if (InstrBags != NULL)
	{
		delete[] InstrBags;
	}
	if (InstrGenerators != NULL)
	{
		delete[] InstrGenerators;
	}
	if (Samples != NULL)
	{
		for (int i = 0; i < NumSamples; ++i)
		{
			if (Samples[i].InMemoryData != NULL)
			{
				delete[] Samples[i].InMemoryData;
			}
		}
		delete[] Samples;
	}
}

bool SFFile::FinalStructureTest()
{
	// All required chunks must be present.
	if (Presets == NULL || PresetBags == NULL || PresetGenerators == NULL ||
		Instruments == NULL || InstrBags == NULL || InstrGenerators == NULL ||
		Samples == NULL)
	{
		return false;
	}
	// What good is it if it has no sample data?
	if (SizeSampleData == 0)
	{
		return false;
	}
	return true;
}

void SFFile::SetOrder(int order, int drum, int bank, int program)
{
	if (drum)
	{
		for (int i = 0; i < NumPresets; ++i)
		{
			if (Percussion[i].Generators.drumset == bank && Percussion[i].Generators.key == program)
			{
				Percussion[i].LoadOrder = order;
			}
		}
	}
	else
	{
		for (int i = 0; i < NumPresets; ++i)
		{
			if (Presets[i].Program == program && Presets[i].Bank == bank)
			{
				Presets[i].LoadOrder = order;
			}
		}
	}
}

void SFFile::SetAllOrders(int order)
{
	for (int i = 0; i < NumPresets; ++i)
	{
		Presets[i].LoadOrder = order;
	}
	for (unsigned int i = 0; i < Percussion.size(); ++i)
	{
		Percussion[i].LoadOrder = order;
	}
}

Instrument *SFFile::LoadInstrument(Renderer *song, int drum, int bank, int program)
{
	return LoadInstrumentOrder(song, -1, drum, bank, program);
}

Instrument *SFFile::LoadInstrumentOrder(Renderer *song, int order, int drum, int bank, int program)
{
	if (drum)
	{
		for (unsigned int i = 0; i < Percussion.size(); ++i)
		{
			if ((order < 0 || Percussion[i].LoadOrder == order) &&
				Percussion[i].Generators.drumset == bank &&
				Percussion[i].Generators.key == program)
			{
				return LoadPercussion(song, &Percussion[i]);
			}
		}
	}
	else
	{
		for (int i = 0; i < NumPresets - 1; ++i)
		{
			if ((order < 0 || Presets[i].LoadOrder == order) &&
				Presets[i].Bank == bank &&
				Presets[i].Program == program)
			{
				return LoadPreset(song, &Presets[i]);
			}
		}
	}
	return NULL;
}

//===========================================================================
//
// SFFile :: CheckBags
//
// For all preset and instrument zones, extract the velocity and key ranges
// and instrument and sample targets.
//
//===========================================================================

void SFFile::CheckBags()
{
	int i;

	for (i = 0; i < NumPresets - 1; ++i)
	{
		if (Presets[i].BagIndex >= Presets[i + 1].BagIndex)
		{ // Preset is empty.
			Presets[i].Bank = ~0;
		}
		else
		{
			CheckZones(Presets[i].BagIndex, Presets[i + 1].BagIndex, 0);
			Presets[i].bHasGlobalZone = PresetBags[Presets[i].BagIndex].Target < 0;
		}
	}
	for (i = 0; i < NumInstruments - 1; ++i)
	{
		if (Instruments[i].BagIndex >= Instruments[i + 1].BagIndex)
		{ // Instrument is empty.
		}
		else
		{
			CheckZones(Instruments[i].BagIndex, Instruments[i + 1].BagIndex, 1);
			Instruments[i].bHasGlobalZone = InstrBags[Instruments[i].BagIndex].Target < 0;
		}
	}
}

//===========================================================================
//
// SFFile :: CheckZones
//
// For every zone in the bag, extract the velocity and key ranges and
// instrument and sample targets.
//
//===========================================================================

void SFFile::CheckZones(int start, int stop, bool instr)
{
	SFBag *bag;
	SFGenList *gens;
	SFGenerator terminal_gen;
	int zone_start, zone_stop;
	int i, j;

	if (!instr)
	{
		bag = PresetBags;
		gens = PresetGenerators;
		terminal_gen = GEN_instrument;
	}
	else
	{
		bag = InstrBags;
		gens = InstrGenerators;
		terminal_gen = GEN_sampleID;
	}
	for (i = start; i < stop; ++i)
	{
		zone_start = bag[i].GenIndex;
		zone_stop = bag[i + 1].GenIndex;

		if (zone_start > zone_stop)
		{
			// Skip empty zones, and mark them inaccessible.
			bag[i].KeyRange.Lo = 255;
			bag[i].KeyRange.Hi = 255;
			bag[i].VelRange.Lo = 255;
			bag[i].VelRange.Hi = 255;
			continue;
		}

		// According to the specs, if keyRange is present, it must be the first generator.
		// If velRange is present, it may only be preceded by keyRange. In real life, there
		// exist Soundfonts that violate this rule, so we need to scan every generator.

		// Preload ranges from the global zone.
		if (i != start && bag[start].Target < 0)
		{
			bag[i].KeyRange = bag[start].KeyRange;
			bag[i].VelRange = bag[start].VelRange;
		}
		for (j = zone_start; j < zone_stop; ++j)
		{
			if (gens[j].Oper == GEN_keyRange)
			{
				bag[i].KeyRange = gens[j].Range;
			}
			else if (gens[j].Oper == GEN_velRange)
			{
				bag[i].VelRange = gens[j].Range;
			}
			else if (gens[j].Oper == terminal_gen)
			{
				if (terminal_gen == GEN_instrument && gens[j].uAmount < NumInstruments - 1)
				{
					bag[i].Target = gens[j].uAmount;
				}
				else if (terminal_gen == GEN_sampleID && gens[j].uAmount < NumSamples - 1)
				{
					bag[i].Target = gens[j].uAmount;
				}
				break;
			}
		}
		if (bag[i].Target < 0 && i != start)
		{
			// Only the first zone may be targetless. If any other zones are,
			// make them inaccessible.
			bag[i].KeyRange.Lo = 255;
			bag[i].KeyRange.Hi = 255;
			bag[i].VelRange.Lo = 255;
			bag[i].VelRange.Hi = 255;
		}

		// Check for swapped ranges. (Should we fix them or ignore them?)
		if (bag[i].KeyRange.Lo > bag[i].KeyRange.Hi)
		{
			swapvalues(bag[i].KeyRange.Lo, bag[i].KeyRange.Hi);
		}
		if (bag[i].VelRange.Lo > bag[i].VelRange.Hi)
		{
			swapvalues(bag[i].VelRange.Lo, bag[i].VelRange.Hi);
		}
	}
}

//===========================================================================
//
// SFFile :: TranslatePercussions
//
// For every percussion instrument, compile a set of composite generators
// for each key, to make creating TiMidity instruments for individual
// percussion parts easier.
//
//===========================================================================

void SFFile::TranslatePercussions()
{
	for (int i = 0; i < NumPresets - 1; ++i)
	{
		if (Presets[i].Bank == 128 && Presets[i].Program < 128)
		{
			TranslatePercussionPreset(&Presets[i]);
		}
	}
}

//===========================================================================
//
// SFFile :: TranslatePercussionPreset
//
// Compile a set of composite generators for each key of this percussion
// instrument. Note that one instrument is actually an entire drumset.
//
//===========================================================================

void SFFile::TranslatePercussionPreset(SFPreset *preset)
{
	SFPerc perc;
	int i;
	bool has_global;

	perc.LoadOrder = preset->LoadOrder;
	i = preset->BagIndex;
	has_global = false;

	for (i = preset->BagIndex; i < (preset + 1)->BagIndex; ++i)
	{
		if (PresetBags[i].Target < 0)
		{ // This preset zone has no instrument.
			continue;
		}
		if (PresetBags[i].KeyRange.Lo > 127 || PresetBags[i].VelRange.Lo > 127)
		{ // This preset zone is inaccesible.
			continue;
		}
		TranslatePercussionPresetZone(preset, &PresetBags[i]);
	}
}

//===========================================================================
//
// SFFile :: TranslatePercussionPresetZone
//
// Create a composite generator set for all keys and velocity ranges in this
// preset zone that intersect with this zone's instrument.
//
//===========================================================================

void SFFile::TranslatePercussionPresetZone(SFPreset *preset, SFBag *pzone)
{
	int key, i;

	for (key = pzone->KeyRange.Lo; key <= pzone->KeyRange.Hi; ++key)
	{
		SFInst *inst = &Instruments[pzone->Target];
		for (i = inst->BagIndex; i < (inst + 1)->BagIndex; ++i)
		{
			if (InstrBags[i].Target < 0)
			{ // This instrument zone has no sample.
				continue;
			}
			if (InstrBags[i].KeyRange.Lo > key || InstrBags[i].KeyRange.Hi < key)
			{ // This instrument zone does not contain the key we want.
				continue;
			}
			if (InstrBags[i].VelRange.Lo > pzone->VelRange.Hi ||
				InstrBags[i].VelRange.Hi < pzone->VelRange.Lo)
			{ // This instrument zone does not intersect the current velocity range.
				continue;
			}
			// An intersection! Add the composite generator for this key and velocity range.
			SFPerc perc;
			perc.LoadOrder = preset->LoadOrder;
			perc.Preset = preset;
			perc.Generators = DefaultGenerators;
			if (inst->bHasGlobalZone)
			{
				SetInstrumentGenerators(&perc.Generators, InstrBags[inst->BagIndex].GenIndex, InstrBags[inst->BagIndex + 1].GenIndex);
			}
			SetInstrumentGenerators(&perc.Generators, InstrBags[i].GenIndex, InstrBags[i + 1].GenIndex);
			AddPresetGenerators(&perc.Generators, pzone->GenIndex, (pzone + 1)->GenIndex, preset);
			perc.Generators.drumset = (uint8_t)preset->Program;
			perc.Generators.key = key;
			perc.Generators.velRange.Lo = MAX(pzone->VelRange.Lo, InstrBags[i].VelRange.Lo);
			perc.Generators.velRange.Hi = MIN(pzone->VelRange.Hi, InstrBags[i].VelRange.Hi);
			perc.Generators.sampleID = InstrBags[i].Target;
			Percussion.push_back(perc);
		}
	}
}

void SFFile::SetInstrumentGenerators(SFGenComposite *composite, int start, int stop)
{
	// Proceed from first to last; later generators override earlier ones.
	SFGenList *gen = &InstrGenerators[start];
	for (int i = stop - start; i != 0; --i, ++gen)
	{
		if (gen->Oper >= GEN_NumGenerators)
		{ // Unknown generator.
			continue;
		}
		if (GenDefs[gen->Oper].StructIndex >= sizeof(SFGenComposite)/2)
		{ // Generator is either unused or ignored.
			continue;
		}
		// Set the generator
		((uint16_t *)composite)[GenDefs[gen->Oper].StructIndex] = gen->uAmount;
		if (gen->Oper == GEN_sampleID)
		{ // Anything past sampleID is ignored.
			break;
		}
	}
}

void SFFile::AddPresetGenerators(SFGenComposite *composite, int start, int stop, SFPreset *preset)
{
	bool gen_set[GEN_NumGenerators] = { false, };
	AddPresetGenerators(composite, start, stop, gen_set);
	if (preset->bHasGlobalZone)
	{
		AddPresetGenerators(composite, PresetBags[preset->BagIndex].GenIndex, PresetBags[preset->BagIndex + 1].GenIndex, gen_set);
	}
}

void SFFile::AddPresetGenerators(SFGenComposite *composite, int start, int stop, bool gen_set[GEN_NumGenerators])
{
	// Proceed from last to first; later generators override earlier ones.
	SFGenList *gen = &PresetGenerators[stop - 1];
	const GenDef *def;

	for (int i = stop - start; i != 0; --i, --gen)
	{
		if (gen->Oper >= GEN_NumGenerators)
		{ // Unknown generator.
			continue;
		}
		if (gen_set[gen->Oper])
		{ // Generator was already set.
			continue;
		}
		def = &GenDefs[gen->Oper];
		if (def->StructIndex >= sizeof(SFGenComposite)/2)
		{ // Generator is either unused or ignored.
			continue;
		}
		if (def->Flags & GENF_InstrOnly)
		{ // Generator is not valid at the preset level.
			continue;
		}
		// Add to instrument/default generator.
		int added = ((int16_t *)composite)[def->StructIndex] + gen->Amount;
		// Clamp to proper range.
		if (added <= -32768 && def->Flags & GENF_32768_Ok)
		{
			added = -32768;
		}
		else
		{
			added = clamp<int>(added, def->Min, def->Max);
		}
		((int16_t *)composite)[def->StructIndex] = added;
		gen_set[gen->Oper] = true;
		if (gen->Oper == GEN_instrument)
		{ // Anything past the instrument generator is ignored.
			break;
		}
	}
}

Instrument *SFFile::LoadPercussion(Renderer *song, SFPerc *perc)
{
	unsigned int i;
	int drumkey;
	int drumset;
	int j;

	Instrument *ip = new Instrument;
	ip->samples = 0;
	drumkey = perc->Generators.key;
	drumset = perc->Generators.drumset;

	// Count all percussion composites that match this one's key and set.
	for (i = 0; i < Percussion.size(); ++i)
	{
		if (Percussion[i].Generators.key == drumkey &&
			Percussion[i].Generators.drumset == drumset &&
			Percussion[i].Generators.sampleID < NumSamples)
		{
			SFSample *sfsamp = &Samples[Percussion[i].Generators.sampleID];
			if (sfsamp->InMemoryData == NULL)
			{
				LoadSample(sfsamp);
			}
			if (sfsamp->InMemoryData != NULL)
			{
				ip->samples++;
			}
		}
	}
	if (ip->samples == 0)
	{ // Nothing here to play.
		delete ip;
		return NULL;
	}
	ip->sample = (Sample *)safe_malloc(sizeof(Sample) * ip->samples);
	memset(ip->sample, 0, sizeof(Sample) * ip->samples);

	// Fill in Sample structure for each composite.
	for (j = 0, i = 0; i < Percussion.size(); ++i)
	{
		SFPerc *zone = &Percussion[i];
		SFGenComposite *gen = &zone->Generators;
		if (gen->key != drumkey ||
			gen->drumset != drumset ||
			gen->sampleID >= NumSamples)
		{
			continue;
		}
		SFSample *sfsamp = &Samples[gen->sampleID];
		if (sfsamp->InMemoryData == NULL)
		{
			continue;
		}
		Sample *sp = ip->sample + j++;

		// Set velocity range
		sp->low_vel = gen->velRange.Lo;
		sp->high_vel = gen->velRange.Hi;

		// Set frequency range
		sp->low_freq = note_to_freq(gen->key);
		sp->high_freq = sp->low_freq;

		ApplyGeneratorsToRegion(gen, sfsamp, song, sp);
	}
	assert(j == ip->samples);
	return ip;
}

//===========================================================================
//
// SFFile :: LoadPreset
//
//===========================================================================

Instrument *SFFile::LoadPreset(Renderer *song, SFPreset *preset)
{
	SFInst *inst;
	SFSample *sfsamp;
	SFGenComposite gen;
	int i, j, k;

	Instrument *ip = new Instrument;
	ip->samples = 0;

	// Count the number of regions we'll need.
	for (i = preset->BagIndex; i < (preset + 1)->BagIndex; ++i)
	{
		if (PresetBags[i].Target < 0)
		{ // Preset zone has no instrument.
			continue;
		}
		inst = &Instruments[PresetBags[i].Target];
		for (j = inst->BagIndex; j < (inst + 1)->BagIndex; ++j)
		{
			if (InstrBags[j].Target < 0)
			{ // Instrument zone has no sample.
				continue;
			}
			if (InstrBags[j].KeyRange.Lo <= PresetBags[i].KeyRange.Hi &&
				InstrBags[j].KeyRange.Hi >= PresetBags[i].KeyRange.Lo &&
				InstrBags[j].VelRange.Lo <= PresetBags[i].VelRange.Hi &&
				InstrBags[j].VelRange.Hi >= PresetBags[i].VelRange.Lo)
			{ // The preset and instrument zones intersect!
				sfsamp = &Samples[InstrBags[j].Target];
				if (sfsamp->InMemoryData == NULL)
				{
					LoadSample(sfsamp);
				}
				if (sfsamp->InMemoryData != NULL)
				{
					ip->samples++;
				}
			}
		}
	}
	if (ip->samples == 0)
	{ // Nothing here to play.
		delete ip;
		return NULL;
	}
	// Allocate the regions and define them
	ip->sample = (Sample *)safe_malloc(sizeof(Sample) * ip->samples);
	memset(ip->sample, 0, sizeof(Sample) * ip->samples);
	k = 0;
	for (i = preset->BagIndex; i < (preset + 1)->BagIndex; ++i)
	{
		if (PresetBags[i].Target < 0)
		{ // Preset zone has no instrument.
			continue;
		}
		inst = &Instruments[PresetBags[i].Target];
		for (j = inst->BagIndex; j < (inst + 1)->BagIndex; ++j)
		{
			if (InstrBags[j].Target < 0)
			{ // Instrument zone has no sample.
				continue;
			}
			if (InstrBags[j].KeyRange.Lo <= PresetBags[i].KeyRange.Hi &&
				InstrBags[j].KeyRange.Hi >= PresetBags[i].KeyRange.Lo &&
				InstrBags[j].VelRange.Lo <= PresetBags[i].VelRange.Hi &&
				InstrBags[j].VelRange.Hi >= PresetBags[i].VelRange.Lo)
			{ // The preset and instrument zones intersect!
				sfsamp = &Samples[InstrBags[j].Target];
				if (sfsamp->InMemoryData == NULL)
				{
					continue;
				}
				Sample *sp = ip->sample + k++;

				// Set velocity range
				sp->low_vel = MAX(InstrBags[j].VelRange.Lo, PresetBags[i].VelRange.Lo);
				sp->high_vel = MIN(InstrBags[j].VelRange.Hi, PresetBags[i].VelRange.Hi);

				// Set frequency range
				sp->low_freq = note_to_freq(MAX(InstrBags[j].KeyRange.Lo, PresetBags[i].KeyRange.Lo));
				sp->high_freq = note_to_freq(MIN(InstrBags[j].KeyRange.Hi, PresetBags[i].KeyRange.Hi));

				gen = DefaultGenerators;
				if (inst->bHasGlobalZone)
				{
					SetInstrumentGenerators(&gen, InstrBags[inst->BagIndex].GenIndex, InstrBags[inst->BagIndex + 1].GenIndex);
				}
				SetInstrumentGenerators(&gen, InstrBags[j].GenIndex, InstrBags[j + 1].GenIndex);
				AddPresetGenerators(&gen, PresetBags[i].GenIndex, PresetBags[i + 1].GenIndex, preset);
				ApplyGeneratorsToRegion(&gen, sfsamp, song, sp);
			}
		}
	}
	assert(k == ip->samples);
	return ip;
}

//===========================================================================
//
// SFFile :: ApplyGeneratorsToRegion
//
// The caller must set the key and velocity ranges. Other information for
// the TiMidity sample will be filled in using the generators given.
//
// FIXME: At least try to do something useful with every parameter.
//
//===========================================================================

void SFFile::ApplyGeneratorsToRegion(SFGenComposite *gen, SFSample *sfsamp, Renderer *song, Sample *sp)
{
	sp->type = INST_SF2;

	// Set loop and sample points
	int start, end;
	start = gen->startAddrsOffset + gen->startAddrsCoarseOffset * 32768;
	end = gen->endAddrsOffset + gen->endAddrsCoarseOffset * 32768;
	start = MAX<int>(sfsamp->Start, sfsamp->Start + start);
	end = MIN<int>(sfsamp->End, sfsamp->End + end);
	sp->loop_start = MAX<int>(start, sfsamp->StartLoop + gen->startLoopAddrsOffset + gen->startLoopAddrsCoarseOffset * 32768);
	sp->loop_end = MIN<int>(end, sfsamp->EndLoop + gen->endLoopAddrsOffset + gen->endLoopAddrsCoarseOffset * 32768);

	sp->loop_start = (sp->loop_start - start) << FRACTION_BITS;
	sp->loop_end = (sp->loop_end - start) << FRACTION_BITS;
	sp->data_length = (end - start) << FRACTION_BITS;
	sp->data = sfsamp->InMemoryData + start - sfsamp->Start;
	if (gen->overridingRootKey >= 0 && gen->overridingRootKey <= 127)
	{
		sp->scale_note = gen->overridingRootKey;
	}
	else
	{
		sp->scale_note = sfsamp->OriginalPitch;
	}
	sp->root_freq = note_to_freq(sp->scale_note);
	sp->sample_rate = sfsamp->SampleRate;
	sp->key_group = gen->exclusiveClass;

	// Set key scaling
	if (gen->keynum >= 0 && gen->keynum <= 127)
	{
		sp->scale_note = gen->keynum;
		sp->scale_factor = 0;
	}
	else if (gen->scaleTuning >= 0)
	{
		sp->scale_factor = gen->scaleTuning * 1024 / 100;
		// Does the root key also serve as the scale key? Assuming it does here.
	}
	else
	{
		sp->scale_factor = 1024;
		sp->scale_note = 60;
	}

	// Set panning
	sp->panning = gen->pan;

	// Set volume envelope
	sp->envelope.sf2.delay_vol = gen->delayVolEnv;
	sp->envelope.sf2.attack_vol = gen->attackVolEnv;
	sp->envelope.sf2.hold_vol = gen->holdVolEnv;
	sp->envelope.sf2.decay_vol = gen->decayVolEnv;
	sp->envelope.sf2.sustain_vol = gen->sustainVolEnv;
	sp->envelope.sf2.release_vol = gen->releaseVolEnv;

	// Set sample modes
	if (gen->sampleModes == 1)
	{
		sp->modes = PATCH_LOOPEN | PATCH_SUSTAIN | PATCH_NO_SRELEASE;
	}
	else if (gen->sampleModes == 3)
	{
		sp->modes = PATCH_LOOPEN | PATCH_SUSTAIN;
	}
	else
	{
		sp->modes = PATCH_SUSTAIN;
	}

	// Set tuning (in cents)
	sp->tune = gen->coarseTune * 100 + gen->fineTune;

	sp->velocity = (int8_t)gen->velocity;
	sp->initial_attenuation = gen->initialAttenuation;
}

//===========================================================================
//
// SFFile :: LoadSample
//
// Loads a sample's data and converts it from 16/24-bit to floating point.
//
//===========================================================================

void SFFile::LoadSample(SFSample *sample)
{
    FILE *fp = fopen(Filename.c_str(), "rb");
	uint32_t i;

	if (fp == NULL)
	{
		return;
	}
	sample->InMemoryData = new float[sample->End - sample->Start + 1];
	fseek(fp, SampleDataOffset + sample->Start * 2, SEEK_SET);
	// Load 16-bit sample data.
	for (i = 0; i < sample->End - sample->Start; ++i)
	{
		int16_t samp;
                fread(&samp, 2, 1, fp);
		sample->InMemoryData[i] = samp / 32768.f;
	}
	if (SampleDataLSBOffset != 0)
	{ // Load lower 8 bits of 24-bit sample data.
		fseek(fp, SampleDataLSBOffset + sample->Start, SEEK_SET);
		for (i = 0; i < sample->End - sample->Start; ++i)
		{
			uint8_t samp;
                        fread(&samp, 1, 1, fp);
			sample->InMemoryData[i] = ((((int32_t(sample->InMemoryData[i] * 32768) << 8) | samp) << 8) >> 8) / 8388608.f;
		}
	}
	// Final 0 byte is for interpolation.
	sample->InMemoryData[i] = 0;
	fclose(fp);
}
