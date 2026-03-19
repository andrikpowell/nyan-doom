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
//	DSDA Big Ammo HUD Component
//

#include "base.h"

#include "big_ammo.h"

typedef struct {
  dsda_patch_component_t component;
  dboolean right_align;
} local_component_t;

static local_component_t* local;

static int font_height;
static int patch_spacing;
static int patch_spacing_x;
static int patch_spacing_y;

static int dsda_GetNumberWidth(void)
{
  return dsda_GetBigNumberWidth(3, 999, local->right_align, false);
}

static int dsda_GetAmmoImage(ammotype_t ammo_type) {
  if (heretic)
  {
    if (ammo_type == am_goldwand) return HERETIC_SPR_AMG1;
    else if (ammo_type == am_crossbow) return HERETIC_SPR_AMC2;
    else if (ammo_type == am_blaster) return HERETIC_SPR_AMB1;
    else if (ammo_type == am_skullrod) return HERETIC_SPR_AMS1;
    else if (ammo_type == am_phoenixrod) return HERETIC_SPR_AMP1;
    else if (ammo_type == am_mace) return HERETIC_SPR_AMM1;
    else return 0;
  }
  else // Doom
  {
    if (ammo_type == am_clip) return SPR_CLIP;
    else if (ammo_type == am_shell) return SPR_SHEL;
    else if (ammo_type == am_cell) return SPR_CELL;
    else if (ammo_type == am_misl) return SPR_ROCK;
    else return 0;
  }

  return 0;
}

static void dsda_DoomAmmoPatchSpacing(void)
{
  int lumps[] = {
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_clip)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_shell)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_cell)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_misl))
  };

  patch_spacing_x = 0;
  patch_spacing_y = 0;

  for (int i = 0; i < sizeof(lumps) / sizeof(lumps[0]); ++i)
  {
    patch_spacing_x = MAX(patch_spacing_x, R_NumPatchWidth(lumps[i]));
    patch_spacing_y = MAX(patch_spacing_y, R_NumPatchHeight(lumps[i]));
  }
}

static void dsda_HereticAmmoPatchSpacing(void)
{
  int lumps[] = {
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_goldwand)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_crossbow)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_blaster)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_skullrod)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_phoenixrod)),
    R_NumPatchForSpriteIndex(dsda_GetAmmoImage(am_mace))
  };

  patch_spacing_x = 0;
  patch_spacing_y = 0;

  for (int i = 0; i < sizeof(lumps) / sizeof(lumps[0]); ++i)
  {
    patch_spacing_x = MAX(patch_spacing_x, R_NumPatchWidth(lumps[i]));
    patch_spacing_y = MAX(patch_spacing_y, R_NumPatchHeight(lumps[i]));
  }
}

static void dsda_DrawBigAmmoIcon(int x, int y, int lump, int flags) {
  int w, h;

  if (!lump)
    return;

  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  // center horizontally
  x += (patch_spacing_x - w) / 2;
  
  // bottom-align vertically
  y += (font_height - patch_spacing_y) / 2;
  y += (patch_spacing_y - h) / 2;

  if (raven)
    V_DrawShadowedNumPatch(x, y, lump, CR_DEFAULT, flags);
  else
    V_DrawMenuNumPatch(x, y, lump, CR_DEFAULT, flags);
}

static void dsda_DrawComponent(void) {
  player_t* player;
  ammotype_t ammo_type;
  int ammo;
  int lump;
  int x, y;
  int flags, numflags;
  int cm;

  if (hexen) return;

  player = &players[displayplayer];
  flags = numflags = local->component.vpt;
  x = local->component.x;
  y = local->component.y;

  // Numbers need offsets (so 1 doesn't have a big space)
  numflags &= ~VPT_NOOFFSET;

  ammo_type = weaponinfo[player->readyweapon].ammo;

  if (ammo_type == am_noammo || !player->maxammo[ammo_type])
    return;

  ammo = player->ammo[ammo_type];
  lump = R_NumPatchForSpriteIndex(dsda_GetAmmoImage(ammo_type));
  cm = heretic ? CR_DEFAULT : dsda_TextCR(dsda_AmmoColorBig(player));

  if (!local->right_align)
  {
    dsda_DrawBigAmmoIcon(x, y, lump, flags);
    x += patch_spacing + patch_spacing_x;
  }

  dsda_DrawBigNumber(x, y, 0, cm, numflags, 3, ammo, local->right_align, false);

  if (local->right_align)
  {
    x += patch_spacing + dsda_GetNumberWidth();
    dsda_DrawBigAmmoIcon(x, y, lump, flags);
  }
}

void dsda_InitBigAmmoHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (hexen) return;

  local->right_align = (arg_count > 0) ? !!args[0] : false;

  // Raven text needs smaller spacing
  if (heretic)  dsda_HereticAmmoPatchSpacing();
  else          dsda_DoomAmmoPatchSpacing();

  patch_spacing = 6;
  font_height = raven ? 20 : 15;

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigAmmoHC(void* data) {
  local = data;

  if (hexen) return;
}

void dsda_DrawBigAmmoHC(void* data) {
  local = data;

  if (hexen) return;

  dsda_DrawComponent();
}
