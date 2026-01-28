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
//	HHE Version
//

#include "doomdef.h"
#include "w_wad.h"
#include "deh/func.h"

#include "heretic/hhe/text.h"
#include "heretic/hhe/version.h"
#include "heretic/hhe/rex_pointer.h"
#include "heretic/hhe/rex_strings.h"
#include "heretic/hhe/sprites.h"

#include "dsda/args.h"
#include "dsda/mobjinfo.h"

// CPhipps - modify to use logical output routine
#include "lprintf.h"

// Version number for patches.

// HHE executable version.  Loading HHE patches is (unfortunately)
// dependent on the version of the Heretic executable used to make them.

const char *hhe_versions[] =
{
    "1.0", "1.2", "1.3"
};

int hhe_numversions = sizeof(hhe_versions) / sizeof(hhe_versions[0]);

deh_hhe_version_t deh_hhe_version = deh_hhe_1_0;
dboolean hhe_version_set = false;

static void HHE_PrintUnknownVersion(const char *wrong_version)
{
  int i;

  lprintf(LO_WARN, "\nUnknown Heretic version: %s\n", wrong_version);
  lprintf(LO_WARN, "Valid versions:\n");

  for (i=0; i < hhe_numversions; ++i)
  {
      lprintf(LO_WARN, "\t%s\n", hhe_versions[i]);
  }
  
  lprintf(LO_WARN, "\n");
}

void HHE_PrintValidVersion(const char *reason)
{
  lprintf(LO_INFO, "%s. Running HHE version %s\n", reason, hhe_versions[deh_hhe_version]);
}

static void HHE_SetVersionFromArg(const char *name)
{
    int i;

    for (i=0; i<hhe_numversions; ++i)
    {
        if (!strcmp(hhe_versions[i], name))
        {
            deh_hhe_version = i;
            hhe_version_set = true;
            HHE_PrintValidVersion("HHE argument detected (-hhever)");
            return;
        }
    }

    HHE_PrintUnknownVersion(name);
}

static void dsda_ParseHHEVERLump(void) {
    int num = W_CheckNumForName("HHEVER");
    int hhe = -1;

    if (num != LUMP_NOT_FOUND)
    {
      const char* data;
      char version[4] = { 0 };

      data = W_LumpByNum(num);

      strncpy(version, data, 3);
      version[3] = '\0';

      if (!strcasecmp("1.0", version))
          hhe = deh_hhe_1_0;
      else if (!strcasecmp("1.2", version))
          hhe = deh_hhe_1_2;
      else if (!strcasecmp("1.3", version))
          hhe = deh_hhe_1_3;
      else
        HHE_PrintUnknownVersion(version);
    }

    if (hhe != -1)
    {
      deh_hhe_version = hhe;
      hhe_version_set = true;
      HHE_PrintValidVersion("HHEVER lump detected");
    }
}

void HHE_GetVersionFromArgOrLump(void)
{
    //!
    // @arg <version>
    // @category mod
    //
    // Select the Heretic version number that was used to generate the
    // HHE patch to be loaded.  Patches for each of the Vanilla
    // Heretic versions (1.0, 1.2, 1.3) can be loaded, but the correct
    // version number must be specified.

    dsda_arg_t *arg = dsda_Arg(dsda_arg_hhever);

    if (arg->found)
      HHE_SetVersionFromArg(arg->value.v_string);
    else
      dsda_ParseHHEVERLump();
}

void HHE_ApplyVersion10Patch(void)
{
    // For v1.0 patches, we must apply a slight change to the states[]
    // table.  The table was changed between 1.0 and 1.3 to add two extra
    // frames to the player "burning death" animation.
    //
    // If we are using a v1.0 patch, we must change the table to cut
    // these out again.

    if (deh_hhe_version == deh_hhe_1_0)
    {
        states[HERETIC_S_PLAY_FDTH18].nextstate = HERETIC_S_NULL;
    }
}

// HHE doesn't know about the last two states in the state table, so
// these are considered invalid.
//#define HHE_HERETIC_NUMSTATES (HERETIC_NUMSTATES - 2)

// It also doesn't know about the last two things in the mobjinfo table
// (which correspond to the states above)
//#define HHE_HERETIC_NUMMOBJTYPES (HERETIC_NUMMOBJTYPES - 2)

void HHE_SuggestHereticVersion(deh_hhe_version_t version)
{
    lprintf(LO_INFO,
    "\n"
    "This patch may be for version %s. You are currently running in\n"
    "Heretic %s mode. For %s mode, add this to your command line:\n"
    "\n"
    "\t-hhever %s\n"
    "\n",
    hhe_versions[version],
    hhe_versions[deh_hhe_version],
    hhe_versions[version],
    hhe_versions[version]);
}

// Here we do voting to find which Hehacked version - :/

