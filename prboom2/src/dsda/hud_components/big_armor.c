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
//	DSDA Big Armor HUD Component
//

#include "base.h"

#include "big_armor.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
  dboolean percent;
} local_component_t;

static local_component_t* local;

static int armor_lump_green;
static int armor_lump_blue;

static int font_height;
static int patch_spacing;
static int patch_spacing_x;
static int patch_spacing_y;

static void dsda_ArmorPatchSpacing(void)
{
  int lumps[] = {
    armor_lump_green,
    armor_lump_blue,
  };

  for (int i = 0; i < sizeof(lumps) / sizeof(lumps[0]); ++i)
  {
    patch_spacing_x = MAX(patch_spacing_x, R_NumPatchWidth(lumps[i]));
    patch_spacing_y = MAX(patch_spacing_y, R_NumPatchHeight(lumps[i]));
  }
}

static void dsda_DrawBigArmorIcon(int x, int y, int lump, int flags) {
  int w, h;

  if (!lump)
    return;

  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  // center horizontally
  x += (patch_spacing_x - w) / 2;
  
  // center vertically
  y += (font_height - patch_spacing_y) / 2;
  y += (patch_spacing_y - h) / 2;

  if (raven)
    V_DrawShadowedNumPatch(x, y, lump, CR_DEFAULT, flags);
  else
    V_DrawMenuNumPatch(x, y, lump, CR_DEFAULT, flags);
}

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;
  int cm;
  int lump;
  int armor;
  int flags, numflags;

  player = &players[displayplayer];
  flags = numflags = local->component.vpt;
  x = local->component.x;
  y = local->component.y;

  if (hexen) {
    armor = dsda_HexenArmor(player);
    cm = dsda_TextCR(dsda_tc_stbar_armor_zero);
    lump = armor_lump_green;
  }
  else {
    armor = st_armor;
    if (armor <= 0) {
      cm = dsda_TextCR(dsda_tc_stbar_armor_zero);
      lump = armor_lump_green;
    }
    else if (player->armortype < 2) {
      cm = dsda_TextCR(dsda_tc_stbar_armor_one);
      lump = armor_lump_green;
    }
    else {
      cm = dsda_TextCR(dsda_tc_stbar_armor_two);
      lump = armor_lump_blue;
    }
  }

  if (!local->right_align)
  {
    dsda_DrawBigArmorIcon(x, y, lump, flags);
    x += patch_spacing + patch_spacing_x;
  }

  // Numbers need offsets (so 1 doesn't have a big space)
  numflags &= ~VPT_NOOFFSET;

  dsda_DrawBigNumber(x, y, 0,
                     cm, numflags, 3, armor, local->right_align, local->percent);

  if (local->right_align)
  {
    x += patch_spacing + dsda_GetBigNumberWidth(3, armor, local->right_align, local->percent);
    dsda_DrawBigArmorIcon(x, y, lump, flags);
  }
}

void dsda_InitBigArmorHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->right_align = (arg_count > 0) ? !!args[0] : false;
  local->percent = (arg_count > 1) ? !!args[1] : false;

  if (heretic) {
    armor_lump_green = R_NumPatchForSpriteIndex(HERETIC_SPR_SHLD);
    armor_lump_blue = (gamemode != shareware) ? R_NumPatchForSpriteIndex(HERETIC_SPR_SHD2) : armor_lump_green;
  }
  else if (hexen) {
    armor_lump_green = R_NumPatchForSpriteIndex(HEXEN_SPR_ARM3);
    armor_lump_blue = armor_lump_green;
  }
  else {
    armor_lump_green = R_NumPatchForSpriteIndex(SPR_ARM1);
    armor_lump_blue = R_NumPatchForSpriteIndex(SPR_ARM2);
  }

  patch_spacing = 6;
  font_height = raven ? 20 : 15;

  dsda_ArmorPatchSpacing();
  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArmorHC(void* data) {
  local = data;
}

void dsda_DrawBigArmorHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
