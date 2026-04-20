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
//	DSDA Big Mana HUD Component
//

#include "base.h"

#include "big_mana.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
  int anchor;
  dboolean horizontal;
  dboolean show_active;
} local_component_t;

static local_component_t* local;

static int patch_spacing;
static int pair_spacing_x;
static int pair_spacing_y;

static int manaLumpBlue;
static int manaLumpGreen;

static const char* mana_lump_name[2][2] = {
  { "MANADIM1", "MANABRT1" }, // Blue
  { "MANADIM2", "MANABRT2" }, // Green
};

static int P_ManaPercent(player_t *player, int mana)
{
  if (!player->ammo[mana])
    return 0;

  return player->ammo[mana] * 100 / MAX_MANA;
}

static int dsda_ManaColorBig(player_t* player, int mana) {
  int mana_percent = P_ManaPercent(player, mana);

  if (mana_percent < hud_ammo_red)
    return dsda_tc_stbar_ammo_bad;
  else if (mana_percent < hud_ammo_yellow)
    return dsda_tc_stbar_ammo_warning;
  else if (mana_percent < 100)
    return dsda_tc_stbar_ammo_ok;
  else
    return dsda_tc_stbar_ammo_full;
}

static void dsda_SetHexenAmmoImage(void) {
  player_t* player;
  const char* manaBlue;
  const char* manaGreen;
  dboolean hasManaBlue;
  dboolean hasManaGreen;

  player = &players[displayplayer];
  hasManaBlue  = player->ammo[MANA_1] > 0;
  hasManaGreen = player->ammo[MANA_2] > 0;

  if (player->readyweapon == wp_first)
  {
    manaBlue = mana_lump_name[MANA_1][false];
    manaGreen = mana_lump_name[MANA_2][false];
  }
  else if (player->readyweapon == wp_second)
  {
    manaBlue = mana_lump_name[MANA_1][hasManaBlue];
    manaGreen = mana_lump_name[MANA_2][false];
  }
  else if (player->readyweapon == wp_third)
  {
    manaBlue = mana_lump_name[MANA_1][false];
    manaGreen = mana_lump_name[MANA_2][hasManaGreen];
  }
  else // wp_fourth
  {
    manaBlue = mana_lump_name[MANA_1][hasManaBlue];
    manaGreen = mana_lump_name[MANA_2][hasManaGreen];
  }

  manaLumpBlue = W_GetNumForName(manaBlue);
  manaLumpGreen = W_GetNumForName(manaGreen);
}

// 3 digits "999"
static int dsda_GetManaTextWidth(void) {
  return dsda_GetMedNumberWidth(3, 999, local->right_align);
}

static int dsda_DrawManaPair(int x, int y, int lump, int cm, int mana, int flags, int numflags) {
  int lump_width = R_NumPatchWidth(lump);
  int text_width = dsda_GetManaTextWidth();
  int pair_width = lump_width + patch_spacing + text_width;

  // [AR] Hexen - Fighter Axe can go negative
  if (mana < 0)
    mana = 0;

  if (!local->right_align)
  {
    V_DrawShadowedNumPatch(x, y, lump, CR_DEFAULT, flags);
    x += lump_width + patch_spacing;
    dsda_DrawMedNumber(x, y, 0, cm, numflags, 3, mana, false);
  }
  else
  {
    //x -= pair_width;
    dsda_DrawMedNumber(x, y, 0, cm, numflags, 3, mana, true);
    x += text_width + patch_spacing;
    V_DrawShadowedNumPatch(x, y, lump, CR_DEFAULT, flags);
  }

  return pair_width;
}

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;
  int flags, numflags;
  int cm1, cm2;
  int mana1, mana2;
  dboolean draw_blue = true;
  dboolean draw_green = true;
  int text_width;
  int blue_width;
  int green_width;
  int total_width;

  player = &players[displayplayer];
  flags = numflags = local->component.vpt;
  x = local->component.x;
  y = local->component.y;

  // Numbers need offsets (so 1 doesn't have a big space)
  numflags &= ~VPT_NOOFFSET;

  mana1 = player->ammo[MANA_1];
  mana2 = player->ammo[MANA_2];
  cm1 = dsda_TextCR(dsda_ManaColorBig(player, MANA_1));
  cm2 = dsda_TextCR(dsda_ManaColorBig(player, MANA_2));

  dsda_SetHexenAmmoImage();

  if (local->show_active)
  {
    switch (player->readyweapon)
    {
      case wp_first:
        draw_blue = false;
        draw_green = false;
        break;

      case wp_second:
        draw_blue = true;
        draw_green = false;
        break;

      case wp_third:
        draw_blue = false;
        draw_green = true;
        break;

      case wp_fourth:
      default:
        draw_blue = true;
        draw_green = true;
        break;
    }
  }

  if (!draw_blue && !draw_green)
    return;

  // anchor
  text_width = dsda_GetManaTextWidth();
  blue_width = draw_blue ? R_NumPatchWidth(manaLumpBlue) + patch_spacing + text_width : 0;
  green_width = draw_green ? R_NumPatchWidth(manaLumpGreen) + patch_spacing + text_width : 0;

  // anchor
  if (local->anchor)
  {
    if (!local->horizontal)
    {
      total_width = MAX(blue_width, green_width);
    }
    else
    {
      if (draw_blue && draw_green)
        total_width = blue_width + pair_spacing_x + green_width;
      else
        total_width = blue_width + green_width;
    }

    if (local->anchor >= ANCHOR_RIGHT)
      x -= total_width;
    else if (local->anchor == ANCHOR_CENTER)
      x -= total_width / 2;
  }

  if (!local->horizontal)
  {
    if (draw_blue)
    {
      dsda_DrawManaPair(x, y, manaLumpBlue, cm1, mana1, flags, numflags);

      if (draw_green)
        y += pair_spacing_y;
    }

    if (draw_green)
    {
      dsda_DrawManaPair(x, y, manaLumpGreen, cm2, mana2, flags, numflags);
    }
  }
  else
  {
    if (draw_blue)
    {
      dsda_DrawManaPair(x, y, manaLumpBlue, cm1, mana1, flags, numflags);

      if (draw_green)
        x += blue_width + pair_spacing_x;
    }

    if (draw_green)
      dsda_DrawManaPair(x, y, manaLumpGreen, cm2, mana2, flags, numflags);
  }
}

void dsda_InitBigManaHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (!hexen) return;

  local->right_align  = (arg_count > 0) ? !!args[0] : false;
  local->anchor = (arg_count > 1) ? args[1] : false;
  local->horizontal   = (arg_count > 2) ? !!args[2] : false;
  local->show_active  = (arg_count > 3) ? !!args[3] : false;

  manaLumpBlue = 0;
  manaLumpGreen = 0;

  patch_spacing = 6;
  pair_spacing_x = 12;
  pair_spacing_y = 15;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigManaHC(void* data) {
  local = data;

  if (!hexen) return;
}

void dsda_DrawBigManaHC(void* data) {
  local = data;

  if (!hexen) return;

  dsda_DrawComponent();
}
