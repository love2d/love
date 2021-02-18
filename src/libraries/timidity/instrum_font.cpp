#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "timidity.h"
#include "sf2.h"

namespace Timidity
{

FontFile *Fonts;

FontFile *ReadDLS(const char *filename, FILE *f)
{
	return NULL;
}

void font_freeall()
{
	FontFile *font, *next;

	for (font = Fonts; font != NULL; font = next)
	{
		next = font->Next;
		delete font;
	}
	Fonts = NULL;
}

FontFile *font_find(const char *filename)
{
	for (FontFile *font = Fonts; font != NULL; font = font->Next)
	{
#ifdef _WIN32
		if (_stricmp(filename, font->Filename.c_str()) == 0)
#else
		if (strcasecmp(filename, font->Filename.c_str()) == 0)
#endif
		{
			return font;
		}
	}
	return NULL;
}

void font_add(const char *filename, int load_order)
{
	FontFile *font;
        FILE *fp;

	font = font_find(filename);
	if (font != NULL)
	{
		font->SetAllOrders(load_order);
	}
	else
	{
            fp = fopen(filename, "rb");
		if (fp != NULL)
		{
			if ((font = ReadSF2(filename, fp)) || (font = ReadDLS(filename, fp)))
			{
				font->SetAllOrders(load_order);
			}
			fclose(fp);
		}
	}
}

void font_remove(const char *filename)
{
	FontFile *font;

	font = font_find(filename);
	if (font != NULL)
	{
		// Don't actually remove the font from the list, because instruments
		// from it might be loaded using the %font extension.
		font->SetAllOrders(255);
	}
}

void font_order(int order, int bank, int preset, int keynote)
{
	for (FontFile *font = Fonts; font != NULL; font = font->Next)
	{
		font->SetOrder(order, bank, preset, keynote);
	}
}

Instrument *load_instrument_font(struct Renderer *song, const char *font, int drum, int bank, int instr)
{
	FontFile *fontfile = font_find(font);
	if (fontfile != NULL)
	{
		return fontfile->LoadInstrument(song, drum, bank, instr);
	}
	return NULL;
}

Instrument *load_instrument_font_order(struct Renderer *song, int order, int drum, int bank, int instr)
{
	for (FontFile *font = Fonts; font != NULL; font = font->Next)
	{
		Instrument *ip = font->LoadInstrument(song, drum, bank, instr);
		if (ip != NULL)
		{
			return ip;
		}
	}
	return NULL;
}

    FontFile::FontFile(std::string filename)
: Filename(filename)
{
	Next = Fonts;
	Fonts = this;
}

FontFile::~FontFile()
{
	for (FontFile **probe = &Fonts; *probe != NULL; probe = &(*probe)->Next)
	{
		if (*probe == this)
		{
			*probe = Next;
			break;
		}
	}
}

}
