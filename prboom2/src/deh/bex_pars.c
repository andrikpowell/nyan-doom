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
//	DEH [PARS] Parser
//

#include "g_game.h"
#include "deh/func.h"
#include "doomdef.h"
#include "m_misc.h"

dboolean deh_pars = false; // in wi_stuff to allow pars in modified games

// ====================================================================
// deh_procPars
// Purpose: Handle BEX extension for PAR times
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPars(DEHFILE *fpin, char *line) // extension
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;
  int episode, level, partime, oldpar;

  // new item, par times
  // usage: After [PARS] Par 0 section identifier, use one or more of these
  // lines:
  //  par 3 5 120
  //  par 14 230
  // The first would make the par for E3M5 be 120 seconds, and the
  // second one makes the par for MAP14 be 230 seconds.  The number
  // of parameters on the line determines which group of par values
  // is being changed.  Error checking is done based on current fixed
  // array sizes of[4][10] and [32]

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Par value at index %d: %s\n", indexnum, key);
  // indexnum is a dummy entry
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(M_Strlwr(inbuffer)); // lowercase it
    if (!*inbuffer) break;      // killough 11/98
    if (3 != sscanf(inbuffer, "par %i %i %i", &episode, &level, &partime))
    { // not 3
      if (2 != sscanf(inbuffer, "par %i %i", &level, &partime))
      { // not 2
        deh_log("Invalid par time setting string: %s\n", inbuffer);
      }
      else
      { // is 2
        // Ty 07/11/98 - wrong range check, not zero-based
        if (level < 1 || level > 32) // base 0 array (but 1-based parm)
        {
          deh_log("Invalid MAPnn value MAP%d\n", level);
        }
        else
        {
          oldpar = cpars[level-1];
          deh_log("Changed par time for MAP%02d from %d to %d\n", level, oldpar, partime);
          cpars[level - 1] = partime;
          deh_pars = true;
        }
      }
    }
    else
    { // is 3
      // note that though it's a [4][10] array, the "left" and "top" aren't used,
      // effectively making it a base 1 array.
      // Ty 07/11/98 - level was being checked against max 3 - dumb error
      // Note that episode 4 does not have par times per original design
      // in Ultimate DOOM so that is not supported here.
      if (episode < 1 || episode > 3 || level < 1 || level > 9)
      {
        deh_log("Invalid ExMx values E%dM%d\n", episode, level);
      }
      else
      {
        oldpar = pars[episode][level];
        pars[episode][level] = partime;
        deh_log("Changed par time for E%dM%d from %d to %d\n", episode, level, oldpar, partime);
        deh_pars = true;
      }
    }
  }
}

const deh_block deh_block_bex_pars = { "[PARS]", deh_procPars };
