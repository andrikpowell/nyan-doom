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
//	DEH [STRINGS] Parser
//

#include "doomstat.h"
#include "doomdef.h"
#include "dstrings.h"  // to get the initial values

#include "deh/func.h"
#include "deh/strings.h"

// Do this for a lookup--the pointer (loaded above) is cross-referenced
// to a string key that is the same as the define above.  We will use
// strdups to set these new values that we read from the file, orphaning
// the original value set above.

// CPhipps - make strings pointed to const
typedef struct {
  const char **ppstr;  // doubly indirect pointer to string
  const char *lookup;  // pointer to lookup string name
  const char *orig;
} deh_strs;

/* CPhipps - const
 *         - removed redundant "Can't XXX in a netgame" strings
 */
static deh_strs deh_strlookup[] = {
  {&s_PRESSKEY,"PRESSKEY"},
  {&s_PRESSYN,"PRESSYN"},
  {&s_QUITMSG,"QUITMSG"},
  {&s_QUITMSG1,"QUITMSG1"},
  {&s_QUITMSG2,"QUITMSG2"},
  {&s_QUITMSG3,"QUITMSG3"},
  {&s_QUITMSG4,"QUITMSG4"},
  {&s_QUITMSG5,"QUITMSG5"},
  {&s_QUITMSG6,"QUITMSG6"},
  {&s_QUITMSG7,"QUITMSG7"},
  {&s_QUITMSG8,"QUITMSG8"},
  {&s_QUITMSG9,"QUITMSG9"},
  {&s_QUITMSG10,"QUITMSG10"},
  {&s_QUITMSG11,"QUITMSG11"},
  {&s_QUITMSG12,"QUITMSG12"},
  {&s_QUITMSG13,"QUITMSG13"},
  {&s_QUITMSG14,"QUITMSG14"},
  {&s_QSAVESPOT,"QSAVESPOT"},
  {&s_SAVEDEAD,"SAVEDEAD"},
  {&s_NEWGAME,"NEWGAME"},
  {&s_RESTARTLEVEL,"RESTARTLEVEL"},
  {&s_NIGHTMARE,"NIGHTMARE"},
  {&s_SWSTRING,"SWSTRING"},
  {&s_MSGOFF,"MSGOFF"},
  {&s_MSGON,"MSGON"},
  {&s_NETEND,"NETEND"},
  {&s_ENDGAME,"ENDGAME"},
  {&s_DOSY,"DOSY"},
  {&s_DETAILHI,"DETAILHI"},
  {&s_DETAILLO,"DETAILLO"},
  {&s_GAMMALVL0,"GAMMALVL0"},
  {&s_GAMMALVL1,"GAMMALVL1"},
  {&s_GAMMALVL2,"GAMMALVL2"},
  {&s_GAMMALVL3,"GAMMALVL3"},
  {&s_GAMMALVL4,"GAMMALVL4"},
  {&s_EMPTYSTRING,"EMPTYSTRING"},
  {&s_GOTARMOR,"GOTARMOR"},
  {&s_GOTMEGA,"GOTMEGA"},
  {&s_GOTHTHBONUS,"GOTHTHBONUS"},
  {&s_GOTARMBONUS,"GOTARMBONUS"},
  {&s_GOTSTIM,"GOTSTIM"},
  {&s_GOTMEDINEED,"GOTMEDINEED"},
  {&s_GOTMEDIKIT,"GOTMEDIKIT"},
  {&s_GOTSUPER,"GOTSUPER"},
  {&s_GOTBLUECARD,"GOTBLUECARD"},
  {&s_GOTYELWCARD,"GOTYELWCARD"},
  {&s_GOTREDCARD,"GOTREDCARD"},
  {&s_GOTBLUESKUL,"GOTBLUESKUL"},
  {&s_GOTYELWSKUL,"GOTYELWSKUL"},
  {&s_GOTREDSKULL,"GOTREDSKULL"},
  {&s_GOTINVUL,"GOTINVUL"},
  {&s_GOTBERSERK,"GOTBERSERK"},
  {&s_GOTINVIS,"GOTINVIS"},
  {&s_GOTSUIT,"GOTSUIT"},
  {&s_GOTMAP,"GOTMAP"},
  {&s_GOTVISOR,"GOTVISOR"},
  {&s_GOTMSPHERE,"GOTMSPHERE"},
  {&s_GOTCLIP,"GOTCLIP"},
  {&s_GOTCLIPBOX,"GOTCLIPBOX"},
  {&s_GOTROCKET,"GOTROCKET"},
  {&s_GOTROCKBOX,"GOTROCKBOX"},
  {&s_GOTCELL,"GOTCELL"},
  {&s_GOTCELLBOX,"GOTCELLBOX"},
  {&s_GOTSHELLS,"GOTSHELLS"},
  {&s_GOTSHELLBOX,"GOTSHELLBOX"},
  {&s_GOTBACKPACK,"GOTBACKPACK"},
  {&s_GOTBFG9000,"GOTBFG9000"},
  {&s_GOTCHAINGUN,"GOTCHAINGUN"},
  {&s_GOTCHAINSAW,"GOTCHAINSAW"},
  {&s_GOTLAUNCHER,"GOTLAUNCHER"},
  {&s_GOTPLASMA,"GOTPLASMA"},
  {&s_GOTSHOTGUN,"GOTSHOTGUN"},
  {&s_GOTSHOTGUN2,"GOTSHOTGUN2"},
  {&s_PD_BLUEO,"PD_BLUEO"},
  {&s_PD_REDO,"PD_REDO"},
  {&s_PD_YELLOWO,"PD_YELLOWO"},
  {&s_PD_BLUEK,"PD_BLUEK"},
  {&s_PD_REDK,"PD_REDK"},
  {&s_PD_YELLOWK,"PD_YELLOWK"},
  {&s_PD_BLUEC,"PD_BLUEC"},
  {&s_PD_REDC,"PD_REDC"},
  {&s_PD_YELLOWC,"PD_YELLOWC"},
  {&s_PD_BLUES,"PD_BLUES"},
  {&s_PD_REDS,"PD_REDS"},
  {&s_PD_YELLOWS,"PD_YELLOWS"},
  {&s_PD_ANY,"PD_ANY"},
  {&s_PD_ALL3,"PD_ALL3"},
  {&s_PD_ALL6,"PD_ALL6"},
  {&s_GGSAVED,"GGSAVED"},
  {&s_HUSTR_MSGU,"HUSTR_MSGU"},
  {&s_HUSTR_E1M1,"HUSTR_E1M1"},
  {&s_HUSTR_E1M2,"HUSTR_E1M2"},
  {&s_HUSTR_E1M3,"HUSTR_E1M3"},
  {&s_HUSTR_E1M4,"HUSTR_E1M4"},
  {&s_HUSTR_E1M5,"HUSTR_E1M5"},
  {&s_HUSTR_E1M6,"HUSTR_E1M6"},
  {&s_HUSTR_E1M7,"HUSTR_E1M7"},
  {&s_HUSTR_E1M8,"HUSTR_E1M8"},
  {&s_HUSTR_E1M9,"HUSTR_E1M9"},
  {&s_HUSTR_E2M1,"HUSTR_E2M1"},
  {&s_HUSTR_E2M2,"HUSTR_E2M2"},
  {&s_HUSTR_E2M3,"HUSTR_E2M3"},
  {&s_HUSTR_E2M4,"HUSTR_E2M4"},
  {&s_HUSTR_E2M5,"HUSTR_E2M5"},
  {&s_HUSTR_E2M6,"HUSTR_E2M6"},
  {&s_HUSTR_E2M7,"HUSTR_E2M7"},
  {&s_HUSTR_E2M8,"HUSTR_E2M8"},
  {&s_HUSTR_E2M9,"HUSTR_E2M9"},
  {&s_HUSTR_E3M1,"HUSTR_E3M1"},
  {&s_HUSTR_E3M2,"HUSTR_E3M2"},
  {&s_HUSTR_E3M3,"HUSTR_E3M3"},
  {&s_HUSTR_E3M4,"HUSTR_E3M4"},
  {&s_HUSTR_E3M5,"HUSTR_E3M5"},
  {&s_HUSTR_E3M6,"HUSTR_E3M6"},
  {&s_HUSTR_E3M7,"HUSTR_E3M7"},
  {&s_HUSTR_E3M8,"HUSTR_E3M8"},
  {&s_HUSTR_E3M9,"HUSTR_E3M9"},
  {&s_HUSTR_E4M1,"HUSTR_E4M1"},
  {&s_HUSTR_E4M2,"HUSTR_E4M2"},
  {&s_HUSTR_E4M3,"HUSTR_E4M3"},
  {&s_HUSTR_E4M4,"HUSTR_E4M4"},
  {&s_HUSTR_E4M5,"HUSTR_E4M5"},
  {&s_HUSTR_E4M6,"HUSTR_E4M6"},
  {&s_HUSTR_E4M7,"HUSTR_E4M7"},
  {&s_HUSTR_E4M8,"HUSTR_E4M8"},
  {&s_HUSTR_E4M9,"HUSTR_E4M9"},
  {&s_HUSTR_1,"HUSTR_1"},
  {&s_HUSTR_2,"HUSTR_2"},
  {&s_HUSTR_3,"HUSTR_3"},
  {&s_HUSTR_4,"HUSTR_4"},
  {&s_HUSTR_5,"HUSTR_5"},
  {&s_HUSTR_6,"HUSTR_6"},
  {&s_HUSTR_7,"HUSTR_7"},
  {&s_HUSTR_8,"HUSTR_8"},
  {&s_HUSTR_9,"HUSTR_9"},
  {&s_HUSTR_10,"HUSTR_10"},
  {&s_HUSTR_11,"HUSTR_11"},
  {&s_HUSTR_12,"HUSTR_12"},
  {&s_HUSTR_13,"HUSTR_13"},
  {&s_HUSTR_14,"HUSTR_14"},
  {&s_HUSTR_15,"HUSTR_15"},
  {&s_HUSTR_16,"HUSTR_16"},
  {&s_HUSTR_17,"HUSTR_17"},
  {&s_HUSTR_18,"HUSTR_18"},
  {&s_HUSTR_19,"HUSTR_19"},
  {&s_HUSTR_20,"HUSTR_20"},
  {&s_HUSTR_21,"HUSTR_21"},
  {&s_HUSTR_22,"HUSTR_22"},
  {&s_HUSTR_23,"HUSTR_23"},
  {&s_HUSTR_24,"HUSTR_24"},
  {&s_HUSTR_25,"HUSTR_25"},
  {&s_HUSTR_26,"HUSTR_26"},
  {&s_HUSTR_27,"HUSTR_27"},
  {&s_HUSTR_28,"HUSTR_28"},
  {&s_HUSTR_29,"HUSTR_29"},
  {&s_HUSTR_30,"HUSTR_30"},
  {&s_HUSTR_31,"HUSTR_31"},
  {&s_HUSTR_32,"HUSTR_32"},
  {&s_HUSTR_33,"HUSTR_33"},
  {&s_PHUSTR_1,"PHUSTR_1"},
  {&s_PHUSTR_2,"PHUSTR_2"},
  {&s_PHUSTR_3,"PHUSTR_3"},
  {&s_PHUSTR_4,"PHUSTR_4"},
  {&s_PHUSTR_5,"PHUSTR_5"},
  {&s_PHUSTR_6,"PHUSTR_6"},
  {&s_PHUSTR_7,"PHUSTR_7"},
  {&s_PHUSTR_8,"PHUSTR_8"},
  {&s_PHUSTR_9,"PHUSTR_9"},
  {&s_PHUSTR_10,"PHUSTR_10"},
  {&s_PHUSTR_11,"PHUSTR_11"},
  {&s_PHUSTR_12,"PHUSTR_12"},
  {&s_PHUSTR_13,"PHUSTR_13"},
  {&s_PHUSTR_14,"PHUSTR_14"},
  {&s_PHUSTR_15,"PHUSTR_15"},
  {&s_PHUSTR_16,"PHUSTR_16"},
  {&s_PHUSTR_17,"PHUSTR_17"},
  {&s_PHUSTR_18,"PHUSTR_18"},
  {&s_PHUSTR_19,"PHUSTR_19"},
  {&s_PHUSTR_20,"PHUSTR_20"},
  {&s_PHUSTR_21,"PHUSTR_21"},
  {&s_PHUSTR_22,"PHUSTR_22"},
  {&s_PHUSTR_23,"PHUSTR_23"},
  {&s_PHUSTR_24,"PHUSTR_24"},
  {&s_PHUSTR_25,"PHUSTR_25"},
  {&s_PHUSTR_26,"PHUSTR_26"},
  {&s_PHUSTR_27,"PHUSTR_27"},
  {&s_PHUSTR_28,"PHUSTR_28"},
  {&s_PHUSTR_29,"PHUSTR_29"},
  {&s_PHUSTR_30,"PHUSTR_30"},
  {&s_PHUSTR_31,"PHUSTR_31"},
  {&s_PHUSTR_32,"PHUSTR_32"},
  {&s_THUSTR_1,"THUSTR_1"},
  {&s_THUSTR_2,"THUSTR_2"},
  {&s_THUSTR_3,"THUSTR_3"},
  {&s_THUSTR_4,"THUSTR_4"},
  {&s_THUSTR_5,"THUSTR_5"},
  {&s_THUSTR_6,"THUSTR_6"},
  {&s_THUSTR_7,"THUSTR_7"},
  {&s_THUSTR_8,"THUSTR_8"},
  {&s_THUSTR_9,"THUSTR_9"},
  {&s_THUSTR_10,"THUSTR_10"},
  {&s_THUSTR_11,"THUSTR_11"},
  {&s_THUSTR_12,"THUSTR_12"},
  {&s_THUSTR_13,"THUSTR_13"},
  {&s_THUSTR_14,"THUSTR_14"},
  {&s_THUSTR_15,"THUSTR_15"},
  {&s_THUSTR_16,"THUSTR_16"},
  {&s_THUSTR_17,"THUSTR_17"},
  {&s_THUSTR_18,"THUSTR_18"},
  {&s_THUSTR_19,"THUSTR_19"},
  {&s_THUSTR_20,"THUSTR_20"},
  {&s_THUSTR_21,"THUSTR_21"},
  {&s_THUSTR_22,"THUSTR_22"},
  {&s_THUSTR_23,"THUSTR_23"},
  {&s_THUSTR_24,"THUSTR_24"},
  {&s_THUSTR_25,"THUSTR_25"},
  {&s_THUSTR_26,"THUSTR_26"},
  {&s_THUSTR_27,"THUSTR_27"},
  {&s_THUSTR_28,"THUSTR_28"},
  {&s_THUSTR_29,"THUSTR_29"},
  {&s_THUSTR_30,"THUSTR_30"},
  {&s_THUSTR_31,"THUSTR_31"},
  {&s_THUSTR_32,"THUSTR_32"},
  {&s_AMSTR_FOLLOWON,"AMSTR_FOLLOWON"},
  {&s_AMSTR_FOLLOWOFF,"AMSTR_FOLLOWOFF"},
  {&s_AMSTR_GRIDON,"AMSTR_GRIDON"},
  {&s_AMSTR_GRIDOFF,"AMSTR_GRIDOFF"},
  {&s_AMSTR_MARKEDSPOT,"AMSTR_MARKEDSPOT"},
  {&s_AMSTR_MARKSCLEARED,"AMSTR_MARKSCLEARED"},
  {&s_STSTR_MUS,"STSTR_MUS"},
  {&s_STSTR_NOMUS,"STSTR_NOMUS"},
  {&s_STSTR_DQDON,"STSTR_DQDON"},
  {&s_STSTR_DQDOFF,"STSTR_DQDOFF"},
  {&s_STSTR_KFAADDED,"STSTR_KFAADDED"},
  {&s_STSTR_FAADDED,"STSTR_FAADDED"},
  {&s_STSTR_NCON,"STSTR_NCON"},
  {&s_STSTR_NCOFF,"STSTR_NCOFF"},
  {&s_STSTR_BEHOLD,"STSTR_BEHOLD"},
  {&s_STSTR_BEHOLDX,"STSTR_BEHOLDX"},
  {&s_STSTR_CHOPPERS,"STSTR_CHOPPERS"},
  {&s_STSTR_CLEV,"STSTR_CLEV"},
  {&s_STSTR_COMPON,"STSTR_COMPON"},
  {&s_STSTR_COMPOFF,"STSTR_COMPOFF"},
  {&s_E1TEXT,"E1TEXT"},
  {&s_E2TEXT,"E2TEXT"},
  {&s_E3TEXT,"E3TEXT"},
  {&s_E4TEXT,"E4TEXT"},
  {&s_C1TEXT,"C1TEXT"},
  {&s_C2TEXT,"C2TEXT"},
  {&s_C3TEXT,"C3TEXT"},
  {&s_C4TEXT,"C4TEXT"},
  {&s_C5TEXT,"C5TEXT"},
  {&s_C6TEXT,"C6TEXT"},
  {&s_P1TEXT,"P1TEXT"},
  {&s_P2TEXT,"P2TEXT"},
  {&s_P3TEXT,"P3TEXT"},
  {&s_P4TEXT,"P4TEXT"},
  {&s_P5TEXT,"P5TEXT"},
  {&s_P6TEXT,"P6TEXT"},
  {&s_T1TEXT,"T1TEXT"},
  {&s_T2TEXT,"T2TEXT"},
  {&s_T3TEXT,"T3TEXT"},
  {&s_T4TEXT,"T4TEXT"},
  {&s_T5TEXT,"T5TEXT"},
  {&s_T6TEXT,"T6TEXT"},
  {&s_CC_ZOMBIE,"CC_ZOMBIE"},
  {&s_CC_SHOTGUN,"CC_SHOTGUN"},
  {&s_CC_HEAVY,"CC_HEAVY"},
  {&s_CC_IMP,"CC_IMP"},
  {&s_CC_DEMON,"CC_DEMON"},
  {&s_CC_LOST,"CC_LOST"},
  {&s_CC_CACO,"CC_CACO"},
  {&s_CC_HELL,"CC_HELL"},
  {&s_CC_BARON,"CC_BARON"},
  {&s_CC_ARACH,"CC_ARACH"},
  {&s_CC_PAIN,"CC_PAIN"},
  {&s_CC_REVEN,"CC_REVEN"},
  {&s_CC_MANCU,"CC_MANCU"},
  {&s_CC_ARCH,"CC_ARCH"},
  {&s_CC_SPIDER,"CC_SPIDER"},
  {&s_CC_CYBER,"CC_CYBER"},
  {&s_CC_HERO,"CC_HERO"},
  {&bgflatE1,"BGFLATE1"},
  {&bgflatE2,"BGFLATE2"},
  {&bgflatE3,"BGFLATE3"},
  {&bgflatE4,"BGFLATE4"},
  {&bgflat06,"BGFLAT06"},
  {&bgflat11,"BGFLAT11"},
  {&bgflat20,"BGFLAT20"},
  {&bgflat30,"BGFLAT30"},
  {&bgflat15,"BGFLAT15"},
  {&bgflat31,"BGFLAT31"},
  {&bgcastcall,"BGCASTCALL"},
  {&s_stcdrom,"STCDROM"},
  {&s_stdisk,"STDISK"},
  {&savegamename,"SAVEGAMENAME"},  // Ty 05/03/98
  {&s_OB_CRUSH, "OB_CRUSH"},   // obituaries
  {&s_OB_SLIME, "OB_SLIME"},
  {&s_OB_LAVA, "OB_LAVA"},
  {&s_OB_KILLEDSELF, "OB_KILLEDSELF"},
  {&s_OB_VOODOO, "OB_VOODOO"},
  {&s_OB_MONTELEFRAG, "OB_MONTELEFRAG"},
  {&s_OB_DEFAULT, "OB_DEFAULT"},
  {&s_OB_MPDEFAULT, "OB_MPDEFAULT"},
  {&s_OB_UNDEADHIT, "OB_UNDEADHIT"},
  {&s_OB_IMPHIT, "OB_IMPHIT"},
  {&s_OB_CACOHIT, "OB_CACOHIT"},
  {&s_OB_DEMONHIT, "OB_DEMONHIT"},
  {&s_OB_SPECTREHIT, "OB_SPECTREHIT"},
  {&s_OB_BARONHIT, "OB_BARONHIT"},
  {&s_OB_KNIGHTHIT, "OB_KNIGHTHIT"},
  {&s_OB_ZOMBIE, "OB_ZOMBIE"},
  {&s_OB_SHOTGUY, "OB_SHOTGUY"},
  {&s_OB_VILE, "OB_VILE"},
  {&s_OB_UNDEAD, "OB_UNDEAD"},
  {&s_OB_FATSO, "OB_FATSO"},
  {&s_OB_CHAINGUY, "OB_CHAINGUY"},
  {&s_OB_SKULL, "OB_SKULL"},
  {&s_OB_IMP, "OB_IMP"},
  {&s_OB_CACO, "OB_CACO"},
  {&s_OB_BARON, "OB_BARON"},
  {&s_OB_KNIGHT, "OB_KNIGHT"},
  {&s_OB_SPIDER, "OB_SPIDER"},
  {&s_OB_BABY, "OB_BABY"},
  {&s_OB_CYBORG, "OB_CYBORG"},
  {&s_OB_WOLFSS, "OB_WOLFSS"},
  {&s_OB_MPFIST, "OB_MPFIST"},
  {&s_OB_MPCHAINSAW, "OB_MPCHAINSAW"},
  {&s_OB_MPPISTOL, "OB_MPPISTOL"},
  {&s_OB_MPSHOTGUN, "OB_MPSHOTGUN"},
  {&s_OB_MPSSHOTGUN, "OB_MPSSHOTGUN"},
  {&s_OB_MPCHAINGUN, "OB_MPCHAINGUN"},
  {&s_OB_MPROCKET, "OB_MPROCKET"},
  {&s_OB_MPPLASMARIFLE, "OB_MPPLASMARIFLE"},
  {&s_OB_MPBFG_BOOM, "OB_MPBFG_BOOM"},
  {&s_OB_MPTELEFRAG, "OB_MPTELEFRAG"},
};

