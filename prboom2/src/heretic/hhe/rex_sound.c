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
//	HHE [SOUNDS] Parser
//

#include "deh/func.h"
#include "dsda/sfx.h"
#include "doomdef.h"

static void hhe_procRexSounds(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All hhe values are ints or longs
  char *strval;  // holds the string value of the line
  char candidate[9]; // Heretic sounds are 8 char (instead of "D_" + "6 char" for Doom)
  int  match;
  size_t len;

  deh_log("Processing sound name substitution\n");

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
      break;
    if (*inbuffer == '#')
      continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer)
      break;  // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // do it
    memset(candidate, 0, 9);
    strncpy(candidate, ptr_lstrip(strval), 8);
    len = strlen(candidate);
    if (len < 1 || len > 8)
    {
      deh_log("Bad length for sound name '%s'\n", candidate);
      continue;
    }

    match = dsda_GetOriginalSFXIndex(key);
    if (match >= 0)
    {
      deh_log("Substituting '%s' for sound '%s'\n", candidate, key);
      dsda_GetDehSFX(match)->name = deh_sfx_name(candidate);
    }
  }
}

const deh_block hhe_block_rex_sounds = { "[SOUNDS]", hhe_procRexSounds };
