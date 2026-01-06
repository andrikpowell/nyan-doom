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

// Lowest supported version
#define ANIMINFO_MIN_MAJ 1
#define ANIMINFO_MIN_MIN 0
#define ANIMINFO_MIN_PAT 0

// Widepic status
#define WIDEPIC_AUTO   (LUMP_NOT_FOUND) // auto "W_" fallback
#define WIDEPIC_CLEAR  (-2)             // disable widescreen

// ANIMINFO Parser
typedef void (*animinfo_errorfunc)(const char *fmt, ...);
static int ParseLumpName(Scanner& scanner, char* buffer);
static void Parse_ANIMINFO_Metadata(Scanner &scanner);
static void Parse_ANIMINFO_Animate(Scanner &scanner, animate_t *a);
static void Parse_ANIMINFO_AnimateObject(Scanner &scanner, animate_t *a);
static void Parse_ANIMINFO_Widepic(Scanner &scanner, animate_t *a);

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
//                     ANIMINFO PARSER
//      ==================================================

static void Parse_ANIMINFO_Lump(Scanner &scanner)
{
    char lumpname[9] = {0};
    int lumpnum;
    animate_t *a = NULL;
    ParseLumpName(scanner, lumpname);

    lumpnum = W_CheckNumForName(lumpname);
    if (lumpnum != LUMP_NOT_FOUND)
        a = N_GetOrCreateAnimByLumpnum(lumpnum);

    scanner.MustGetToken('{');

    while (!scanner.CheckToken('}'))
    {
        scanner.MustGetToken(TK_Identifier);

        if (scanner.StringMatch("animate"))
        {
            Parse_ANIMINFO_Animate(scanner, a);
        }
        else if (scanner.StringMatch("widepic"))
        {
            Parse_ANIMINFO_Widepic(scanner, a);
        }
        else
        {
            scanner.ErrorF("Unknown lump property '%s'", scanner.string);
            scanner.SkipLine();
        }
    }
}

static int Parse_ANIMINFO(const unsigned char *buffer, size_t length, animinfo_errorfunc err)
{
    Scanner scanner((const char*)buffer, (int)length);
    scanner.SetErrorCallback(err);

    while (scanner.TokensLeft())
    {
        scanner.MustGetToken(TK_Identifier);

        if (scanner.StringMatch("metadata"))
        {
            Parse_ANIMINFO_Metadata(scanner);
        }
        else if (scanner.StringMatch("lump"))
        {
            Parse_ANIMINFO_Lump(scanner);
        }
        else
        {
            scanner.ErrorF("Unknown top-level keyword '%s'", scanner.string);
            scanner.SkipLine();
        }
    }
    return 1;
}

static int ParseLumpName(Scanner &scanner, char *buffer)
{
	scanner.MustGetToken(TK_StringConst);
	if (strlen(scanner.string) > 8)
	{
		scanner.ErrorF("String too long. Maximum size is 8 characters.");
		return 0;
	}
	strncpy(buffer, scanner.string, 8);
	buffer[8] = 0;
	M_Strupr(buffer);
	return 1;
}

//
//
// ANIMINFO Metadata
//
//

static int Parse_ANIMINFO_Version(const char *s, int *outMaj, int *outMin, int *outPat)
{
    int maj = 0, min = 0, pat = 0;
    char tail = 0; // anything after

    if (sscanf(s, "%d.%d.%d%c", &maj, &min, &pat, &tail) != 3)
        return false;

    if (maj < 0 || min < 0 || pat < 0)
        return false;

    *outMaj = maj; *outMin = min; *outPat = pat;
    return true;
}

static int Compare_ANIMINFO_Version(int curMaj, int curMin, int curPat, int lowMaj, int lowMin, int lowPat)
{
    if (curMaj != lowMaj) return (curMaj < lowMaj) ? -1 : 1;
    if (curMin != lowMin) return (curMin < lowMin) ? -1 : 1;
    if (curPat != lowPat) return (curPat < lowPat) ? -1 : 1;
    return 0;
}

