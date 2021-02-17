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

	timidity.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
   #include <io.h>
   #define access    _access_s
#else
   #include <unistd.h>
#endif

#include "timidity.h"
#include "common.h"

#define MAXWORDS 10

std::string midi_config = CONFIG_FILE;
int midi_voices = 32;
std::string gus_patchdir = "";
int gus_memsize = 0;

namespace Timidity
{

ToneBank *tonebank[MAXBANK], *drumset[MAXBANK];

    static std::string def_instr_name;

static int read_config_file(const char *name, bool ismain)
{
    FILE *fp;
    char *tmp, *w[MAXWORDS], *cp;
	ToneBank *bank = NULL;
	int i, j, k, line = 0, words;
	static int rcf_count = 0;
	int lumpnum;
        size_t line_size;

	if (rcf_count > 50)
	{
		printf("Timidity: Probable source loop in configuration files\n");
		return -1;
	}

        if (!(fp = open_file(name)))
            return -1;

        tmp = (char*)malloc(line_size);

	while (getline(&tmp, &line_size, fp) > 0)
	{
		line++;
                tmp[strlen(tmp)-1] = '\0';

                words = 0;
                w[0] = strtok(tmp, " \t\240");
                if (!w[0]) continue;

		/* Originally the TiMidity++ extensions were prefixed like this */
		if (strcmp(w[0], "#extension") == 0)
		{
                        w[0] = strtok(tmp, " \t\240");
                        if (!w[0]) continue;
		}

		if (*w[0] == '#')
		{
			continue;
		}

                while (w[words] && *w[words] != '#' && (words < (MAXWORDS-1)))
                    w[++words]=strtok(0," \t\240");

		if (words == 0) continue;

		/*
		 * TiMidity++ adds a number of extensions to the config file format.
		 * Many of them are completely irrelevant to SDL_sound, but at least
		 * we shouldn't choke on them.
		 *
		 * Unfortunately the documentation for these extensions is often quite
		 * vague, gramatically strange or completely absent.
		 */
		if (
			!strcmp(w[0], "comm")			/* "comm" program second        */
			|| !strcmp(w[0], "HTTPproxy")	/* "HTTPproxy" hostname:port    */
			|| !strcmp(w[0], "FTPproxy")	/* "FTPproxy" hostname:port     */
			|| !strcmp(w[0], "mailaddr")	/* "mailaddr" your-mail-address */
			|| !strcmp(w[0], "opt")			/* "opt" timidity-options       */
			)
		{
			/*
			* + "comm" sets some kind of comment -- the documentation is too
			*   vague for me to understand at this time.
			* + "HTTPproxy", "FTPproxy" and "mailaddr" are for reading data
			*   over a network, rather than from the file system.
			* + "opt" specifies default options for TiMidity++.
			*
			* These are all quite useless for our version of TiMidity, so
			* they can safely remain no-ops.
			*/
		}
		else if (!strcmp(w[0], "timeout"))	/* "timeout" program second */
		{
			/*
			* Specifies a timeout value of the program. A number of seconds
			* before TiMidity kills the note. This may be useful to implement
			* later, but I don't see any urgent need for it.
			*/
			//printf("FIXME: Implement \"timeout\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "copydrumset")	/* "copydrumset" drumset */
			|| !strcmp(w[0], "copybank"))		/* "copybank" bank       */
		{
			/*
			* Copies all the settings of the specified drumset or bank to
			* the current drumset or bank. May be useful later, but not a
			* high priority.
			*/
			//printf("FIXME: Implement \"%s\" in TiMidity config.\n", w[0]);
		}
		else if (!strcmp(w[0], "undef"))		/* "undef" progno */
		{
			/*
			* Undefines the tone "progno" of the current tone bank (or
			* drum set?). Not a high priority.
			*/
			//printf("FIXME: Implement \"undef\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "altassign")) /* "altassign" prog1 prog2 ... */
		{
			/*
			* Sets the alternate assign for drum set. Whatever that's
			* supposed to mean.
			*/
			//printf("FIXME: Implement \"altassign\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "soundfont"))
		{
			/*
			* "soundfont" sf_file "remove"
			* "soundfont" sf_file ["order=" order] ["cutoff=" cutoff]
			*                     ["reso=" reso] ["amp=" amp]
			*/
			if (words < 2)
			{
				printf("%s: line %d: No soundfont given\n", name, line);
				goto fail;
			}
			if (words > 2 && !strcmp(w[2], "remove"))
			{
				font_remove(w[1]);
			}
			else
			{
				int order = 0;

				for (i = 2; i < words; ++i)
				{
					if (!(cp = strchr(w[i], '=')))
					{
						printf("%s: line %d: bad soundfont option %s\n", name, line, w[i]);
						goto fail;
					}
				}
				font_add(w[1], order);
			}
		}
		else if (!strcmp(w[0], "font"))
		{
			/*
			* "font" "exclude" bank preset keynote
			* "font" "order" order bank preset keynote
			*/
			int order, drum = -1, bank = -1, instr = -1;

			if (words < 3)
			{
				printf("%s: line %d: syntax error\n", name, line);
				goto fail;
			}

			if (!strcmp(w[1], "exclude"))
			{
				order = 254;
				i = 2;
			}
			else if (!strcmp(w[1], "order"))
			{
				order = atoi(w[2]);
				i = 3;
			}
			else
			{
				printf("%s: line %d: font subcommand must be 'order' or 'exclude'\n", name, line);
				goto fail;
			}
			if (i < words)
			{
				drum = atoi(w[i++]);
			}
			if (i < words)
			{
				bank = atoi(w[i++]);
			}
			if (i < words)
			{
				instr = atoi(w[i++]);
			}
			if (drum != 128)
			{
				instr = bank;
				bank = drum;
				drum = 0;
			}
			else
			{
				drum = 1;
			}
			font_order(order, drum, bank, instr);
		}
		else if (!strcmp(w[0], "progbase"))
		{
			/*
			* The documentation for this makes absolutely no sense to me, but
			* apparently it sets some sort of base offset for tone numbers.
			* Why anyone would want to do this is beyond me.
			*/
			//printf("FIXME: Implement \"progbase\" in TiMidity config.\n");
		}
		else if (!strcmp(w[0], "map")) /* "map" name set1 elem1 set2 elem2 */
		{
			/*
			* This extension is the one we will need to implement, as it is
			* used by the "eawpats". Unfortunately I cannot find any
			* documentation whatsoever for it, but it looks like it's used
			* for remapping one instrument to another somehow.
			*/
			//printf("FIXME: Implement \"map\" in TiMidity config.\n");
		}

		/* Standard TiMidity config */

		else if (!strcmp(w[0], "dir"))
		{
			if (words < 2)
			{
				printf("%s: line %d: No directory given\n", name, line);
				goto fail;
			}
			for (i = 1; i < words; i++)
				add_to_pathlist(w[i]);
		}
		else if (!strcmp(w[0], "source"))
		{
			if (words < 2)
			{
				printf("%s: line %d: No file name given\n", name, line);
				goto fail;
			}
			for (i=1; i<words; i++)
			{
				rcf_count++;
				read_config_file(w[i], false);
				rcf_count--;
			}
		}
		else if (!strcmp(w[0], "default"))
		{
			if (words != 2)
			{
				printf("%s: line %d: Must specify exactly one patch name\n", name, line);
				goto fail;
			}
			def_instr_name = w[1];
		}
		else if (!strcmp(w[0], "drumset"))
		{
			if (words < 2)
			{
				printf("%s: line %d: No drum set number given\n", name, line);
				goto fail;
			}
			i = atoi(w[1]);
			if (i < 0 || i > 127)
			{
				printf("%s: line %d: Drum set must be between 0 and 127\n", name, line);
				goto fail;
			}
			if (drumset[i] == NULL)
			{
				drumset[i] = new ToneBank;
			}
			bank = drumset[i];
		}
		else if (!strcmp(w[0], "bank"))
		{
			if (words < 2)
			{
				printf("%s: line %d: No bank number given\n", name, line);
				goto fail;
			}
			i = atoi(w[1]);
			if (i < 0 || i > 127)
			{
				printf("%s: line %d: Tone bank must be between 0 and 127\n", name, line);
				goto fail;
			}
			if (tonebank[i] == NULL)
			{
				tonebank[i] = new ToneBank;
			}
			bank = tonebank[i];
		}
		else
		{
			if ((words < 2) || (*w[0] < '0' || *w[0] > '9'))
			{
				printf("%s: line %d: syntax error\n", name, line);
				goto fail;
			}
			i = atoi(w[0]);
			if (i < 0 || i > 127)
			{
				printf("%s: line %d: Program must be between 0 and 127\n", name, line);
				goto fail;
			}
			if (bank == NULL)
			{
				printf("%s: line %d: Must specify tone bank or drum set before assignment\n", name, line);
				goto fail;
			}
			bank->tone[i].note = bank->tone[i].pan =
				bank->tone[i].fontbank = bank->tone[i].fontpreset =
				bank->tone[i].fontnote = bank->tone[i].strip_loop =
				bank->tone[i].strip_envelope = bank->tone[i].strip_tail = -1;

			if (!strcmp(w[1], "%font"))
			{
				bank->tone[i].name = w[2];
				bank->tone[i].fontbank = atoi(w[3]);
				bank->tone[i].fontpreset = atoi(w[4]);
				if (words > 5 && (bank->tone[i].fontbank == 128 || (w[5][0] >= '0' && w[5][0] <= '9')))
				{
					bank->tone[i].fontnote = atoi(w[5]);
					j = 6;
				}
				else
				{
					j = 5;
				}
				font_add(w[2], 254);
			}
			else
			{
				bank->tone[i].name = w[1];
				j = 2;
			}

			for (; j<words; j++)
			{
				if (!(cp=strchr(w[j], '=')))
				{
					printf("%s: line %d: bad patch option %s\n", name, line, w[j]);
					goto fail;
				}
				*cp++ = 0;
				if (!strcmp(w[j], "amp"))
				{
					/* Ignored */
				}
				else if (!strcmp(w[j], "note"))
				{
					k = atoi(cp);
					if ((k < 0 || k > 127) || (*cp < '0' || *cp > '9'))
					{
						printf("%s: line %d: note must be between 0 and 127\n", name, line);
						goto fail;
					}
					bank->tone[i].note = k;
				}
				else if (!strcmp(w[j], "pan"))
				{
					if (!strcmp(cp, "center"))
						k = 64;
					else if (!strcmp(cp, "left"))
						k = 0;
					else if (!strcmp(cp, "right"))
						k = 127;
					else
						k = ((atoi(cp)+100) * 100) / 157;
					if ((k < 0 || k > 127) ||
						(k == 0 && *cp != '-' && (*cp < '0' || *cp > '9')))
					{
						printf("%s: line %d: panning must be left, right, "
							"center, or between -100 and 100\n", name, line);
						goto fail;
					}
					bank->tone[i].pan = k;
				}
				else if (!strcmp(w[j], "keep"))
				{
					if (!strcmp(cp, "env"))
						bank->tone[i].strip_envelope = 0;
					else if (!strcmp(cp, "loop"))
						bank->tone[i].strip_loop = 0;
					else
					{
						printf("%s: line %d: keep must be env or loop\n", name, line);
						goto fail;
					}
				}
				else if (!strcmp(w[j], "strip"))
				{
					if (!strcmp(cp, "env"))
						bank->tone[i].strip_envelope = 1;
					else if (!strcmp(cp, "loop"))
						bank->tone[i].strip_loop = 1;
					else if (!strcmp(cp, "tail"))
						bank->tone[i].strip_tail = 1;
					else
					{
						printf("%s: line %d: strip must be env, loop, or tail\n", name, line);
						goto fail;
					}
				}
				else
				{
					printf("%s: line %d: bad patch option %s\n", name, line, w[j]);
					goto fail;
				}
			}
		}
	}
	/*
	if (ferror(fp))
	{
		printf("Can't read %s: %s\n", name, strerror(errno));
		close_file(fp);
		return -2;
	}
	*/

        free(tmp);
	return 0;

fail:
        free(tmp);
        return -2;
}

void FreeAll()
{
	free_instruments();
	font_freeall();
	for (int i = 0; i < MAXBANK; ++i)
	{
		if (tonebank[i] != NULL)
		{
			delete tonebank[i];
			tonebank[i] = NULL;
		}
		if (drumset[i] != NULL)
		{
			delete drumset[i];
			drumset[i] = NULL;
		}
	}
}

int LoadConfig(const char *filename)
{
	/* !!! FIXME: This may be ugly, but slightly less so than requiring the
	 *			  default search path to have only one element. I think.
	 *
	 *			  We only need to include the likely locations for the config
	 *			  file itself since that file should contain any other directory
	 *			  that needs to be added to the search path.
	 */
#ifdef _WIN32
    add_to_pathlist("C:\\TIMIDITY");
    add_to_pathlist("\\TIMIDITY");
    add_to_pathlist(progdir);
#else
    add_to_pathlist("/usr/local/lib/timidity");
    add_to_pathlist("/etc/timidity");
    add_to_pathlist("/etc/timidity++");
    add_to_pathlist("/etc");
#endif

	/* Some functions get aggravated if not even the standard banks are available. */
	if (tonebank[0] == NULL)
	{
		tonebank[0] = new ToneBank;
		drumset[0] = new ToneBank;
	}

	return read_config_file(filename, true);
}

int LoadConfig()
{
    return LoadConfig(midi_config.c_str());
}

DLS_Data *LoadDLS(FILE *src);
void FreeDLS(DLS_Data *data);

Renderer::Renderer(float sample_rate, const char *args)
{
	// 'args' should be used to load a custom config or DMXGUS, but since setup currently requires a snd_reset call, this will need some refactoring first
	rate = sample_rate;
	patches = NULL;
	resample_buffer_size = 0;
	resample_buffer = NULL;
	voice = NULL;
	adjust_panning_immediately = false;

	control_ratio = clamp(int(rate / CONTROLS_PER_SECOND), 1, MAX_CONTROL_RATIO);

	lost_notes = 0;
	cut_notes = 0;

	default_instrument = NULL;
	default_program = DEFAULT_PROGRAM;
	if (!def_instr_name.empty())
            set_default_instrument(def_instr_name.c_str());

	voices = MAX(midi_voices, 16);
	voice = new Voice[voices];
	drumchannels = DEFAULT_DRUMCHANNELS;
#if 0
	FILE *f = fopen("c:\\windows\\system32\\drivers\\gm.dls", "rb");
	patches = LoadDLS(f);
	fclose(f);
#endif
}

Renderer::~Renderer()
{
	if (resample_buffer != NULL)
	{
		free(resample_buffer);
	}
	if (voice != NULL)
	{
		delete[] voice;
	}
	if (patches != NULL)
	{
		FreeDLS(patches);
	}
}

void Renderer::ComputeOutput(float *buffer, int count)
{
	// count is in samples, not bytes.
	if (count <= 0)
	{
		return;
	}
	Voice *v = &voice[0];

	memset(buffer, 0, sizeof(float)*count*2);		// An integer 0 is also a float 0.
	if (resample_buffer_size < count)
	{
		resample_buffer_size = count;
		resample_buffer = (sample_t *)realloc(resample_buffer, count * sizeof(float) * 2);
	}
	for (int i = 0; i < voices; i++, v++)
	{
		if (v->status & VOICE_RUNNING)
		{
			mix_voice(this, buffer, v, count);
		}
	}
}

void Renderer::MarkInstrument(int banknum, int percussion, int instr)
{
	ToneBank *bank;

	if (banknum >= MAXBANK)
	{
		return;
	}
	if (banknum != 0)
	{
		/* Mark the standard bank in case it's not defined by this one. */
		MarkInstrument(0, percussion, instr);
	}
	if (percussion)
	{
		bank = drumset[banknum];
	}
	else
	{
		bank = tonebank[banknum];
	}
	if (bank == NULL)
	{
		return;
	}
	if (bank->instrument[instr] == NULL)
	{
		bank->instrument[instr] = MAGIC_LOAD_INSTRUMENT;
	}
}

void cmsg(int type, int verbosity_level, const char *fmt, ...)
{
	/*
	va_list args;
	va_start(args, fmt);
	Vprintf(PRINT_HIGH, fmt, args);
	msg.VFormat(fmt, args);
	*/
#ifdef _WIN32
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	I_DebugPrint(buf);
#endif
}

}
