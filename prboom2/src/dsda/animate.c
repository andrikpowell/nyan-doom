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

typedef struct
{
    int lump;
    int wide;
    int ani_start;
    int ani_end;
    int ani_speed;
} animate_t;

int animateLumps;
int widescreenLumps;

int Check_Stbar_Wide;
int Check_Skull_Animate;
int Check_Stbar_Animate;

#define MAXANIMS 32
static animate_t* n_lastanim;
static animate_t* n_anims;
static size_t n_maxanims;

void AnimateTicker(void)
{
  AnimateTime++;
}

static int nyan_ParseAnimateLump(char** lines, int line_i) {
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

    count = sscanf(args, "%8s %8s %8s %d", lump, start, end, &speed);
    if (count != 4)
        I_Error("Invalid NYANANIM arguments \"%s\"", line);

    if (speed <= 1)
        speed = 1;

    N_AddPatchAnimateLump(lump, start, end, speed);
  }

  // roll back the line that wasn't part of this config
  return line_i - 1;
}

static void nyan_LoadAnimateLump(void) {
  DO_ONCE
    char* animate_lump = NULL;
    char** lines;
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
            if (target_format) {
                line_i = nyan_ParseAnimateLump(lines, line_i);
                break;
            }
        }

        Z_Free(lines);
      }

      Z_Free(animate_lump);
    }
  END_ONCE
}

void N_InitAnimateLumps(void) {
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);

        nyan_LoadAnimateLump();
        Check_Skull_Animate = N_CheckAnimate(mskull1);
        Check_Stbar_Animate = N_CheckAnimate(stbar);
        Check_Stbar_Wide = N_CheckWide(stbar);
    }
}

void N_ReloadAnimateLumps(void)
{
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);
        ST_SetScaledWidth();
        ST_SetResolution();
    }
}

const int N_CheckWide(const char* lump)
{
    int i;
    for (i = 0; (size_t)i < n_maxanims; i++)
        if (n_anims[i].lump == W_GetNumForName(lump) && n_anims[i].wide != LUMP_NOT_FOUND)
            return true;

    return false;
}

const int N_CheckAnimate(const char* lump)
{
    int i;
    int SLump, ELump;

    for (i = 0; (size_t)i < n_maxanims; i++)
    {
        if (n_anims[i].lump == W_GetNumForName(lump) && n_anims[i].ani_start != LUMP_NOT_FOUND)
        {
            SLump = n_anims[i].ani_start;
            ELump = n_anims[i].ani_end;

            if ((SLump != LUMP_NOT_FOUND) && (ELump != LUMP_NOT_FOUND))
                if (SLump <= ELump)
                    return true;
        }
    }

    return false;
}

static int N_SetupWidePatch(const char* lump)
{
    return W_CheckNumForName(PrefixCombine("W_", lump));
}

int N_SetupAnimatePatch(const char* prefix, const char* lump)
{
    return W_CheckNumForName(PrefixCombine(prefix, lump));
}

void N_ExtendAnimateLimit(void)
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

    if (W_CheckNumForName(lump) == LUMP_NOT_FOUND)
        return;

    // Check if lump already exists, and update info instead
    for (i = 0; (size_t)i < n_maxanims; i++)
    {
        if (n_anims[i].lump == W_GetNumForName(lump))
        {
            n_anims[i].lump = W_GetNumForName(lump);
            n_anims[i].wide = N_SetupWidePatch(lump);
            n_anims[i].ani_start = W_CheckNumForName(slump);
            n_anims[i].ani_end = W_CheckNumForName(elump);
            n_anims[i].ani_speed = speed;
            return;
        }
    }

    N_ExtendAnimateLimit();
    n_lastanim->lump = W_GetNumForName(lump);
    n_lastanim->wide = N_SetupWidePatch(lump);
    n_lastanim->ani_start = W_CheckNumForName(slump);
    n_lastanim->ani_end = W_CheckNumForName(elump);
    n_lastanim->ani_speed = speed;
    n_lastanim++;
}

void N_AddPatchAnimateNum(const char* lump)
{
    int i;

    if (W_CheckNumForName(lump) == LUMP_NOT_FOUND)
        return;

    // Check if lump already exists, and update info instead
    for (i = 0; (size_t)i < n_maxanims; i++)
    {
        if (n_anims[i].lump == W_GetNumForName(lump))
        {
            n_anims[i].lump = W_GetNumForName(lump);
            n_anims[i].wide = N_SetupWidePatch(lump);
            n_anims[i].ani_start = N_SetupAnimatePatch("S_",lump);
            n_anims[i].ani_end = N_SetupAnimatePatch("E_",lump);
            n_anims[i].ani_speed = 8;
            return;
        }
    }

    N_ExtendAnimateLimit();
    n_lastanim->lump = W_GetNumForName(lump);
    n_lastanim->wide = N_SetupWidePatch(lump);
    n_lastanim->ani_start = N_SetupAnimatePatch("S_",lump);
    n_lastanim->ani_end = N_SetupAnimatePatch("E_",lump);
    n_lastanim->ani_speed = 8;
    n_lastanim++;
}

static int N_PlayAnimatePatch(int aninum)
{
    int SLump = n_anims[aninum].ani_start;
    int ELump = n_anims[aninum].ani_end;
    int speed = n_anims[aninum].ani_speed;

    if (SLump <= ELump)
    {
        int frame = (AnimateTime / speed) % (ELump - SLump + 1);
        return SLump + frame;
    }

    return false;
}

int N_GetPatchAnimateNum(const char* lump)
{
    int lumpNum = W_GetNumForName(lump);
    int LumpCheck = 0;
    int AniCheck = 0;
    int WideCheck = 0;
    int i;

    for (i = 0; (size_t)i < n_maxanims; i++)
        if (n_anims[i].lump == W_GetNumForName(lump))
            LumpCheck = true;

    if (!LumpCheck)
        N_AddPatchAnimateNum(lump);

    if (animateLumps)
    {
        for (i = 0; (size_t)i < n_maxanims; i++)
            if (n_anims[i].lump == W_GetNumForName(lump) && n_anims[i].ani_start != LUMP_NOT_FOUND)
                AniCheck = N_PlayAnimatePatch(i);

        if (AniCheck)
            lumpNum = AniCheck;
    }
    if (widescreenLumps && !AniCheck)
    {
        for (i = 0; (size_t)i < n_maxanims; i++)
            if (n_anims[i].lump == W_GetNumForName(lump) && n_anims[i].wide != LUMP_NOT_FOUND)
                WideCheck = n_anims[i].wide;

        if (WideCheck)
            lumpNum = WideCheck;
    }

    return lumpNum;
}

void V_DrawNamePatchAnimate(const int x, const int y, const int scrn, const char* lump, const int color, const int flags)
{
    V_DrawNumPatch(x, y, scrn, N_GetPatchAnimateNum(lump), color, flags);
}

void V_DrawNamePatchAnimateFS(const int x, const int y, const int scrn, const char* lump, const int color, const int flags)
{
    V_DrawNumPatchFS(x, y, scrn, N_GetPatchAnimateNum(lump), color, flags);
}

void V_DrawBackgroundAnimate(const char* lump, const int scrn)
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

    V_DrawBackgroundNum(lumpNum, scrn);
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
