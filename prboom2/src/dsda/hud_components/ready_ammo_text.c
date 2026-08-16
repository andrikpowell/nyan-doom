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
//	DSDA Ready Ammo Text HUD Component
//

#include "base.h"

#include "ready_ammo_text.h"
#include "boom_bar.h"

typedef struct {
  dsda_text_t label;
  dsda_patch_component_t ammo_bar;
  dsda_text_t value;
  dboolean draw_boom_bar;
} local_component_t;

static local_component_t* local;

static const char* label;
static boom_bar_t boom_bar;
static int label_width;

static int dsda_ReadyAmmoColor(player_t* player) {
  ammotype_t ammo_type = dsda_GetReadyAmmo(player);

  // Weapon has no ammo
  if (dsda_WeaponNoAmmo(player, ammo_type))
    return dsda_tc_exhud_ammo_value;

  // Weapon ran out of ammo
  else if (dsda_OutOfAmmo(player, ammo_type))
    return dsda_tc_exhud_ammo_out;

  // draw normal ammo
  else
  {
    int ammo_percent = P_AmmoPercent(player, player->readyweapon);

    if (ammo_percent < hud_ammo_red)
      return dsda_tc_exhud_ammo_bad;
    else if (ammo_percent < hud_ammo_yellow)
      return dsda_tc_exhud_ammo_warning;
    else if (ammo_percent < 100)
      return dsda_tc_exhud_ammo_ok;
    else
      return dsda_tc_exhud_ammo_full;
  }
}

static void dsda_DrawBar(player_t* player) {
  int cur_ammo, max_ammo, cm;

  // setup bar
  {
    ammotype_t ammo_type = dsda_GetReadyAmmo(player);
    dboolean has_ammo = ammo_type != am_noammo && player->ammo[ammo_type] > 0;

    // if weapon uses ammo and has ammo, else blank bar
    cur_ammo = has_ammo ? player->ammo[ammo_type]     : 0;
    max_ammo = has_ammo ? player->maxammo[ammo_type]  : 1;
  }

  // color
  cm = dsda_TextCR(dsda_ReadyAmmoColor(player));

  // draw bar after label
  local->ammo_bar.x = local->label.text.x + label_width;

  // draw bar
  dsda_DrawBoomBar(&boom_bar, local->ammo_bar, cur_ammo, max_ammo, cm, cm, false);

  // draw value after bar
  local->value.text.x = local->ammo_bar.x + boom_bar.width;
}

static void dsda_DrawComponent(void) {
  player_t* player;
  
  player = &players[displayplayer];

  dsda_DrawBasicText(&local->label);

  if (local->draw_boom_bar)
    dsda_DrawBar(player);
  else
    // draw text after label (instead of bar)
    local->value.text.x = local->label.text.x + label_width;

  dsda_DrawBasicText(&local->value);
}

static void dsda_UpdateComponentText(void) {
  player_t* player;
  const char* label_cm;
  const char* ammo_cm;

  player = &players[displayplayer];
  
  label_cm = (local->draw_boom_bar) ? dsda_TextColor(dsda_ReadyAmmoColor(player)) :
                                      dsda_TextColor(dsda_tc_exhud_ammo_label);
  // Get label text
  snprintf(local->label.msg, sizeof(local->label.msg), "%s%s", label_cm, label);

  // Get ammo text
  {
    if (hexen) {
      int mana1 = player->ammo[0];
      int mana2 = player->ammo[1];

      // [AR] Hexen - Fighter Axe can go negative
      if (mana1 < 0)  mana1 = 0;
      if (mana2 < 0)  mana2 = 0;

      snprintf(local->value.msg, sizeof(local->value.msg), "%s%3d %s%3d",
              dsda_TextColor(dsda_tc_exhud_ammo_mana1), mana1,
              dsda_TextColor(dsda_tc_exhud_ammo_mana2), mana2);
    }
    else {
      ammotype_t ammo_type = dsda_GetReadyAmmo(player);
      ammo_cm = dsda_TextColor(dsda_ReadyAmmoColor(player));

      if (dsda_WeaponNoAmmo(player, ammo_type))
        snprintf(local->value.msg, sizeof(local->value.msg), "%s N/A", ammo_cm);
      else if (local->draw_boom_bar)
        snprintf(local->value.msg, sizeof(local->value.msg), "%s%d/%d", ammo_cm, player->ammo[ammo_type], player->maxammo[ammo_type]);
      else
        snprintf(local->value.msg, sizeof(local->value.msg), "%s%d", ammo_cm, player->ammo[ammo_type]);
    }
  }
}

void dsda_InitReadyAmmoTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  label = "AMM ";
  local->draw_boom_bar = (arg_count > 0) ? !!args[0] : false;

  if (hexen)
    local->draw_boom_bar = false;

  dsda_InitTextHC(&local->label, x_offset, y_offset, vpt);
  dsda_InitTextHC(&local->value, x_offset, y_offset, vpt);
  dsda_InitPatchHC(&local->ammo_bar, x_offset, y_offset, vpt);

  label_width = HU_FontStringWidth(&exhud_font, label);
  dsda_SetBoomBarInfo(&boom_bar);
}

void dsda_UpdateReadyAmmoTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText();
  dsda_RefreshHudText(&local->label);
  dsda_RefreshHudText(&local->value);
}

void dsda_DrawReadyAmmoTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
