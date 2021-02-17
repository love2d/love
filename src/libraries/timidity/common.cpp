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

	common.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "timidity.h"

/* The path separator (D.M.) */
#if defined(__WIN32__) || defined(__OS2__)
#  define PATH_SEP '\\'
#else
#  define PATH_SEP '/'
#endif

namespace Timidity
{

typedef struct {
  char *path;
  void *next;
} PathList;

/* The paths in this list will be tried whenever we're reading a file */
static PathList *pathlist = NULL; /* This is a linked list */

/* This is meant to find and open files for reading */
FILE *open_file(const char *name)
{
  FILE *fp;

  if (!name || !(*name))
    {
      printf("Attempted to open nameless file.\n");
      return 0;
    }

  /* First try the given name */

  printf("Trying to open %s\n", name);
  if ((fp = fopen(name, "rb")))
    return fp;

  if (name[0] != PATH_SEP)
  {
    char current_filename[1024];
    PathList *plp = pathlist;
    size_t l;

    while (plp)  /* Try along the path then */
      {
	*current_filename = 0;
	l = strlen(plp->path);
	if(l)
	  {
	    strcpy(current_filename, plp->path);
	    if(current_filename[l - 1] != PATH_SEP)
	    {
	      current_filename[l] = PATH_SEP;
	      current_filename[l + 1] = '\0';
	    }
	  }
	strcat(current_filename, name);
	printf("Trying to open %s\n", current_filename);
	if ((fp = fopen(current_filename, "rb")))
	  return fp;
	plp = (PathList*)plp->next;
      }
  }

  /* Nothing could be opened. */
  printf("Could not open %s\n", name);
  return 0;
}


/* This'll allocate memory or die. */
void *safe_malloc(size_t count)
{
	void *p;
	if (count > (1 << 21))
	{
		printf("Timidity: Tried allocating %zu bytes. This must be a bug.", count);
	}
	else if ((p = malloc(count)))
	{
		return p;
	}
	else
	{
		printf("Timidity: Couldn't malloc %zu bytes.", count);
	}
	return 0;
}


/* This adds a directory to the path list */
void add_to_pathlist(const char *s)
{
    PathList *plp = (PathList*)safe_malloc(sizeof(PathList));

  if (plp == NULL)
      return;

  plp->path = (char*)safe_malloc(strlen(s) + 1);
  if (plp->path == NULL)
  {
      free(plp);
      return;
  }

  strcpy(plp->path, s);
  plp->next = pathlist;
  pathlist = plp;
}

void free_pathlist(void)
{
    PathList *plp = pathlist;
    PathList *next;

    while (plp)
    {
	next = (PathList*)plp->next;
	free(plp->path);
	free(plp);
	plp = next;
    }
    pathlist = NULL;
}


}