static void Parse_ANIMINFO_Metadata(Scanner &scanner)
{
    int haveVersion;
    int maj, min, pat;

    scanner.MustGetToken(TK_StringConst);

    // case-insensitive
    if (stricmp(scanner.string, "ANIMINFO") != 0)
        scanner.ErrorF("ANIMINFO: metadata must be \"ANIMINFO\" (got \"%s\")", scanner.string);

    haveVersion = false;
    maj = 0, min = 0, pat = 0;

    scanner.MustGetToken('{');
    while (!scanner.CheckToken('}'))
    {
        char key[64];
        char version[64];

        scanner.MustGetToken(TK_Identifier);

        strncpy(key, scanner.string, sizeof(key) - 1);
        key[sizeof(key) - 1] = 0;

        scanner.MustGetToken('=');
        scanner.MustGetToken(TK_StringConst);
    
        strncpy(version, scanner.string, sizeof(version) - 1);
        version[sizeof(version) - 1] = 0;

        scanner.MustGetToken(';');

        if (!stricmp(key, "version"))
        {
            haveVersion = true;

            if (!Parse_ANIMINFO_Version(version, &maj, &min, &pat))
                scanner.ErrorF("ANIMINFO: invalid version \"%s\" (expected \"MAJOR.MINOR.PATCH\")", version);

            if (Compare_ANIMINFO_Version(maj, min, pat, ANIMINFO_MIN_MAJ, ANIMINFO_MIN_MIN, ANIMINFO_MIN_PAT) < 0)
            {
                scanner.ErrorF("ANIMINFO: version %d.%d.%d is too old (minimum supported is %d.%d.%d)",
                               maj, min, pat, ANIMINFO_MIN_MAJ, ANIMINFO_MIN_MIN, ANIMINFO_MIN_PAT);
            }
        }

        // new key logic would go here
    }

    if (!haveVersion)
        scanner.ErrorF("ANIMINFO: missing required metadata field: version");
}

//
//
// Parse Widescreen lumps
//
//

static void Parse_ANIMINFO_Widepic(Scanner &scanner, animate_t *a)
{
    scanner.MustGetToken('=');

    // allow: clear;
    if (scanner.CheckToken(TK_Identifier) && scanner.StringMatch("clear"))
    {
        if (a) a->widepic = WIDEPIC_CLEAR;
        scanner.MustGetToken(';');
        return;
    }

    char wname[9] = {0};
    ParseLumpName(scanner, wname);

    if (a)
        a->widepic = W_CheckNumForName(wname);

    scanner.MustGetToken(';');
}

//
//
// Parse Animate lumps
//
//

static void Parse_ANIMINFO_Animate(Scanner &scanner, animate_t *a)
{
    scanner.MustGetToken('=');

    // animate = clear;
    if (scanner.CheckToken(TK_Identifier) && scanner.StringMatch("clear"))
    {
        if (a) N_AnimClearAnimate(a);
        scanner.MustGetToken(';');
        return;
    }

    // animate = { ... }
    scanner.MustGetToken('{');
    Parse_ANIMINFO_AnimateObject(scanner, a);

    // optional
    scanner.CheckToken(';');
}

// Get Tics from Animate block
static void GetAnimateTics(Scanner &scanner, anim_frame_t *f)
{
    int min, max;

    // tics =
    if (scanner.CheckInteger())
    {
        f->mode = ANIM_TICS_FIXED;
        f->tics_min = scanner.number;
        f->tics_max = scanner.number;
        return;
    }

    scanner.MustGetToken(TK_Identifier);
    if (!scanner.StringMatch("rand"))
    {
        scanner.ErrorF("Expected integer or rand(min,max)");
        return;
    }

    scanner.MustGetToken('(');
    scanner.MustGetInteger();
    min = scanner.number;
    scanner.MustGetToken(',');
    scanner.MustGetInteger();
    max = scanner.number;
    scanner.MustGetToken(')');

    f->mode = ANIM_TICS_RANDOM;
    f->tics_min = min;
    f->tics_max = max;
}

