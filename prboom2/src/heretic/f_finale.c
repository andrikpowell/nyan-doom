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
// F_finale.c

#include "doomstat.h"
#include "w_wad.h"
#include "m_menu.h"
#include "v_video.h"
#include "s_sound.h"
#include "sounds.h"

#include "dsda/palette.h"

#include "heretic/def.h"
#include "heretic/dstrings.h"

#include "heretic/f_finale.h"

static int finalestage;                // 0 = text, 1 = art screen
static int finalecount;
static dboolean heretic_e2_palette;

#define TEXTSPEED       3
#define TEXTWAIT        250
#define NEWTEXTWAIT     1000

// Stuff needed to advance forward
extern void WI_checkForAccelerate(void);
extern float Get_TextSpeed(void);
extern int acceleratestage;
extern int midstage;

static const char *finaletext;
static const char *finaleflat;

static int FontABaseLump;

/*
=======================
=
= F_StartFinale
=
=======================
*/

void Heretic_F_StartFinale(void)
{
  gameaction = ga_nothing;
  gamestate = GS_FINALE;
  automap_active = false;

  switch (gameepisode)
  {
    case 1:
      finaleflat = HERETIC_BGFLATE1;
      finaletext = HERETIC_E1TEXT;
      break;
    case 2:
      finaleflat = HERETIC_BGFLATE2;
      finaletext = HERETIC_E2TEXT;
      break;
    case 3:
      finaleflat = HERETIC_BGFLATE3;
      finaletext = HERETIC_E3TEXT;
      break;
    case 4:
      finaleflat = HERETIC_BGFLATE4;
      finaletext = HERETIC_E4TEXT;
      break;
    case 5:
      finaleflat = HERETIC_BGFLATE5;
      finaletext = HERETIC_E5TEXT;
      break;
  }

  acceleratestage = midstage = 0;

  finalestage = 0;
  finalecount = 0;
  FontABaseLump = W_GetNumForName("FONTA_S") + 1;
  S_ChangeMusic(heretic_mus_cptd, true);
}

dboolean F_BlockingInput(void)   // Avoid bringing up menu when loading Heretic's custom E2 palette
{
  return heretic_e2_palette && finalestage == 1 && gameepisode == 2;
}

dboolean Heretic_F_Responder(event_t * event)
{
  if (event->type != ev_keydown)
  {
    return false;
  }

  if (F_BlockingInput())
  {                           // we're showing the water pic, make any key kick to demo mode
    finalestage++;
    S_StartVoidSound(g_sfx_swtchx);
    V_SetPlayPal(playpal_default);
    return true;
  }

  return false;
}

/*
=======================
=
= F_Ticker
=
=======================
*/

void Heretic_F_Ticker(void)
{
  if (allow_incompatibility)
    WI_checkForAccelerate();

  finalecount++;

  if (!finalestage)
  {
    if (finalecount > strlen(finaletext) * TEXTSPEED + (midstage ? NEWTEXTWAIT : TEXTWAIT) || (midstage && acceleratestage))
    {
      finalecount = 0;
      finalestage = 1;
    }
  }
}

/*
=======================
=
= F_TextWrite
=
=======================
*/

static void Heretic_F_TextWrite(void)
{
  int count;
  const char *ch;
  int c;
  int cx, cy;
  int lump;
  int width;

  // e6y: wide-res
  V_ClearBorder(NULL);

  //
  // erase the entire screen to a tiled background
  //
  V_DrawBackgroundName(finaleflat);

  //
  // draw some of the text onto the screen
  //
  cx = 20;
  cy = 5;
  ch = finaletext;

  count = (int)((float)(finalecount - 10) / Get_TextSpeed());
  if (count < 0)
    count = 0;
  for (; count; count--)
  {
    c = *ch++;
    if (!c)
      break;
    if (c == '\n')
    {
      cx = 20;
      cy += 9;
      continue;
    }

    c = toupper(c);
    if (c < 33)
    {
      cx += 5;
      continue;
    }

    lump = FontABaseLump + c - 33;
    width = R_NumPatchWidth(lump);
    if (cx + width > SCREENWIDTH)
      break;
    V_DrawMenuNumPatch(cx, cy, lump, CR_DEFAULT, VPT_STRETCH);
    cx += width;
  }
}

/*
==================
=
= F_DemonScroll
=
==================
*/

static void F_DemonScroll(void)
{
  float scrolled = 200 - ((float)finalecount-70)/3;
  int lump_height = 200;

  V_ClearBorder("FINAL1");

  if (scrolled <= 0) {
    V_DrawRawScreenOffset("FINAL2", 0, 0, VPT_STRETCH);
  } else if (scrolled >= lump_height) {
    V_DrawRawScreenOffset("FINAL1", 0, 0, VPT_STRETCH);
  } else {
    V_DrawRawScreenOffset("FINAL1", 0, (float)(lump_height - scrolled), VPT_STRETCH);
    V_DrawRawScreenOffset("FINAL2", 0, -scrolled, VPT_STRETCH);
  }
}

/*
==================
=
= F_DrawUnderwater
=
==================
*/

static void F_DrawUnderwater(void)
{
  heretic_e2_palette = false;

  switch (finalestage)
  {
    case 1:
      if (menuactive) // Force menu off to avoid bad palette on menu
      {
        M_LeaveSetupMenu();
        M_ClearMenus();
        S_StartVoidSound(g_sfx_swtchx);
      }
      V_SetPlayPal(playpal_heretic_e2end);
      V_DrawRawScreen("E2END");
      heretic_e2_palette = true;

      break;
    case 2:
      V_DrawRawScreen("TITLE");
      heretic_e2_palette = false;
  }
}

/*
=======================
=
= F_Drawer
=
=======================
*/

void Heretic_F_Drawer(void)
{
  if (!finalestage)
    Heretic_F_TextWrite();
  else
  {
    switch (gameepisode)
    {
      case 1:
        if (gamemode == shareware)
        {
          V_DrawRawScreen("ORDER");
        }
        else
        {
          V_DrawRawScreen("CREDIT");
        }
        break;
      case 2:
        F_DrawUnderwater();
        break;
      case 3:
        F_DemonScroll();
        break;
      case 4:            // Just show credits screen for extended episodes
      case 5:
        V_DrawRawScreen("CREDIT");
        break;
    }
  }
}
