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
  dsda_text_t dm_stats;
  dboolean include_kills, include_items, include_secrets;
  dsda_patch_component_t icons;
  int stats_count;
} local_component_t;

static local_component_t* local;

static void dsda_DMStats(char* str, size_t max_size) {
  int i, p;
  size_t length;

  length = 0;

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

        length += dsda_PrintDMStat(
          str + length,
          max_size - length,
          color,
          result,
          others,
          " "
        );

      if (length >= max_size)
        break;
  }
}

static void dsda_DrawMapIcon(int x, int y, const char* lumpname, int color)
{
  int flags = local->icons.vpt;
  dboolean from_pwad;

  if (!lumpname || !W_LumpNameExists(lumpname))
    return;

  from_pwad = W_PWADLumpNameExists2(lumpname);

  if (!from_pwad)
    flags |= VPT_COLOR;

  V_DrawMenuNamePatch(x, y, lumpname, color, flags);
}

static void dsda_DrawMapTotalsIcons(void)
{
  int x = local->icons.x;
  int y = local->icons.y;
  int y_spacing = raven ? 10 : 8;

  if (raven)
    y += 3;

  if (local->include_kills)
  {
    int color = raven ? dsda_tc_map_raven_icon_kills : dsda_tc_map_icon_kills;
    const char* kill_icon_lump =  hexen   ? "AMKILLS3" :
                                  heretic ? "AMKILLS2" :
                                            "AMKILLS";
    dsda_DrawMapIcon(x, y, kill_icon_lump, dsda_TextCR(color));
    y += y_spacing;
  }

  if (local->include_items)
  {
    int color = raven ? dsda_tc_map_raven_icon_items : dsda_tc_map_icon_items;
    const char* item_icon_lump =  hexen   ? "AMITEM3" :
                                  heretic ? "AMITEM2" :
                                            "AMITEM";
    dsda_DrawMapIcon(x, y, item_icon_lump, dsda_TextCR(color));
    y += y_spacing;
  }

  if (local->include_secrets)
  {
    int color = raven ? dsda_tc_map_raven_icon_secrets : dsda_tc_map_icon_secrets;
    const char* secret_icon_lump =  hexen   ? "AMSECR3" :
                                    heretic ? "AMSECR2" :
                                              "AMSECR";
    dsda_DrawMapIcon(x, y, secret_icon_lump, dsda_TextCR(color));
  }
}

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

static const char* dsda_StatSeparator() {
  return local->stats_count > 0 ? "\n" : "";
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  size_t length;
  const char* killcolor;
  const char* itemcolor;
  const char* secretcolor;

  length = 0;
  local->stats_count = 0;

  killcolor   = (dsda_IsAllKills()    ? dsda_TextColor(dsda_tc_map_totals_max) :
                                        dsda_TextColor(dsda_tc_map_totals_value));
  itemcolor   = (dsda_IsAllItems()    ? dsda_TextColor(dsda_tc_map_totals_max) :
                                        dsda_TextColor(dsda_tc_map_totals_value));
  secretcolor = (dsda_IsAllSecrets()  ? dsda_TextColor(dsda_tc_map_totals_max) :
                                        dsda_TextColor(dsda_tc_map_totals_value));


  if (local->include_kills)   local->stats_count++;
  if (local->include_items)   local->stats_count++;
  if (local->include_secrets) local->stats_count++;

  if (local->include_kills)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, NULL, killcolor, dsda_GetCurrentKills(), dsda_GetMaxKills(), true, true, dsda_StatSeparator());
  }

  if (local->include_items)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, NULL, itemcolor, dsda_GetCurrentItems(), dsda_GetMaxItems(), false, true, dsda_StatSeparator());
  }

  if (local->include_secrets)
  {
    local->stats_count--;
    length += dsda_PrintStats(length, str + length, max_size - length, NULL, secretcolor, dsda_GetCurrentSecrets(), dsda_GetMaxSecrets(), false, true, dsda_StatSeparator());
  }
}

void dsda_InitMapTotalsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->include_kills = args[0];
  local->include_items = args[1];
  local->include_secrets = args[2];

  if (!local->include_kills && !local->include_items && !local->include_secrets)
    local->include_kills = local->include_items = local->include_secrets = true;

  dsda_InitBlockyHC(&local->dm_stats, x_offset, y_offset, vpt);
  dsda_InitBlockyHC(&local->label, x_offset, y_offset, vpt);
  dsda_InitBlockyHC(&local->component, x_offset + 12, y_offset, vpt);
  dsda_InitPatchHC(&local->icons, x_offset, y_offset, vpt);
}

void dsda_UpdateMapTotalsHC(void* data) {
  local = data;

  if (deathmatch)
  {
    dsda_DMStats(local->dm_stats.msg, sizeof(local->dm_stats.msg));
    dsda_RefreshHudText(&local->dm_stats);
  }
  else 
  {
    dsda_UpdateLabelComponentText(local->label.msg, sizeof(local->label.msg));
    dsda_RefreshHudText(&local->label);
    dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
    dsda_RefreshHudText(&local->component);
  }
}

void dsda_DrawMapTotalsHC(void* data) {
  local = data;

  if (deathmatch)
  {
    dsda_DrawBasicShadowedText(&local->dm_stats);
  }
  else
  {
    if (dsda_IntConfig(dsda_config_map_stat_icons))
      dsda_DrawMapTotalsIcons();
    else
      dsda_DrawBasicShadowedText(&local->label);
    dsda_DrawBasicShadowedText(&local->component);
  }
}
