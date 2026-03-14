/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Mission begin melt/wipe screen special effect.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"
#include "doomdef.h"
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"
#include "r_fps.h"
#include "f_wipe.h"
#include "gl_struct.h"
#include "e6y.h"//e6y

#include "dsda/settings.h"
#include "dsda/stretch.h"
#include "dsda/time.h"

//
// SCREEN WIPE PACKAGE
//

// Parts re-written to support true-color video modes. Column-major
// formatting removed. - POPE

static screeninfo_t wipe_scr_start;
static screeninfo_t wipe_scr_end;
static screeninfo_t wipe_scr;

#define WIPE_ROWS 200
static int wipe_columns;
static int wipe_rows;

static fixed_t wipe_frac;

static int *ybuff1;
static int *ybuff2;
static int *curry;
static int *prevy;

static void wipe_BindScreens(void)
{
  screens[WIPE_SRC]  = wipe_scr_start;
  screens[WIPE_DST]  = wipe_scr_end;
  screens[WIPE_TEMP] = wipe_scr;
}

static fixed_t wipe_GetFrac(void)
{
  unsigned long long fractic;
  unsigned long long frac64;

  if (!R_ViewInterpolation())
    return FRACUNIT;

  fractic = dsda_TickElapsedTime();

  frac64 = fractic * TICRATE * FRACUNIT / 1000000ULL;

  if (frac64 > FRACUNIT)
    frac64 = FRACUNIT;

  return (fixed_t) frac64;
}

static void wipe_EnsureBuffer(screeninfo_t *scr)
{
  int pitch = screens[FG].pitch;

  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(pitch % 1024))
    pitch += 32;

  if (scr->data &&
      scr->width == SCREENWIDTH &&
      scr->height == SCREENHEIGHT &&
      scr->pitch == pitch)
    return;

  if (scr->data)
    V_FreeScreen(scr);

  scr->width = SCREENWIDTH;
  scr->height = SCREENHEIGHT;
  scr->pitch = pitch;
  scr->not_on_heap = false;
  V_AllocScreen(scr);
}

static int wipe_initMelt(int ticks)
{
  int i;
  stretch_param_t *params = dsda_StretchParams(VPT_STRETCH);

  wipe_columns = MAX(1, (SCREENWIDTH * 320 + params->video->width - 1) / params->video->width / 2);
  wipe_rows = WIPE_ROWS;

  ybuff1 = Z_Malloc(wipe_columns * sizeof(*ybuff1));
  ybuff2 = Z_Malloc(wipe_columns * sizeof(*ybuff2));

  curry = ybuff1;
  prevy = ybuff2;

  // setup initial column positions (curry < 0 => not ready to scroll yet)
  curry[0] = -(M_Random() % 16);
  for (i = 1; i < wipe_columns; i++)
  {
    int r = (M_Random() % 3) - 1;
    curry[i] = curry[i - 1] + r;
    if (curry[i] > 0)
      curry[i] = 0;
    else
      if (curry[i] == -16)
        curry[i] = -15;
  }

  memcpy(prevy, curry, wipe_columns * sizeof(*prevy));
  return 0;
}

int wipe_MeltCurrentPosition(int col)
{
  if (col < 0 || col >= wipe_columns)
    return wipe_rows;

  if (R_ViewInterpolation())
    return prevy[col] + FixedMul(curry[col] - prevy[col], wipe_frac);

  return curry[col];
}

static dboolean wipe_updateMelt(int ticks)
{
  dboolean done = true;

  if (ticks > 0)
  {
    while (ticks--)
    {
      int *temp = prevy;
      prevy = curry;
      curry = temp;

      for (int col = 0; col < wipe_columns; ++col)
      {
        if (prevy[col] < 0)
        {
          curry[col] = prevy[col] + 1;
          done = false;
        }
        else if (prevy[col] < wipe_rows) {
          int dy = (prevy[col] < 16) ? prevy[col] + 1 : 8;
          curry[col] = MIN(prevy[col] + dy, wipe_rows);
          done = false;
        }
        else
        {
          curry[col] = wipe_rows;
        }
      }
    }
  }
  else
  {
    for (int col = 0; col < wipe_columns; ++col)
    {
      done &= curry[col] >= wipe_rows;
    }
  }
  return done;
}

