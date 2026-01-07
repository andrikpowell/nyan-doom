//
// Copyright(C) 2024-2026 by Andrik Powell
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
//	NYAN ANIMINFO
//

#ifndef __NYAN_ANIMINFO__
#define __NYAN_ANIMINFO__

#ifdef __cplusplus
extern "C"
{
#endif

extern int AnimateTime;
extern int animateLumps;
extern int widescreenLumps;

// Animation data sets

typedef enum {
    ANIM_NONE,
    ANIM_RANGE,
    ANIM_SEQUENCE,
} anim_type_t;

typedef enum
{
    ANIM_TICS_FIXED,
    ANIM_TICS_RANDOM,
} anim_frame_mode_t;

typedef struct {
    int pic_lumpnum;
    anim_frame_mode_t mode;
    int tics_min;
    int tics_max;
} anim_frame_t;

typedef struct
{
    int lump;         // original lump
    int widepic;      // widescreen lump
    anim_type_t type; // animation

    // animation - range
    int startpic;
    int endpic;
    int tics;
    int validcycle;

    // animation - sequence
    anim_frame_t *frames;
    int num_frames;
    int seq_index;
    int seq_remaining;
} animate_t;

// Setup / Control Animations
void AnimateTicker(void);
void N_InitAnimateLumps(void);
void N_ReloadAnimateLumps(void);

// Main Draw Functions
#define V_DrawNamePatchAnimate(x,y,lump,color,flags) V_DrawNumPatch(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawNamePatchAnimateFS(x,y,lump,color,flags) V_DrawNumPatchFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawNamePatchAnimatePrecise(x,y,lump,color,flags) V_DrawNumPatchPrecise(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawNamePatchAnimatePreciseFS(x,y,lump,color,flags) V_DrawNumPatchPreciseFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags)

#define V_DrawMenuNamePatchAnimate(x,y,lump,color,flags) V_DrawMenuNumPatch(x, y, N_GetPatchAnimateNum(lump, true), color, flags)
#define V_DrawMenuNamePatchAnimateFS(x,y,lump,color,flags) V_DrawMenuNumPatchFS(x, y, N_GetPatchAnimateNum(lump, true), color, flags)

// Dynamic Credits
void V_DrawBackgroundAnimate(const char* lump);

// m_menu.c for M_SKULL1/2
const int N_CheckAnimate(const char* lump);

// Bunny screens and stbar color background
int N_GetPatchAnimateNum(const char* lump, int animation);

#ifdef __cplusplus
}
#endif

#endif
