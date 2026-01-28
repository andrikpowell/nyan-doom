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
//	DEH Parser
//

#include "p_inter.h"
#include "doomdef.h"
#include "p_enemy.h"
#include "p_map.h"
#include "w_wad.h"

#include "deh/thing.h"
#include "deh/frame.h"
#include "deh/text.h"
#include "deh/cheat.h"
#include "deh/misc.h"
#include "deh/strings.h"
#include "deh/compatibility.h"

#include "deh/bex_pointer.h"

#include "dsda/state.h"
#include "dsda/sprite.h"
#include "dsda/sfx.h"
#include "dsda/music.h"
#include "dsda/mobjinfo.h"

// CPhipps - modify to use logical output routine
#include "lprintf.h"

// Error block
static void deh_procError(DEHFILE *fpin, char *line)
{
  char inbuffer[DEH_BUFFERMAX];

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  deh_log("Unmatched Block: '%s'\n", inbuffer);
}

const deh_block deh_block_error = { "", deh_procError };

// Grab info from all deh blocks
extern const deh_block deh_block_thing;
extern const deh_block deh_block_frame;
extern const deh_block deh_block_pointer;
extern const deh_block deh_block_sounds;
extern const deh_block deh_block_ammo;
extern const deh_block deh_block_weapon;
extern const deh_block deh_block_sprite;
extern const deh_block deh_block_cheat;
extern const deh_block deh_block_misc;
extern const deh_block deh_block_text;
extern const deh_block deh_block_bex_strings;
extern const deh_block deh_block_bex_pars;
extern const deh_block deh_block_bex_pointers;
extern const deh_block deh_block_bex_helper;
extern const deh_block deh_block_bex_sprites;
extern const deh_block deh_block_bex_sounds;
extern const deh_block deh_block_bex_music;

// Put all the block header values, and the function to be called when that
// one is encountered, in this array:
static const deh_block *deh_blocks[] = { // CPhipps - static const
  /* 0 */  &deh_block_thing,
  /* 1 */  &deh_block_frame,
  /* 2 */  &deh_block_pointer,
  /* 3 */  &deh_block_sounds,  // Ty 03/16/98 corrected from "Sounds"
  /* 4 */  &deh_block_ammo,
  /* 5 */  &deh_block_weapon,
  /* 6 */  &deh_block_sprite,
  /* 7 */  &deh_block_cheat,
  /* 8 */  &deh_block_misc,
  /* 9 */  &deh_block_text,  // --  end of standard "deh" entries,

  //     begin BOOM Extensions (BEX)

  /* 10 */ &deh_block_bex_strings,  // [STRINGS] new string changes
  /* 11 */ &deh_block_bex_pars,     // [PARS] alternative block marker
  /* 12 */ &deh_block_bex_pointers, // [CODEPTR] bex codepointers by mnemonic
  /* 13 */ &deh_block_bex_helper,   // [HELPER] helper thing substitution haleyjd 9/22/99
  /* 14 */ &deh_block_bex_sprites,  // [SPRITES] bex style sprites
  /* 15 */ &deh_block_bex_sounds,   // [SOUNDS] bex style sounds
  /* 16 */ &deh_block_bex_music,    // [MUSIC] bex style music
  /* 17 */ &deh_block_error         // dummy to handle anything else
};

#define DEH_BLOCKMAX (sizeof(deh_blocks) / sizeof(*deh_blocks))  // size of array



// ====================================================================
// ProcessDehFile
// Purpose: Read and process a DEH or BEX file
// Args:    filename    -- name of the DEH/BEX file
//          outfilename -- output file (DEHOUT.TXT), appended to here
// Returns: void
//
// killough 10/98:
// substantially modified to allow input from wad lumps instead of .deh files.

static int processed_dehacked;

