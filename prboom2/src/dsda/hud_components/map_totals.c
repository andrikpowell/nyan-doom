//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Map Totals HUD Component
//

#include "dsda/skill_info.h"

#include "base.h"
#include "stat_totals.h"

#include "map_totals.h"

typedef struct {
  dsda_text_t label;
  dsda_text_t component;
  dboolean include_kills, include_items, include_secrets;
  int stat_format;
} local_component_t;

static local_component_t* local;

static void dsda_UpdateLabelComponentText(char* str, size_t max_size) {
  size_t length = 0;

  if (local->include_kills) {
      length += snprintf(
        str,
        max_size,
        "%sK\n",
        dsda_TextColor(dsda_tc_map_totals_label)
      );
  }

  if (local->include_items) {
      length += snprintf(
        str + length,
        max_size - length,
        "%sI\n",
        dsda_TextColor(dsda_tc_map_totals_label)
      );
  }

  if (local->include_secrets) {
      snprintf(
        str + length,
        max_size - length,
        "%sS",
        dsda_TextColor(dsda_tc_map_totals_label)
      );
  }
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int i;
  size_t length;
  int fullkillcount, fullitemcount, fullsecretcount;
  const char* killcolor;
  const char* itemcolor;
  const char* secretcolor;
  int kill_percent_count;
  int max_kill_requirement;

  length = 0;
  fullkillcount = 0;
  fullitemcount = 0;
  fullsecretcount = 0;
  kill_percent_count = 0;
  max_kill_requirement = dsda_MaxKillRequirement();

  for (i = 0; i < g_maxplayers; ++i) {
    if (playeringame[i]) {
      fullkillcount += players[i].killcount - players[i].maxkilldiscount;
      fullitemcount += players[i].itemcount;
      fullsecretcount += players[i].secretcount;
      kill_percent_count += players[i].killcount;
    }
  }

  if (skill_info.respawn_time) {
    fullkillcount = kill_percent_count;
    max_kill_requirement = totalkills;
  }

  killcolor = (fullkillcount >= max_kill_requirement ? dsda_TextColor(dsda_tc_map_totals_max) :
                                                       dsda_TextColor(dsda_tc_map_totals_value));
  secretcolor = (fullsecretcount >= totalsecret ? dsda_TextColor(dsda_tc_map_totals_max) :
                                                  dsda_TextColor(dsda_tc_map_totals_value));
  itemcolor = (fullitemcount >= totalitems ? dsda_TextColor(dsda_tc_map_totals_max) :
                                             dsda_TextColor(dsda_tc_map_totals_value));

  if (local->include_kills)
    length += dsda_PrintStats(length, str, max_size, local->stat_format, NULL, killcolor, fullkillcount, max_kill_requirement, true);

  if (local->include_items)
    length += dsda_PrintStats(length, str + length, max_size - length, local->stat_format, NULL, itemcolor, fullitemcount, totalitems, true);

  if (local->include_secrets)
    dsda_PrintStats(length, str + length, max_size - length, local->stat_format, NULL, secretcolor, fullsecretcount, totalsecret, false);
}

void dsda_InitMapTotalsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->include_kills = args[0];
  local->include_items = args[1];
  local->include_secrets = args[2];

  local->stat_format = args[3];

  if (!local->include_kills && !local->include_items && !local->include_secrets)
    local->include_kills = local->include_items = local->include_secrets = true;

  dsda_InitBlockyHC(&local->label, x_offset, y_offset, vpt);
  dsda_InitBlockyHC(&local->component, x_offset + 12, y_offset, vpt);
}

void dsda_UpdateMapTotalsHC(void* data) {
  local = data;

  dsda_UpdateLabelComponentText(local->label.msg, sizeof(local->label.msg));
  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->label);
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapTotalsHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->label);
  dsda_DrawBasicText(&local->component);
}
