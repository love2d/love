/*

	TiMidity -- Experimental MIDI to WAVE converter
	Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	instrum_dls.c

*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <climits>

#include "timidity.h"
#include "common.h"

#define __Sound_SetError(x)

namespace Timidity
{

/*-------------------------------------------------------------------------*/
/* * * * * * * * * * * * * * * * * load_riff.h * * * * * * * * * * * * * * */
/*-------------------------------------------------------------------------*/
struct RIFF_Chunk
{
	RIFF_Chunk()
	{
		memset(this, 0, sizeof(*this));
	}
	~RIFF_Chunk()
	{
		// data is not freed here because it may be owned by a parent chunk
		if (child != NULL)
		{
			delete child;
		}
		if (next != NULL)
		{
			delete next;
		}
	}

	uint32_t	magic;
	uint32_t	length;
	uint32_t	subtype;
	uint8_t	*data;
	RIFF_Chunk *child;
	RIFF_Chunk *next;
};

RIFF_Chunk *LoadRIFF(FILE *src);
void FreeRIFF(RIFF_Chunk *chunk);
void PrintRIFF(RIFF_Chunk *chunk, int level);
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------*/
/* * * * * * * * * * * * * * * * * load_riff.c * * * * * * * * * * * * * * */
/*-------------------------------------------------------------------------*/
#define RIFF        MAKE_ID('R','I','F','F')
#define LIST        MAKE_ID('L','I','S','T')

static bool ChunkHasSubType(uint32_t magic)
{
	return (magic == RIFF || magic == LIST);
}

static int ChunkHasSubChunks(uint32_t magic)
{
	return (magic == RIFF || magic == LIST);
}

static void LoadSubChunks(RIFF_Chunk *chunk, uint8_t *data, uint32_t left)
{
	uint8_t *subchunkData;
	uint32_t subchunkDataLen;

	while ( left > 8 ) {
		RIFF_Chunk *child = new RIFF_Chunk;
		RIFF_Chunk *next, *prev = NULL;
		for ( next = chunk->child; next; next = next->next ) {
			prev = next;
		}
		if ( prev ) {
			prev->next = child;
		} else {
			chunk->child = child;
		}

		child->magic = *(uint32_t *)data;
		data += 4;
		left -= 4;
		child->length = *(uint32_t *)data; // LittleLong(*(uint32_t *)data);
		data += 4;
		left -= 4;
		child->data = data;

		if ( child->length > left ) {
			child->length = left;
		}

		subchunkData = child->data;
		subchunkDataLen = child->length;
		if ( ChunkHasSubType(child->magic) && subchunkDataLen >= 4 ) {
			child->subtype = *(uint32_t *)subchunkData;
			subchunkData += 4;
			subchunkDataLen -= 4;
		}
		if ( ChunkHasSubChunks(child->magic) ) {
			LoadSubChunks(child, subchunkData, subchunkDataLen);
		}

		data += child->length + (child->length & 1);
		left -= child->length + (child->length & 1);
	}
}

RIFF_Chunk *LoadRIFF(FILE *src)
{
	RIFF_Chunk *chunk;
	uint8_t *subchunkData;
	uint32_t subchunkDataLen;

	/* Allocate the chunk structure */
	chunk = new RIFF_Chunk;

	/* Make sure the file is in RIFF format */
	fread(&chunk->magic, 4, 1, src);
	fread(&chunk->length, 4, 1, src);
	chunk->length = chunk->length; // LittleLong(chunk->length);
	if ( chunk->magic != RIFF ) {
		__Sound_SetError("Not a RIFF file");
		delete chunk;
		return NULL;
	}
	chunk->data = (uint8_t *)malloc(chunk->length);
	if ( chunk->data == NULL ) {
		__Sound_SetError(ERR_OUT_OF_MEMORY);
		delete chunk;
		return NULL;
	}
	if ( fread(chunk->data, chunk->length, 1, src) != 1 ) {
		__Sound_SetError(ERR_IO_ERROR);
		FreeRIFF(chunk);
		return NULL;
	}
	subchunkData = chunk->data;
	subchunkDataLen = chunk->length;
	if ( ChunkHasSubType(chunk->magic) && subchunkDataLen >= 4 ) {
		chunk->subtype = *(uint32_t *)subchunkData;
		subchunkData += 4;
		subchunkDataLen -= 4;
	}
	if ( ChunkHasSubChunks(chunk->magic) ) {
		LoadSubChunks(chunk, subchunkData, subchunkDataLen);
	}
	return chunk;
}

void FreeRIFF(RIFF_Chunk *chunk)
{
	free(chunk->data);
	delete chunk;
}

void PrintRIFF(RIFF_Chunk *chunk, int level)
{
	static char prefix[128];

	if ( level == sizeof(prefix)-1 ) {
		return;
	}
	if ( level > 0 ) {
		prefix[(level-1)*2] = ' ';
		prefix[(level-1)*2+1] = ' ';
	}
	prefix[level*2] = '\0';
	printf("%sChunk: %c%c%c%c (%d bytes)", prefix,
		((chunk->magic >>  0) & 0xFF),
		((chunk->magic >>  8) & 0xFF),
		((chunk->magic >> 16) & 0xFF),
		((chunk->magic >> 24) & 0xFF), chunk->length);
	if ( chunk->subtype ) {
		printf(" subtype: %c%c%c%c",
			((chunk->subtype >>  0) & 0xFF),
			((chunk->subtype >>  8) & 0xFF),
			((chunk->subtype >> 16) & 0xFF),
			((chunk->subtype >> 24) & 0xFF));
	}
	printf("\n");
	if ( chunk->child ) {
		printf("%s{\n", prefix);
		PrintRIFF(chunk->child, level + 1);
		printf("%s}\n", prefix);
	}
	if ( chunk->next ) {
		PrintRIFF(chunk->next, level);
	}
	if ( level > 0 ) {
		prefix[(level-1)*2] = '\0';
	}
}

