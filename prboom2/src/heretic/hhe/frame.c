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
//	HHE Frames
//

#include "doomdef.h"
#include "deh/func.h"

#include "heretic/hhe/version.h"
#include "heretic/hhe/rex_pointer.h"

#include "dsda/state.h"

// STATE - Hehacked block name = "Frame" and "Pointer"
// Usage: Frame nn
// Usage: Pointer nn (Frame nn)
// These are indexed separately, for lookup to the actual
// function pointers.  Here we'll take whatever Hehacked gives
// us and go from there.  The (Frame nn) after the pointer is the
// real place to put this value.  The "Pointer" value is an xref
// that Hehacked uses and is useless to us.
// * states are base zero and have a dummy #0 (TROO)

static const char *hhe_state_fields[] = // CPhipps - static const*
{
  "Sprite number",    // .sprite (spritenum_t) // an enum
  "Sprite subnumber", // .frame (long)
  "Duration",         // .tics (long)
  "Next frame",       // .nextstate (statenum_t)
  "Action pointer",   // pointer to first use of action (actionf_t)
  "Unknown 1",        // .misc1 (long)
  "Unknown 2",        // .misc2 (long)
};

int HHE_MapHereticFrameNumber(int frame)
{
    if (deh_hhe_version < deh_hhe_1_2)
    {
        // Between Heretic 1.0 and 1.2, two new frames
        // were added to the "states" table, to extend the "flame death"
        // animation displayed when the player is killed by fire.  Therefore,
        // we must map Heretic 1.0 frame numbers to corresponding indexes
        // for our state table.

        if (frame >= HERETIC_S_PLAY_FDTH19)
        {
            frame = (frame - HERETIC_S_PLAY_FDTH19) + HERETIC_S_BLOODYSKULL1;
        }
    }
    else
    {
        // After Heretic 1.2, three unused frames were removed from the
        // states table, unused phoenix rod frames.  Our state table includes
        // these missing states for backwards compatibility.  We must therefore
        // adjust frame numbers for v1.2/v1.3 to corresponding indexes for
        // our state table.

        if (frame >= HERETIC_S_PHOENIXFXIX_1)
        {
            frame = (frame - HERETIC_S_PHOENIXFXIX_1) + HERETIC_S_PHOENIXPUFF1;
        }
    }

    return frame;
}

// ====================================================================
// HHE Pointers (they are done here, as pointers are defined in frames)
//
//
//
//

static void HHE_Pointer_SuggestOtherVersions(unsigned int offset)
{
    int i, v;

    for (i=0; i < hhe_numrexptr; ++i)
    {
        for (v=0; v < deh_hhe_num_versions; ++v)
        {
            if (hhe_rexptrs[i].offsets[v] == offset)
            {
                HHE_SuggestHereticVersion(v);
            }
        }
    }
}

static actionf_t hhe_GetOffsetPointer(int offset)
{
  int i;

  if (offset != 0)
  {
    for (i = 0; hhe_rexptrs[i].cptr != NULL; ++i)
    {
      if (hhe_rexptrs[i].offsets[deh_hhe_version] == offset)
        return hhe_rexptrs[i].cptr;
    }
  }

  HHE_Pointer_SuggestOtherVersions(offset);
  deh_log("Unknown HHE action pointer offset %d (hhever=%d)\n", offset, deh_hhe_version);
  return NULL;
}

// ====================================================================
// hhe_procFrame
// Purpose: Handle HHE Frame block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void hhe_procFrame(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All hhe values are ints or longs
  int og_indexnum;
  int indexnum;
  char *strval;
  int bGetData;
  dsda_deh_state_t hhe_state;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow rex numbers in input:
  sscanf(inbuffer, "%s %i", key, &og_indexnum);
  indexnum = HHE_MapHereticFrameNumber(og_indexnum);
  deh_log("Processing Frame %d (mapped %d): %s\n", og_indexnum, indexnum, key);
  if (indexnum < 0 || indexnum >= num_states)
  {
    deh_log("Invalid Frame %d (mapped %d)\n", og_indexnum, indexnum);
    return;
  }

  hhe_state = dsda_GetDehState(indexnum);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;         // killough 11/98
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    if (!deh_strcasecmp(key, hhe_state_fields[0]))  // Sprite number
    {
      deh_log(" - sprite = %ld\n", (long)value);
      hhe_state.state->sprite = (spritenum_t)value;
    }
    else if (!deh_strcasecmp(key, hhe_state_fields[1]))  // Sprite subnumber
    {
      deh_log(" - frame = %ld\n", (long)value);
      hhe_state.state->frame = (long)value; // long
    }
    else if (!deh_strcasecmp(key, hhe_state_fields[2]))  // Duration
    {
      deh_log(" - tics = %ld\n", (long)value);
      hhe_state.state->tics = (long)value; // long
    }
    else if (!deh_strcasecmp(key, hhe_state_fields[3]))  // Next frame
    {
      int next = HHE_MapHereticFrameNumber((int)value); // get correct offset for version
      if (next >= 0)
      {
        deh_log(" - nextstate = %ld (mapped %d)\n", (long)value, next);
        hhe_state.state->nextstate = (statenum_t)next;
      }
      else
      {
        deh_log(" - nextstate invalid: %ld (mapped %d) -> leaving unchanged\n", (long)value, next);
      }
    }
    else if (!deh_strcasecmp(key, hhe_state_fields[4]))  // Action pointer (unlike Doom, we do this here for Heretic)
    {
      int offset = (int)value;
      actionf_t translated_value = hhe_GetOffsetPointer(offset); // get correct offset for version

      deh_log(" - action pointer = %d -> %s\n", offset, translated_value ? "mapped" : "NULL/unknown");
      hhe_state.state->action = translated_value;
    }
    else if (!deh_strcasecmp(key, hhe_state_fields[5]))  // Unknown 1
    {
      deh_log(" - misc1 = %ld\n", (long)value);
      hhe_state.state->misc1 = (long)value; // long
    }
    else if (!deh_strcasecmp(key, hhe_state_fields[6]))  // Unknown 2
    {
      deh_log(" - misc2 = %ld\n", (long)value);
      hhe_state.state->misc2 = (long)value; // long
    }
    else
      deh_log("Invalid frame string index for '%s'\n", key);
  }
}

const deh_block hhe_block_frame = { "Frame", hhe_procFrame };
