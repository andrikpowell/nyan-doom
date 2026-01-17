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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "scanner.h"
#include "animinfo.h"

extern "C"
{
#include "lprintf.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_spec.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "dsda/utility.h"
}

int animateLumps;
int widescreenLumps;

#define MAXANIMS 32
static animate_t* n_lastanim;
static animate_t* n_anims;
static size_t n_maxanims;

// Widepic status
#define WIDEPIC_AUTO   (LUMP_NOT_FOUND) // auto "W_" fallback
#define WIDEPIC_CLEAR  (-2)             // disable widescreen

// ANIMINFO functions
static animate_t* N_GetOrCreateAnimByLumpnum(int lumpnum);
static void N_AnimClearAnimate(animate_t *a);
static void N_AnimSetRange(animate_t *a, int start, int end, int tics);
static void N_AnimSetSequence(animate_t *a, const anim_frame_t *f, int num);

// Main functions
static void N_CombinePrefixedLump(char out[9], const char *prefix, const char *name);
static int  N_CheckNumForPrefixedName(const char *prefix, const char *base);
static void N_AddPatchAnimateLump(const char* lump, const char* slump, const char* elump, int speed);

//      ==================================================
//                    MAIN ANIMATION LOGIC
//      ==================================================


static size_t N_AnimCount(void)
{
    return (n_anims && n_lastanim) ? (size_t)(n_lastanim - n_anims) : 0;
}

//
//
// Random Frame Duration
//
//

static uint32_t animinfo_rng = 0x12345678u;

static int N_AnimRandom(int lo, int hi)
{
    animinfo_rng = animinfo_rng * 1664525u + 1013904223u;

    if (hi <= lo)
        return lo;

    return lo + (int)(animinfo_rng % (uint32_t)(hi - lo + 1));
}

static int N_GetFrameTics(const anim_frame_t *f)
{
    if (f->mode == ANIM_TICS_RANDOM)
    {
        int lo = f->tics_min;
        int hi = f->tics_max;
        if (hi < lo) hi = lo;
        return N_AnimRandom(lo, hi);
    }

    return f->tics_min;
}

static void N_TickSequence(void)
{
    for (size_t i = 0; i < N_AnimCount(); i++)
    {
        animate_t *a = &n_anims[i];
        if (a->type == ANIM_SEQUENCE)
        {
            if (a->num_frames <= 0 || !a->frames)
                return;
            
            if (a->seq_index < 0 || a->seq_index >= a->num_frames)
                a->seq_index = 0;

            if (a->seq_remaining <= 0)
            {
                // start or just advanced into a frame
                const anim_frame_t *f = &a->frames[a->seq_index];
                a->seq_remaining = N_GetFrameTics(f);
                if (a->seq_remaining <= 0) a->seq_remaining = 1;
            }

            a->seq_remaining--;

            if (a->seq_remaining <= 0)
            {
                a->seq_index++;
                if (a->seq_index >= a->num_frames) a->seq_index = 0;
            }
        }
    }
}

//
//
// Animate Ticker
//
//

void AnimateTicker(void)
{
  AnimateTime++;

  // tick all sequence anims once per tic
  // Avoid random tics from re-rolling
  N_TickSequence();
}

//
//
// Add animations from ANIMINFO lump
//
//

static void N_AnimClearAnimate(animate_t *a)
{
    // clear range
    a->startpic   = LUMP_NOT_FOUND;
    a->endpic     = LUMP_NOT_FOUND;
    a->tics       = 0;
    a->validcycle = false;

    // clear sequence
    if (a->frames)
        Z_Free(a->frames);
    a->frames        = NULL;
    a->num_frames    = 0;
    a->seq_index     = 0;
    a->seq_remaining = 0;

    a->type = ANIM_NONE;
}

//
// Widepic
//

static int N_GetWidepic(const animate_t *a, const char *lump)
{
    if (a->widepic == WIDEPIC_CLEAR)
        return LUMP_NOT_FOUND;

    if (a->widepic >= 0)
        return a->widepic;

    // WIDEPIC_AUTO
    return N_CheckNumForPrefixedName("W_", lump);
}

//
// Animation - Range
//

static int N_CheckAnimateCycle(int SLump, int ELump)
{
    if ((SLump != LUMP_NOT_FOUND) && (ELump != LUMP_NOT_FOUND))
        if (SLump < ELump)
            return true;

    return false;
}