#ifdef TEST_MAIN_RIFF

main(int argc, char *argv[])
{
	int i;
	for ( i = 1; i < argc; ++i ) {
		RIFF_Chunk *chunk;
		SDL_RWops *src = SDL_RWFromFile(argv[i], "rb");
		if ( !src ) {
			fprintf(stderr, "Couldn't open %s: %s", argv[i], SDL_GetError());
			continue;
		}
		chunk = LoadRIFF(src);
		if ( chunk ) {
			PrintRIFF(chunk, 0);
			FreeRIFF(chunk);
		} else {
			fprintf(stderr, "Couldn't load %s: %s\n", argv[i], SDL_GetError());
		}
		SDL_RWclose(src);
	}
}

#endif // TEST_MAIN
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------*/
/* * * * * * * * * * * * * * * * * load_dls.h  * * * * * * * * * * * * * * */
/*-------------------------------------------------------------------------*/
/* This code is based on the DLS spec version 1.1, available at:
http://www.midi.org/about-midi/dls/dlsspec.shtml
*/

/* Some typedefs so the public dls headers don't need to be modified */
#define FAR
typedef int16_t	SHORT;
typedef uint16_t	USHORT;
typedef int32_t	LONG;
typedef uint32_t	ULONG;
typedef uint8_t	BYTE;
typedef uint32_t	DWORD;
#define mmioFOURCC	MAKE_ID
#define DEFINE_GUID(A, B, C, E, F, G, H, I, J, K, L, M)

#include "dls1.h"
#include "dls2.h"

struct WaveFMT
{
	uint16_t wFormatTag;
	uint16_t wChannels;
	uint32_t dwSamplesPerSec;
	uint32_t dwAvgBytesPerSec;
	uint16_t wBlockAlign;
	uint16_t wBitsPerSample;
};

struct DLS_Wave
{
	WaveFMT *format;
	uint8_t *data;
	uint32_t length;
	WSMPL *wsmp;
	WLOOP *wsmp_loop;
};

struct DLS_Region
{
	RGNHEADER *header;
	WAVELINK *wlnk;
	WSMPL *wsmp;
	WLOOP *wsmp_loop;
	CONNECTIONLIST *art;
	CONNECTION *artList;
};

struct DLS_Instrument
{
	const char *name;
	INSTHEADER *header;
	DLS_Region *regions;
	CONNECTIONLIST *art;
	CONNECTION *artList;
};

struct DLS_Data
{
	RIFF_Chunk *chunk;

	uint32_t cInstruments;
	DLS_Instrument *instruments;

	POOLTABLE *ptbl;
	POOLCUE *ptblList;
	DLS_Wave *waveList;

	const char *name;
	const char *artist;
	const char *copyright;
	const char *comments;
};

DLS_Data *LoadDLS(FILE *src);
void FreeDLS(DLS_Data *chunk);
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------*/
/* * * * * * * * * * * * * * * * * load_dls.c  * * * * * * * * * * * * * * */
/*-------------------------------------------------------------------------*/

#define FOURCC_LIST    mmioFOURCC('L','I','S','T')
#define FOURCC_FMT     mmioFOURCC('f','m','t',' ')
#define FOURCC_DATA    mmioFOURCC('d','a','t','a')
#define FOURCC_INFO    mmioFOURCC('I','N','F','O')
#define FOURCC_IARL    mmioFOURCC('I','A','R','L')
#define FOURCC_IART    mmioFOURCC('I','A','R','T')
#define FOURCC_ICMS    mmioFOURCC('I','C','M','S')
#define FOURCC_ICMT    mmioFOURCC('I','C','M','T')
#define FOURCC_ICOP    mmioFOURCC('I','C','O','P')
#define FOURCC_ICRD    mmioFOURCC('I','C','R','D')
#define FOURCC_IENG    mmioFOURCC('I','E','N','G')
#define FOURCC_IGNR    mmioFOURCC('I','G','N','R')
#define FOURCC_IKEY    mmioFOURCC('I','K','E','Y')
#define FOURCC_IMED    mmioFOURCC('I','M','E','D')
#define FOURCC_INAM    mmioFOURCC('I','N','A','M')
#define FOURCC_IPRD    mmioFOURCC('I','P','R','D')
#define FOURCC_ISBJ    mmioFOURCC('I','S','B','J')
#define FOURCC_ISFT    mmioFOURCC('I','S','F','T')
#define FOURCC_ISRC    mmioFOURCC('I','S','R','C')
#define FOURCC_ISRF    mmioFOURCC('I','S','R','F')
#define FOURCC_ITCH    mmioFOURCC('I','T','C','H')


static void FreeRegions(DLS_Instrument *instrument)
{
	if ( instrument->regions ) {
		free(instrument->regions);
	}
}

static void AllocRegions(DLS_Instrument *instrument)
{
	int datalen = (instrument->header->cRegions * sizeof(DLS_Region));
	FreeRegions(instrument);
	instrument->regions = (DLS_Region *)malloc(datalen);
	if ( instrument->regions ) {
		memset(instrument->regions, 0, datalen);
	}
}

