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
//	DSDA Big Ammo Text HUD Component
//

#include "base.h"

#include "big_ammo_text.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
  int anchor;
} local_component_t;

static local_component_t* local;

static void dsda_DrawComponent(void) {
  player_t* player;
  ammotype_t ammo_type;
  int ammo;
  int flags;
  int x, y;
  int cm;

  if (hexen)
    return;

  player = &players[displayplayer];
  ammo_type = dsda_GetReadyAmmo(player);

  flags = local->component.vpt;
  x = local->component.x;
  y = local->component.y;

  if (ammo_type == am_noammo || !player->maxammo[ammo_type])
    return;

  ammo = player->ammo[ammo_type];
  cm = raven ? CR_DEFAULT : dsda_TextCR(dsda_AmmoColorBig(player));

  dsda_DrawBigNumber(x, y, 0, cm, flags, 3, ammo, local->right_align, local->anchor, false);
}

void dsda_InitBigAmmoTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->right_align = (arg_count > 0) ? !!args[0] : false;
  local->anchor = (arg_count > 1) ? args[1] : false;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigAmmoTextHC(void* data) {
  local = data;
}

void dsda_DrawBigAmmoTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
