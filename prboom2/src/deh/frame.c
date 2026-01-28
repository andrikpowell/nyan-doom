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
//	DEH Frames
//

#include "deh/func.h"
#include "dsda/state.h"
#include "doomdef.h"

// STATE - Dehacked block name = "Frame" and "Pointer"
// Usage: Frame nn
// Usage: Pointer nn (Frame nn)
// These are indexed separately, for lookup to the actual
// function pointers.  Here we'll take whatever Dehacked gives
// us and go from there.  The (Frame nn) after the pointer is the
// real place to put this value.  The "Pointer" value is an xref
// that Dehacked uses and is useless to us.
// * states are base zero and have a dummy #0 (TROO)

const char *deh_state_fields[] = // CPhipps - static const*
{
  "Sprite number",    // .sprite (spritenum_t) // an enum
  "Sprite subnumber", // .frame (long)
  "Duration",         // .tics (long)
  "Next frame",       // .nextstate (statenum_t)
  // This is set in a separate "Pointer" block from Dehacked
  "Codep Frame",      // pointer to first use of action (actionf_t)
  "Unknown 1",        // .misc1 (long)
  "Unknown 2",        // .misc2 (long)
  "Args1",            // .args[0] (statearg_t)
  "Args2",            // .args[1] (statearg_t)
  "Args3",            // .args[2] (statearg_t)
  "Args4",            // .args[3] (statearg_t)
  "Args5",            // .args[4] (statearg_t)
  "Args6",            // .args[5] (statearg_t)
  "Args7",            // .args[6] (statearg_t)
  "Args8",            // .args[7] (statearg_t)
  "MBF21 Bits",       // .flags
};

static const struct deh_flag_s deh_stateflags_mbf21[] = {
  { "SKILL5FAST", STATEF_SKILL5FAST }, // tics halve on nightmare skill
  { NULL }
};

// ====================================================================
// deh_procFrame
// Purpose: Handle DEH Frame block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procFrame(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  char *strval;
  int bGetData;
  dsda_deh_state_t deh_state;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Frame at index %d: %s\n", indexnum, key);
  if (indexnum < 0)
  {
    deh_log("Frame number must be positive (%d)\n", indexnum);
    return;
  }

  deh_state = dsda_GetDehState(indexnum);

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

    if (!deh_strcasecmp(key, deh_state_fields[0]))  // Sprite number
    {
      deh_log(" - sprite = %ld\n", (long)value);
      deh_state.state->sprite = (spritenum_t)value;
    }
    else if (!deh_strcasecmp(key, deh_state_fields[1]))  // Sprite subnumber
    {
      deh_log(" - frame = %ld\n", (long)value);
      deh_state.state->frame = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[2]))  // Duration
    {
      deh_log(" - tics = %ld\n", (long)value);
      deh_state.state->tics = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[3]))  // Next frame
    {
      deh_log(" - nextstate = %ld\n", (long)value);
      deh_state.state->nextstate = (statenum_t)value;
    }
    else if (!deh_strcasecmp(key, deh_state_fields[4]))  // Codep frame (not set in Frame deh block)
    {
      deh_log(" - codep, should not be set in Frame section!\n");
      /* nop */ ;
    }
    else if (!deh_strcasecmp(key, deh_state_fields[5]))  // Unknown 1
    {
      deh_log(" - misc1 = %ld\n", (long)value);
      deh_state.state->misc1 = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[6]))  // Unknown 2
    {
      deh_log(" - misc2 = %ld\n", (long)value);
      deh_state.state->misc2 = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[7]))  // Args1
    {
      deh_log(" - args[0] = %lld\n", (statearg_t)value);
      deh_state.state->args[0] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 0);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[8]))  // Args2
    {
      deh_log(" - args[1] = %lld\n", (statearg_t)value);
      deh_state.state->args[1] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 1);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[9]))  // Args3
    {
      deh_log(" - args[2] = %lld\n", (statearg_t)value);
      deh_state.state->args[2] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 2);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[10]))  // Args4
    {
      deh_log(" - args[3] = %lld\n", (statearg_t)value);
      deh_state.state->args[3] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 3);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[11]))  // Args5
    {
      deh_log(" - args[4] = %lld\n", (statearg_t)value);
      deh_state.state->args[4] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 4);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[12]))  // Args6
    {
      deh_log(" - args[5] = %lld\n", (statearg_t)value);
      deh_state.state->args[5] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 5);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[13]))  // Args7
    {
      deh_log(" - args[6] = %lld\n", (statearg_t)value);
      deh_state.state->args[6] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 6);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[14]))  // Args8
    {
      deh_log(" - args[7] = %lld\n", (statearg_t)value);
      deh_state.state->args[7] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 7);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[15]))  // MBF21 Bits
    {
      if (bGetData == 1)
      {
        value = deh_translate_bits(value, deh_stateflags_mbf21);
      }
      else
      {
        for (value = 0; (strval = strtok(strval, deh_getBitsDelims())); strval = NULL) {
          const struct deh_flag_s *flag;

          for (flag = deh_stateflags_mbf21; flag->name; flag++) {
            if (deh_strcasecmp(strval, flag->name)) continue;

            value |= flag->value;
            break;
          }

          if (!flag->name) {
            deh_log("Could not find MBF21 frame bit mnemonic %s\n", strval);
          }
        }
      }

      deh_state.state->flags = (int)value;
    }
    else
      deh_log("Invalid frame string index for '%s'\n", key);
  }
}

const deh_block deh_block_frame = { "Frame", deh_procFrame };