static void Parse_ANIMINFO_AnimateObject(Scanner &scanner, animate_t *a)
{
    char type[16] = {0};

    // range fields
    int start_lump = LUMP_NOT_FOUND;
    int end_lump   = LUMP_NOT_FOUND;
    anim_frame_t range_tics;
    int have_range_tics = false;

    // sequence frames
    anim_frame_t *frames = NULL;
    int num_frames = 0, cap_frames = 0;

    memset(&range_tics, 0, sizeof(range_tics));

    while (!scanner.CheckToken('}'))
    {
        scanner.MustGetToken(TK_Identifier);

        if (scanner.StringMatch("type"))
        {
            scanner.MustGetToken('=');
            if (scanner.CheckToken(TK_StringConst)) { /* ok */ }
            else scanner.MustGetToken(TK_Identifier);

            strncpy(type, scanner.string, sizeof(type)-1);
            type[sizeof(type)-1] = 0;
            scanner.MustGetToken(';');
            continue;
        }

        // if missing "type", then error
        if (!type[0])
        {
            scanner.ErrorF("ANIMINFO: animate block missing 'type' before other fields");
            return;
        }

        // RANGE
        if (!stricmp(type, "range"))
        {
            if (scanner.StringMatch("startpic"))
            {
                char sname[9] = {0};
                scanner.MustGetToken('=');
                ParseLumpName(scanner, sname);
                start_lump = W_CheckNumForName(sname);
                scanner.MustGetToken(';');
            }
            else if (scanner.StringMatch("endpic"))
            {
                char ename[9] = {0};
                scanner.MustGetToken('=');
                ParseLumpName(scanner, ename);
                end_lump = W_CheckNumForName(ename);
                scanner.MustGetToken(';');
            }
            else if (scanner.StringMatch("tics"))
            {
                scanner.MustGetToken('=');
                GetAnimateTics(scanner, &range_tics);
                have_range_tics = true;
                scanner.MustGetToken(';');
            }
            else
            {
                scanner.ErrorF("Unknown range field '%s'", scanner.string);
                scanner.SkipLine();
            }
        }
        // SEQUENCE 
        else if (!stricmp(type, "sequence"))
        {
            // pic="S_TITLEP"; tics=5;
            if (!scanner.StringMatch("pic"))
            {
                scanner.ErrorF("sequence expects 'pic' entries");
                scanner.SkipLine();
                continue;
            }

            anim_frame_t f;
            char pname[9] = {0};
            memset(&f, 0, sizeof(f));

            scanner.MustGetToken('=');
            ParseLumpName(scanner, pname);
            f.pic_lumpnum = W_CheckNumForName(pname);
            if (f.pic_lumpnum == LUMP_NOT_FOUND)
                scanner.ErrorF("Unknown pic lump '%s'", pname);
            scanner.MustGetToken(';');

            scanner.MustGetIdentifier("tics");
            scanner.MustGetToken('=');
            GetAnimateTics(scanner, &f);
            scanner.MustGetToken(';');

            if (num_frames >= cap_frames)
            {
                cap_frames = cap_frames ? cap_frames * 2 : 8;
                frames = (anim_frame_t*)Z_Realloc(frames, cap_frames * sizeof(*frames));
            }
            frames[num_frames++] = f;
        }
        else
        {
            scanner.ErrorF("Unknown animate type '%s'", type);
            scanner.SkipLine();
        }
    }

    if (a) // Animation exists
    {
        // RANGE ANIMATION
        if (!stricmp(type, "range"))
        {
            if (start_lump == LUMP_NOT_FOUND || end_lump == LUMP_NOT_FOUND || !have_range_tics)
            {
                scanner.ErrorF("range requires startpic, endpic, and tics");
                if (frames) Z_Free(frames);
                return;
            }

            if (range_tics.mode == ANIM_TICS_RANDOM)
            {
                int num;
                anim_frame_t *tmp;

                if (end_lump < start_lump)
                    scanner.ErrorF("range endpic must be after startpic");

                num = end_lump - start_lump + 1;
                tmp = (anim_frame_t*)Z_Malloc(num * sizeof(*tmp));

                for (int i = 0; i < num; i++)
                {
                    tmp[i].pic_lumpnum = start_lump + i;
                    tmp[i].mode = ANIM_TICS_RANDOM;
                    tmp[i].tics_min = range_tics.tics_min;
                    tmp[i].tics_max = range_tics.tics_max;
                }

                N_AnimSetSequence(a, tmp, num);
                Z_Free(tmp);
            }
            else
            {
                N_AnimSetRange(a, start_lump, end_lump, range_tics.tics_min);
            }
        }
        // RANGE SEQUENCE
        else if (!stricmp(type, "sequence"))
        {
            if (num_frames <= 0)
                scanner.ErrorF("sequence must have at least one pic/tics pair");
            else
                N_AnimSetSequence(a, frames, num_frames);
        }
        // UNKNOWN
        else
        {
            scanner.ErrorF("animate block missing/invalid type");
        }
    }

    // Clear frames
    if (frames) Z_Free(frames);
}


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

static void N_LoadAnimateLump(void) {
  int p;

  if (raven)
    return;

  p = -1;
  while ((p = W_ListNumFromName("ANIMINFO", p)) >= 0) {
    const unsigned char * lump = (const unsigned char *) W_LumpByNum(p);
    Parse_ANIMINFO(lump, W_LumpLength(p), I_Error);
  }
}

void N_InitAnimateLumps(void) {
    if (!raven) {
        animateLumps = dsda_IntConfig(nyan_config_enable_animate_lumps);
        widescreenLumps = dsda_IntConfig(nyan_config_enable_widescreen_lumps);

        // Add default "weird" lumps
        N_AddPatchAnimateLump("M_SKULL1", "S_SKULL", "E_SKULL", 8);
        N_AddPatchAnimateLump("M_DOOM", "S_DOOM", "E_DOOM", 8);

        // Disable loading lumps for now
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
        anim_t* anim = anim_flats[SLump - firstflat].anim;
        if (anim)
        {
            int frame = (AnimateTime / anim->speed) % (anim->picnum - anim->basepic + 1);
            lumpNum = anim->basepic + frame;
        }
    }

    V_DrawBackgroundNum(lumpNum);
}
