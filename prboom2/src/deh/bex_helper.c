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
//	DEH Helper
//

#include "doomdef.h"
#include "e6y.h"//e6y
#include "lprintf.h"

#include "deh/func.h"
#include "dsda/mobjinfo.h"

// haleyjd 9/22/99
int HelperThing = -1;     // in P_SpawnMapThing to substitute helper thing

//========================================================================
// haleyjd 9/22/99
//
// deh_procHelperThing
//
// Allows handy substitution of any thing for helper dogs.  DEH patches
// are being made frequently for this purpose and it requires a complete
// rewiring of the DOG thing.  I feel this is a waste of effort, and so
// have added this new [HELPER] BEX block

static void deh_procHelperThing(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // Otherwise it's ok
    deh_log("Processing Helper Thing item '%s'\nvalue is %i", key, (int)value);
    if (!strncasecmp(key, "type", 4))
      HelperThing = dsda_TranslateDehMobjIndex((int)value);
  }
}

const deh_block deh_block_bex_helper = { "[HELPER]", deh_procHelperThing };
