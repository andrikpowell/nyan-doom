//
// Copyright(C) 1999-2004 by Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
// Copyright(C) 2005-2006 by Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
// Copyright(C) 2026 by Andrik Powell
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DEH Sound
//

#include "deh/func.h"
#include "dsda/sfx.h"
#include "doomdef.h"

// ====================================================================
// deh_procSounds
// Purpose: Handle DEH Sounds block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSounds(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  sfxinfo_t *deh_sfx;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Sounds at index %d: %s\n", indexnum, key);
  if (indexnum < 0)
    deh_log("Sound number must be positive (%d)\n", indexnum);

  deh_sfx = dsda_GetDehSFX(indexnum);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;         // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    if (!deh_strcasecmp(key, "Offset"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Zero/One"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Value"))
      deh_sfx->priority = (int)value;
    else if (!deh_strcasecmp(key, "Zero 1"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Zero 2"))
      deh_sfx->pitch = (int)value;
    else if (!deh_strcasecmp(key, "Zero 3"))
      ;
    else if (!deh_strcasecmp(key, "Zero 4"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Neg. One 1"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Neg. One 2"))
      ; // ignored
    else
      deh_log("Invalid sound string index for '%s'\n", key);
  }
}

const deh_block deh_block_sounds = { "Sound", deh_procSounds };
