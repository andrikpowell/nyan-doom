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
//	DSDA Target Health HUD Component
//

#include "base.h"

#include "target_health.h"
#include "dsda/tracker.h"
#include "g_game.h"

typedef struct {
  dsda_text_t component;
  dboolean center;
} local_component_t;

static int target_health_tics;
static int target_cur_health;
static int target_spawn_health;
static int last_target_id = -1;

static local_component_t* local;

static const char* dsda_HealthColor(const int cur_health, const int spawn_health)
{
  return
      cur_health <= spawn_health / 4 ? HU_ColorFromValue(CR_RED) :
      cur_health <= spawn_health / 2 ? HU_ColorFromValue(CR_GOLD) :
                                       HU_ColorFromValue(CR_GREEN);
}

static void dsda_ResetTargetHealthData(void)
{
    last_target_id = -1;
    target_health_tics = 0;
    target_cur_health = 0;
    target_spawn_health = 0;
    hu_target_health_reset = false;
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  mobj_t* target;

  // Reset targets on level load
  if (hu_target_health_reset)// || players[consoleplayer].playerstate == PST_DEAD)
  {
    str[0] = '\0';
    dsda_ResetTargetHealthData();
    return;
  }

  if (target_health_tics > 0)
    target_health_tics--;

  target = HU_SafeIntercept();

  if (target && ((target->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL) && (target->health > 0))
  {
    target_cur_health = target->health;
    target_spawn_health = P_MobjSpawnHealth(target);
    target_health_tics = TICRATE;
    last_target_id = target->index; // track target

    snprintf(
      str,
      max_size,
      "%s%i/%i",
      dsda_HealthColor(target_cur_health, target_spawn_health),
      target_cur_health,
      target_spawn_health
    );
  }

  // Track target and clear string if dead
  // We need to be careful here as mobjs change
  else if (!target && last_target_id >= 0)
  {
    mobj_t *last = dsda_FindMobj(last_target_id);

    // mobj no longer exists
    if (!last)
    {
      last_target_id = -1;
    }

    // Target is dead, clear
    else if (last->health <= 0)
    {
      target_health_tics = 0;
      last_target_id = -1;
      str[0] = '\0';
      return;
    }
  }

  // Hold on to string for at least a gametic
  if (target_health_tics > 0 && target_cur_health > 0)
  {
    // This looks awkward if it disappears immediately
  }
  else
  {
    str[0] = '\0';
  }
}

void dsda_InitTargetHealthHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->center = arg_count > 0 ? !!args[0] : true;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateTargetHealthHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);

  if (local->center)
    HUlib_setTextXCenter(&local->component.text);
}

void dsda_DrawTargetHealthHC(void* data) {
  local = data;

  dsda_DrawBasicShadowedText(&local->component);
}
