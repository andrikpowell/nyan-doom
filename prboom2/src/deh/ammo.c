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
//	DEH Ammo
//

#include "deh/func.h"
#include "p_inter.h"
#include "doomdef.h"

// AMMO - Dehacked block name = "Ammo"
// usage = Ammo n (name)
// Ammo information for the few types of ammo

static const char *deh_ammo[] = // CPhipps - static const*
{
  "Max ammo",   // maxammo[]
  "Per ammo"    // clipammo[]
};

// ====================================================================
// deh_procAmmo
// Purpose: Handle DEH Ammo block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procAmmo(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Ammo at index %d: %s\n", indexnum, key);
  if (indexnum < 0 || indexnum >= NUMAMMO)
    deh_log("Bad ammo number %d of %d\n", indexnum, NUMAMMO);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    if (!deh_strcasecmp(key, deh_ammo[0]))  // Max ammo
      maxammo[indexnum] = (int)value;
    else if (!deh_strcasecmp(key, deh_ammo[1]))  // Per ammo
      clipammo[indexnum] = (int)value;
    else
      deh_log("Invalid ammo string index for '%s'\n", key);
  }
}

const deh_block deh_block_ammo = { "Ammo", deh_procAmmo };