static void N_AnimSetRange(animate_t *a, int start, int end, int tics)
{
    N_AnimClearAnimate(a);

    a->type       = ANIM_RANGE;
    a->startpic   = start;
    a->endpic     = end;
    a->tics       = tics;
    a->validcycle = N_CheckAnimateCycle(start, end) && tics > 0;
}

//
// Animation - Sequence
//

static void N_CheckBadFrame(anim_frame_t *f)
{
    if (f->mode == ANIM_TICS_RANDOM)
    {
        if (f->tics_min <= 0 || f->tics_max < f->tics_min)
            I_Error("ANIMINFO: rand(min,max) tics must be > 0");
    }
    else
    {
        if (f->tics_min <= 0)
            I_Error("ANIMINFO: tics must be > 0");
        f->tics_max = f->tics_min;
    }
}

static void N_AnimSetSequence(animate_t *a, const anim_frame_t *frames, int num)
{
    if (num <= 0)
        I_Error("ANIMINFO: sequence must have at least 1 frame");

    N_AnimClearAnimate(a);

    a->type = ANIM_SEQUENCE;

    a->frames = (anim_frame_t*)Z_Malloc(num * sizeof(*a->frames));
    memcpy(a->frames, frames, num * sizeof(*a->frames));
    a->num_frames = num;

    for (int i = 0; i < num; i++)
        N_CheckBadFrame(&a->frames[i]);
}

//
//
// Setup / Reload
//
//

void N_InitAnimateLumps(void) {
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);

        // Add default "weird" lumps
        N_AddPatchAnimateLump("M_SKULL1", "S_SKULL", "E_SKULL", 8);
        N_AddPatchAnimateLump("M_DOOM", "S_DOOM", "E_DOOM", 8);
    }
}

void N_ReloadAnimateLumps(void)
{
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);
    }
}

//
//
// Indexing Animations
//
//

static void N_AddPatchAnimateLump(const char* lump, const char* slump, const char* elump, int speed)
{
    animate_t *a;
    int start, end;

    int lumpnum = W_CheckNumForName(lump);
    if (lumpnum == LUMP_NOT_FOUND)
        return;

    //a->widepic = N_CheckNumForPrefixedName("W_", lump);

    start = W_CheckNumForName(slump);
    end   = W_CheckNumForName(elump);

    // if the range doesn't exist, don't store
    if (start == LUMP_NOT_FOUND || end == LUMP_NOT_FOUND || speed <= 0)
        return;

    a = N_GetOrCreateAnimByLumpnum(lumpnum);
    N_AnimSetRange(a, start, end, speed);
}

static int N_AddPatchAnimateNum(const char* lump)
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

