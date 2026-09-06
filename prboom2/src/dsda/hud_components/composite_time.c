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
//	DSDA Composite Time HUD Component
//

#include "base.h"

#include "composite_time.h"

typedef struct {
  dsda_text_t component;
  char label[8];
} local_component_t;

static local_component_t* local;

static int dsda_FormatCompositeTime(char* str, size_t max_size, int tics, const char* color, dboolean show_subseconds) {
  dboolean show_hours_config = dsda_IntConfig(dsda_config_composite_time_hours);

  if (show_hours_config && tics >= 35 * 60 * 60)
  {
    if (show_subseconds)
      return snprintf(
        str,
        max_size,
        "%s%d:%02d:%05.2f ",
        color,
        tics / (35 * 60 * 60),
        (tics / (35 * 60)) % 60,
        (float)(tics % (60 * 35)) / 35
      );

    return snprintf(
      str,
      max_size,
      "%s%d:%02d:%02d ",
      color,
      tics / (35 * 60 * 60),
      (tics / (35 * 60)) % 60,
      (tics / 35) % 60
    );
  }

  if (show_subseconds)
    return snprintf(
      str,
      max_size,
      "%s%d:%05.2f ",
      color,
      tics / 35 / 60,
      (float)(tics % (60 * 35)) / 35
    );

  return snprintf(
    str,
    max_size,
    "%s%d:%02d ",
    color,
    tics / 35 / 60,
    (tics % (60 * 35)) / 35
  );
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  extern dboolean dsda_reborn;

  int total_time;
  int length;

  total_time = hexen ?
               players[consoleplayer].worldTimer :
               totalleveltimes + leveltime;

  // Label
  length = snprintf(
    str,
    max_size,
    "%s%s",
    dsda_TextColor(dsda_tc_exhud_time_label),
    local->label
  );

  // Total time
  if (total_time != leveltime)
    length += dsda_FormatCompositeTime(
      str + length,
      max_size - length,
      total_time,
      dsda_TextColor(dsda_tc_exhud_total_time),
      false
    );

  // Level time
  length += dsda_FormatCompositeTime(
    str + length,
    max_size - length,
    leveltime,
    dsda_TextColor(dsda_tc_exhud_level_time),
    true
  );

  // Demo time
  if (dsda_reborn && (demorecording || demoplayback)) {
    length += dsda_FormatCompositeTime(
      str + length,
      max_size - length,
      dsda_DemoTic(),
      dsda_TextColor(dsda_tc_exhud_demo_length),
      false
    );
  }
}

void dsda_InitCompositeTimeHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  if (arg_count < 1 || args[0])
    snprintf(local->label, sizeof(local->label), "time ");
  else
    local->label[0] = '\0';

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateCompositeTimeHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawCompositeTimeHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
