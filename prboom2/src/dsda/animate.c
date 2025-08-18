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

const int N_CheckWide(const char* lump)
{
    int i;
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return false;

    for (i = 0; (size_t)i < n_maxanims; i++)
        if (n_anims[i].lump == lumpnum && n_anims[i].widescrn != LUMP_NOT_FOUND)
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
    int i;
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return false;

    for (i = 0; (size_t)i < n_maxanims; i++)
    {
        if (n_anims[i].lump == lumpnum)
        {
            if (N_CheckAnimateCycle(n_anims[i].ani_start, n_anims[i].ani_end))
                return true;
        }
    }

    return false;
}

static int N_SetupWidePatch(const char* lump)
{
    return W_CheckNumForName(PrefixCombine("W_", lump));
}

static int N_SetupAnimatePatch(const char* prefix, const char* lump)
{
    return W_CheckNumForName(PrefixCombine(prefix, lump));
}

static void N_ExtendAnimateLimit(void)
{
    if (n_lastanim >= n_anims + n_maxanims)
    {
        size_t n_newmax = n_maxanims ? n_maxanims*2 : MAXANIMS;
        n_anims = Z_Realloc(n_anims, n_newmax*sizeof(*n_anims));
        n_lastanim = n_anims + n_maxanims;
        n_maxanims = n_newmax;
    }
}

void N_AddPatchAnimateLump(const char* lump, const char* slump, const char* elump, int speed)
{
    int i;
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return;

    // Check if lump already exists, and update info instead
    for (i = 0; (size_t)i < n_maxanims; i++)
    {
        if (n_anims[i].lump == lumpnum)
        {
            n_anims[i].lump = lumpnum;
            n_anims[i].widescrn = W_CheckNumForName(PrefixCombine("W_", lump));
            n_anims[i].ani_start = W_CheckNumForName(slump);
            n_anims[i].ani_end = W_CheckNumForName(elump);
            n_anims[i].ani_speed = speed;
            n_anims[i].validcycle = N_CheckAnimateCycle(n_anims[i].ani_start, n_anims[i].ani_end);
            return;
        }
    }

    N_ExtendAnimateLimit();
    n_lastanim->lump = lumpnum;
    n_lastanim->widescrn = W_CheckNumForName(PrefixCombine("W_", lump));
    n_lastanim->ani_start = W_CheckNumForName(slump);
    n_lastanim->ani_end = W_CheckNumForName(elump);
    n_lastanim->ani_speed = speed;
    n_lastanim->validcycle = N_CheckAnimateCycle(n_lastanim->ani_start, n_lastanim->ani_end);
    n_lastanim++;
}

static void N_AddPatchAnimateNum(const char* lump)
{
    char slump[9] = "";
    char elump[9] = "";
    const char* slump_str;
    const char* elump_str;

    slump_str = PrefixCombine("S_", lump);
    strcpy(slump, slump_str);

    elump_str = PrefixCombine("E_", lump);
    strcpy(elump, elump_str);

    N_AddPatchAnimateLump(lump, slump, elump, 8);
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
    int lumpnum = W_GetNumForName(lump);
    int index = LUMP_NOT_FOUND;

    do {
        for (int i = 0; (size_t)i < n_maxanims; i++)
            if (n_anims[i].lump == lumpnum)
                index = i;

        if (index == LUMP_NOT_FOUND)
            N_AddPatchAnimateNum(lump);

    } while (index == LUMP_NOT_FOUND);

    return index;
}

int N_GetPatchAnimateNum(const char* lump, dboolean animation)
{
    if (animateLumps || widescreenLumps)
    {
        int index = N_GetPatchAnimateIndex(lump);
        int animate  = animateLumps && n_anims[index].validcycle;
        int widescrn = widescreenLumps && n_anims[index].widescrn != LUMP_NOT_FOUND;

        if (animate)
            return animation ? N_PlayAnimatePatch(index) : n_anims[index].ani_start;
        else if (widescrn)
            return n_anims[index].widescrn;
    }

    return W_GetNumForName(lump);
}

void V_DrawNamePatchAnimate(const int x, const int y, const char* lump, const int color, const int flags)
{
    V_DrawNumPatch(x, y, N_GetPatchAnimateNum(lump, true), color, flags);
}

void V_DrawNamePatchAnimateFS(const int x, const int y, const char* lump, const int color, const int flags)
{
    V_DrawNumPatchFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags);
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

const char* PrefixCombine(const char *lump_prefix, const char *lump_main)
{
    static char result[9];

    if (lump_prefix == NULL)
        lump_prefix = "W_";

    memcpy(result, lump_prefix, 2);
    memcpy(&result[2], lump_main, 6);
    result[8] = 0;
    return result;
}
