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
//	DSDA Status Timers HUD Component
//

#include "base.h"

#include "status_timers.h"
#include "powerups.h"

typedef struct {
    dsda_text_t component;
    dboolean text_align_right;
} local_component_t;

static int *powerup_index;

static local_component_t* local;

static void dsda_UpdateComponentText(char *str, size_t max_size)
{
  player_t *p = &players[displayplayer];

  int *idx = powerup_index;
  int n = 0;

  size_t length = 0;
  dboolean powerup_active = false;
  int line_len;

  if (max_size > 0)
    str[0] = '\0';

  dsda_SortPowerups(p, idx, &n, POWERUP_STATUS_TEXT);

  for (int i = 0; i < n; i++)
  {
    const dsda_powerup_t *pwr = &powerups[idx[i]];
    int tics = dsda_NormalizePowerupTics(pwr, pwr->tics(p));
    char line[64];

    if (!dsda_PowerupTimer(line, sizeof(line), tics, pwr->max_tics, pwr->blink_tics, dsda_PowerupLabel(pwr), dsda_PowerupTextColor(pwr)))
      continue;

    powerup_active = true;

    line_len = snprintf(str + length, max_size - length, "%s\n", line);

    if (line_len < 0)
      line_len = 0;
  
    if ((size_t)line_len >= max_size - length)
      break;

    length += (size_t)line_len;
  }

  if (!powerup_active && max_size > 0)
    str[0] = '\0';
}

void dsda_InitStatusTimersHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  powerup_index = Z_Malloc(dsda_powerup_count * sizeof(*powerup_index));
  local->text_align_right = arg_count > 0 ? !!args[0] : false;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatusTimersHC(void* data) {
    local = data;

    if(!dsda_IntConfig(nyan_config_ex_timer_widget))
      return;

    dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
    dsda_RefreshHudText(&local->component);

    if (local->text_align_right)
      HUlib_setTextXRightAlign(&local->component.text);
}

void dsda_DrawStatusTimersHC(void* data) {
    local = data;

    if(!dsda_IntConfig(nyan_config_ex_timer_widget))
      return;

    dsda_DrawBasicText(&local->component);
}
