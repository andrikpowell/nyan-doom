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
//	DEH Text
//

#include "doomstat.h"
#include "doomdef.h"
#include "dstrings.h"  // to get the initial values

#include "deh/func.h"
#include "deh/strings.h"

#include "dsda/sprite.h"
#include "dsda/music.h"
#include "dsda/sfx.h"

// TEXT - Dehacked block name = "Text"
// Usage: Text fromlen tolen
// Dehacked allows a bit of adjustment to the length (why?)

// flag to skip included deh-style text, used with INCLUDE NOTEXT directive
dboolean deh_includenotext = false;

// ====================================================================
// deh_procText
// Purpose: Handle DEH Text block
// Notes:   We look things up in the current information and if found
//          we replace it.  At the same time we write the new and
//          improved BEX syntax to the log file for future use.
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procText(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX * 2];  // can't use line -- double size buffer too.
  int i; // loop variable
  int fromlen, tolen;  // as specified on the text block line
  int usedlen;  // shorter of fromlen and tolen if not matched
  dboolean found = false;  // to allow early exit once found
  char* line2 = NULL;   // duplicate line for rerouting

  // Ty 04/11/98 - Included file may have NOTEXT skip flag set
  if (deh_includenotext) // flag to skip included deh-style text
  {
    deh_log("Skipped text block because of notext directive\n");
    strcpy(inbuffer,line);
    while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
      dehfgets(inbuffer, sizeof(inbuffer), fpin);  // skip block
    // Ty 05/17/98 - don't care if this fails
    return; // ************** Early return
  }

  // killough 8/98: allow hex numbers in input:
  sscanf(line, "%s %i %i", key, &fromlen, &tolen);
  deh_log("Processing Text (key=%s, from=%d, to=%d)\n", key, fromlen, tolen);

  // killough 10/98: fix incorrect usage of feof
  {
    int c, totlen = 0;
    while (totlen < fromlen + tolen && (c = dehfgetc(fpin)) != EOF)
      if (c != '\r')
        inbuffer[totlen++] = c;
    inbuffer[totlen]='\0';
  }

  // if the from and to are 4, this may be a sprite rename.  Check it
  // against the array and process it as such if it matches.  Remember
  // that the original names are (and should remain) uppercase.
  // Future: this will be from a separate [SPRITES] block.
  if (fromlen == 4 && tolen == 4)
  {
    i = dsda_GetDehSpriteIndex(inbuffer);

    if (i >= 0)
    {
      char *s;

      deh_log("Changing name of sprite at index %d from %s to %*s\n",
              i, sprnames[i], tolen, &inbuffer[fromlen]);

      // CPhipps - fix constness problem
      sprnames[i] = s = Z_Strdup(sprnames[i]);
      strncpy(s, &inbuffer[fromlen], tolen);

      found = true;
    }
  }

  if (!found && fromlen < 7 && tolen < 7)  // lengths of music and sfx are 6 or shorter
  {
    usedlen = (fromlen < tolen) ? fromlen : tolen;
    if (fromlen != tolen)
      deh_log("Warning: Mismatched lengths from=%d, to=%d, used %d\n", fromlen, tolen, usedlen);

    i = dsda_GetDehSFXIndex(inbuffer, (size_t) fromlen);
    if (i >= 0)
    {
      deh_log("Changing name of sfx from %s to %*s\n",
              S_sfx[i].name, usedlen, &inbuffer[fromlen]);

      S_sfx[i].name = deh_sfx_name(&inbuffer[fromlen]);

      found = true;
    }
    else
    {
      i = dsda_GetDehMusicIndex(inbuffer, (size_t) fromlen);
      if (i >= 0)
      {
        deh_log("Changing name of music from %s to %*s\n",
                S_music[i].name, usedlen, &inbuffer[fromlen]);

        S_music[i].name = Z_Strdup(&inbuffer[fromlen]);

        found = true;
      }
    }
  }

  if (!found) // Nothing we want to handle here--see if strings can deal with it.
  {
    deh_log("Checking text area through strings for '%.12s%s' from=%d to=%d\n",
            inbuffer, (strlen(inbuffer) > 12) ? "..." : "", fromlen, tolen);
    if ((size_t)fromlen <= strlen(inbuffer))
    {
      line2 = Z_Strdup(&inbuffer[fromlen]);
      inbuffer[fromlen] = '\0';
    }

    deh_procStringSub(NULL, inbuffer, line2);
  }
  Z_Free(line2); // may be NULL, ignored by free()
}

const deh_block deh_block_text = { "Text", deh_procText };
