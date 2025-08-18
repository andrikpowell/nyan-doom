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
 *      Status bar code.
 *      Does the face/direction indicator animatin.
 *      Does palette indicators as well (red pain/berserk, bright pickup)
 *
 *-----------------------------------------------------------------------------*/

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "r_defs.h"

// Size of statusbar.
// Now sensitive for scaling.

// proff 08/18/98: Changed for high-res
#define ST_HEIGHT 32
#define ST_WIDTH  320
#define ST_Y      (200 - ST_HEIGHT)

// e6y: wide-res
extern int ST_SCALED_HEIGHT;
extern int ST_SCALED_WIDTH;
extern int ST_SCALED_Y;
extern int ST_SCALED_OFFSETX;

void ST_SetScaledWidth(void);
void ST_LoadTextColors(void);

extern dboolean drawdisk;
extern int drawdisktics;
extern const int DRAWDISKTICS;

//
// STATUS BAR
//

// Called by main loop.
dboolean ST_Responder(event_t* ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(void);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// After changing videomode;
void ST_SetResolution(void);

int ST_HealthColor(int health);

// States for status bar code.
typedef enum
{
  AutomapState,
  FirstPersonState
} st_stateenum_t;

extern int st_palette;    // cph 2006/04/06 - make palette visible

typedef enum
{
  BERSERK_ICON_OFF,
  BERSERK_ICON_ON
} berserk_icon_t;

typedef enum
{
  ARMOR_ICON_OFF,
  ARMOR_ICON_1,
  ARMOR_ICON_2
} armor_icon_t;

extern berserk_icon_t berserk_icon;
extern armor_icon_t armor_icon;

extern int sts_solid_bg_color;

// [Alaux]
extern int st_health;
extern int st_armor;
extern int SmoothCount(int shownval, int realval);

// [crispy] blinking key or skull in the status bar
extern int sts_blink_keys;
#define KEYBLINKMASK 0x8
#define KEYBLINKTICS (7*KEYBLINKMASK)
extern void ST_updateBlinkingKeys(player_t* plyr);
extern void ST_SetKeyBlink(player_t* player, int blue, int yellow, int red);
extern int  ST_BlinkKey(player_t* player, int index);
extern int  st_keyorskull[3];
extern int  st_keytype[6];

// e6y: makes sense for wide resolutions
extern patchnum_t grnrock;
extern patchnum_t brdr_t, brdr_b, brdr_l, brdr_r;
extern patchnum_t brdr_tl, brdr_tr, brdr_bl, brdr_br;

#endif
