//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA TRANMAP
//

#ifndef __DSDA_TRANMAP__
#define __DSDA_TRANMAP__

#include "doomtype.h"

const byte* dsda_TranMap(unsigned int alpha);
const byte* dsda_DefaultTranMap(void);
extern int tran_filter_pct;
extern int exhud_tran_filter_pct;
extern int exhud_tint_filter_pct;
extern int exhud_alttint_filter_pct;
extern int shadow_filter_pct;
extern int alttint_filter_pct;
extern int exhud_shadow_filter_pct;
extern int shadow_ui_filter_pct;
extern int gl_alttint_filter_pct;
extern int gl_exhud_alttint_filter_pct;

#endif