static void FreeInstruments(DLS_Data *data)
{
	if ( data->instruments ) {
		uint32_t i;
		for ( i = 0; i < data->cInstruments; ++i ) {
			FreeRegions(&data->instruments[i]);
		}
		free(data->instruments);
	}
}

static void AllocInstruments(DLS_Data *data)
{
	int datalen = (data->cInstruments * sizeof(DLS_Instrument));
	FreeInstruments(data);
	data->instruments = (DLS_Instrument *)malloc(datalen);
	if ( data->instruments ) {
		memset(data->instruments, 0, datalen);
	}
}

static void FreeWaveList(DLS_Data *data)
{
	if ( data->waveList ) {
		free(data->waveList);
	}
}

static void AllocWaveList(DLS_Data *data)
{
	int datalen = (data->ptbl->cCues * sizeof(DLS_Wave));
	FreeWaveList(data);
	data->waveList = (DLS_Wave *)malloc(datalen);
	if ( data->waveList ) {
		memset(data->waveList, 0, datalen);
	}
}

static void Parse_colh(DLS_Data *data, RIFF_Chunk *chunk)
{
	data->cInstruments = *(uint32_t *)chunk->data; // LittleLong(*(uint32_t *)chunk->data);
	AllocInstruments(data);
}

static void Parse_insh(DLS_Data *data, RIFF_Chunk *chunk, DLS_Instrument *instrument)
{
	INSTHEADER *header = (INSTHEADER *)chunk->data;
	header->cRegions = header->cRegions; // LittleLong(header->cRegions);
	header->Locale.ulBank = header->Locale.ulBank; // LittleLong(header->Locale.ulBank);
	header->Locale.ulInstrument = header->Locale.ulInstrument; // LittleLong(header->Locale.ulInstrument);
	instrument->header = header;
	AllocRegions(instrument);
}

static void Parse_rgnh(DLS_Data *data, RIFF_Chunk *chunk, DLS_Region *region)
{
	RGNHEADER *header = (RGNHEADER *)chunk->data;
	header->RangeKey.usLow = header->RangeKey.usLow; // LittleShort(header->RangeKey.usLow);
	header->RangeKey.usHigh = header->RangeKey.usHigh; // LittleShort(header->RangeKey.usHigh);
	header->RangeVelocity.usLow = header->RangeVelocity.usLow; // LittleShort(header->RangeVelocity.usLow);
	header->RangeVelocity.usHigh = header->RangeVelocity.usHigh; // LittleShort(header->RangeVelocity.usHigh);
	header->fusOptions = header->fusOptions; // LittleShort(header->fusOptions);
	header->usKeyGroup = header->usKeyGroup; // LittleShort(header->usKeyGroup);
	region->header = header;
}

static void Parse_wlnk(DLS_Data *data, RIFF_Chunk *chunk, DLS_Region *region)
{
	WAVELINK *wlnk = (WAVELINK *)chunk->data;
	wlnk->fusOptions = wlnk->fusOptions; // LittleShort(wlnk->fusOptions);
	wlnk->usPhaseGroup = wlnk->usPhaseGroup; // LittleShort(wlnk->usPhaseGroup);
	wlnk->ulChannel = (unsigned int)wlnk->ulChannel; // LittleLong((unsigned int)wlnk->ulChannel);
	wlnk->ulTableIndex = (unsigned int)wlnk->ulTableIndex; // LittleLong((unsigned int)wlnk->ulTableIndex);
	region->wlnk = wlnk;
}

static void Parse_wsmp(DLS_Data *data, RIFF_Chunk *chunk, WSMPL **wsmp_ptr, WLOOP **wsmp_loop_ptr)
{
	uint32_t i;
	WSMPL *wsmp = (WSMPL *)chunk->data;
	WLOOP *loop;
	wsmp->cbSize = wsmp->cbSize; // LittleLong(wsmp->cbSize);
	wsmp->usUnityNote = wsmp->usUnityNote; // LittleShort(wsmp->usUnityNote);
	wsmp->sFineTune = wsmp->sFineTune; // LittleShort(wsmp->sFineTune);
	wsmp->lAttenuation = wsmp->lAttenuation; // LittleLong(wsmp->lAttenuation);
	wsmp->fulOptions = wsmp->fulOptions; // LittleLong(wsmp->fulOptions);
	wsmp->cSampleLoops = wsmp->cSampleLoops; // LittleLong(wsmp->cSampleLoops);
	loop = (WLOOP *)((uint8_t *)chunk->data + wsmp->cbSize);
	*wsmp_ptr = wsmp;
	*wsmp_loop_ptr = loop;
	for ( i = 0; i < wsmp->cSampleLoops; ++i ) {
		loop->cbSize = loop->cbSize; // LittleLong(loop->cbSize);
		loop->ulType = loop->ulType; // LittleLong(loop->ulType);
		loop->ulStart = loop->ulStart; // LittleLong(loop->ulStart);
		loop->ulLength = loop->ulLength; // LittleLong(loop->ulLength);
		++loop;
	}
}

