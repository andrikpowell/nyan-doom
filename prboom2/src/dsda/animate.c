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

void N_InitAnimateLumps(void) {
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);

        N_AddPatchAnimateLump("M_SKULL1", "S_SKULL", "E_SKULL", 8);
        N_AddPatchAnimateLump("M_DOOM", "S_DOOM", "E_DOOM", 8);
        N_AddPatchAnimateLump("STBAR", "S_STBAR", "E_STBAR", 8);
        // Disabled for now
        //nyan_LoadAnimateLump();
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

    if (W_CheckNumForName(lump) == LUMP_NOT_FOUND)
        return false;

    for (i = 0; (size_t)i < n_maxanims; i++)
        if (n_anims[i].lump == W_GetNumForName(lump) && n_anims[i].wide != LUMP_NOT_FOUND)
            return true;

    return false;
}

const int N_CheckAnimate(const char* lump)
{
    int i;
    int SLump, ELump;

    if (W_CheckNumForName(lump) == LUMP_NOT_FOUND)
        return false;

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
