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
//	DSDA Keys HUD Component
//

#include "base.h"

#include "keys.h"
#include "st_stuff.h"

#define PATCH_DELTA 3

typedef struct {
  dsda_text_t label;
  dsda_patch_component_t component;
  dboolean horizontal;
  dboolean boom_classic;
  dboolean boom_label;
  int spacing;
} local_component_t;

static local_component_t* local;

static int key_patch_num[NUMCARDS];

static int font_height;
static int patch_spacing_x;
static int patch_spacing_y;

static void dsda_KeyPatchSpacing(void)
{
  int i;

  patch_spacing_x = 0;
  patch_spacing_y = 0;

  if (heretic)
  {
    const char* names[] = { "ykeyicon", "gkeyicon", "bkeyicon" };

    for (i = 0; i < 3; ++i)
    {
      int lump = W_GetNumForName(names[i]);
      patch_spacing_x = MAX(patch_spacing_x, R_NumPatchWidth(lump));
      patch_spacing_y = MAX(patch_spacing_y, R_NumPatchHeight(lump));
    }
  }
  else
  {
    const char* names[] = {
      "STKEYS0", "STKEYS1", "STKEYS2",
      "STKEYS3", "STKEYS4", "STKEYS5",
      "STKEYS6", "STKEYS7", "STKEYS8"
    };

    for (i = 0; i < 9; ++i)
    {
      int lump = W_GetNumForName(names[i]);
      patch_spacing_x = MAX(patch_spacing_x, R_NumPatchWidth(lump));
      patch_spacing_y = MAX(patch_spacing_y, R_NumPatchHeight(lump));
    }
  }
}

static const char* dsda_SeparateKeyName(player_t* player, int slot)
{
  int color;
  dboolean skull;

  switch (slot)
  {
    case 0: color = 0; skull = false; break; // blue keycard
    case 1: color = 1; skull = false; break; // yellow keycard
    case 2: color = 2; skull = false; break; // red keycard
    case 3: color = 0; skull = true;  break; // blue skull
    case 4: color = 1; skull = true;  break; // yellow skull
    case 5: color = 2; skull = true;  break; // red skull
    default: return NULL;
  }

  if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
  {
    switch (ST_BlinkKey(player, color))
    {
      case KEYBLINK_CARD:
        return skull ? NULL :
               (color == 0 ? "STKEYS0" :
                color == 1 ? "STKEYS1" : "STKEYS2");

      case KEYBLINK_SKULL:
        return skull ? 
               (color == 0 ? "STKEYS3" :
                color == 1 ? "STKEYS4" : "STKEYS5")
               : NULL;

      case KEYBLINK_BOTH:
        return skull ?
               (color == 0 ? "STKEYS3" :
                color == 1 ? "STKEYS4" : "STKEYS5")
               :
               (color == 0 ? "STKEYS0" :
                color == 1 ? "STKEYS1" : "STKEYS2");

      case KEYBLINK_NONE:
      default:
        return NULL;
    }
  }

  switch (slot)
  {
    case 0: return player->cards[0] ? "STKEYS0" : NULL; // blue keycard
    case 1: return player->cards[1] ? "STKEYS1" : NULL; // yellow keycard
    case 2: return player->cards[2] ? "STKEYS2" : NULL; // red keycard
    case 3: return player->cards[3] ? "STKEYS3" : NULL; // blue skull
    case 4: return player->cards[4] ? "STKEYS4" : NULL; // yellow skull
    case 5: return player->cards[5] ? "STKEYS5" : NULL; // red skull
  }

  return NULL;
}

static const char* dsda_SeparateKeyNameHeretic(player_t* player, int slot)
{
  int color;

  switch (slot)
  {
    case 0: color = key_yellow; break;
    case 1: color = key_green;  break;
    case 2: color = key_blue;   break;
    default: return NULL;
  }

  // Crispy-style blink support
  if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
  {
    switch (ST_BlinkKey(player, color))
    {
      case KEYBLINK_CARD:
      case KEYBLINK_BOTH:
        return (color == key_yellow ? "ykeyicon" :
                color == key_green  ? "gkeyicon" :
                                      "bkeyicon");

      case KEYBLINK_NONE:
      default:
        return NULL;
    }
  }

  // Normal ownership
  switch (color)
  {
    case key_yellow: return player->cards[key_yellow] ? "ykeyicon" : NULL;
    case key_green:  return player->cards[key_green]  ? "gkeyicon" : NULL;
    case key_blue:   return player->cards[key_blue]   ? "bkeyicon" : NULL;
  }

  return NULL;
}