static void Parse_art(DLS_Data *data, RIFF_Chunk *chunk, CONNECTIONLIST **art_ptr, CONNECTION **artList_ptr)
{
	uint32_t i;
	CONNECTIONLIST *art = (CONNECTIONLIST *)chunk->data;
	CONNECTION *artList;
	art->cbSize = art->cbSize; // LittleLong(art->cbSize);
	art->cConnections = art->cConnections; // LittleLong(art->cConnections);
	artList = (CONNECTION *)((uint8_t *)chunk->data + art->cbSize);
	*art_ptr = art;
	*artList_ptr = artList;
	for ( i = 0; i < art->cConnections; ++i ) {
		artList->usSource = artList->usSource; // LittleShort(artList->usSource);
		artList->usControl = artList->usControl; // LittleShort(artList->usControl);
		artList->usDestination = artList->usDestination; // LittleShort(artList->usDestination);
		artList->usTransform = artList->usTransform; // LittleShort(artList->usTransform);
		artList->lScale = artList->lScale; // LittleLong(artList->lScale);
		++artList;
	}
}

static void Parse_lart(DLS_Data *data, RIFF_Chunk *chunk, CONNECTIONLIST **conn_ptr, CONNECTION **connList_ptr)
{
	/* FIXME: This only supports one set of connections */
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_ART1:
			case FOURCC_ART2:
				Parse_art(data, chunk, conn_ptr, connList_ptr);
				return;
		}
	}
}

static void Parse_rgn(DLS_Data *data, RIFF_Chunk *chunk, DLS_Region *region)
{
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_RGNH:
				Parse_rgnh(data, chunk, region);
				break;
			case FOURCC_WLNK:
				Parse_wlnk(data, chunk, region);
				break;
			case FOURCC_WSMP:
				Parse_wsmp(data, chunk, &region->wsmp, &region->wsmp_loop);
				break;
			case FOURCC_LART:
			case FOURCC_LAR2:
				Parse_lart(data, chunk, &region->art, &region->artList);
				break;
		}
	}
}

static void Parse_lrgn(DLS_Data *data, RIFF_Chunk *chunk, DLS_Instrument *instrument)
{
	uint32_t region = 0;
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_RGN:
			case FOURCC_RGN2:
				if ( region < instrument->header->cRegions ) {
					Parse_rgn(data, chunk, &instrument->regions[region++]);
				}
				break;
		}
	}
}

static void Parse_INFO_INS(DLS_Data *data, RIFF_Chunk *chunk, DLS_Instrument *instrument)
{
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_INAM: /* Name */
				instrument->name = (const char *)chunk->data;
				break;
		}
	}
}

static void Parse_ins(DLS_Data *data, RIFF_Chunk *chunk, DLS_Instrument *instrument)
{
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_INSH:
				Parse_insh(data, chunk, instrument);
				break;
			case FOURCC_LRGN:
				Parse_lrgn(data, chunk, instrument);
				break;
			case FOURCC_LART:
			case FOURCC_LAR2:
				Parse_lart(data, chunk, &instrument->art, &instrument->artList);
				break;
			case FOURCC_INFO:
				Parse_INFO_INS(data, chunk, instrument);
				break;
		}
	}
}

static void Parse_lins(DLS_Data *data, RIFF_Chunk *chunk)
{
	uint32_t instrument = 0;
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_INS:
				if ( instrument < data->cInstruments ) {
					Parse_ins(data, chunk, &data->instruments[instrument++]);
				}
				break;
		}
	}
}

static void Parse_ptbl(DLS_Data *data, RIFF_Chunk *chunk)
{
	uint32_t i;
	POOLTABLE *ptbl = (POOLTABLE *)chunk->data;
	ptbl->cbSize = ptbl->cbSize; // LittleLong(ptbl->cbSize);
	ptbl->cCues = ptbl->cCues; // LittleLong(ptbl->cCues);
	data->ptbl = ptbl;
	data->ptblList = (POOLCUE *)((uint8_t *)chunk->data + ptbl->cbSize);
	for ( i = 0; i < ptbl->cCues; ++i ) {
		data->ptblList[i].ulOffset = data->ptblList[i].ulOffset; // LittleLong(data->ptblList[i].ulOffset);
	}
	AllocWaveList(data);
}

static void Parse_fmt(DLS_Data *data, RIFF_Chunk *chunk, DLS_Wave *wave)
{
	WaveFMT *fmt = (WaveFMT *)chunk->data;
	fmt->wFormatTag = fmt->wFormatTag; // LittleShort(fmt->wFormatTag);
	fmt->wChannels = fmt->wChannels; // LittleShort(fmt->wChannels);
	fmt->dwSamplesPerSec = fmt->dwSamplesPerSec; // LittleLong(fmt->dwSamplesPerSec);
	fmt->dwAvgBytesPerSec = fmt->dwAvgBytesPerSec; // LittleLong(fmt->dwAvgBytesPerSec);
	fmt->wBlockAlign = fmt->wBlockAlign; // LittleShort(fmt->wBlockAlign);
	fmt->wBitsPerSample = fmt->wBitsPerSample; // LittleShort(fmt->wBitsPerSample);
	wave->format = fmt;
}

static void Parse_data(DLS_Data *data, RIFF_Chunk *chunk, DLS_Wave *wave)
{
	wave->data = chunk->data;
	wave->length = chunk->length;
}

static void Parse_wave(DLS_Data *data, RIFF_Chunk *chunk, DLS_Wave *wave)
{
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_FMT:
				Parse_fmt(data, chunk, wave);
				break;
			case FOURCC_DATA:
				Parse_data(data, chunk, wave);
				break;
			case FOURCC_WSMP:
				Parse_wsmp(data, chunk, &wave->wsmp, &wave->wsmp_loop);
				break;
		}
	}
}

static void Parse_wvpl(DLS_Data *data, RIFF_Chunk *chunk)
{
	uint32_t wave = 0;
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_wave:
				if ( wave < data->ptbl->cCues ) {
					Parse_wave(data, chunk, &data->waveList[wave++]);
				}
				break;
		}
	}
}

