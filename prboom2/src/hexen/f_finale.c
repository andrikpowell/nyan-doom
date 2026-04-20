//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

#include "doomstat.h"
#include "w_wad.h"
#include "v_video.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"

#include "dsda/palette.h"

#include "hexen/f_finale.h"

#define	TEXTSPEED	3
#define	TEXTWAIT	250

static void TextWrite(void);
static void DrawPic(void);
static void InitializeFade(dboolean fadeIn);
static void DeInitializeFade(void);
static void FadePic(void);
static char *GetFinaleText(int sequence);

static int FinaleStage;
static int FinaleCount;
static int FinaleEndCount;
static const char* FinaleLumpName;
static int FontABaseLump;
static char *FinaleText;

void Hexen_F_StartFinale(void)
{
    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    automap_full = false;

    FinaleStage = 0;
    FinaleCount = 0;
    FinaleText = GetFinaleText(0);
    FinaleEndCount = 70;
    FinaleLumpName = "FINALE1";
    FontABaseLump = W_GetNumForName("FONTA_S") + 1;
    InitializeFade(1);

    S_StartSongName("hall", false);     // don't loop the song
}

dboolean Hexen_F_Responder(event_t * event)
{
    return false;
}

void Hexen_F_Ticker(void)
{
    FinaleCount++;
    if (FinaleStage < 5 && FinaleCount >= FinaleEndCount)
    {
        FinaleCount = 0;
        FinaleStage++;
        switch (FinaleStage)
        {
            case 1:            // Text 1
                FinaleEndCount = (int)strlen(FinaleText) * TEXTSPEED + TEXTWAIT;
                break;
            case 2:            // Pic 2, Text 2
                FinaleText = GetFinaleText(1);
                FinaleEndCount = (int)strlen(FinaleText) * TEXTSPEED + TEXTWAIT;
                FinaleLumpName = "FINALE2";
                S_StartSongName("orb", false);
                break;
            case 3:            // Pic 2 -- Fade out
                FinaleEndCount = 70;
                DeInitializeFade();
                InitializeFade(0);
                break;
            case 4:            // Pic 3 -- Fade in
                FinaleLumpName = "FINALE3";
                FinaleEndCount = 71;
                DeInitializeFade();
                InitializeFade(1);
                S_StartSongName("chess", true);
                break;
            case 5:            // Pic 3 , Text 3
                FinaleText = GetFinaleText(2);
                DeInitializeFade();
                break;
            default:
                break;
        }
        return;
    }
    if (FinaleStage == 0 || FinaleStage == 3 || FinaleStage == 4)
    {
        FadePic();
    }
}

static void DrawChessOverlay(void)
{
    // Coop / Random Class shows all 3 classes, else show current class
    if (netgame || (PlayerClass[consoleplayer] == PCLASS_RANDOM))
    {
        V_DrawNamePatch(20, 0, "chessall", CR_DEFAULT, VPT_STRETCH);
    }
    else if (PlayerClass[consoleplayer] > 1)
    {
        V_DrawNumPatch(60, 0,
                        W_GetNumForName("chessc") + PlayerClass[consoleplayer] - 2,
                        CR_DEFAULT, VPT_STRETCH);
    }
}

static void TextWrite(void)
{
    int count;
    char *ch;
    int c;
    int cx, cy;
    int lump;
    int width;

    V_DrawRawScreen(FinaleLumpName);
    if (FinaleStage == 5)
    {                           // Chess pic, draw the correct character graphic
        DrawChessOverlay();
    }
    // Draw the actual text
    if (FinaleStage == 5)
    {
        cy = 135;
    }
    else
    {
        cy = 5;
    }
    cx = 20;
    ch = FinaleText;
    count = (FinaleCount - 10) / TEXTSPEED;
    if (count < 0)
    {
        count = 0;
    }
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
        {
            break;
        }
        if (c == '\n')
        {
            cx = 20;
            cy += 9;
            continue;
        }
        if (c < 32)
        {
            continue;
        }
        c = toupper(c);
        if (c == 32)
        {
            cx += 5;
            continue;
        }
        lump = FontABaseLump + c - 33;
        width = R_NumPatchWidth(lump);
        if (cx + width > SCREENWIDTH)
        {
            break;
        }
        V_DrawMenuNumPatch(cx, cy, lump, CR_DEFAULT, VPT_STRETCH);
        cx += width;
    }
}

//
// FINALE PALETTE FADE
//
// This palette fade has the downside of fading the entire screen.
// I tested doing a palette / colormap shading on top, but they
// both didn't look as nice as the vanilla fade.
//
// The good news is that I reset the palette when starting
// a new game, which fixes a vanilla bug that leaves the screen
// with a dark shaded palette.
//
// Note: for OpenGL, we use a simple translucent fill block instead
//

