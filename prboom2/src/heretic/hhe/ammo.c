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
//	HHE Ammo
//

#include "deh/func.h"
#include "p_inter.h"
#include "doomdef.h"

// AMMO - Hehacked block name = "Ammo"
// usage = Ammo n (name)
// Ammo information for the few types of ammo

static const char *hhe_ammo[] = // CPhipps - static const*
{
  "Max ammo",   // maxammo[]
  "Per ammo"    // GetWeaponAmmo[] - Different in Heretic (see note below)
};

// ====================================================================
// hhe_procAmmo
// Purpose: Handle DEH Ammo block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procAmmo(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow rex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Ammo at index %d: %s\n", indexnum, key);
  if (indexnum < 0 || indexnum >= HERETIC_NUMAMMO)
    deh_log("Bad ammo number %d of %d\n", indexnum, HERETIC_NUMAMMO);

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
    if (!deh_strcasecmp(key, hhe_ammo[0]))  // Max ammo
    {
      maxammo[indexnum] = (int)value;
    }
    else if (!deh_strcasecmp(key, hhe_ammo[1])) // "Per ammo"
    {
      // Heretic doesn't have a "per clip" ammo array, instead
      // it is per weapon.  However, the weapon number lines
      // up with the ammo number if we add one.

      GetWeaponAmmo[indexnum + 1] = (int)value;
    }
    else
      deh_log("Invalid ammo string index for '%s'\n", key);
  }
}

const deh_block hhe_block_ammo = { "Ammo", hhe_procAmmo };
