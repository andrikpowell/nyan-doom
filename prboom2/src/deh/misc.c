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
//	DEH Misc
//

#include "deh/func.h"
#include "p_inter.h"
#include "doomdef.h"

dboolean bfgcells_modified = false;

int deh_maxhealth;
int deh_max_soul;
int deh_mega_health;

dboolean IsDehMaxHealth = false;
dboolean IsDehMaxSoul = false;
dboolean IsDehMegaHealth = false;

// MISC - Dehacked block name = "Misc"
// Usage: Misc 0
// Always uses a zero in the dehacked file, for consistency.  No meaning.

static const char *deh_misc[] = // CPhipps - static const*
{
  "Initial Health",    // initial_health
  "Initial Bullets",   // initial_bullets
  "Max Health",        // maxhealth
  "Max Armor",         // max_armor
  "Green Armor Class", // green_armor_class
  "Blue Armor Class",  // blue_armor_class
  "Max Soulsphere",    // max_soul
  "Soulsphere Health", // soul_health
  "Megasphere Health", // mega_health
  "God Mode Health",   // god_health
  "IDFA Armor",        // idfa_armor
  "IDFA Armor Class",  // idfa_armor_class
  "IDKFA Armor",       // idkfa_armor
  "IDKFA Armor Class", // idkfa_armor_class
  "BFG Cells/Shot",    // BFGCELLS
  "Monsters Infight"   // Unknown--not a specific number it seems, but
  // the logic has to be here somewhere or
  // it'd happen always
};

// ====================================================================
// deh_procMisc
// Purpose: Handle DEH Misc block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procMisc(DEHFILE *fpin, char *line) // done
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;    // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // Otherwise it's ok
    deh_log("Processing Misc item '%s'\n", key);

    if (!deh_strcasecmp(key, deh_misc[0]))  // Initial Health
      initial_health = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[1]))  // Initial Bullets
      initial_bullets = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[2]))  // Max Health
    {
      IsDehMaxHealth = true;
      deh_maxhealth = (int)value; //e6y
    }
    else if (!deh_strcasecmp(key, deh_misc[3]))  // Max Armor
      max_armor = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[4]))  // Green Armor Class
      green_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[5]))  // Blue Armor Class
      blue_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[6]))  // Max Soulsphere
    {
      IsDehMaxSoul = true;
      deh_max_soul = (int)value; //e6y
    }
    else if (!deh_strcasecmp(key, deh_misc[7]))  // Soulsphere Health
      soul_health = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[8]))  // Megasphere Health
    {
      IsDehMegaHealth = true;
      deh_mega_health = (int)value; //e6y
    }
    else if (!deh_strcasecmp(key, deh_misc[9]))  // God Mode Health
      god_health = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[10]))  // IDFA Armor
      idfa_armor = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[11]))  // IDFA Armor Class
      idfa_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[12]))  // IDKFA Armor
      idkfa_armor = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[13]))  // IDKFA Armor Class
      idkfa_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[14]))  // BFG Cells/Shot
    {
      weaponinfo[wp_bfg].ammopershot = bfgcells = (int)value;
      bfgcells_modified = true;
    }
    else if (!deh_strcasecmp(key, deh_misc[15]))  // Monsters Infight
    {
      // e6y: Dehacked support - monsters infight
      if (value == 202) monsters_infight = 0;
      else if (value == 221) monsters_infight = 1;
      else deh_log("Invalid value for 'Monsters Infight': %i", (int)value);

      /* No such switch in DOOM - nop */ //e6y ;
    }
    else
      deh_log("Invalid misc item string index for '%s'\n", key);
  }
}

const deh_block deh_block_misc = { "Misc", deh_procMisc };
