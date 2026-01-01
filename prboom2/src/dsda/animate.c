//
// Copyright(C) 2024 by Andrik Powell
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
//	DSDA Animate
//

#include <string.h>
#include <ctype.h>

#include "library.h"
#include "lprintf.h"
#include "m_menu.h"
#include "p_spec.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda/utility.h"

#include "animate.h"


int animateLumps;
int widescreenLumps;

#define MAXANIMS 32
static animate_t* n_lastanim;
static animate_t* n_anims;
static size_t n_maxanims;

void AnimateTicker(void)
{
  AnimateTime++;
}

static void N_CombinePrefixedLump(char out[9], const char *prefix, const char *name);
static int  N_CheckNumForPrefixedName(const char *prefix, const char *base);

/*static int N_ParseAnimateLump(char** lines, int line_i) {
  int count;
  const char* line;
  static char lump[9] = { 0 };
  static char start[9] = { 0 };
  static char end[9] = { 0 };
  static int speed;
  char args[64];

  for (++line_i; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (line[0] == '#' || line[0] == '/' || line[0] == '!' || !line[0])
      continue;

    count = sscanf(line, "%63[^\n\r]", args);
    if (count != 1)
        I_Error("Invalid NYANANIM definition \"%s\"", line);

    // The start of another definition
    if (!strncmp(args, "doom", sizeof(args)) ||
        !strncmp(args, "heretic", sizeof(args)) ||
        !strncmp(args, "hexen", sizeof(args)))
        break;

    count = sscanf(args, "\"%8[^\"]\" \"%8[^\"]\" \"%8[^\"]\" %d", lump, start, end, &speed);
    if (count != 4)
        I_Error("Invalid NYANANIM arguments \"%s\"", line);

    N_AddPatchAnimateLump(lump, start, end, speed);
  }

  // roll back the line that wasn't part of this config
  return line_i - 1;
}

static void N_LoadAnimateLump(void) {
  DO_ONCE
    char* animate_lump = NULL;
    char** lines;
    const char* line;
    int line_i;
    const char* target_format;
    int lump;
    int length = 0;

    lump = -1;
    while ((lump = W_FindNumFromName("NYANANIM", lump)) >= 0) {
      if (!animate_lump) {
        animate_lump = W_ReadLumpToString(lump);
        length = W_LumpLength(lump);
      }
      else {
        animate_lump = Z_Realloc(animate_lump, length + W_LumpLength(lump) + 2);
        animate_lump[length++] = '\n'; // in case the file didn't end in a new line
        W_ReadLump(lump, animate_lump + length);
        length += W_LumpLength(lump);
        animate_lump[length] = '\0';
      }
    }

    if (animate_lump) {
      lines = dsda_SplitString(animate_lump, "\n\r");

      if (lines) {
        target_format = hexen ? "hexen" : heretic ? "heretic" : "doom";

        for (line_i = 0; lines[line_i]; ++line_i) {
            line = lines[line_i];

            if (target_format) {
                line_i = N_ParseAnimateLump(lines, line_i);
                break;
            }
        }

        Z_Free(lines);
      }

      Z_Free(animate_lump);
    }
  END_ONCE
}*/

void N_InitAnimateLumps(void) {
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);

        N_AddPatchAnimateLump("M_SKULL1", "S_SKULL", "E_SKULL", 8);
        N_AddPatchAnimateLump("M_DOOM", "S_DOOM", "E_DOOM", 8);
        N_AddPatchAnimateLump("STBAR", "S_STBAR", "E_STBAR", 8);
        // Disabled for now
        //N_LoadAnimateLump();
    }
}

void N_ReloadAnimateLumps(void)
{
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);
    }
}

static size_t N_AnimCount(void)
{
    return (n_anims && n_lastanim) ? (size_t)(n_lastanim - n_anims) : 0;
}

const int N_CheckWide(const char* lump)
{
    size_t i;
    size_t used = N_AnimCount();
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return false;

    for (i = 0; i < used; i++)
        if (n_anims[i].lump == lumpnum &&
            n_anims[i].widescrn != LUMP_NOT_FOUND)
            return true;

    return false;
}

static int N_CheckAnimateCycle(int SLump, int ELump)
{
    if ((SLump != LUMP_NOT_FOUND) && (ELump != LUMP_NOT_FOUND))
        if (SLump < ELump)
            return true;

    return false;
}

const int N_CheckAnimate(const char* lump)
{
    size_t i;
    size_t used = N_AnimCount();
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return false;

    for (i = 0; i < used; i++)
    {
        if (n_anims[i].lump == lumpnum)
        {
            if (N_CheckAnimateCycle(n_anims[i].ani_start, n_anims[i].ani_end))
                return true;
        }
    }

    return false;
}

static void N_ExtendAnimateLimit(void)
{
    if (!n_anims) {
        n_maxanims = MAXANIMS;
        n_anims = Z_Malloc(n_maxanims * sizeof(*n_anims));
        n_lastanim = n_anims; // 0 used
        return;
    }

    if (n_lastanim >= n_anims + n_maxanims) {
        size_t used = (size_t)(n_lastanim - n_anims);
        size_t newmax = n_maxanims * 2;

        n_anims = Z_Realloc(n_anims, newmax * sizeof(*n_anims));
        n_maxanims = newmax;
        n_lastanim = n_anims + used;
    }
}

