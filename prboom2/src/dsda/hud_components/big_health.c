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
//	DSDA Big Health HUD Component
//

#include "base.h"

#include "big_health.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
  dboolean right_anchor;
  dboolean percent;
} local_component_t;

static local_component_t* local;

static int health_lump;
static int strength_lump;

static int font_height;
static int patch_spacing;
static int patch_spacing_x;
static int patch_spacing_y;

static void dsda_HealthPatchSpacing(void)
{
  int lumps[] = {
    health_lump,
    strength_lump,
  };

  patch_spacing_x = 0;
  patch_spacing_y = 0;

  for (int i = 0; i < sizeof(lumps) / sizeof(lumps[0]); ++i)
  {
    patch_spacing_x = MAX(patch_spacing_x, R_NumPatchWidth(lumps[i]));
    patch_spacing_y = MAX(patch_spacing_y, R_NumPatchHeight(lumps[i]));
  }
}

static int dsda_GetNumberWidth(void)
{
  return dsda_GetBigNumberWidth(3, 999, local->right_align, local->percent, false);
}

static void dsda_DrawBigHealthIcon(int x, int y, int lump, int flags) {
  int w, h;
  int shadow = raven ? SHADOW_ALWAYS_RAVEN : SHADOW_EXTRA;

  if (!lump)
    return;

  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  // center horizontally
  x += (patch_spacing_x - w) / 2;
  
  // center vertically
  y += (font_height - patch_spacing_y) / 2;
  y += (patch_spacing_y - h) / 2;

  V_DrawShadowedNumPatchAdv(x, y, lump, shadow, CR_DEFAULT, flags);
}

static void dsda_DrawComponent(void) {
  player_t* player;
  int health;
  int lump;
  int x, y;
  int cm;
  int flags, numflags;
  int lump_width;
  int text_width;
  int total_width;

  player = &players[displayplayer];
  flags = numflags = local->component.vpt;
  x = local->component.x;
  y = local->component.y;

  // Animated health
  health = st_health;
  lump = player->powers[pw_strength] ? strength_lump : health_lump;
  cm = raven ? CR_DEFAULT :
       health <= hud_health_red ? dsda_TextCR(dsda_tc_stbar_health_bad) :
       health <= hud_health_yellow ? dsda_TextCR(dsda_tc_stbar_health_warning) :
       health <= hud_health_green ? dsda_TextCR(dsda_tc_stbar_health_ok) :
       dsda_TextCR(dsda_tc_stbar_health_super);

  lump_width = patch_spacing_x;
  text_width = dsda_GetNumberWidth();
  total_width = lump_width + patch_spacing + text_width;

  if (local->right_anchor)
    x -= total_width;

  if (!local->right_align)
  {
    dsda_DrawBigHealthIcon(x, y, lump, flags);
    x += patch_spacing + lump_width;
  }

  // Numbers need offsets (so 1 doesn't have a big space)
  numflags &= ~VPT_NOOFFSET;

  dsda_DrawBigNumber(x, y, 0, cm, numflags, 3, health, local->right_align, false, local->percent, false);

  if (local->right_align)
  {
    x += patch_spacing + text_width;
    dsda_DrawBigHealthIcon(x, y, lump, flags);
  }
}

void dsda_InitBigHealthHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->right_align = (arg_count > 0) ? !!args[0] : false;
  local->right_anchor = (arg_count > 1) ? !!args[1] : false;
  local->percent = (arg_count > 2) ? !!args[2] : false;

  if (heretic) {
    health_lump = R_NumPatchForSpriteIndex(HERETIC_SPR_PTN1);
    strength_lump = health_lump;
  }
  else if (hexen) {
    health_lump = R_NumPatchForSpriteIndex(HEXEN_SPR_PTN1);
    strength_lump = health_lump;
  }
  else {
    health_lump = R_NumPatchForSpriteIndex(SPR_MEDI);
    strength_lump = (gamemode != shareware) ? R_NumPatchForSpriteIndex(SPR_PSTR) : health_lump; // berserk doesn't exist in shareware
  }

  patch_spacing = 6;
  font_height = raven ? 20 : 15;

  dsda_HealthPatchSpacing();
  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigHealthHC(void* data) {
  local = data;
}

void dsda_DrawBigHealthHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
