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
//	HHE Parser
//

#include "doomdef.h"
#include "w_wad.h"

#include "deh/func.h"

#include "heretic/hhe/text.h"
#include "heretic/hhe/rex_pointer.h"
#include "heretic/hhe/version.h"

#include "dsda/state.h"
#include "dsda/sprite.h"
#include "dsda/sfx.h"
#include "dsda/music.h"
#include "dsda/mobjinfo.h"

// CPhipps - modify to use logical output routine
#include "lprintf.h"

// Error block
static void hhe_procError(DEHFILE *fpin, char *line)
{
  char inbuffer[DEH_BUFFERMAX];

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  deh_log("Unmatched Block: '%s'\n", inbuffer);
}

const deh_block hhe_block_error = { "", hhe_procError };

// Grab info from all hhe blocks
extern const deh_block hhe_block_thing;
extern const deh_block hhe_block_frame;
extern const deh_block hhe_block_sounds;
extern const deh_block hhe_block_ammo;
extern const deh_block hhe_block_weapon;
extern const deh_block hhe_block_text;
extern const deh_block hhe_block_rex_strings;
extern const deh_block hhe_block_rex_pointers;
extern const deh_block hhe_block_rex_sprites;
extern const deh_block hhe_block_rex_sounds;
extern const deh_block hhe_block_rex_music;

// Put all the block header values, and the function to be called when that
// one is encountered, in this array:
static const deh_block *hhe_blocks[] = { // CPhipps - static const
  /* 0 */  &hhe_block_thing,
  /* 1 */  &hhe_block_frame,
  /* 2 */  // Pointers are handled in frame definiton
  /* 3 */  &hhe_block_sounds,  // Ty 03/16/98 corrected from "Sounds"
  /* 4 */  &hhe_block_ammo,
  /* 5 */  &hhe_block_weapon,
  /* 6 */  // Sprites are handled in text definition
  /* 7 */  // HHE does not have "cheats" block
  /* 8 */  // HHE does not have "misc" block
  /* 9 */  &hhe_block_text,  // --  end of standard "hhe" entries,

  //     begin RAVEN Extensions (REX)

  /* 10 */ &hhe_block_rex_strings,  // [STRINGS] new string changes
  /* 11 */ // Heretic does not have PARS
  /* 12 */ &hhe_block_rex_pointers, // [CODEPTR] rex codepointers by mnemonic
  /* 13 */ // Heretic does not have helper dogs
  /* 14 */ &hhe_block_rex_sprites,  // [SPRITES] rex style sprites
  /* 15 */ &hhe_block_rex_sounds,   // [SOUNDS] rex style sounds
  /* 16 */ &hhe_block_rex_music,    // [MUSIC] rex style music
  /* 17 */ &hhe_block_error         // dummy to handle anything else
};

#define HHE_BLOCKMAX (sizeof(hhe_blocks) / sizeof(*hhe_blocks))  // size of array

// ====================================================================
// ProcessHheFile
// Purpose: Read and process a HHE or REX file
// Args:    filename    -- name of the HHE/REX file
//          outfilename -- output file (HHEOUT.TXT), appended to here
// Returns: void
//
// killough 10/98:
// substantially modified to allow input from wad lumps instead of .hhe files.

static int processed_hehacked;

