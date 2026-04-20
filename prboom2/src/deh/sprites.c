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
//	DEH Sprites
//

#include "doomdef.h"
#include "deh/func.h"

//
//
// NOTE - This Sprite Block is unsupported, so we skip it
//
//

// ====================================================================
// deh_procSprite
// Purpose: Dummy - we do not support the DEH Sprite block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSprite(DEHFILE *fpin, char *line) // Not supported
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;

  // Too little is known about what this is supposed to do, and
  // there are better ways of handling sprite renaming.  Not supported.

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Ignoring Sprite offset change at index %d: %s\n", indexnum, key);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;      // killough 11/98
    // ignore line
    deh_log("- %s\n", inbuffer);
  }
}

const deh_block deh_block_sprite = { "Sprite", deh_procSprite };
