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
//	DSDA Map Title HUD Component
//

#include "base.h"

#include "map_title.h"

extern dsda_string_t hud_title_cycle;
extern dsda_string_t hud_title;
extern dsda_string_t hud_author;

typedef struct {
  dsda_text_t component;
  dboolean no_author;
} local_component_t;

static local_component_t* local;

int titleTime = 3 * TICRATE;
short titleCounter;
short titleSwap;
static int titlePhase;

void dsda_UpdateTitleSwap(void)
{
  // Reset / init counter
  if (hud_title_cycle.string == NULL)
  {
    titleSwap = 0;
    titleCounter = titleTime;
    titlePhase = 0;

    dsda_StringPrintF(&hud_title_cycle, "%s", hud_title.string);
    return;
  }

  titleCounter--;

  // Restart counter + init swap 
  if (titleCounter <= 0)
  {
    titleSwap ^= 1;
    titleCounter = titleTime;
    titlePhase++;

    // Swap title to map or author
    if (titleSwap)
      dsda_StringPrintF(&hud_title_cycle, "Author: %s", hud_author.string);
    else
      dsda_StringPrintF(&hud_title_cycle, "%s", hud_title.string);
  }
}

static void dsda_UpdateMapTitleFade(void)
{
  const int fade_tics = MESSAGE_FADE_TICS;
  dboolean first_phase;
  dboolean cycle_title_active = (hud_author.string != NULL || local->no_author) &&
                                (dsda_IntConfig(dsda_config_map_title_author_cycle));

  local->component.text.fade_alpha = 100;

  if (!cycle_title_active || !dsda_FadeMessages())
    return;

  first_phase = (titlePhase == 0);

  // First title: show instantly, then fade out
  if (first_phase)
  {
    if (titleCounter <= fade_tics)
      local->component.text.fade_alpha = dsda_MessageFadeOut(titleCounter, true);
  }
  // fade in, fade out
  else
  {
    int elapsed = titleTime - titleCounter;

    if (elapsed < fade_tics)
      local->component.text.fade_alpha = dsda_MessageFadeIn(elapsed, true);
    else if (titleCounter <= fade_tics)
      local->component.text.fade_alpha = dsda_MessageFadeOut(titleCounter, true);
  }
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  dboolean cycle_title_active = (hud_author.string != NULL || local->no_author) &&
                                (dsda_IntConfig(dsda_config_map_title_author_cycle));

  if (cycle_title_active)
    dsda_UpdateTitleSwap();

  snprintf(
    str,
    max_size,
    "%s%s",
    titleSwap ? dsda_TextColor(dsda_tc_map_author) : dsda_TextColor(dsda_tc_map_title),
    cycle_title_active ? hud_title_cycle.string : hud_title.string
  );
}

void dsda_InitMapTitleHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->no_author = arg_count > 0 ? !!args[0] : false;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapTitleHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);

  dsda_UpdateMapTitleFade();
}

void dsda_DrawMapTitleHC(void* data) {
  local = data;

  dsda_DrawBasicShadowedText(&local->component);
}