// CPhipps - modified to allocate and deallocate screens[2 to 3] as needed, saving memory
static void wipe_renderMelt(void)
{
  dboolean done = true;

  // Scale up and then down to handle arbitrary dimensions with integer math
  int width = SCREENWIDTH;
  int height = SCREENHEIGHT;
  int currcol;
  int currcolend;
  int currrow;

  wipe_BindScreens();
  V_CopyScreen(WIPE_DST, WIPE_TEMP);

  for (int col = 0; col < wipe_columns; ++col)
  {
    int current = wipe_MeltCurrentPosition(col);

    if (current < 0)
    {
      currcol = (col * width) / wipe_columns;
      currcolend = ((col + 1) * width) / wipe_columns;
      for (; currcol < currcolend; ++currcol)
      {
        byte *source = wipe_scr_start.data + currcol;
        byte *dest   = wipe_scr.data + currcol;

        for (int i = 0; i < height; ++i)
        {
          *dest = *source;
          dest += wipe_scr.pitch;
          source += wipe_scr_start.pitch;
        }
      }
    }
    else if (current < wipe_rows)
    {
      currcol = (col * width) / wipe_columns;
      currcolend = ((col + 1) * width) / wipe_columns;

      currrow = (current * height) / wipe_rows;

      for (; currcol < currcolend; ++currcol)
      {
        byte *source = wipe_scr_start.data + currcol;
        byte *dest   = wipe_scr.data + currcol + (currrow * wipe_scr.pitch);

        for (int i = 0; i < height - currrow; ++i)
        {
          *dest = *source;
          dest += width;
          source += width;
        }
      }
    }
  }

  V_CopyScreen(WIPE_TEMP, FG);
}

static int wipe_exitMelt(int ticks)
{
  if (V_IsOpenGLMode())
  {
    gld_wipe_exitMelt(ticks);
    return 0;
  }

  Z_Free(ybuff1);
  Z_Free(ybuff2);

  V_FreeScreen(&wipe_scr_start);
  wipe_scr_start.width = 0;
  wipe_scr_start.height = 0;
  wipe_scr_start.pitch = 0;

  V_FreeScreen(&wipe_scr_end);
  wipe_scr_end.width = 0;
  wipe_scr_end.height = 0;
  wipe_scr_end.pitch = 0;

  V_FreeScreen(&wipe_scr);
  wipe_scr.width = 0;
  wipe_scr.height = 0;
  wipe_scr.pitch = 0;

  // Paranoia
  screens[WIPE_SRC]  = wipe_scr_start;
  screens[WIPE_DST]  = wipe_scr_end;
  screens[WIPE_TEMP] = wipe_scr;
  return 0;
}

int wipe_StartScreen(void)
{
  if(dsda_PendingSkipWipe() || wasWiped) return 0;//e6y
  wasWiped = true;//e6y

  if (V_IsOpenGLMode())
  {
    gld_wipe_StartScreen();
    return 0;
  }

  wipe_EnsureBuffer(&wipe_scr_start);
  wipe_BindScreens();

  V_CopyScreen(FG, WIPE_SRC); // Copy start screen to buffer
  return 0;
}

int wipe_EndScreen(void)
{
  if(dsda_PendingSkipWipe() || !wasWiped) return 0;//e6y
  wasWiped = false;//e6y

  if (V_IsOpenGLMode())
  {
    gld_wipe_EndScreen();
    return 0;
  }

  wipe_EnsureBuffer(&wipe_scr_end);
  wipe_EnsureBuffer(&wipe_scr);
  wipe_BindScreens();

  V_CopyScreen(FG, WIPE_DST); // Copy end screen to buffer
  V_CopyScreen(WIPE_SRC, FG); // restore start screen
  return 0;
}

static dboolean wipe_doMelt(int ticks)
{
  dboolean done;

  wipe_frac = wipe_GetFrac();
  done = wipe_updateMelt(ticks);

  if (V_IsOpenGLMode())
    gld_wipe_renderMelt(wipe_columns, wipe_rows);
  else
    wipe_renderMelt();

  return done;
}

// killough 3/5/98: reformatted and cleaned up
int wipe_ScreenWipe(int ticks)
{
  static dboolean go;                               // when zero, stop the wipe

  if (!dsda_RenderWipeScreen())
    return 0;//e6y

  if (!go)                                         // initial stuff
  {
    go = 1;
    wipe_initMelt(ticks);
  }

  // do a piece of wipe-in
  if (wipe_doMelt(ticks))     // final stuff
  {
    wipe_exitMelt(ticks);
    go = 0;
  }
  return !go;
}