static void Parse_INFO_DLS(DLS_Data *data, RIFF_Chunk *chunk)
{
	for ( chunk = chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_IARL: /* Archival Location */
				break;
			case FOURCC_IART: /* Artist */
				data->artist = (const char *)chunk->data;
				break;
			case FOURCC_ICMS: /* Commisioned */
				break;
			case FOURCC_ICMT: /* Comments */
				data->comments = (const char *)chunk->data;
				break;
			case FOURCC_ICOP: /* Copyright */
				data->copyright = (const char *)chunk->data;
				break;
			case FOURCC_ICRD: /* Creation Date */
				break;
			case FOURCC_IENG: /* Engineer */
				break;
			case FOURCC_IGNR: /* Genre */
				break;
			case FOURCC_IKEY: /* Keywords */
				break;
			case FOURCC_IMED: /* Medium */
				break;
			case FOURCC_INAM: /* Name */
				data->name = (const char *)chunk->data;
				break;
			case FOURCC_IPRD: /* Product */
				break;
			case FOURCC_ISBJ: /* Subject */
				break;
			case FOURCC_ISFT: /* Software */
				break;
			case FOURCC_ISRC: /* Source */
				break;
			case FOURCC_ISRF: /* Source Form */
				break;
			case FOURCC_ITCH: /* Technician */
				break;
		}
	}
}

DLS_Data *LoadDLS(FILE *src)
{
	RIFF_Chunk *chunk;
	DLS_Data *data = (DLS_Data *)malloc(sizeof(*data));
	if ( !data ) {
		__Sound_SetError(ERR_OUT_OF_MEMORY);
		return NULL;
	}
	memset(data, 0, sizeof(*data));

	data->chunk = LoadRIFF(src);
	if ( !data->chunk ) {
		FreeDLS(data);
		return NULL;
	}

	for ( chunk = data->chunk->child; chunk; chunk = chunk->next ) {
		uint32_t magic = (chunk->magic == FOURCC_LIST) ? chunk->subtype : chunk->magic;
		switch(magic) {
			case FOURCC_COLH:
				Parse_colh(data, chunk);
				break;
			case FOURCC_LINS:
				Parse_lins(data, chunk);
				break;
			case FOURCC_PTBL:
				Parse_ptbl(data, chunk);
				break;
			case FOURCC_WVPL:
				Parse_wvpl(data, chunk);
				break;
			case FOURCC_INFO:
				Parse_INFO_DLS(data, chunk);
				break;
		}
	}
	return data;
}

void FreeDLS(DLS_Data *data)
{
	if ( data->chunk ) {
		FreeRIFF(data->chunk);
	}
	FreeInstruments(data);
	FreeWaveList(data);
	free(data);
}

static const char *SourceToString(USHORT usSource)
{
	static char unknown[32];
	switch(usSource) {
		case CONN_SRC_NONE:
			return "NONE";
		case CONN_SRC_LFO:
			return "LFO";
		case CONN_SRC_KEYONVELOCITY:
			return "KEYONVELOCITY";
		case CONN_SRC_KEYNUMBER:
			return "KEYNUMBER";
		case CONN_SRC_EG1:
			return "EG1";
		case CONN_SRC_EG2:
			return "EG2";
		case CONN_SRC_PITCHWHEEL:
			return "PITCHWHEEL";
		case CONN_SRC_CC1:
			return "CC1";
		case CONN_SRC_CC7:
			return "CC7";
		case CONN_SRC_CC10:
			return "CC10";
		case CONN_SRC_CC11:
			return "CC11";
		case CONN_SRC_POLYPRESSURE:
			return "POLYPRESSURE";
		case CONN_SRC_CHANNELPRESSURE:
			return "CHANNELPRESSURE";
		case CONN_SRC_VIBRATO:
			return "VIBRATO";
		case CONN_SRC_MONOPRESSURE:
			return "MONOPRESSURE";
		case CONN_SRC_CC91:
			return "CC91";
		case CONN_SRC_CC93:
			return "CC93";
		default:
			return unknown;
	}
}

static const char *TransformToString(USHORT usTransform)
{
	static char unknown[32];
	switch (usTransform) {
		case CONN_TRN_NONE:
			return "NONE";
		case CONN_TRN_CONCAVE:
			return "CONCAVE";
		case CONN_TRN_CONVEX:
			return "CONVEX";
		case CONN_TRN_SWITCH:
			return "SWITCH";
		default:
			return unknown;
	}
}

