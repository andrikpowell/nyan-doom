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

#include "animate.h"

int animateLumps;
int widescreenLumps;

int Check_Stbar_Wide;
int Check_Skull_Animate;
int Check_Stbar_Animate;

void AnimateTicker(void)
{
  AnimateTime++;
}

void dsda_InitNyanLumps(void) {
    if (!raven) {
        Check_Skull_Animate = D_CheckAnimate(mskull1);
        Check_Stbar_Animate = D_CheckAnimate(stbar);
        Check_Stbar_Wide = D_CheckWide(stbar);
    }
}

void dsda_ReloadNyanLumps(void)
{
    if (!raven) {
        animateLumps = (dsda_IntConfig(nyan_config_enable_animate_lumps) ? 1 : 0);
        widescreenLumps = (dsda_IntConfig(nyan_config_enable_widescreen_lumps) ? 1 : 0);
        ST_SetScaledWidth();
    }
}


const int D_CheckWide(const char* lump)
{   
    if (!widescreenLumps)
        return false;

    if (W_CheckNumForName(PrefixCombine("W_", lump)) != LUMP_NOT_FOUND)
        return true;

    return false;
}

int D_SetupWidePatch(const char* lump)
{
    int WLump = W_CheckNumForName(PrefixCombine("W_", lump));

    if (WLump != LUMP_NOT_FOUND)
        return WLump;
    
    return false;
}

const int D_CheckAnimate(const char* lump)
{
    int SLump, ELump;

    if (!animateLumps)
        return false;

    if (!strcmp(lump, mskull1)) { lump = "SKULL"; }
    if (!strcmp(lump, mdoom))   { lump = "DOOM";  }

    SLump = W_CheckNumForName(PrefixCombine("S_", lump));
    ELump = W_CheckNumForName(PrefixCombine("E_", lump));

    if ((SLump != LUMP_NOT_FOUND) && (ELump != LUMP_NOT_FOUND))
        if (SLump <= ELump)
            return true;

    return false;
}

int D_SetupAnimatePatch(const char* lump)
{
    int SLump = W_CheckNumForName(PrefixCombine("S_", lump));
    int ELump = W_CheckNumForName(PrefixCombine("E_", lump));

    if ((SLump != LUMP_NOT_FOUND) && (ELump != LUMP_NOT_FOUND))
    {
        if (SLump <= ELump)
        {
            int speed = 8;
            int frame = (AnimateTime / speed) % (ELump - SLump + 1);
            return SLump + frame;
        }
    }

    return false;
}

void V_DrawNameNyanPatch(const int x, const int y, const int scrn, const char* lump, const int color, const int flags)
{
    int lumpNum = W_GetNumForName(lump);
    int AniCheck = 0;
    int WideCheck = 0;
    int SkipWide = 0;

    if (!strcmp(lump, mskull1))  { SkipWide = 1; lump = "SKULL"; }
    if (!strcmp(lump, mdoom))    { SkipWide = 1; lump = "DOOM"; }
    if (!strcmp(lump, starms))   { SkipWide = 1; }

    if (animateLumps)
    {
        AniCheck = D_SetupAnimatePatch(lump);

        if (AniCheck)
            lumpNum = AniCheck;
    }
    if (widescreenLumps && !AniCheck && !SkipWide)
    {
        WideCheck = D_SetupWidePatch(lump);

        if (WideCheck)
            lumpNum = WideCheck;
    }

    V_DrawNumPatch(x, y, scrn, lumpNum, color, flags);
}

void V_DrawNyanBackground(const char* lump, const int scrn)
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
