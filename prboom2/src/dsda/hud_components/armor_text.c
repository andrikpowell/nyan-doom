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
//	DSDA Armor Text HUD Component
//

#include "base.h"

#include "armor_text.h"
#include "boom_bar.h"

typedef struct {
  dsda_text_t label;
  dsda_patch_component_t armor_bar;
  dsda_text_t percentage;
  dboolean draw_boom_bar;
} local_component_t;

static local_component_t* local;

static const char* label;
static boom_bar_t boom_bar;
static int label_width;

static int dsda_ArmorTextColor(int armor)
{
  player_t* player = &players[displayplayer];

  return (armor <= 0)             ? dsda_tc_exhud_armor_zero :
         (player->armortype == 1) ? dsda_tc_exhud_armor_one :
                                    dsda_tc_exhud_armor_two;
}

static void dsda_DrawBar(player_t* player) {
  extern int idfa_armor;
  int armor       = st_armor;
  int armor_max   = hexen ? (pclass[player->pclass].armor_max / (5 * FRACUNIT)) : idfa_armor / 2;
  int color       = hexen ? dsda_TextCR(dsda_tc_stbar_armor_zero) : dsda_TextCR(dsda_ArmorTextColor(armor));
  int supercolor  = dsda_TextCR(dsda_tc_exhud_armor_two);

  // draw bar after label
  local->armor_bar.x = local->label.text.x + label_width;

  // draw bar
  dsda_DrawBoomBar(&boom_bar, local->armor_bar, armor, armor_max, color, supercolor, false);

  // draw value after bar
  local->percentage.text.x = local->armor_bar.x + boom_bar.width;
}

static void dsda_DrawComponent(void)
{
  player_t* player;

  player = &players[displayplayer];

  dsda_DrawBasicText(&local->label);

  if (local->draw_boom_bar)
    dsda_DrawBar(player);
  else
    // draw text after label (instead of bar)
    local->percentage.text.x = local->label.text.x + label_width;

  dsda_DrawBasicText(&local->percentage);
}

static void dsda_UpdateComponentText(void) {
  int armor = st_armor;

  if (hexen)
  {
    const char* cm  = dsda_TextColor(dsda_tc_stbar_armor_zero);
    snprintf(local->label.msg, sizeof(local->label.msg), "%s%s", cm, label);
    snprintf(local->percentage.msg, sizeof(local->percentage.msg), "%s%2d", cm, armor);
  }
  else
  {
    const char* cm  = dsda_TextColor(dsda_ArmorTextColor(armor));
    snprintf(local->label.msg, sizeof(local->label.msg), "%s%s", cm, label);
    snprintf(local->percentage.msg, sizeof(local->percentage.msg), "%s%3d%%", cm, armor);
  }
}

void dsda_InitArmorTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  const char* hexen_label;

  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->draw_boom_bar = (arg_count > 0) ? !!args[0] : false;
  hexen_label = local->draw_boom_bar ? "A.C " : "A.C. "; // we want the bars to line up :/
  label = hexen ? hexen_label : "ARM ";

  dsda_InitTextHC(&local->label,  x_offset, y_offset, vpt);
  dsda_InitTextHC(&local->percentage, x_offset, y_offset, vpt);
  dsda_InitPatchHC(&local->armor_bar, x_offset, y_offset, vpt);

  label_width = HU_FontStringWidth(&exhud_font, label);
  dsda_SetBoomBarInfo(&boom_bar);
}

void dsda_UpdateArmorTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText();
  dsda_RefreshHudText(&local->label);
  dsda_RefreshHudText(&local->percentage);
}

void dsda_DrawArmorTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