static int N_GetAnimateIndex(const char* lump)
{
    size_t i;
    size_t used;
    int lumpnum;
    int index;

    lumpnum = W_CheckNumForName(lump);
    index = LUMP_NOT_FOUND;

    if (lumpnum == LUMP_NOT_FOUND)
        return LUMP_NOT_FOUND;

    for (;;) // retry after adding animation
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

static int N_GetAnimateRange(int aninum)
{
    int SLump = n_anims[aninum].startpic;
    int ELump = n_anims[aninum].endpic;
    int speed = n_anims[aninum].tics;
    int frame = (AnimateTime / speed) % (ELump - SLump + 1);

    return SLump + frame;
}

static int N_GetAnimateSequence(const animate_t *a)
{
    if (a->num_frames <= 0 || !a->frames)
        return LUMP_NOT_FOUND;

    if (a->seq_index < 0 || a->seq_index >= a->num_frames)
        return a->frames[0].pic_lumpnum;

    return a->frames[a->seq_index].pic_lumpnum;
}

//
//
// Build animation database
//
//

static void N_ExtendAnimateLimit(void)
{
    if (!n_anims) {
        n_maxanims = MAXANIMS;
        n_anims = (animate_t*)Z_Malloc(n_maxanims * sizeof(*n_anims));
        n_lastanim = n_anims; // 0 used
        return;
    }

    if (n_lastanim >= n_anims + n_maxanims) {
        size_t used = (size_t)(n_lastanim - n_anims);
        size_t newmax = n_maxanims * 2;

        n_anims = (animate_t*)Z_Realloc(n_anims, newmax * sizeof(*n_anims));
        n_maxanims = newmax;
        n_lastanim = n_anims + used;
    }
}

static animate_t* N_GetOrCreateAnimByLumpnum(int lumpnum)
{
    size_t i;
    size_t used = N_AnimCount();
    animate_t *a;

    // Find existing
    for (i = 0; i < used; i++)
        if (n_anims[i].lump == lumpnum)
            return &n_anims[i];

    // Create new
    N_ExtendAnimateLimit();

    a = n_lastanim++;
    memset(a, 0, sizeof(*a));

    a->lump = lumpnum;

    // lump defaults
    a->widepic = WIDEPIC_AUTO;
    a->type = ANIM_NONE;

    // range defaults
    a->startpic   = LUMP_NOT_FOUND;
    a->endpic     = LUMP_NOT_FOUND;
    a->tics       = 0;
    a->validcycle = false;

    // sequence defaults
    a->frames = NULL;
    a->num_frames = 0;

    return a;
}

//
//
// MAIN FUNCTION
//
//

int N_GetPatchAnimateNum(const char* lump, int animation)
{
    int index;
    const animate_t *a;
    int wide_lump;

    if (!(animateLumps || widescreenLumps))
        return W_GetNumForName(lump);

    index = N_GetAnimateIndex(lump);
    if (index == LUMP_NOT_FOUND)
    {
        // no animation, but allow "W_" widescreen
        if (widescreenLumps)
        {
            int wide = N_CheckNumForPrefixedName("W_", lump);
            if (wide != LUMP_NOT_FOUND)
                return wide;
        }
        return W_GetNumForName(lump);
    }

    a = &n_anims[index];

    // animation
    if (animateLumps)
    {
        if (n_anims[index].type == ANIM_RANGE)
            return animation ? N_GetAnimateRange(index) : n_anims[index].startpic;

        if (n_anims[index].type == ANIM_SEQUENCE)
        {
            if (!a->frames || a->num_frames <= 0)
                return W_GetNumForName(lump);

            return animation ? N_GetAnimateSequence(&n_anims[index]) : n_anims[index].frames[0].pic_lumpnum;
        }
    }

    // widescreen
    if (widescreenLumps)
    {
        wide_lump = N_GetWidepic(a, lump);
        if (wide_lump != LUMP_NOT_FOUND)
            return wide_lump;
    }

    return W_GetNumForName(lump);
}

//
//
// Prefixed Lumps
//
//

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
    M_Strupr(out);
}

static int N_CheckNumForPrefixedName(const char *prefix, const char *base)
{
    char name[9];
    N_CombinePrefixedLump(name, prefix, base);
    return W_CheckNumForName(name);
}

//
//
// External Functions
//
//

const int N_CheckAnimate(const char* lump)
{
    size_t i;
    size_t used = N_AnimCount();
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return false;

    for (i = 0; i < used; i++)
    {
        if (n_anims[i].lump == lumpnum) {
            if (n_anims[i].type == ANIM_RANGE && n_anims[i].validcycle)
                return true;
            if (n_anims[i].type == ANIM_SEQUENCE && n_anims[i].num_frames > 0 && n_anims[i].frames)
                return true;
        }
    }

    return false;
}

/*
const int N_CheckWide(const char* lump)
{
    size_t i;
    size_t used = N_AnimCount();
    int lumpnum = W_CheckNumForName(lump);

    if (lumpnum == LUMP_NOT_FOUND)
        return false;

    for (i = 0; i < used; i++)
    {
        if (n_anims[i].lump != lumpnum)
            continue;

        if (n_anims[i].widepic == WIDEPIC_CLEAR)
            return false;

        if (n_anims[i].widepic >= 0)
            return true;

        // AUTO => fallback to W_
        return (N_CheckNumForPrefixedName("W_", lump) != LUMP_NOT_FOUND);
    }

    // nothing => fallback to W_
    return (N_CheckNumForPrefixedName("W_", lump) != LUMP_NOT_FOUND);
}
*/

//
//
// Flat Animate Background (Uses ANIMATED System)
//
//

void V_DrawBackgroundAnimate(const char* lump)
{
    int lumpNum = R_FlatNumForName(g_menu_flat);
    int SLump = W_CheckNumForName2(lump, ns_flats);

    if ((SLump != LUMP_NOT_FOUND))
    {
        int flat_index = P_FlatIndexFromLump(SLump);
        int swirl_flat = P_IsSwirlingFlat(flat_index);

        // do swirl instead
        if (swirl_flat)
        {
            V_DrawBackgroundSwirlNum(flat_index);
            return;
        }

        // Normal Animate Logic
        anim_t* anim = anim_flats[flat_index].anim;
        if (anim)
        {
            int frame = (AnimateTime / anim->speed) % anim->numpics;
            lumpNum = anim->basepic + frame;
        }
    }

    V_DrawBackgroundNum(lumpNum);
}