static const char *DestinationToString(USHORT usDestination)
{
	static char unknown[32];
	switch (usDestination) {
		case CONN_DST_NONE:
			return "NONE";
		case CONN_DST_ATTENUATION:
			return "ATTENUATION";
		case CONN_DST_PITCH:
			return "PITCH";
		case CONN_DST_PAN:
			return "PAN";
		case CONN_DST_LFO_FREQUENCY:
			return "LFO_FREQUENCY";
		case CONN_DST_LFO_STARTDELAY:
			return "LFO_STARTDELAY";
		case CONN_DST_EG1_ATTACKTIME:
			return "EG1_ATTACKTIME";
		case CONN_DST_EG1_DECAYTIME:
			return "EG1_DECAYTIME";
		case CONN_DST_EG1_RELEASETIME:
			return "EG1_RELEASETIME";
		case CONN_DST_EG1_SUSTAINLEVEL:
			return "EG1_SUSTAINLEVEL";
		case CONN_DST_EG2_ATTACKTIME:
			return "EG2_ATTACKTIME";
		case CONN_DST_EG2_DECAYTIME:
			return "EG2_DECAYTIME";
		case CONN_DST_EG2_RELEASETIME:
			return "EG2_RELEASETIME";
		case CONN_DST_EG2_SUSTAINLEVEL:
			return "EG2_SUSTAINLEVEL";
		case CONN_DST_KEYNUMBER:
			return "KEYNUMBER";
		case CONN_DST_LEFT:
			return "LEFT";
		case CONN_DST_RIGHT:
			return "RIGHT";
		case CONN_DST_CENTER:
			return "CENTER";
		case CONN_DST_LEFTREAR:
			return "LEFTREAR";
		case CONN_DST_RIGHTREAR:
			return "RIGHTREAR";
		case CONN_DST_LFE_CHANNEL:
			return "LFE_CHANNEL";
		case CONN_DST_CHORUS:
			return "CHORUS";
		case CONN_DST_REVERB:
			return "REVERB";
		case CONN_DST_VIB_FREQUENCY:
			return "VIB_FREQUENCY";
		case CONN_DST_VIB_STARTDELAY:
			return "VIB_STARTDELAY";
		case CONN_DST_EG1_DELAYTIME:
			return "EG1_DELAYTIME";
		case CONN_DST_EG1_HOLDTIME:
			return "EG1_HOLDTIME";
		case CONN_DST_EG1_SHUTDOWNTIME:
			return "EG1_SHUTDOWNTIME";
		case CONN_DST_EG2_DELAYTIME:
			return "EG2_DELAYTIME";
		case CONN_DST_EG2_HOLDTIME:
			return "EG2_HOLDTIME";
		case CONN_DST_FILTER_CUTOFF:
			return "FILTER_CUTOFF";
		case CONN_DST_FILTER_Q:
			return "FILTER_Q";
		default:
			return unknown;
	}
}

static void PrintArt(const char *type, CONNECTIONLIST *art, CONNECTION *artList)
{
	uint32_t i;
	printf("%s Connections:\n", type);
	for ( i = 0; i < art->cConnections; ++i ) {
		printf("  Source: %s, Control: %s, Destination: %s, Transform: %s, Scale: %d\n",
			SourceToString(artList[i].usSource),
			SourceToString(artList[i].usControl),
			DestinationToString(artList[i].usDestination),
			TransformToString(artList[i].usTransform),
			artList[i].lScale);
	}
}

static void PrintWave(DLS_Wave *wave, uint32_t index)
{
	WaveFMT *format = wave->format;
	if ( format ) {
		printf("  Wave %u: Format: %hu, %hu channels, %u Hz, %hu bits (length = %u)\n", index, format->wFormatTag, format->wChannels, format->dwSamplesPerSec, format->wBitsPerSample, wave->length);
	}
	if ( wave->wsmp ) {
		uint32_t i;
		printf("    wsmp->usUnityNote = %hu\n", wave->wsmp->usUnityNote);
		printf("    wsmp->sFineTune = %hd\n", wave->wsmp->sFineTune);
		printf("    wsmp->lAttenuation = %d\n", wave->wsmp->lAttenuation);
		printf("    wsmp->fulOptions = 0x%8.8x\n", wave->wsmp->fulOptions);
		printf("    wsmp->cSampleLoops = %u\n", wave->wsmp->cSampleLoops);
		for ( i = 0; i < wave->wsmp->cSampleLoops; ++i ) {
			WLOOP *loop = &wave->wsmp_loop[i];
			printf("    Loop %u:\n", i);
			printf("      ulStart = %u\n", loop->ulStart);
			printf("      ulLength = %u\n", loop->ulLength);
		}
	}
}

static void PrintRegion(DLS_Region *region, uint32_t index)
{
	printf("  Region %u:\n", index);
	if ( region->header ) {
		printf("    RangeKey = { %hu - %hu }\n", region->header->RangeKey.usLow, region->header->RangeKey.usHigh);
		printf("    RangeVelocity = { %hu - %hu }\n", region->header->RangeVelocity.usLow, region->header->RangeVelocity.usHigh);
		printf("    fusOptions = 0x%4.4hx\n", region->header->fusOptions);
		printf("    usKeyGroup = %hu\n", region->header->usKeyGroup);
	}
	if ( region->wlnk ) {
		printf("    wlnk->fusOptions = 0x%4.4hx\n", region->wlnk->fusOptions);
		printf("    wlnk->usPhaseGroup = %hu\n", region->wlnk->usPhaseGroup);
		printf("    wlnk->ulChannel = %u\n", region->wlnk->ulChannel);
		printf("    wlnk->ulTableIndex = %u\n", region->wlnk->ulTableIndex);
	}
	if ( region->wsmp ) {
		uint32_t i;
		printf("    wsmp->usUnityNote = %hu\n", region->wsmp->usUnityNote);
		printf("    wsmp->sFineTune = %hd\n", region->wsmp->sFineTune);
		printf("    wsmp->lAttenuation = %d\n", region->wsmp->lAttenuation);
		printf("    wsmp->fulOptions = 0x%8.8x\n", region->wsmp->fulOptions);
		printf("    wsmp->cSampleLoops = %u\n", region->wsmp->cSampleLoops);
		for ( i = 0; i < region->wsmp->cSampleLoops; ++i ) {
			WLOOP *loop = &region->wsmp_loop[i];
			printf("    Loop %u:\n", i);
			printf("      ulStart = %u\n", loop->ulStart);
			printf("      ulLength = %u\n", loop->ulLength);
		}
	}
	if ( region->art && region->art->cConnections > 0 ) {
		PrintArt("Region", region->art, region->artList);
	}
}

