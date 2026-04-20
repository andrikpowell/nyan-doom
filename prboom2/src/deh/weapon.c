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
//	DEH Weapon
//

#include "deh/func.h"
#include "p_inter.h"
#include "doomdef.h"

// WEAPONS - Dehacked block name = "Weapon"
// Usage: Weapon nn (name)
// Basically a list of frames and what kind of ammo (see above)it uses.

static const char *deh_weapon[] = // CPhipps - static const*
{
  "Ammo type",      // .ammo
  "Deselect frame", // .upstate
  "Select frame",   // .downstate
  "Bobbing frame",  // .readystate
  "Shooting frame", // .atkstate
  "Firing frame",   // .flashstate
  "Ammo per shot",  // .ammopershot [XA] new to mbf21
  "MBF21 Bits",     // .flags
};

static const struct deh_flag_s deh_weaponflags_mbf21[] = {
  { "NOTHRUST",       WPF_NOTHRUST }, // doesn't thrust Mobj's
  { "SILENT",         WPF_SILENT }, // weapon is silent
  { "NOAUTOFIRE",     WPF_NOAUTOFIRE }, // weapon won't autofire in A_WeaponReady
  { "FLEEMELEE",      WPF_FLEEMELEE }, // monsters consider it a melee weapon
  { "AUTOSWITCHFROM", WPF_AUTOSWITCHFROM }, // can be switched away from when ammo is picked up
  { "NOAUTOSWITCHTO", WPF_NOAUTOSWITCHTO }, // cannot be switched to when ammo is picked up
  { NULL }
};

// ====================================================================
// deh_procWeapon
// Purpose: Handle DEH Weapon block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procWeapon(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  char *strval;
  int bGetData;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Weapon at index %d: %s\n", indexnum, key);
  if (indexnum < 0 || indexnum >= NUMWEAPONS)
    deh_log("Bad weapon number %d of %d\n", indexnum, NUMAMMO);

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
    if (!deh_strcasecmp(key, deh_weapon[0]))  // Ammo type
    {
      if (!raven && value == 5) value = am_noammo;
      weaponinfo[indexnum].ammo = (ammotype_t)value;
    }
    else if (!deh_strcasecmp(key, deh_weapon[1]))  // Deselect frame
      weaponinfo[indexnum].upstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[2]))  // Select frame
      weaponinfo[indexnum].downstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[3]))  // Bobbing frame
      weaponinfo[indexnum].readystate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[4]))  // Shooting frame
      weaponinfo[indexnum].atkstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[5]))  // Firing frame
      weaponinfo[indexnum].flashstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[6]))  // Ammo per shot
    {
      weaponinfo[indexnum].ammopershot = (int)value;
      weaponinfo[indexnum].intflags |= WIF_ENABLEAPS;
    }
    else if (!deh_strcasecmp(key, deh_weapon[7]))  // MBF21 Bits
    {
      if (bGetData == 1)
      {
        value = deh_translate_bits(value, deh_weaponflags_mbf21);
      }
      else
      {
        for (value = 0; (strval = strtok(strval, deh_getBitsDelims())); strval = NULL) {
          const struct deh_flag_s *flag;

          for (flag = deh_weaponflags_mbf21; flag->name; flag++) {
            if (deh_strcasecmp(strval, flag->name)) continue;

            value |= flag->value;
            break;
          }

          if (!flag->name) {
            deh_log("Could not find MBF21 weapon bit mnemonic %s\n", strval);
          }
        }
      }

      weaponinfo[indexnum].flags = (int)value;
    }
    else
      deh_log("Invalid weapon string index for '%s'\n",key);
  }
}

const deh_block deh_block_weapon = { "Weapon", deh_procWeapon };
