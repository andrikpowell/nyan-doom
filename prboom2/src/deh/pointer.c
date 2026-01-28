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
//	DEH Pointer
//

#include "doomdef.h"

#include "deh/func.h"
#include "deh/frame.h"
#include "deh/bex_pointer.h"

#include "dsda/state.h"

// ====================================================================
// deh_procPointer
// Purpose: Handle DEH Code pointer block, can use BEX [CODEPTR] instead
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPointer(DEHFILE *fpin, char *line) // done
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  size_t i; // looper
  dsda_deh_state_t deh_state, ptr_state;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  // NOTE: different format from normal

  // killough 8/98: allow hex numbers in input, fix error case:
  if (sscanf(inbuffer, "%*s %*i (%s %i)", key, &indexnum) != 2)
  {
    deh_log("Bad data pair in '%s'\n", inbuffer);
    return;
  }

  deh_log("Processing Pointer at index %d: %s\n", indexnum, key);
  if (indexnum < 0)
  {
    deh_log("Pointer number must be positive (%d)\n", indexnum);
    return;
  }

  deh_state = dsda_GetDehState(indexnum);

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

    ptr_state = dsda_GetDehState((int)value);

    if (!deh_strcasecmp(key, deh_state_fields[4]))  // Codep frame (not set in Frame deh block)
    {
      deh_state.state->action = *ptr_state.codeptr;
      deh_log(" - applied from codeptr[%ld] to states[%d]\n", (long)value, indexnum);
      // Write BEX-oriented line to match:
      for (i = 0; i < deh_numbexptr; i++)
      {
        if (!memcmp(&deh_bexptrs[i].cptr, ptr_state.codeptr, sizeof(actionf_t)))
        {
          deh_log("BEX [CODEPTR] -> FRAME %d = %s\n", indexnum, &deh_bexptrs[i].lookup[2]);
          break;
        }
        if (deh_bexptrs[i].cptr == NULL) // stop at null entry
          break;
      }
    }
    else
      deh_log("Invalid frame pointer index for '%s' at %ld\n", key, (long)value);
  }
}

const deh_block deh_block_pointer = { "Pointer", deh_procPointer };
