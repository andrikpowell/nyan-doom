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
  dboolean classic;
  dboolean classic_label;
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

static const char* dsda_Key1Name(player_t* player) {
  // [crispy] blinking key or skull in the status bar
  if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
  {
      switch (ST_BlinkKey(player, !heretic ? 0 : key_yellow))
      {
          case KEYBLINK_NONE:
            return NULL;
            break;

          case KEYBLINK_CARD:
            return !heretic ? "STKEYS0" : "ykeyicon";
            break;

          case KEYBLINK_SKULL:
            return "STKEYS3";
            break;

          case KEYBLINK_BOTH:
            return "STKEYS6";
            break;

          default:
            break;
      }
  }

  if (heretic) {
    if (player->cards[key_yellow])
      return "ykeyicon";
  }
  else {
    if (player->cards[0] && player->cards[3])
      return "STKEYS6";
    else if (player->cards[0])
      return "STKEYS0";
    else if (player->cards[3])
      return "STKEYS3";
  }

  return NULL;
}

static const char* dsda_Key2Name(player_t* player) {
  // [crispy] blinking key or skull in the status bar
  if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
  {
      switch (ST_BlinkKey(player, !heretic ? 1 : key_green))
      {
          case KEYBLINK_NONE:
            return NULL;
            break;

          case KEYBLINK_CARD:
            return !heretic ? "STKEYS1" : "gkeyicon";
            break;

          case KEYBLINK_SKULL:
            return "STKEYS4";
            break;

          case KEYBLINK_BOTH:
            return "STKEYS7";
            break;

          default:
            break;
      }
  }

  if (heretic) {
    if (player->cards[key_green])
      return "gkeyicon";
  }
  else {
    if (player->cards[1] && player->cards[4])
      return "STKEYS7";
    else if (player->cards[1])
      return "STKEYS1";
    else if (player->cards[4])
      return "STKEYS4";
  }

  return NULL;
}

static const char* dsda_Key3Name(player_t* player) {
  // [crispy] blinking key or skull in the status bar
  if (player->keyblinktics && sts_blink_keys && !dsda_StrictMode())
  {
      switch (ST_BlinkKey(player, !heretic ? 2 : key_blue))
      {
          case KEYBLINK_NONE:
            return NULL;
            break;

          case KEYBLINK_CARD:
            return !heretic ? "STKEYS2" : "bkeyicon";
            break;

          case KEYBLINK_SKULL:
            return "STKEYS5";
            break;

          case KEYBLINK_BOTH:
            return "STKEYS8";
            break;

          default:
            break;
      }
  }

  if (heretic) {
    if (player->cards[key_blue])
      return "bkeyicon";
  }
  else {
    if (player->cards[2] && player->cards[5])
      return "STKEYS8";
    else if (player->cards[2])
      return "STKEYS2";
    else if (player->cards[5])
      return "STKEYS5";
  }

  return NULL;
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

void dsda_DrawKeyNamePatch(int x, int y, const char* name) {
  int lump;
  int w, h;

  if (!name)
    return;

  lump = W_GetNumForName(name);
  w = R_NumPatchWidth(lump);
  h = R_NumPatchHeight(lump);

  x += (patch_spacing_x - w) / 2;
  y += (font_height - h) / 2;

  if (raven)
    V_DrawShadowedNamePatch(x, y, name, CR_DEFAULT, local->component.vpt);
  else
    V_DrawMenuNamePatch(x, y, name, CR_DEFAULT, local->component.vpt);
}

void drawKey(player_t* player, int* x, int* y, const char* (*key)(player_t*)) {
  const char* name;

  name = key(player);

  if (name)
    dsda_DrawKeyNamePatch(*x, *y, name);

  if (local->horizontal)
    *x += patch_spacing_x + local->spacing;
  else
    *y += patch_spacing_y + local->spacing;
}

static void dsda_DrawSeparateKeys(player_t* player, int x, int y, dboolean compact)
{
  int i;

  for (i = 0; i < 6; ++i)
  {
    const char* name = dsda_SeparateKeyName(player, i);

    if (name)
      dsda_DrawKeyNamePatch(x, y, name);

    if (!name && compact)
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

  if (local->classic_label)
  {
    dsda_DrawBasicText(&local->label);
    x += HU_FontStringWidth(&exhud_font, "KEY "); // draw icons after label
  }

  if (local->classic)
  {
    dsda_DrawSeparateKeys(player, x, y, true);
    return;
  }

  drawKey(player, &x, &y, dsda_Key1Name);
  drawKey(player, &x, &y, dsda_Key2Name);
  drawKey(player, &x, &y, dsda_Key3Name);
}

void dsda_InitKeysHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->horizontal = (arg_count > 0) ? !!args[0] : false;
  local->spacing    = (arg_count > 1) ?   args[1] : PATCH_DELTA;
  local->classic    = (arg_count > 2) ? !!args[2] : false;

  local->classic_label     = (local->classic && local->horizontal && !hexen);

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

  if (local->classic_label)
    dsda_InitTextHC(&local->label, x_offset, y_offset, vpt);
}

void dsda_UpdateKeysHC(void* data) {
  local = data;

  if (local->classic_label)
  {
    dsda_UpdateComponentText(local->label.msg, sizeof(local->label.msg));
    dsda_RefreshHudText(&local->label);
  }
}

void dsda_DrawKeysHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