void N_AddPatchAnimateLump(const char* lump, const char* slump, const char* elump, int speed)
{
    size_t i;
    size_t used = N_AnimCount();
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return;

    // Check if lump already exists, and update info instead
    for (i = 0; i < used; i++)
    {
        if (n_anims[i].lump == lumpnum)
        {
            n_anims[i].lump = lumpnum;
            n_anims[i].widescrn = N_CheckNumForPrefixedName("W_", lump);
            n_anims[i].ani_start = W_CheckNumForName(slump);
            n_anims[i].ani_end = W_CheckNumForName(elump);
            n_anims[i].ani_speed = speed;
            n_anims[i].validcycle = N_CheckAnimateCycle(n_anims[i].ani_start, n_anims[i].ani_end);
            return;
        }
    }

    N_ExtendAnimateLimit();
    n_lastanim->lump = lumpnum;
    n_lastanim->widescrn = N_CheckNumForPrefixedName("W_", lump);
    n_lastanim->ani_start = W_CheckNumForName(slump);
    n_lastanim->ani_end = W_CheckNumForName(elump);
    n_lastanim->ani_speed = speed;
    n_lastanim->validcycle = N_CheckAnimateCycle(n_lastanim->ani_start, n_lastanim->ani_end);
    n_lastanim++;
}

static dboolean N_AddPatchAnimateNum(const char* lump)
{
    char slump[9], elump[9];

    N_CombinePrefixedLump(slump, "S_", lump);
    N_CombinePrefixedLump(elump, "E_", lump);

    if (W_CheckNumForName(slump) == LUMP_NOT_FOUND ||
        W_CheckNumForName(elump) == LUMP_NOT_FOUND)
        return false;

    N_AddPatchAnimateLump(lump, slump, elump, 8);
    return true;
}

static int N_PlayAnimatePatch(int aninum)
{
    int SLump = n_anims[aninum].ani_start;
    int ELump = n_anims[aninum].ani_end;
    int speed = n_anims[aninum].ani_speed;
    int frame = (AnimateTime / speed) % (ELump - SLump + 1);

    return SLump + frame;
}

static int N_GetPatchAnimateIndex(const char* lump)
{
    size_t i;
    size_t used;
    int lumpnum;
    int index;

    lumpnum = W_CheckNumForName(lump);
    index = LUMP_NOT_FOUND;

    if (lumpnum == LUMP_NOT_FOUND)
        return LUMP_NOT_FOUND;

    for (;;) // retry after adding animations
    {
        used = N_AnimCount();
        index = LUMP_NOT_FOUND;

        for (i = 0; i < used; i++)
            if (n_anims[i].lump == lumpnum) { index = (int)i; break; }

        if (index != LUMP_NOT_FOUND) break;
        if (!N_AddPatchAnimateNum(lump)) break;
    }

    return index;
}

int N_GetPatchAnimateNum(const char* lump, dboolean animation)
{
    int index;
    int animate;
    int widescrn;

    if (!(animateLumps || widescreenLumps))
        return W_GetNumForName(lump);

    index = N_GetPatchAnimateIndex(lump);
    if (index == LUMP_NOT_FOUND)
        return W_GetNumForName(lump);

    animate  = animateLumps && n_anims[index].validcycle;
    widescrn = widescreenLumps && n_anims[index].widescrn != LUMP_NOT_FOUND;

    if (animate)
        return animation ? N_PlayAnimatePatch(index) : n_anims[index].ani_start;
    else if (widescrn)
        return n_anims[index].widescrn;

    return W_GetNumForName(lump);
}

void V_DrawBackgroundAnimate(const char* lump)
{
    extern const char* g_menu_flat;
    int lumpNum = R_FlatNumForName(g_menu_flat);
    int SLump = W_CheckNumForName2(lump, ns_flats);

    if ((SLump != LUMP_NOT_FOUND))
    {
        anim_t* anim = anim_flats[SLump - firstflat].anim;
        if (anim)
        {
            int frame = (AnimateTime / anim->speed) % (anim->picnum - anim->basepic + 1);
            lumpNum = anim->basepic + frame;
        }
    }

    V_DrawBackgroundNum(lumpNum);
}

static void N_CombinePrefixedLump(char out[9], const char *prefix, const char *name)
{
    size_t i = 0, j = 0;

    if (!prefix) prefix = "W_";

    while (prefix[i] && i < 8) {
        out[i] = prefix[i];
        i++;
    }

    while (name[j] && i < 8) {
        out[i] = name[j];
        i++; j++;
    }

    out[i] = '\0';
    {
        char *p = out;
        while (*p)
        {
            *p = toupper((unsigned char)*p);
            p++;
        }
    }
}

static int N_CheckNumForPrefixedName(const char *prefix, const char *base)
{
    char name[9];
    N_CombinePrefixedLump(name, prefix, base);
    return W_CheckNumForName(name);
}
