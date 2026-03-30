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
//	DSDA Big Armor Text HUD Component
//

#include "base.h"

#include "big_armor_text.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
  int anchor;
  dboolean percent;
} local_component_t;

static local_component_t* local;

static int dsda_GetWidgetWidth(void)
{
  return dsda_GetBigNumberWidth(3, 999, local->right_align, local->anchor, local->percent);
}

static void dsda_DrawComponent(void) {
  player_t* player;
  int cm;
  int armor;
  int x, y;

  player = &players[displayplayer];
  armor = st_armor;

  if (!hexen && armor <= 0)
    return;

  if (hexen) {
    cm = dsda_TextCR(dsda_tc_stbar_armor_hexen);
  }
  else {
    if (heretic)
      cm = CR_DEFAULT;
    else // Doom
    {
      if (armor <= 0)
        cm = dsda_TextCR(dsda_tc_stbar_armor_zero);
      else if (player->armortype < 2)
        cm = dsda_TextCR(dsda_tc_stbar_armor_one);
      else
        cm = dsda_TextCR(dsda_tc_stbar_armor_two);
    }
  }

  x = local->component.x;
  y = local->component.y;

  dsda_DrawBigNumber(x, y, 0, cm, local->component.vpt, 3, armor, local->right_align, local->anchor, local->percent);
}

void dsda_InitBigArmorTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->right_align = (arg_count > 0) ? !!args[0] : false;
  local->anchor = (arg_count > 1) ? args[1] : false;
  local->percent = (arg_count > 2) ? !!args[2] : false;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArmorTextHC(void* data) {
  local = data;
}

void dsda_DrawBigArmorTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
