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
//	DSDA HUD Boom Bar
//

#include "boom_bar.h"

//
// Boom bar
//

static int dsda_BoomBarWidth(boom_bar_t* bar)
{
  return bar->count * bar->seg_w + (bar->count - 1) * bar->gap + bar->spacing;
}

static int dsda_FillSegments(int value, int max, int count)
{
  if (count <= 0 || max <= 0 || value <= 0)
    return 0;

  if (value >= max)
    return count;

  value = (value * count) / max;
  return value > 0 ? value : 1;
}

void dsda_SetBoomBarInfo(boom_bar_t* bar)
{
  const char* lump = hexen ? "STBR125" : heretic ? "STBR124" : "STBR123";
  bar->lump = W_GetNumForName(lump);
  bar->seg_w = R_NumPatchWidth(bar->lump);
  bar->spacing = HU_FontStringWidth(&exhud_font, " ");
  bar->count = 25;
  bar->gap = 1;
  bar->width = dsda_BoomBarWidth(bar);
}

void dsda_DrawBoomBar(boom_bar_t* bar, dsda_patch_component_t local_bar, int value, int value_max, int color, int supercolor, dboolean double_bar)
{
  int base_fill = 0;
  int super_fill = 0;
  int count = bar->count;
  int flags = VPT_COLOR | local_bar.vpt;
  int over_max = value_max * 2;

  base_fill = dsda_FillSegments(value, value_max, count);

  for (int i = 0; i < count; ++i)
  {
    int cm = (i < base_fill) ? color : CR_BLACK;

    V_DrawNumPatch(local_bar.x + i * (bar->seg_w + bar->gap),
                   local_bar.y,
                   bar->lump, cm, flags);
  }

  if (double_bar)
  {
    if (value > value_max)
    {
      super_fill = dsda_FillSegments(value - value_max, value_max, count);

      for (int i = 0; i < super_fill; ++i)
        V_DrawNumPatch(local_bar.x + i * (bar->seg_w + bar->gap),
                       local_bar.y,
                       bar->lump, supercolor, flags);
    }
  }
}
