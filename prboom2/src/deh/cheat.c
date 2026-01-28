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
//	DEH Cheats
//

#include "deh/func.h"
#include "m_cheat.h"
#include "doomdef.h"

// CHEATS - Dehacked block name = "Cheat"
// Usage: Cheat 0
// Always uses a zero in the dehacked file, for consistency.  No meaning.
// These are just plain funky terms compared with id's
//
// killough 4/18/98: integrated into main cheat table now (see st_stuff.c)

void dsda_CopyDefaultCheats(void)
{
  int ix;
  for (ix = 0; cheat[ix].cheat; ix++)
    cheat[ix].deh_cheat = cheat[ix].cheat;
}

// ====================================================================
// deh_procCheat
// Purpose: Handle DEH Cheat block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procCheat(DEHFILE *fpin, char *line) // done
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  char ch = 0; // CPhipps - `writable' null string to initialise...
  char *strval = &ch;  // pointer to the value area
  int ix, iy;   // array indices
  char *p;  // utility pointer

  deh_log("Processing Cheat: %s\n", line);

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // Otherwise we got a (perhaps valid) cheat name,
    // so look up the key in the array

    // killough 4/18/98: use main cheat code table in st_stuff.c now
    for (ix = 0; cheat[ix].cheat; ix++)
      if (cheat[ix].deh_cheat_desc)   // killough 4/18/98: skip non-deh
      {
        if (!stricmp(key,cheat[ix].deh_cheat_desc))  // found the cheat, ignored case
        {
          // replace it but don't overflow it.  Use current length as limit.
          // Ty 03/13/98 - add 0xff code
          // Deal with the fact that the cheats in deh files are extended
          // with character 0xFF to the original cheat length, which we don't do.
          for (iy = 0; strval[iy]; iy++)
            strval[iy] = (strval[iy] == (char)0xff) ? '\0' : strval[iy];

          iy = ix;     // killough 4/18/98

          // Ty 03/14/98 - skip leading spaces
          p = strval;
          while (*p == ' ') ++p;

          // Arsinikk - parse dehacked cheats separately
          cheat[iy].deh_cheat = Z_Strdup(p);
          deh_log("Assigned new cheat '%s' to cheat '%s'at index %d\n",
                  p, cheat[ix].deh_cheat_desc, iy); // killough 4/18/98
        }
      }
    deh_log("- %s\n", inbuffer);
  }
}

const deh_block deh_block_cheat = { "Cheat", deh_procCheat };