void ProcessDehFile(const char *filename, const char *outfilename, int lumpnum)
{
  DEHFILE infile, *filein = &infile;    // killough 10/98
  char inbuffer[DEH_BUFFERMAX];  // Place to put the primary infostring
  const char *file_or_lump;
  static unsigned last_block;
  static long filepos;

  processed_dehacked = true;

  // Open output file if we're writing output
  if (outfilename && *outfilename && !deh_log_file)
  {
    static dboolean firstfile = true; // to allow append to output log
    if (!strcmp(outfilename, "-"))
      deh_log_file = stdout;
    else if (!(deh_log_file = M_OpenFile(outfilename, firstfile ? "wt" : "at")))
    {
      lprintf(LO_WARN, "Could not open -dehout file %s\n... using stdout.\n", outfilename);
      deh_log_file = stdout;
    }
    firstfile = false;
  }

  // killough 10/98: allow DEH files to come from wad lumps

  if (filename)
  {
    if (!(infile.f = M_OpenFile(filename, "rt")))
    {
      lprintf(LO_WARN, "-deh file %s not found\n", filename);
      return;  // should be checked up front anyway
    }
    infile.lump = NULL;
    file_or_lump = "file";
  }
  else  // DEH file comes from lump indicated by third argument
  {
    infile.size = W_LumpLength(lumpnum);
    infile.inp = infile.lump = W_LumpByNum(lumpnum);
    // [FG] skip empty DEHACKED lumps
    if (!infile.inp)
    {
      lprintf(LO_WARN, "skipping empty DEHACKED (%d) lump\n", lumpnum);
      return;
    }
    filename = lumpinfo[lumpnum].wadfile->name;
    file_or_lump = "lump from";
  }

  lprintf(LO_INFO, "Loading DEH %s %s\n", file_or_lump, filename);
  deh_log("\nLoading DEH %s %s\n\n", file_or_lump, filename);

  // loop until end of file

  last_block = DEH_BLOCKMAX - 1;
  filepos = 0;
  while (dehfgets(inbuffer,sizeof(inbuffer),filein))
  {
    dboolean match;
    unsigned i;

    lfstrip(inbuffer);
    deh_log("Line='%s'\n", inbuffer);
    if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ')
      continue; /* Blank line or comment line */

    // -- If DEH_BLOCKMAX is set right, the processing is independently
    // -- handled based on data in the deh_blocks[] structure array

    // killough 10/98: INCLUDE code rewritten to allow arbitrary nesting,
    // and to greatly simplify code, fix memory leaks, other bugs

    if (!strnicmp(inbuffer, "INCLUDE", 7)) // include a file
    {
      // preserve state while including a file
      // killough 10/98: moved to here

      char *nextfile;
      dboolean oldnotext = deh_includenotext;       // killough 10/98

      // killough 10/98: exclude if inside wads (only to discourage
      // the practice, since the code could otherwise handle it)

      if (infile.lump)
      {
        deh_log("No files may be included from wads: %s\n", inbuffer);
        continue;
      }

      // check for no-text directive, used when including a DEH
      // file but using the BEX format to handle strings

      if (!strnicmp(nextfile = ptr_lstrip(inbuffer + 7), "NOTEXT", 6))
        deh_includenotext = true, nextfile = ptr_lstrip(nextfile + 6);

      deh_log("Branching to include file %s...\n", nextfile);

      // killough 10/98:
      // Second argument must be NULL to prevent closing deh_log_file too soon

      ProcessDehFile(nextfile, NULL, 0); // do the included file

      deh_includenotext = oldnotext;
      deh_log("...continuing with %s\n", filename);
      continue;
    }

    for (match = 0, i = 0; i < DEH_BLOCKMAX - 1; i++)
      if (!strncasecmp(inbuffer, deh_blocks[i]->key, strlen(deh_blocks[i]->key)))
      { // matches one
        match = 1;
        break;  // we got one, that's enough for this block
      }

    if (match) // inbuffer matches a valid block code name
      last_block = i;
    else if (last_block >= 10 && last_block < DEH_BLOCKMAX - 1) // restrict to BEX style lumps
    { // process that same line again with the last valid block code handler
      i = last_block;
      dehfseek(filein, filepos);
    }

    deh_log("Processing function [%d] for %s\n", i, deh_blocks[i]->key);
    deh_blocks[i]->fptr(filein, inbuffer);  // call function

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

  deh_applyCompatibility();
}

static deh_bexptr null_bexptr = { NULL, "(NULL)" };

void PostProcessDeh(void)
{
  int i, j;
  const deh_bexptr *bexptr_match;

  // sanity-check bfgcells and bfg ammopershot
  if (
    bfgcells_modified &&
    weaponinfo[wp_bfg].intflags & WIF_ENABLEAPS &&
    bfgcells != weaponinfo[wp_bfg].ammopershot
  )
    I_Error("Mismatch between bfgcells and bfg ammo per shot modifications! Check your dehacked.");

  if (processed_dehacked)
  {
    extern byte* defined_codeptr_args;

    for (i = 0; i < num_states; i++)
    {
      bexptr_match = &null_bexptr;

      for (j = 0; deh_bexptrs[j].cptr != NULL; ++j)
        if (states[i].action == deh_bexptrs[j].cptr)
        {
          bexptr_match = &deh_bexptrs[j];
          break;
        }

      // ensure states don't use more mbf21 args than their
      // action pointer expects, for future-proofing's sake
      for (j = MAXSTATEARGS - 1; j >= bexptr_match->argcount; j--)
        if (states[i].args[j] != 0)
          I_Error("Action %s on state %d expects no more than %d nonzero args (%d found). Check your dehacked.",
            bexptr_match->lookup, i, bexptr_match->argcount, j+1);

      // replace unset fields with default values
      for (; j >= 0; j--)
        if (!(defined_codeptr_args[i] & (1 << j)))
          states[i].args[j] = bexptr_match->default_args[j];

      // State arguments that refer to thing indices need to be translated
      if (bexptr_match->ti_flags)
      {
        short args_i;
        short ti_flags = bexptr_match->ti_flags;

        if (ti_flags & TI_MISC1)
          states[i].misc1 = dsda_TranslateDehMobjIndex(states[i].misc1);

        if (ti_flags & TI_MISC2)
          states[i].misc2 = dsda_TranslateDehMobjIndex(states[i].misc2);

        ti_flags >>= TI_ARGSSHIFT;
        for (args_i = 0; args_i < MAXSTATEARGS; ++args_i)
          if (ti_flags & (1 << args_i))
            states[i].args[args_i] = dsda_TranslateDehMobjIndex((int)states[i].args[args_i]);
      }

      // Flags specifications aren't cross-port consistent -> must translate / mask bits
      if (bexptr_match->cptr == A_AddFlags || bexptr_match->cptr == A_RemoveFlags)
      {
        states[i].args[0] = deh_translate_bits(states[i].args[0], deh_mobjflags_standard);
        states[i].args[1] = deh_translate_bits(states[i].args[1], deh_mobjflags_mbf21);
      }
      else if (bexptr_match->cptr == A_JumpIfFlagsSet)
      {
        states[i].args[1] = deh_translate_bits(states[i].args[1], deh_mobjflags_standard);
        states[i].args[2] = deh_translate_bits(states[i].args[2], deh_mobjflags_mbf21);
      }
    }
  }

  // [FG] fix desyncs by SSG-flash correction
  dsda_SSGFrameFix();

  dsda_FreeDehStates();
  dsda_FreeDehSprites();
  dsda_FreeDehSFX();
  dsda_FreeDehMusic();
  dsda_FreeDehMobjInfo();
}