void dsda_DrawKeyNamePatch(int x, int y, const char* name) {
  int lump;
  int w, h;
  int shadow = raven ? SHADOW_ALWAYS_RAVEN : SHADOW_EXTRA;

  if (!name)
    return;

  lump = W_GetNumForName(name);
  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  x += (patch_spacing_x - w) / 2;
  y += (font_height - h) / 2;

  V_DrawShadowedNamePatchAdv(x, y, name, shadow, CR_DEFAULT, local->component.vpt);
}

static void dsda_DrawKeysEx(player_t* player, int x, int y, int vpt, dboolean horizontal, int spacing)
{
  int i;

  // If vertical, align keys offset by patch width
  if (!horizontal)
    x -= patch_spacing_x / 2;

  for (i = 0; i < 3; i++)
  {
    const char* keyname = ST_GetKeyName(player, i);

    if (keyname)
      dsda_DrawKeyNamePatch(x, y, keyname);

    if (horizontal)
      x += patch_spacing_x + spacing;
    else
      y += patch_spacing_y + spacing;
  }
}

static void dsda_DrawSeparateKeys(player_t* player, int x, int y, dboolean compact)
{
  int i;

  for (i = 0; i < 6; ++i)
  {
    const char* keyname = heretic ? dsda_SeparateKeyNameHeretic(player, i) : dsda_SeparateKeyName(player, i);

    if (keyname)
      dsda_DrawKeyNamePatch(x, y, keyname);

    if (!keyname && compact)
      continue;

    if (local->horizontal)
      x += patch_spacing_x + local->spacing;
    else
      y += patch_spacing_y + local->spacing;
  }
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  snprintf(
    str,
    max_size,
    "%sKEY ",
    dsda_TextColor(dsda_tc_exhud_keys_label)
  );
}

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;

  player = &players[displayplayer];

  x = local->component.x;
  y = local->component.y;

  if (hexen) {
    int i;

    for (i = 0; i < NUMCARDS; ++i)
      if (player->cards[i]) {
        V_DrawShadowedNumPatch(x, y, key_patch_num[i], CR_DEFAULT, local->component.vpt);
        x += R_NumPatchWidth(key_patch_num[i]) + 4;
      }

    return;
  }

  if (local->boom_label)
  {
    dsda_DrawBasicText(&local->label);
    x += HU_FontStringWidth(&exhud_font, "KEY "); // draw icons after label
  }

  if (local->boom_classic)
  {
    dsda_DrawSeparateKeys(player, x, y, true);
    return;
  }

  dsda_DrawKeysEx(player, x, y, local->component.vpt, local->horizontal, local->spacing);
}

void dsda_InitKeysHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->horizontal     = (arg_count > 0) ? !!args[0] : false;
  local->spacing        = (arg_count > 1) ?   args[1] : PATCH_DELTA;
  local->boom_classic   = (arg_count > 2) ? !!args[2] : false;

  local->boom_label     = (local->boom_classic && local->horizontal && !hexen);

  if (hexen) {
    int i;

    for (i = 0; i < NUMCARDS; ++i)
      key_patch_num[i] = R_NumPatchForSpriteIndex(HEXEN_SPR_KEY1 + i);
  }
  else
  {
    font_height = 8;
    dsda_KeyPatchSpacing();
  }

  dsda_InitPatchHC(&local->component, x_offset, y_offset, vpt);
  dsda_InitTextHC(&local->label, x_offset, y_offset, vpt);
}

void dsda_UpdateKeysHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->label.msg, sizeof(local->label.msg));
  dsda_RefreshHudText(&local->label);
}

void dsda_DrawKeysHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
