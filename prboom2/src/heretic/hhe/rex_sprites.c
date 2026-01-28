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
//	HHE [SPRITES] Parser
//

#include "doomdef.h"
#include "doomstat.h"

#include "deh/func.h"
#include "heretic/hhe/text.h"

#include "dsda/sprite.h"

//
// hhe_procRexSprites
//
// Supports sprite name substitutions without requiring use
// of the HeHackEd Text block
//
static void hhe_procRexSprites(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;  // holds the string value of the line
  const char *src;
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

    src = ptr_lstrip(strval);
    //
    match = dsda_GetOriginalSpriteIndex(key);
    if (match >= 0)
    {
      if (!HHE_RenameSprite(match, src, key, 0))
        deh_log("Bad sprite name (too short) for '%s'\n", key);
    }
    else
    {
      deh_log("Unknown sprite key '%s'\n", key);
    }
  }
}

const deh_block hhe_block_rex_sprites = { "[SPRITES]", hhe_procRexSprites };