static void PrintInstrument(DLS_Instrument *instrument, uint32_t index)
{
	printf("Instrument %u:\n", index);
	if ( instrument->name ) {
		printf("  Name: %s\n", instrument->name);
	}
	if ( instrument->header ) {
		uint32_t i;
		printf("  ulBank = 0x%8.8x\n", instrument->header->Locale.ulBank);
		printf("  ulInstrument = %u\n", instrument->header->Locale.ulInstrument);
		printf("  Regions: %u\n", instrument->header->cRegions);
		for ( i = 0; i < instrument->header->cRegions; ++i ) {
			PrintRegion(&instrument->regions[i], i);
		}
	}
	if ( instrument->art && instrument->art->cConnections > 0 ) {
		PrintArt("Instrument", instrument->art, instrument->artList);
	}
};

void PrintDLS(DLS_Data *data)
{
	printf("DLS Data:\n");
	printf("cInstruments = %u\n", data->cInstruments);
	if ( data->instruments ) {
		uint32_t i;
		for ( i = 0; i < data->cInstruments; ++i ) {
			PrintInstrument(&data->instruments[i], i);
		}
	}
	if ( data->ptbl && data->ptbl->cCues > 0 ) {
		uint32_t i;
		printf("Cues: ");
		for ( i = 0; i < data->ptbl->cCues; ++i ) {
			if ( i > 0 ) {
				printf(", ");
			}
			printf("%u", data->ptblList[i].ulOffset);
		}
		printf("\n");
	}
	if ( data->waveList && data->ptbl ) {
		uint32_t i;
		printf("Waves:\n");
		for ( i = 0; i < data->ptbl->cCues; ++i ) {
			PrintWave(&data->waveList[i], i);
		}
	}
	if ( data->name ) {
		printf("Name: %s\n", data->name);
	}
	if ( data->artist ) {
		printf("Artist: %s\n", data->artist);
	}
	if ( data->copyright ) {
		printf("Copyright: %s\n", data->copyright);
	}
	if ( data->comments ) {
		printf("Comments: %s\n", data->comments);
	}
}

#ifdef TEST_MAIN_DLS
}

int main(int argc, char *argv[])
{
	int i;
	for ( i = 1; i < argc; ++i ) {
		Timidity::DLS_Data *data;
		FILE *src = fopen(argv[i], "rb");
		if ( !src ) {
			fprintf(stderr, "Couldn't open %s: %s", argv[i], strerror(errno));
			continue;
		}
		data = Timidity::LoadDLS(src);
		if ( data ) {
			Timidity::PrintRIFF(data->chunk, 0);
			Timidity::PrintDLS(data);
			Timidity::FreeDLS(data);
		} else {
			fprintf(stderr, "Couldn't load %s: %s\n", argv[i], strerror(errno));
		}
		fclose(src);
	}
	return 0;
}

