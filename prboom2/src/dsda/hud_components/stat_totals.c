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
//	DSDA Stat Totals HUD Component
//

#include "dsda/skill_info.h"

#include "base.h"

#include "stat_totals.h"

typedef struct {
  dsda_text_t component;
  dboolean include_kills, include_items, include_secrets;
  dboolean hide_totals;
  const char* label_k;
  const char* label_i;
  const char* label_s;
  const char* stat_separator;
} local_component_t;

static local_component_t* local;

int dsda_PrintStats(size_t length, char *buffer, size_t size, const char* label, const char* cm, const int th_count, const int th_total, dboolean separator)
{
    int stat_config = dsda_IntConfig(dsda_config_stats_format);
    char print_stats[32] = "";
    int has_label = (label != NULL);

    int ratio   = (stat_config==0);
    int percent = (stat_config==1);
    int count   = (stat_config==2);
    int remain  = (stat_config==3);
    int dsda    = (stat_config==4);
    int first   = (length == 0);

    if (dsda && first && has_label)
      sprintf(print_stats, "%d/%d %d%%", th_count, th_total, !th_total ? 100 : th_count * 100 / th_total);
    else if (percent)
      sprintf(print_stats, "%d%%", !th_total ? 100 : th_count * 100 / th_total);
    else if (count)
      sprintf(print_stats, "%d", th_count);
    else if (remain)
      sprintf(print_stats, "%d", th_total - th_count);
    else
      sprintf(print_stats, "%d/%d", th_count, th_total);

    if (has_label)
    {
      return snprintf( buffer, size,
                        "%s%s%s%s%s",
                        dsda_TextColor(dsda_tc_exhud_totals_label),
                        label, cm, print_stats,
                        separator ? local->stat_separator : "" );
    }
    else
    {
      return snprintf( buffer, size,
                        "%s%s%s",
                        cm, print_stats,
                        separator ? "\n" : "" );
    }
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int i;
  size_t length;
  int show_totals;
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

  killcolor = (fullkillcount >= max_kill_requirement ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                                       dsda_TextColor(dsda_tc_exhud_totals_value));
  secretcolor = (fullsecretcount >= totalsecret ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                                  dsda_TextColor(dsda_tc_exhud_totals_value));
  itemcolor = (fullitemcount >= totalitems ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                             dsda_TextColor(dsda_tc_exhud_totals_value));

  if (local->include_kills)
    length += dsda_PrintStats(length, str, max_size, local->label_k, killcolor, fullkillcount, max_kill_requirement, true);

  if (local->include_items)
    length += dsda_PrintStats(length, str + length, max_size - length, local->label_i, itemcolor, fullitemcount, totalitems, true);

  if (local->include_secrets)
    dsda_PrintStats(length, str + length, max_size - length, local->label_s, secretcolor, fullsecretcount, totalsecret, false);
}

void dsda_InitStatTotalsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->include_kills = args[0];
  local->include_items = args[1];
  local->include_secrets = args[2];

  // vertical orientation
  local->stat_separator = args[3] ? "\n" : " ";

  if (arg_count < 5 || args[4]) {
    local->label_k = "K ";
    local->label_i = "I ";
    local->label_s = "S ";
  }
  else {
    local->label_k = "";
    local->label_i = "";
    local->label_s = "";
  }

  local->hide_totals = args[5];

  if (!local->include_kills && !local->include_items && !local->include_secrets) {
    local->include_kills = local->include_items = local->include_secrets = true;
  }

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatTotalsHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawStatTotalsHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
