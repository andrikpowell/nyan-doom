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
//	HHE Weapon
//

#include "deh/func.h"
#include "p_inter.h"
#include "doomdef.h"

#include "heretic/hhe/frame.h"

static const char *hhe_heretic_weapon[] = // CPhipps - static const*
{
  "Ammo type",      // .ammo
  "Deselect frame", // .upstate
  "Select frame",   // .downstate
  "Bobbing frame",  // .readystate
  "Shooting frame", // .atkstate
  "Firing frame",   // .holdatkstate   (Heretic difference)
  "Unknown frame",  // .flashstate     (Heretic difference)
  // (optional) "Ammo per shot" etc if you support it in your format
};

// WEAPONS - Hehacked block name = "Weapon"
// Usage: Weapon nn (name)
// Basically a list of frames and what kind of ammo (see above)it uses.

// ====================================================================
// hhe_procWeapon
// Purpose: Handle HHE Weapon block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procWeapon(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  char *strval;
  int bGetData;

  weaponinfo_t *weapon = NULL;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow rex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Weapon at index %d: %s\n", indexnum, key);

  // Heretic weapons are doubled (tome of power)
  if (indexnum < 0 || indexnum >= NUMWEAPONS * 2)
  {
    deh_log("Bad weapon number %d (valid range 0-%d)\n", indexnum, NUMWEAPONS * 2 - 1);
  }
  // Decide weapon level (tome of power)
  else if (indexnum < NUMWEAPONS)
    weapon = &wpnlev1info[indexnum];
  else
    weapon = &wpnlev2info[indexnum - NUMWEAPONS];

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    // Heretic - gotta conver the frame number :/
    if (deh_strcasestr(key, "frame"))
      value = HHE_MapHereticFrameNumber((int)value);

    // invalid weapon index, ignore assignments but keep parsing lines
    if (!weapon)
      continue;

    if (!deh_strcasecmp(key, hhe_heretic_weapon[0]))  // Ammo type
      weapon->ammo = (ammotype_t)value;
    else if (!deh_strcasecmp(key, hhe_heretic_weapon[1]))  // Deselect frame
      weapon->upstate = (int)value;
    else if (!deh_strcasecmp(key, hhe_heretic_weapon[2]))  // Select frame
      weapon->downstate = (int)value;
    else if (!deh_strcasecmp(key, hhe_heretic_weapon[3]))  // Bobbing frame
      weapon->readystate = (int)value;
    else if (!deh_strcasecmp(key, hhe_heretic_weapon[4]))  // Shooting frame
      weapon->atkstate = (int)value;
    else if (!deh_strcasecmp(key, hhe_heretic_weapon[5]))  // Firing frame (Heretic)
      weapon->holdatkstate = (int)value;
    else if (!deh_strcasecmp(key, hhe_heretic_weapon[6]))  // Unknown frame (Heretic!)
      weapon->flashstate = (int)value;
    else
      deh_log("Invalid weapon string '%s'\n",key);
  }
}

const deh_block hhe_block_weapon = { "Weapon", hhe_procWeapon };