namespace Timidity
{
#endif // TEST_MAIN
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------*/
/* * * * * * * * * * * * * * * * * instrum_dls.c * * * * * * * * * * * * * */
/*-------------------------------------------------------------------------*/

#ifndef TEST_MAIN_DLS

DLS_Data *Timidity_LoadDLS(FILE *src)
{
	return LoadDLS(src);
}

void Timidity_FreeDLS(DLS_Data *patches)
{
	FreeDLS(patches);
}

/* convert timecents to sec */
static double to_msec(int timecent)
{
	if (timecent == INT_MIN || timecent == 0)
		return 0.0;
	return 1000.0 * pow(2.0, (double)(timecent / 65536) / 1200.0);
}

/* convert decipercent to {0..1} */
static double to_normalized_percent(int decipercent)
{
	return ((double)(decipercent / 65536)) / 1000.0;
}

/* convert from 8bit value to fractional offset (15.15) */
static int32_t to_offset(int offset)
{
	return (int32_t)offset << (7+15);
}

/* calculate ramp rate in fractional unit;
* diff = 8bit, time = msec
*/
static int32_t calc_rate(Renderer *song, int diff, int sample_rate, double msec)
{
	double rate;

	if(msec < 6)
		msec = 6;
	if(diff == 0)
		diff = 255;
	diff <<= (7+15);
	rate = ((double)diff / song->rate) * song->control_ratio * 1000.0 / msec;
	return (int32_t)rate;
}

static int load_connection(ULONG cConnections, CONNECTION *artList, USHORT destination)
{
	ULONG i;
	int value = 0;
	for (i = 0; i < cConnections; ++i) {
		CONNECTION *conn = &artList[i];
		if(conn->usDestination == destination) {
			// The formula for the destination is:
			// usDestination = usDestination + usTransform(usSource * (usControl * lScale))
			// Since we are only handling source/control of NONE and identity
			// transform, this simplifies to: usDestination = usDestination + lScale
			if (conn->usSource == CONN_SRC_NONE &&
				conn->usControl == CONN_SRC_NONE &&
				conn->usTransform == CONN_TRN_NONE)
				value += conn->lScale;
		}
	}
	return value;
}

static void load_region_dls(Renderer *song, Sample *sample, DLS_Instrument *ins, uint32_t index)
{
	DLS_Region *rgn = &ins->regions[index];
	DLS_Wave *wave = &song->patches->waveList[rgn->wlnk->ulTableIndex];

	sample->type = INST_DLS;
	sample->self_nonexclusive = !!(rgn->header->fusOptions & F_RGN_OPTION_SELFNONEXCLUSIVE);
	sample->key_group = (int8_t)rgn->header->usKeyGroup;
	sample->low_freq = note_to_freq(rgn->header->RangeKey.usLow);
	sample->high_freq = note_to_freq(rgn->header->RangeKey.usHigh);
	sample->root_freq = note_to_freq(rgn->wsmp->usUnityNote + rgn->wsmp->sFineTune * .01f);
	sample->low_vel = (uint8_t)rgn->header->RangeVelocity.usLow;
	sample->high_vel = (uint8_t)rgn->header->RangeVelocity.usHigh;

	sample->modes = wave->format->wBitsPerSample == 8 ? PATCH_UNSIGNED : PATCH_16;
	sample->sample_rate = wave->format->dwSamplesPerSec;
	sample->data = NULL;
	sample->data_length = wave->length;
	convert_sample_data(sample, wave->data);
	if (rgn->wsmp->cSampleLoops)
	{
		sample->modes |= (PATCH_LOOPEN | PATCH_SUSTAIN/* | PATCH_NO_SRELEASE*/);
		sample->loop_start = rgn->wsmp_loop->ulStart / 2;
		sample->loop_end = sample->loop_start + (rgn->wsmp_loop->ulLength / 2);
	}
	sample->scale_factor = 1024;
	sample->scale_note = rgn->wsmp->usUnityNote;

	if (sample->modes & PATCH_SUSTAIN)
	{
		int value;
		int attack, hold, decay, release; int sustain;
		CONNECTIONLIST *art = NULL;
		CONNECTION *artList = NULL;

		if (ins->art && ins->art->cConnections > 0 && ins->artList) {
			art = ins->art;
			artList = ins->artList;
		} else {
			art = rgn->art;
			artList = rgn->artList;
		}

		attack = load_connection(art->cConnections, artList, CONN_DST_EG1_ATTACKTIME);
		hold = load_connection(art->cConnections, artList, CONN_DST_EG1_HOLDTIME);
		decay = load_connection(art->cConnections, artList, CONN_DST_EG1_DECAYTIME);
		release = load_connection(art->cConnections, artList, CONN_DST_EG1_RELEASETIME);
		sustain = load_connection(art->cConnections, artList, CONN_DST_EG1_SUSTAINLEVEL);
		value = load_connection(art->cConnections, artList, CONN_DST_PAN);
		sample->panning = (int)((0.5 + to_normalized_percent(value)) * 16383.f);

		/*
		printf("%d, Rate=%d LV=%d HV=%d Low=%d Hi=%d Root=%d Pan=%d Attack=%f Hold=%f Sustain=%d Decay=%f Release=%f\n", index, sample->sample_rate, rgn->header->RangeVelocity.usLow, rgn->header->RangeVelocity.usHigh, sample->low_freq, sample->high_freq, sample->root_freq, sample->panning, attack, hold, sustain, decay, release);
		*/

		sample->envelope.sf2.delay_vol = -32768;
		sample->envelope.sf2.attack_vol = (short)(attack >> 16);
		sample->envelope.sf2.hold_vol = (short)(hold >> 16);
		sample->envelope.sf2.decay_vol = (short)(decay >> 16);
		sample->envelope.sf2.release_vol = (short)(release >> 16);
		sample->envelope.sf2.sustain_vol = (short)(sustain >> 16);
	}

	sample->data_length <<= FRACTION_BITS;
	sample->loop_start <<= FRACTION_BITS;
	sample->loop_end <<= FRACTION_BITS;
}

Instrument *load_instrument_dls(Renderer *song, int drum, int bank, int instrument)
{
	Instrument *inst;
	uint32_t i;
	DLS_Instrument *dls_ins = NULL;

	if (song->patches == NULL)
	{
		return NULL;
	}
	drum = drum ? 0x80000000 : 0;
	for (i = 0; i < song->patches->cInstruments; ++i)
	{
		dls_ins = &song->patches->instruments[i];
		if ((dls_ins->header->Locale.ulBank & 0x80000000) == (ULONG)drum &&
			((dls_ins->header->Locale.ulBank >> 8) & 0xFF) == (ULONG)bank &&
			dls_ins->header->Locale.ulInstrument == (ULONG)instrument)
			break;
	}
	if (i == song->patches->cInstruments && bank == 0)
	{
		for (i = 0; i < song->patches->cInstruments; ++i)
		{
			dls_ins = &song->patches->instruments[i];
			if ((dls_ins->header->Locale.ulBank & 0x80000000) == (ULONG)drum &&
				dls_ins->header->Locale.ulInstrument == (ULONG)instrument)
				break;
		}
	}
	if (i == song->patches->cInstruments)
	{
//		SNDDBG(("Couldn't find %s instrument %d in bank %d\n", drum ? "drum" : "melodic", instrument, bank));
		return NULL;
	}

	inst = (Instrument *)safe_malloc(sizeof(Instrument));
	inst->samples = dls_ins->header->cRegions;
	inst->sample = (Sample *)safe_malloc(inst->samples * sizeof(Sample));
	memset(inst->sample, 0, inst->samples * sizeof(Sample));
	/*
	printf("Found %s instrument %d in bank %d named %s with %d regions\n", drum ? "drum" : "melodic", instrument, bank, dls_ins->name, inst->samples);
	*/
	for (i = 0; i < dls_ins->header->cRegions; ++i)
	{
		load_region_dls(song, &inst->sample[i], dls_ins, i);
	}
	return inst;
}
#endif	/* !TEST_MAIN_DLS */

}
