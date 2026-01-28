//
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
//	HHE Sound
//

#include "deh/func.h"
#include "dsda/sfx.h"
#include "doomdef.h"

static char *hhe_RenameSound(int indexnum, sfxinfo_t *hhe_sfx, char *strval)
{
  char candidate[9];
  memset(candidate, 0, sizeof(candidate));
  strncpy(candidate, ptr_lstrip(strval), 8);

  // validate 1..8
  if (!candidate[0])
    return NULL;

  for (int i = 0; candidate[i]; ++i)
    candidate[i] = toupper((unsigned char)candidate[i]);

  // optional: validate 1..8 and maybe uppercase
  if (candidate[0])
  {
    deh_log("Renaming sfx[%d] from %s to %s\n",
            indexnum, hhe_sfx && hhe_sfx->name ? hhe_sfx->name : "(null)", candidate);
    return deh_sfx_name(candidate);
  }

  return NULL;
}

// ====================================================================
// hhe_procSounds
// Purpose: Handle DEH Sounds block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procSounds(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  sfxinfo_t *hhe_sfx;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow rex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Sounds at index %d: %s\n", indexnum, key);

  if (indexnum < 0)
  {
    deh_log("Sound number must be positive (%d)\n", indexnum);
    return;
  }

  hhe_sfx = dsda_GetDehSFX(indexnum);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    char *strval = NULL;
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;         // killough 11/98


    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    if (!deh_strcasecmp(key, "Name")) // well we have to do this here cuz heretic doesn't store sound names in strings :)
    {
      char *newname = hhe_RenameSound(indexnum, hhe_sfx, strval);
      if (newname)
        hhe_sfx->name = newname;
      else
        deh_log("Bad/empty sound name for sfx[%d]\n", indexnum);
    }
    else if (!deh_strcasecmp(key, "Special"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Value"))
      hhe_sfx->priority = (int)value;
    else if (!deh_strcasecmp(key, "Unknown 1"))
      ; // ignored - usefulness
    else if (!deh_strcasecmp(key, "Unknown 2"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Unknown 3"))
      ; // ignored
    else if (!deh_strcasecmp(key, "One/Two"))
      hhe_sfx->numchannels = (int)value;
    else
      deh_log("Invalid sound string index for '%s'\n", key);
  }
}

const deh_block hhe_block_sounds = { "Sound", hhe_procSounds };
