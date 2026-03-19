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
  int stat_format;
  dboolean include_sts_label;
  const char* label_sts;
  const char* label_k;
  const char* label_i;
  const char* label_s;
  const char* stat_separator;
  int stats_count;
} local_component_t;

static local_component_t* local;

int dsda_PrintDMStat(char *buffer, size_t size, const char *cm, int result, int others, const char* separator)
{
  return snprintf(
    buffer, size,
    "%s%i/%i%s",
    cm, result, others,
    separator ? separator : ""
  );
}

static void dsda_DMStats(char* str, size_t max_size) {
  int i, p;
  size_t length;
  int player_count = 0;

  length = 0;

  if (local->include_sts_label)
    length += snprintf(str + length, max_size - length, "%s%s%s", dsda_TextColor(dsda_tc_exhud_totals_sts_label), local->label_sts, local->stat_separator );

  for (i = 0; i < g_maxplayers; ++i)
    if (playeringame[i])
      player_count++;

  for (i = 0; i < g_maxplayers; ++i) {
      int result = 0, others = 0;
      const char *color;

      if (!playeringame[i])
          continue;

      for (p = 0; p < g_maxplayers; ++p)
      {
          if (!playeringame[p])
              continue;

          if (i != p)
          {
              result += players[i].frags[p];
              others -= players[p].frags[i];
          }
          else
          {
              result -= players[i].frags[p];
          }
      }

      color = (i == displayplayer) ? dsda_TextColor(dsda_tc_exhud_totals_max)
                                   : dsda_TextColor(dsda_tc_exhud_totals_value);

      player_count--;

      length += dsda_PrintDMStat(
        str + length, max_size - length,
        color, result, others,
        player_count > 0 ? local->stat_separator : ""
      );

      if (length >= max_size)
        break;
  }
}

int dsda_PrintStats(size_t length, char *buffer, size_t size, int format, const char* label, const char* cm, const int th_count, const int th_total, dboolean allow_dsda, const char *separator)
{
    int stat_config = dsda_IntConfig(dsda_config_stats_format);
    int ratio, percent, count, remain, dsda;
    char print_stats[32] = "";
    int has_label = (label != NULL);
    int first   = (length == 0);
    int nyanhud = (stat_config == 0);

    // Use NYANHUD format
    if (nyanhud)
      stat_config = format + 1;

    ratio   = (stat_config == 1);
    percent = (stat_config == 2);
    count   = (stat_config == 3);
    remain  = (stat_config == 4);
    dsda    = (stat_config == 5);

    if (dsda && first && (has_label || nyanhud))
      sprintf(print_stats, "%d/%d %d%%", th_count, th_total, !th_total ? 100 : th_count * 100 / th_total);
    else if (ratio)
      sprintf(print_stats, "%d/%d", th_count, th_total);
    else if (percent)
      sprintf(print_stats, "%d%%", !th_total ? 100 : th_count * 100 / th_total);
    else if (count)
      sprintf(print_stats, "%d", th_count);
    else if (remain)
      sprintf(print_stats, "%d", th_total - th_count);
    else // fallback
      sprintf(print_stats, "%d/%d", th_count, th_total);

    if (has_label)
    {
      return snprintf( buffer, size,
                        "%s%s%s%s%s",
                        dsda_TextColor(dsda_tc_exhud_totals_label),
                        label, cm, print_stats,
                        separator ? separator : "");
    }
    else
    {
      return snprintf( buffer, size,
                        "%s%s%s",
                        cm, print_stats,
                        separator ? separator : "");
    }
}

static const char* dsda_StatSeparator() {
  return local->stats_count > 0 ? local->stat_separator : "";
}

static void dsda_LevelStats(char* str, size_t max_size) {
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
  local->stats_count = 0;

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

  if (local->include_sts_label)
    length += snprintf(str + length, max_size - length, "%s%s%s", dsda_TextColor(dsda_tc_exhud_totals_sts_label), local->label_sts, local->stat_separator );

  if (local->include_kills)   local->stats_count++;
  if (local->include_items)   local->stats_count++;
  if (local->include_secrets) local->stats_count++;

  if (local->include_kills)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, local->stat_format, local->label_k, killcolor, fullkillcount, max_kill_requirement, true, dsda_StatSeparator());
  }

  if (local->include_items)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, local->stat_format, local->label_i, itemcolor, fullitemcount, totalitems, false, dsda_StatSeparator());
  }

  if (local->include_secrets)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, local->stat_format, local->label_s, secretcolor, fullsecretcount, totalsecret, false, dsda_StatSeparator());
  }
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  if (deathmatch)
  {
    dsda_DMStats(str, max_size);
  }
  else
  {
    dsda_LevelStats(str, max_size);
  }
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

  local->stat_format = (arg_count > 5) ? args[5] : false;
  local->include_sts_label = (arg_count > 6) ? !!args[6] : false;

  local->label_sts = "STS";

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