void ProcessHheFile(const char *filename, const char *outfilename, int lumpnum)
{
  DEHFILE infile, *filein = &infile;    // killough 10/98
  char inbuffer[DEH_BUFFERMAX];  // Place to put the primary infostring
  const char *file_or_lump;
  static unsigned last_block;
  static long filepos;

  processed_hehacked = true;

  // Open output file if we're writing output
  if (outfilename && *outfilename && !deh_log_file)
  {
    static dboolean firstfile = true; // to allow append to output log
    if (!strcmp(outfilename, "-"))
      deh_log_file = stdout;
    else if (!(deh_log_file = M_OpenFile(outfilename, firstfile ? "wt" : "at")))
    {
      lprintf(LO_WARN, "Could not open -hheout file %s\n... using stdout.\n", outfilename);
      deh_log_file = stdout;
    }
    firstfile = false;
  }

  // killough 10/98: allow HHE files to come from wad lumps

  if (filename)
  {
    if (!(infile.f = M_OpenFile(filename, "rt")))
    {
      lprintf(LO_WARN, "-hhe file %s not found\n", filename);
      return;  // should be checked up front anyway
    }
    infile.lump = NULL;
    file_or_lump = "file";
  }
  else  // HHE file comes from lump indicated by third argument
  {
    infile.size = W_LumpLength(lumpnum);
    infile.inp = infile.lump = W_LumpByNum(lumpnum);
    // [FG] skip empty HEHACKED lumps
    if (!infile.inp)
    {
      lprintf(LO_WARN, "skipping empty HEHACKED (%d) lump\n", lumpnum);
      return;
    }
    filename = lumpinfo[lumpnum].wadfile->name;
    file_or_lump = "lump from";
  }

  lprintf(LO_INFO, "Loading HHE %s %s\n", file_or_lump, filename);
  deh_log("\nLoading HHE %s %s\n\n", file_or_lump, filename);

  // picks arg/lump/default
  if (!hhe_version_set)
    HHE_GetVersionFromArgOrLump();

  // auto-detect only if not set
  if (!hhe_version_set)
  {
    int detected;
    lprintf(LO_INFO, "Checking HHE version from first patch\n");

    detected = HHE_DetectVersionFromOffsets(filein, filename);

    if (detected >= 0)
    {
      deh_hhe_version = detected;
      HHE_PrintValidVersion("Auto-detected from offsets");
      hhe_version_set = true; // lock once auto decides
    }
    else
    {
      lprintf(LO_INFO, "No HHE version detected. Defaulting to HHE version %s\n", hhe_versions[deh_hhe_version]);
    }

    hhe_version_set = true;

    // IMPORTANT - rewind stream so the real parser starts from the beginning
    dehfseek(filein, 0);
  }

  // For v1.0 patches, we must apply a slight change to the states[]
  // table.  The table was changed between 1.0 and 1.3 to add two extra
  // frames to the player "burning death" animation.
  //
  // If we are using a v1.0 patch, we must change the table to cut
  // these out again.
  if (deh_hhe_version == deh_hhe_1_0)
    HHE_ApplyVersion10Patch();

  // loop until end of file

  last_block = HHE_BLOCKMAX - 1;
  filepos = 0;
  while (dehfgets(inbuffer,sizeof(inbuffer),filein))
  {
    dboolean match;
    unsigned i;

    lfstrip(inbuffer);
    deh_log("Line='%s'\n", inbuffer);
    if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ')
      continue; /* Blank line or comment line */

    // -- If HHE_BLOCKMAX is set right, the processing is independently
    // -- handled based on data in the hhe_blocks[] structure array

    // killough 10/98: INCLUDE code rewritten to allow arbitrary nesting,
    // and to greatly simplify code, fix memory leaks, other bugs

    if (!strnicmp(inbuffer, "INCLUDE", 7)) // include a file
    {
      // preserve state while including a file
      // killough 10/98: moved to here

      char *nextfile;
      dboolean oldnotext = hhe_includenotext;       // killough 10/98

      // killough 10/98: exclude if inside wads (only to discourage
      // the practice, since the code could otherwise handle it)

      if (infile.lump)
      {
        deh_log("No files may be included from wads: %s\n", inbuffer);
        continue;
      }

      // check for no-text directive, used when including a HHE
      // file but using the REX format to handle strings

      if (!strnicmp(nextfile = ptr_lstrip(inbuffer + 7), "NOTEXT", 6))
        hhe_includenotext = true, nextfile = ptr_lstrip(nextfile + 6);

      deh_log("Branching to include file %s...\n", nextfile);

      // killough 10/98:
      // Second argument must be NULL to prevent closing deh_log_file too soon

      ProcessHheFile(nextfile, NULL, 0); // do the included file

      hhe_includenotext = oldnotext;
      deh_log("...continuing with %s\n", filename);
      continue;
    }

    for (match = 0, i = 0; i < HHE_BLOCKMAX - 1; i++)
      if (!strncasecmp(inbuffer, hhe_blocks[i]->key, strlen(hhe_blocks[i]->key)))
      { // matches one
        match = 1;
        break;  // we got one, that's enough for this block
      }

    if (match) // inbuffer matches a valid block code name
      last_block = i;
    else if (last_block >= 10 && last_block < HHE_BLOCKMAX - 1) // restrict to REX style lumps
    { // process that same line again with the last valid block code handler
      i = last_block;
      dehfseek(filein, filepos);
    }

    deh_log("Processing function [%d] for %s\n", i, hhe_blocks[i]->key);
    hhe_blocks[i]->fptr(filein, inbuffer);  // call function

    filepos = dehftell(filein);
  }

  if (!infile.lump)
    fclose(infile.f);                         // Close real file

  if (outfilename)   // killough 10/98: only at top recursion level
  {
    if (deh_log_file != stdout)
      fclose(deh_log_file);
    deh_log_file = NULL;
  }
}

void PostProcessHhe(void)
{
  dsda_FreeDehStates();
  dsda_FreeDehSprites();
  dsda_FreeDehSFX();
  dsda_FreeDehMusic();
  dsda_FreeDehMobjInfo();
}