static int HHE_VoteStringOffset(int offset, int votes[deh_hhe_num_versions])
{
  int i, v, hit = 0;

  if (offset <= 0) return 0;

  // String Replacements
  for (i = 0; i < hhe_numstrlookup; ++i)
    for (v = 0; v < deh_hhe_num_versions; ++v)
      if (hhe_strlookup[i].offsets[v] == offset)
      {
        votes[v]++, hit = 1;
        deh_log("Found %s string replacement: %i", hhe_versions[v], offset);
      }

  // Sprite Replacements
  for (i = 0; i < hhe_numsprites; ++i)
    for (v = 0; v < deh_hhe_num_versions; ++v)
      if (hhe_sprites[i].offsets[v] == offset)
      {
        votes[v]++, hit = 1;
        deh_log("Found %s sprite replacement: %i", hhe_versions[v], offset);
      }

  //
  // These next 2 blocks are only for hhe version detection
  //

  // Unsupported Crispy Heretic strings
  for (i = 0; i < hhe_numcrispystrings; ++i)
    for (v = 0; v < deh_hhe_num_versions; ++v)
      if (crispystrings[i].offsets[v] == offset)
      {
        votes[v]++, hit = 1;
        deh_log("Unsupported %s string replacement: %i", hhe_versions[v], offset);
      }

  // Completely Unsupported strings (1.0)
  for (v = 0; v < deh_hhe_num_versions; ++v)
  {
    const int *list = unsupported_strings[v];
    if (!list) continue;

    for (i = 0; list[i] >= 0; ++i)
    {
      if (list[i] == offset)
      {
        votes[v]++, hit = 1;
        deh_log("Unsupported %s string replacement: %i", hhe_versions[v], offset);
        break; // found string, move to next list
      }
    }
  }

  return hit;
}

static int HHE_VotePointerOffset(int offset, int votes[deh_hhe_num_versions])
{
  int i, v, hit = 0;

  if (offset <= 0) return 0;

  for (i = 0; hhe_rexptrs[i].cptr != NULL; ++i)
    for (v = 0; v < deh_hhe_num_versions; ++v)
      if (hhe_rexptrs[i].offsets[v] == offset)
      {
        votes[v]++, hit = 1;
        deh_log("Found %s pointer offset: %i", hhe_versions[v], offset);
      }

  return hit;
}

int HHE_DetectVersionFromOffsets(DEHFILE *fpin, const char *label)
{
  char line[DEH_BUFFERMAX];
  int votes[deh_hhe_num_versions] = {0};

  dehfseek(fpin, 0);

  while (dehfgets(line, sizeof(line), fpin))
  {
    lfstrip(line);

    if (!line[0] || line[0] == '#' || line[0] == ' ')
      continue;

    // 1) HeHackEd Text header (offset-based)
    if (!strncasecmp(line, "Text", 4))
    {
      char key[DEH_MAXKEYLEN];
      int offset, repl_len;

      if (sscanf(line, "%31s %d %d", key, &offset, &repl_len) == 3)
        HHE_VoteStringOffset(offset, votes);

      continue;
    }

    // 2) Frame "Action pointer = <offset>" (offset-based)
    if (strchr(line, '='))
    {
      char key[DEH_MAXKEYLEN];
      uint64_t value;
      char *strval;
      int ok;

      // deh_GetData wants a mutable buffer; line is mutable here.
      ok = deh_GetData(line, key, &value, &strval);
      if (ok == 1 && !deh_strcasecmp(key, "Action pointer"))
        HHE_VotePointerOffset((int)value, votes);
    }
  }

  // pick winner + confidence gate
  {
    int best_votes = -1;
    int best = -1;      // will be highest index among those with best_votes
    int second_votes = -1;
    int v;

    // Find best_votes; if tie, prefer higher version index (newest)
    for (v = 0; v < deh_hhe_num_versions; ++v)
    {
      if (votes[v] > best_votes)
      {
        best_votes = votes[v];
        best = v;
      }
      else if (votes[v] == best_votes && v > best)
      {
        best = v; // tie -> newest
      }
    }

    // Find second_votes (highest votes among versions not equal to 'best')
    for (v = 0; v < deh_hhe_num_versions; ++v)
    {
      if (v == best) continue;
      if (votes[v] > second_votes)
        second_votes = votes[v];
    }

    // If we have at least some evidence, accept.
    // - If tie occurred, 'best' is already the newest among the tied max.
    // - If no tie, keep the old "win by +2" confidence gate.
    if (best_votes >= 2)
    {
      dboolean tie_for_best = false;

      for (v = 0; v < deh_hhe_num_versions; ++v)
        if (v != best && votes[v] == best_votes)
        {
          tie_for_best = true;
          break;
        }

      if (tie_for_best || best_votes >= second_votes + 2)
      {
        lprintf(LO_INFO,
          "Auto-detected HHE version %s (votes: 1.0 = %d, 1.2 = %d, 1.3 = %d)%s\n",
          hhe_versions[best], votes[0], votes[1], votes[2],
          tie_for_best ? " [tie -> chose newest]" : ""
        );
        return best;
      }
    }

    // Not enough confidence
    lprintf(LO_INFO,
      "HHE auto-detect inconclusive (votes: 1.0 = %d, 1.2 = %d, 1.3 = %d)\n",
      votes[0], votes[1], votes[2]
    );
    return -1;
  }
}
