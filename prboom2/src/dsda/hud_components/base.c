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
//	DSDA HUD Component Base
//

#include <math.h>

#include "base.h"

int dsda_HudComponentY(int y_offset, int vpt, double ratio) {
  int dsda_ExHudVerticalOffset(void);

  int y = 0;
  int vpt_align;

  if (ratio)
    y_offset = (int)(y_offset * ratio);

  vpt_align = vpt & VPT_ALIGN_MASK;
  if (BOTTOM_ALIGNMENT(vpt_align)) {
    y = 200;
    y_offset = -y_offset;

    y -= dsda_ExHudVerticalOffset();
  }

  return y + y_offset;
}

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  static double ratio;
  int x, y;

  DO_ONCE
    if (exhud_font.line_height != 8)
      ratio = (double) exhud_font.line_height / 8.0;
  END_ONCE

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, ratio);

  HUlib_initTextLine(&component->text, x, y, &exhud_font, CR_GRAY, vpt);
}

void dsda_InitBlockyHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  static double ratio;
  int x, y;

  DO_ONCE
    if (hud_font.line_height != 8)
      ratio = (double) hud_font.line_height / 8.0;
  END_ONCE

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, ratio);

  HUlib_initTextLine(&component->text, x, y, &hud_font, CR_GRAY, vpt);
}

void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt) {
  int x, y;

  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt, 0);

  component->x = x;
  component->y = y;
  component->vpt = vpt;
}

void dsda_DrawBasicText(dsda_text_t* component) {
  HUlib_drawTextLine(&component->text, false, false, false);
}

void dsda_DrawYellowText(dsda_text_t* component, dboolean yellow) {
  HUlib_drawTextLine(&component->text, yellow, false, false);
}

void dsda_DrawBasicShadowedText(dsda_text_t* component) {
  HUlib_drawTextLine(&component->text, false, true, false);
}

void dsda_DrawYellowShadowedText(dsda_text_t* component, dboolean yellow) {
  HUlib_drawTextLine(&component->text, yellow, true, false);
}

void dsda_RefreshHudText(dsda_text_t* component) {
  const char* s;

  HUlib_clearTextLine(&component->text);

  s = component->msg;
  while (*s) HUlib_addCharToTextLine(&component->text, *(s++));
}

void dsda_RefreshHudTextWrapped(dsda_text_t* component, int centered, int max_lines)
{
  const char* s;

  HUlib_clearTextLine(&component->text);

  s = component->msg;
  HUlib_WrapStringToTextLines(&component->text, s, centered, max_lines);
}

//
//
// ammo stuff
//
//

ammotype_t dsda_GetReadyAmmo(player_t* player) {
  return weaponinfo[player->readyweapon].ammo;
}

ammotype_t dsda_GetWeaponAmmo(player_t* player, int weapon) {
  return weaponinfo[weapon].ammo;
}

dboolean dsda_WeaponNoAmmo(player_t* player, ammotype_t ammo_type) {
  return ammo_type == am_noammo || !player->maxammo[ammo_type];
}

dboolean dsda_OutOfAmmo(player_t* player, ammotype_t ammo_type) {
  return ammo_type != am_noammo && player->ammo[ammo_type] <= 0;
}

int dsda_AmmoColorBig(player_t* player) {
  int ammo_percent;

  ammo_percent = P_AmmoPercent(player, player->readyweapon);

  if (ammo_percent < hud_ammo_red)
    return dsda_tc_stbar_ammo_bad;
  else if (ammo_percent < hud_ammo_yellow)
    return dsda_tc_stbar_ammo_warning;
  else if (ammo_percent < 100)
    return dsda_tc_stbar_ammo_ok;
  else
    return dsda_tc_stbar_ammo_full;
}