#define HEXEN_FADE_STEPS 70

static dboolean gl_FadeActive;
static int gl_FadeLevel;
static int gl_FadeDelta;

static void gl_InitializeFade(dboolean fadeIn)
{
  gl_FadeActive = true;

  if (fadeIn)
  {
    gl_FadeLevel = HEXEN_FADE_STEPS;
    gl_FadeDelta = -1;
  }
  else
  {
    gl_FadeLevel = 0;
    gl_FadeDelta = 1;
  }
}

static void gl_DeInitializeFade(void)
{
  gl_FadeActive = false;

  gl_FadeLevel = 0;
  gl_FadeDelta = 0;
}

static void gl_FadePic(void)
{
  gl_FadeLevel += gl_FadeDelta;

  if (gl_FadeLevel < 0)
    gl_FadeLevel = 0;
  else if (gl_FadeLevel > HEXEN_FADE_STEPS)
    gl_FadeLevel = HEXEN_FADE_STEPS;
}

// hexen_note: dynamic palette editing
static fixed_t *Palette;
static fixed_t *PaletteDelta;
static byte *RealPalette;

static void InitializeFade(dboolean fadeIn)
{
    unsigned i;
    const byte *playpal;

    if (V_IsOpenGLMode())
    {
        gl_InitializeFade(fadeIn);
    }

    playpal = V_GetPlaypal();

    Palette = Z_Malloc(768 * sizeof(*Palette));
    PaletteDelta = Z_Malloc(768 * sizeof(*PaletteDelta));
    RealPalette = Z_Malloc(768 * sizeof(*RealPalette));

    if (fadeIn)
    {
        memset(RealPalette, 0, 768 * sizeof(*RealPalette));
        for (i = 0; i < 768; i++)
        {
            Palette[i] = 0;
            PaletteDelta[i] = FixedDiv((int)playpal[i] << FRACBITS,
                                    HEXEN_FADE_STEPS * FRACUNIT);
        }
    }
    else
    {
        for (i = 0; i < 768; i++)
        {
            RealPalette[i] = playpal[i];
            Palette[i] = RealPalette[i] << FRACBITS;
            PaletteDelta[i] = FixedDiv(Palette[i], -HEXEN_FADE_STEPS * FRACUNIT);
        }
    }
    V_SetDynamicPalette(RealPalette);
}

static void DeInitializeFade(void)
{
    if (V_IsOpenGLMode())
    {
        gl_DeInitializeFade();
    }

    V_ClearDynamicPalette();

    Z_Free(Palette);
    Z_Free(PaletteDelta);
    Z_Free(RealPalette);
}

static void FadePic(void)
{
    unsigned i;

    if (V_IsOpenGLMode())
    {
        gl_FadePic();
    }

    for (i = 0; i < 768; i++)
    {
        Palette[i] += PaletteDelta[i];
        RealPalette[i] = Palette[i] >> FRACBITS;
    }
    V_TouchPalette();
}

static void DrawPic(void)
{
    V_DrawRawScreen(FinaleLumpName);
    if (FinaleStage == 4 || FinaleStage == 5)
    {                           // Chess pic, draw the correct character graphic
        DrawChessOverlay();
    }

    // For OpenGL, we use a simple translucent fill block instead
    if (V_IsOpenGLMode())
    {
        if (gl_FadeActive)
        {
            extern void gld_FillBlock(int x, int y, int width, int height, int col, int trans_percent);
            gld_FillBlock(0, 0, SCREENWIDTH, SCREENHEIGHT, playpal_darkest,
                          gl_FadeLevel * 100 / HEXEN_FADE_STEPS);
        }
    }
}

void Hexen_F_Drawer(void)
{
    switch (FinaleStage)
    {
        case 0:                // Fade in initial finale screen
            DrawPic();
            break;
        case 1:
        case 2:
            TextWrite();
            break;
        case 3:                // Fade screen out
            DrawPic();
            break;
        case 4:                // Fade in chess screen
            DrawPic();
            break;
        case 5:
            TextWrite();
            break;
    }
}

extern char ClusterMessage[MAX_INTRMSN_MESSAGE_SIZE];

static char *GetFinaleText(int sequence)
{
    const char *msgLumpName;
    int msgSize;
    int msgLump;
    static const char *winMsgLumpNames[] = {
        "win1msg",
        "win2msg",
        "win3msg"
    };

    msgLumpName = winMsgLumpNames[sequence];
    msgLump = W_GetNumForName(msgLumpName);
    msgSize = W_LumpLength(msgLump);
    if (msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
    {
        I_Error("Finale message too long (%s)", msgLumpName);
    }

    memcpy(ClusterMessage, W_LumpByNum(msgLump), msgSize);
    ClusterMessage[msgSize] = '\0';        // Append terminator
    return ClusterMessage;
}
