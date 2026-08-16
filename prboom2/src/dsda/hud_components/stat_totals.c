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

typedef enum {
  STAT_RATIO,
  STAT_PERCENT,
  STAT_COUNT,
  STAT_REMAIN,
  STAT_BOOLEAN,
  STAT_DSDA,
} stat_config_t;

static int dsda_GetStatFormat(dboolean automap_stats)
{
  int exhud_stat_config  = dsda_IntConfig(dsda_config_exhud_stats_format);
  int automap_stat_config = dsda_IntConfig(dsda_config_automap_stats_format);

  if (!automap_stats)
    return exhud_stat_config;

  if (automap_stat_config == 0) // Match Hud
    return exhud_stat_config;

  return automap_stat_config - 1; // Normalise map stats
}

int dsda_PrintStats(size_t length, char *buffer, size_t size, const char* label, const char* cm, const int th_count, const int th_total, dboolean allow_dsda, dboolean automap_stats, const char *separator)
{
    int stat_format = dsda_GetStatFormat(automap_stats);
    int ratio, percent, count, remain, boolean, dsda;
    char print_stats[32] = "";
    int has_label = (label != NULL);
    int dsda_percent = allow_dsda && (has_label || automap_stats);

    ratio   = (stat_format == STAT_RATIO);
    percent = (stat_format == STAT_PERCENT);
    count   = (stat_format == STAT_COUNT);
    remain  = (stat_format == STAT_REMAIN);
    boolean = (stat_format == STAT_BOOLEAN);
    dsda    = (stat_format == STAT_DSDA);

    if (dsda && dsda_percent)
      sprintf(print_stats, "%d/%d %d%%", th_count, th_total, !th_total ? 100 : th_count * 100 / th_total);
    else if (ratio || dsda)
      sprintf(print_stats, "%d/%d", th_count, th_total);
    else if (percent)
      sprintf(print_stats, "%d%%", !th_total ? 100 : th_count * 100 / th_total);
    else if (count)
      sprintf(print_stats, "%d", th_count);
    else if (remain)
      sprintf(print_stats, "%d", th_total - th_count);
    else if (boolean)
      sprintf(print_stats, "%s", th_count >= th_total ? "Y" : "N");
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
  size_t length;
  const char* killcolor;
  const char* itemcolor;
  const char* secretcolor;

  length = 0;
  local->stats_count = 0;

  killcolor   = (dsda_IsAllKills()    ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                        dsda_TextColor(dsda_tc_exhud_totals_value));
  itemcolor   = (dsda_IsAllItems()    ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                        dsda_TextColor(dsda_tc_exhud_totals_value));
  secretcolor = (dsda_IsAllSecrets()  ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                        dsda_TextColor(dsda_tc_exhud_totals_value));

  if (local->include_sts_label)
    length += snprintf(str + length, max_size - length, "%s%s%s", dsda_TextColor(dsda_tc_exhud_totals_sts_label), local->label_sts, local->stat_separator );

  if (local->include_kills)   local->stats_count++;
  if (local->include_items)   local->stats_count++;
  if (local->include_secrets) local->stats_count++;

  if (local->include_kills)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, local->label_k, killcolor, dsda_GetCurrentKills(), dsda_GetMaxKills(), true, false, dsda_StatSeparator());
  }

  if (local->include_items)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, local->label_i, itemcolor, dsda_GetCurrentItems(), dsda_GetMaxItems(), false, false, dsda_StatSeparator());
  }

  if (local->include_secrets)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, local->label_s, secretcolor, dsda_GetCurrentSecrets(), dsda_GetMaxSecrets(), false, false, dsda_StatSeparator());
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

  local->include_sts_label = (arg_count > 5) ? !!args[5] : false;

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
