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

#ifndef __DSDA_ANIMATE__
#define __DSDA_ANIMATE__

extern int AnimateTime;
extern int animateLumps;
extern int widescreenLumps;

typedef struct
{
    int lump;
    int widescrn;
    int ani_start;
    int ani_end;
    int ani_speed;
    int validcycle;
} animate_t;

void AnimateTicker(void);

void N_InitAnimateLumps(void);
void N_ReloadAnimateLumps(void);

int N_GetPatchAnimateNum(const char* lump, dboolean animation);
void N_AddPatchAnimateLump(const char* lump, const char* slump, const char* elump, int speed);

void V_DrawBackgroundAnimate(const char* lump);

#define V_DrawNamePatchAnimate(x,y,lump,color,flags) V_DrawNumPatch(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawNamePatchAnimateFS(x,y,lump,color,flags) V_DrawNumPatchFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawNamePatchAnimatePrecise(x,y,lump,color,flags) V_DrawNumPatchPrecise(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawNamePatchAnimatePreciseFS(x,y,lump,color,flags) V_DrawNumPatchPreciseFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags)

#define V_DrawMenuNamePatchAnimate(x,y,lump,color,flags) V_DrawMenuNumPatch(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawMenuNamePatchAnimateFS(x,y,lump,color,flags) V_DrawMenuNumPatchFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags)

const int N_CheckAnimate(const char* lump); // called in m_menu.c for M_SKULL1/2

#endif
