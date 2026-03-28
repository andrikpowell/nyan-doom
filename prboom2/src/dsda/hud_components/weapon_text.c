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
//	DSDA Weapon Text HUD Component
//

#include "base.h"

#include "weapon_text.h"

typedef struct {
  dsda_text_t component;
  dboolean grid;
  const char* label_w;
  const char* label_p;
  const char* label_n;
  const char* label_wpn;
  dboolean color_ammo;
} local_component_t;

static local_component_t* local;

static dboolean dsda_WeaponNoAmmo(player_t* player, int ammo_i) {
  return ammo_i == am_noammo || !player->maxammo[ammo_i];
}

static dboolean dsda_OutOfAmmo(player_t* player, int ammo_i) {
  return ammo_i != am_noammo && player->ammo[ammo_i] <= 0;
}

int dsda_GetWeaponAmmoColor(player_t* player, int num) {
  int ammo_i = weaponinfo[num].ammo;

  // Weapon has no ammo
  if (dsda_WeaponNoAmmo(player, ammo_i))
    return dsda_tc_exhud_ammo_value;

  // Weapon ran out of ammo
  else if (dsda_OutOfAmmo(player, ammo_i))
    return dsda_tc_exhud_ammo_none;

  // draw normal ammo
  else
  {
    int ammo_percent = P_AmmoPercent(player, num);

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

static const char* dsda_WeaponColor(player_t* player, int num)
{
  int ammo_color = (!hexen && local->color_ammo) ? dsda_GetWeaponAmmoColor(player, num) : dsda_tc_exhud_weapon_owned;

  if (!raven)
  {
    // Berserk colour
    if (num == 0)
    {
      return player->powers[pw_strength] ? dsda_TextColor(dsda_tc_exhud_weapon_berserk) :
                                           dsda_TextColor(ammo_color);
    }
  }

  return dsda_TextColor(ammo_color);
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  if (local->grid)
    snprintf(
      str,
      max_size,
      "%s%s %s%c %s%c %s%c\n"
      "%s%s %s%c %s%c %s%c\n"
      "%s%s %s%c %s%c %s%c",
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_w,
      dsda_WeaponColor(player, 0), player->weaponowned[0] ? '1' : ' ',
      dsda_WeaponColor(player, 1), player->weaponowned[1] ? '2' : ' ',
      dsda_WeaponColor(player, 2), player->weaponowned[2] ? '3' : ' ',
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_p,
      dsda_WeaponColor(player, 3), player->weaponowned[3] ? '4' : ' ',
      dsda_WeaponColor(player, 4), player->weaponowned[4] ? '5' : ' ',
      dsda_WeaponColor(player, 5), player->weaponowned[5] ? '6' : ' ',
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_n,
      dsda_WeaponColor(player, 6), player->weaponowned[6] ? '7' : ' ',
      dsda_WeaponColor(player, 7), player->weaponowned[7] ? '8' : ' ',
      dsda_WeaponColor(player, 8), player->weaponowned[8] ? '9' : ' '
    );
  else
    snprintf(
      str,
      max_size,
      "%s%s%s%c %s%c %s%c %s%c %s%c %s%c %s%c %s%c %s%c",
      dsda_TextColor(dsda_tc_exhud_weapon_label),
      local->label_wpn,
      dsda_WeaponColor(player, 0), player->weaponowned[0] ? '1' : ' ',
      dsda_WeaponColor(player, 1), player->weaponowned[1] ? '2' : ' ',
      dsda_WeaponColor(player, 2), player->weaponowned[2] ? '3' : ' ',
      dsda_WeaponColor(player, 3), player->weaponowned[3] ? '4' : ' ',
      dsda_WeaponColor(player, 4), player->weaponowned[4] ? '5' : ' ',
      dsda_WeaponColor(player, 5), player->weaponowned[5] ? '6' : ' ',
      dsda_WeaponColor(player, 6), player->weaponowned[6] ? '7' : ' ',
      dsda_WeaponColor(player, 7), player->weaponowned[7] ? '8' : ' ',
      dsda_WeaponColor(player, 8), player->weaponowned[8] ? '9' : ' '
    );
}

void dsda_InitWeaponTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->grid = args[0];

  if (arg_count < 2 || args[1]) {
    local->label_w = "W ";
    local->label_p = "P ";
    local->label_n = "N ";
    local->label_wpn = "WPN ";
  }
  else {
    local->label_w = "";
    local->label_p = "";
    local->label_n = "";
    local->label_wpn = "";
  }

  local->color_ammo = arg_count > 3 ? !!args[2] : false;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateWeaponTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawWeaponTextHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