static int deh_numstrlookup = sizeof(deh_strlookup) / sizeof(deh_strlookup[0]);

//
// Obituaries
//
static dboolean deh_procObituarySub(char *key, char *newstring)
{
  dboolean found = false;
  int actor = -1;

  // parse actor id
  if (sscanf(key, "Obituary_Deh_Actor_%d", &actor) == 1)
  {
    if (actor >= 0 && actor < num_mobj_types)
    {
      // decide which slot
      if (deh_StringEndsWith(key, "_Melee"))
        mobjinfo[actor].obituary_melee = Z_Strdup(newstring);
      else
        mobjinfo[actor].obituary = Z_Strdup(newstring);

      // guess we ignoring "_Self"?

      found = true;
    }
  }

  return found;
}

// ====================================================================
// deh_procStrings
// Purpose: Handle BEX [STRINGS] extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procStrings(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;      // holds the string value of the line
  static size_t maxstrlen = 128; // maximum string length, bumped 128 at
  // a time as needed
  // holds the final result of the string after concatenation
  static char *holdstring = NULL;
  dboolean found = false;  // looking for string continuation

  deh_log("Processing extended string substitution\n");

  if (!holdstring) holdstring = Z_Malloc(maxstrlen * sizeof(*holdstring));

  *holdstring = '\0';  // empty string to start with
  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  // Ty 04/24/98 - have to allow inbuffer to start with a blank for
  // the continuations of C1TEXT etc.
  while (!dehfeof(fpin) && *inbuffer)  /* && (*inbuffer != ' ') */
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    if (*inbuffer == '#') continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer && !*holdstring) break;  // killough 11/98
    if (!*holdstring) // first one--get the key
    {
      if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
      {
        deh_log("Bad data pair in '%s'\n", inbuffer);
        continue;
      }
    }
    while (strlen(holdstring) + strlen(inbuffer) > maxstrlen) // Ty03/29/98 - fix stupid error
    {
      // killough 11/98: allocate enough the first time
      maxstrlen = strlen(holdstring) + strlen(inbuffer);
      deh_log("* increased buffer from to %ld for buffer size %d\n",
              (long)maxstrlen, (int)strlen(inbuffer));
      holdstring = Z_Realloc(holdstring, maxstrlen * sizeof(*holdstring));
    }
    // concatenate the whole buffer if continuation or the value iffirst
    strcat(holdstring, ptr_lstrip(((*holdstring) ? inbuffer : strval)));
    rstrip(holdstring);
    // delete any trailing blanks past the backslash
    // note that blanks before the backslash will be concatenated
    // but ones at the beginning of the next line will not, allowing
    // indentation in the file to read well without affecting the
    // string itself.
    if (holdstring[strlen(holdstring) - 1] == '\\')
    {
      holdstring[strlen(holdstring) - 1] = '\0';
      continue; // ready to concatenate
    }
    if (*holdstring) // didn't have a backslash, trap above would catch that
    {
      // go process the current string
      found = deh_procStringSub(key, NULL, holdstring);  // supply keyand not search string

      // Obituaries
      if (!found)
        deh_procObituarySub(key, holdstring);

      if (!found)
        deh_log("Invalid string key '%s', substitution skipped.\n", key);

      *holdstring = '\0';  // empty string for the next one
    }
  }
}

