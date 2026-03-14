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
//	DSDA Health Text HUD Component
//

#include "base.h"

#include "health_text.h"
#include "boom_bar.h"

typedef struct {
  dsda_text_t label;
  dsda_patch_component_t health_bar;
  dsda_text_t percentage;
  dboolean draw_boom_bar;
} local_component_t;

static local_component_t* local;

static const char* label;
static boom_bar_t boom_bar;
static int label_width;

static int dsda_HealthTextColor(int health)
{
  return (health <= hud_health_red)    ? dsda_tc_exhud_health_bad :
         (health <= hud_health_yellow) ? dsda_tc_exhud_health_warning :
         (health <= hud_health_green)  ? dsda_tc_exhud_health_ok :
                                         dsda_tc_exhud_health_super;
}

static int dsda_HealthBarColor(int health)
{
  return (health <= hud_health_red)    ? dsda_tc_exhud_health_bad :
         (health <= hud_health_yellow) ? dsda_tc_exhud_health_warning :
         (health <= hud_health_green)  ? dsda_tc_exhud_health_ok :
                                         dsda_tc_exhud_health_super_dark;
}

static void dsda_DrawBar(player_t* player) {
  int healthmax = hud_health_green;
  int healthcolor = dsda_TextCR(dsda_HealthBarColor(st_health));
  int supercolor = dsda_TextCR(dsda_tc_exhud_health_super);

  // draw bar after label
  local->health_bar.x = local->label.text.x + label_width;

  // draw bar
  dsda_DrawBoomBar(&boom_bar, local->health_bar, st_health, healthmax, healthcolor, supercolor, true);

  // draw value after bar
  local->percentage.text.x = local->health_bar.x + boom_bar.width;
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
  const char* cm = dsda_TextColor(dsda_HealthTextColor(st_health));

  snprintf(local->label.msg, sizeof(local->label.msg), "%s%s", cm, label);
  snprintf(local->percentage.msg, sizeof(local->percentage.msg), "%s%3d%%", cm, st_health);
}

void dsda_InitHealthTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  label = "HEL ";
  local->draw_boom_bar = (arg_count > 0) ? !!args[0] : false;

  dsda_InitTextHC(&local->label,  x_offset, y_offset, vpt);
  dsda_InitTextHC(&local->percentage, x_offset, y_offset, vpt);
  dsda_InitPatchHC(&local->health_bar, x_offset, y_offset, vpt);

  label_width = HU_FontStringWidth(&exhud_font, label);
  dsda_SetBoomBarInfo(&boom_bar);
}

void dsda_UpdateHealthTextHC(void* data) {
  local = data;

  dsda_UpdateComponentText();
  dsda_RefreshHudText(&local->label);
  dsda_RefreshHudText(&local->percentage);
}

void dsda_DrawHealthTextHC(void* data) {
  local = data;

  dsda_DrawComponent();
}
