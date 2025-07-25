//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Stat Totals HUD Component
//

#ifndef __DSDA_HUD_COMPONENT_STAT_TOTALS__
#define __DSDA_HUD_COMPONENT_STAT_TOTALS__

int dsda_PrintStats(size_t length, char *buffer, size_t size, int format, const char* label, const char* cm, const int th_count, const int th_total, dboolean separator);
void dsda_InitStatTotalsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateStatTotalsHC(void* data);
void dsda_DrawStatTotalsHC(void* data);

#endif
