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

static void dsda_SetHexenAmmoImage(void) {
  player_t* player;
  const char* manaBlue;
  const char* manaGreen;
  dboolean hasManaBlue;
  dboolean hasManaGreen;

  player = &players[displayplayer];
  hasManaBlue = !!player->ammo[MANA_1];
  hasManaGreen = !!player->ammo[MANA_2];

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
  int width1;
  dboolean draw_blue = true;
  dboolean draw_green = true;

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
    width1 = draw_blue  ? R_NumPatchWidth(manaLumpBlue)  + patch_spacing + dsda_GetManaTextWidth() : 0;

    if (draw_blue)
    {
      dsda_DrawManaPair(x, y, manaLumpBlue, cm1, mana1, flags, numflags);

      if (draw_green)
        x += width1 + pair_spacing_x;
    }

    if (draw_green)
      dsda_DrawManaPair(x, y, manaLumpGreen, cm2, mana2, flags, numflags);
  }
}

void dsda_InitBigManaHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (!hexen) return;

  local->right_align = (arg_count > 0) ? !!args[0] : false;
  local->horizontal  = (arg_count > 1) ? !!args[1] : false;
  local->show_active = (arg_count > 2) ? !!args[2] : false;

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