// ====================================================================
// deh_procStringSub
// Purpose: Common string parsing and handling routine for DEH and BEX
// Args:    key       -- place to put the mnemonic for the string if found
//          lookfor   -- original value string to look for
//          newstring -- string to put in its place if found
// Returns: dboolean: True if string found, false if not
//
dboolean deh_procStringSub(char *key, char *lookfor, char *newstring)
{
  dboolean found; // loop exit flag
  int i;  // looper

  found = false;
  for (i = 0; i < deh_numstrlookup; i++)
  {
    if (deh_strlookup[i].orig == NULL)
    {
      deh_strlookup[i].orig = *deh_strlookup[i].ppstr;
    }
    found = lookfor ?
      !stricmp(deh_strlookup[i].orig, lookfor) :
      !stricmp(deh_strlookup[i].lookup, key);

    if (found)
    {
      char *t;
      *deh_strlookup[i].ppstr = t = Z_Strdup(newstring); // orphan originalstring
      found = true;
      // Handle embedded \n's in the incoming string, convert to 0x0a's
      {
        const char *s;
        for (s = *deh_strlookup[i].ppstr; *s; ++s, ++t)
        {
          if (*s == '\\' && (s[1] == 'n' || s[1] == 'N')) //found one
            ++s, *t = '\n';  // skip one extra for second character
          else
            *t = *s;
        }
        *t = '\0';  // cap off the target string
      }

      if (key)
        deh_log("Assigned key %s => '%s'\n", key, newstring);

      if (!key)
        deh_log("Assigned '%.12s%s' to'%.12s%s' at key %s\n",
                lookfor, (strlen(lookfor) > 12) ? "..." : "",
                newstring, (strlen(newstring) > 12) ? "..." :"",
                deh_strlookup[i].lookup);

      if (!key) // must have passed an old style string so showBEX
        deh_log("*BEX FORMAT:\n%s = %s\n*END BEX\n",
                deh_strlookup[i].lookup, dehReformatStr(newstring));

      break;
    }
  }
  if (!found)
    deh_log("Could not find '%.12s'\n", key ? key : lookfor);

  return found;
}

const deh_block deh_block_bex_strings = { "[STRINGS]", deh_procStrings };
