//
// Copyright(C) 2026 by Andrik Powell
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
//	DSDA ST Fonts
//

#ifndef __STFONT__
#define __STFONT__

// We are referring to patches.
#include "r_defs.h"
#include "v_video.h"  // color ranges

extern void dsda_InitPatchNumbers(void);

typedef enum
{
  ANCHOR_NONE,
  ANCHOR_LEFT = ANCHOR_NONE,
  ANCHOR_CENTER,
  ANCHOR_RIGHT,
  ANCHOR_PERCENT,
} anchor_mode_t;

// Big Numbers
extern void dsda_DrawBigNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align, int anchor, int percent);
extern int dsda_GetBigNumberWidth(int count, int n, int right_align, int anchor, int percent);

// Med Numbers (Raven)
extern void dsda_DrawMedNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align);
extern int dsda_GetMedNumberWidth(int count, int n, int right_align);

// Small Numbers
extern void dsda_DrawSmallNumber(int x, int y, int delta_y, int cm, int vpt, int count, int n, int right_align, int anchor);
extern int dsda_GetSmallNumberWidth(int count, int n, int right_align);

#endif
