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
//	DEH [SPRITES] Parser
//

#include "doomstat.h"
#include "deh/func.h"
#include "dsda/sprite.h"
#include "doomdef.h"

//
// deh_procBexSprites
//
// Supports sprite name substitutions without requiring use
// of the DeHackEd Text block
//
static void deh_procBexSprites(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;  // holds the string value of the line
  char candidate[5];
  int  match;

  deh_log("Processing sprite name substitution\n");

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
    memset(candidate, 0, sizeof(candidate));
    strncpy(candidate, ptr_lstrip(strval), 4);
    if (strlen(candidate) != 4)
    {
      deh_log("Bad length for sprite name '%s'\n", candidate);
      continue;
    }

    match = dsda_GetOriginalSpriteIndex(key);
    if (match >= 0)
    {
      deh_log("Substituting '%s' for sprite '%s'\n", candidate, key);
      sprnames[match] = Z_Strdup(candidate);
    }
  }
}

const deh_block deh_block_bex_sprites = { "[SPRITES]", deh_procBexSprites };
