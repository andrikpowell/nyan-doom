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
//	DSDA Big Health Text HUD Component
//

#include "base.h"

#include "big_health_text.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
  dboolean right_anchor;
  dboolean percent;
} local_component_t;

static local_component_t* local;

static void dsda_DrawComponent(void) {
  int cm;
  int health;
  int x, y;

  // Animated health
  health = st_health;

  cm = raven ? CR_DEFAULT :
       health <= hud_health_red ? dsda_TextCR(dsda_tc_stbar_health_bad) :
       health <= hud_health_yellow ? dsda_TextCR(dsda_tc_stbar_health_warning) :
       health <= hud_health_green ? dsda_TextCR(dsda_tc_stbar_health_ok) :
       dsda_TextCR(dsda_tc_stbar_health_super);

  x = local->component.x;
  y = local->component.y;

  dsda_DrawBigNumber(x, y, 0, cm, local->component.vpt, 3, health, local->right_align, local->right_anchor, local->percent, false);
}

void dsda_InitBigHealthTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->right_align = (arg_count > 0) ? !!args[0] : false;
  local->right_anchor = (arg_count > 1) ? !!args[1] : false;
  local->percent = (arg_count > 2) ? !!args[2] : false;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigHealthTextHC(void* data) {
  local = data;
}

void dsda_DrawBigHealthTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
