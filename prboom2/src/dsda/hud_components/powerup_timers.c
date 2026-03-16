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
//	DSDA Powerup Timers HUD Component
//

#include "base.h"

#include "powerup_timers.h"

typedef struct {
    dsda_text_t component;
    dboolean text_align_right;
} local_component_t;

dboolean hide_time;

static local_component_t* local;

typedef int (*powerup_tics_f)(player_t *);

typedef enum {
  POWERUP_BLINK_DEFAULT,
  POWERUP_BLINK_RAVEN_ICONS,
  POWERUP_BLINK_INVUL,
} powerup_blink_t;

typedef struct {
  int config;
  powerup_tics_f tics;
  int max_tics;
  powerup_blink_t blink_tics;
  const char *label;
  int color;
} powerup_timer_t;

#define PERMATICS       -4
#define MULTILEVELTICS  -3
#define LEVELTICS       -2
#define INFINITETICS    -1

typedef enum {
    POWERUP_FOREVER,
    POWERUP_MULTILEVEL,
    POWERUP_INFINITE,
    POWERUP_LEVEL,
    POWERUP_NORMAL,
    POWERUP_MAX,
} powerup_type_t;

// order to draw powerups
static int dsda_PowerupGroup(const powerup_timer_t *p, int tics)
{
  if (p->max_tics == PERMATICS)       return POWERUP_FOREVER;
  if (p->max_tics == MULTILEVELTICS)  return POWERUP_MULTILEVEL;
  if (tics <= INFINITETICS)           return POWERUP_INFINITE;
  if (p->max_tics == LEVELTICS)       return POWERUP_LEVEL;
  return POWERUP_NORMAL;
}

static int dsda_GetStrength(player_t *p)       { return p->powers[pw_strength]; }
static int dsda_GetAllMap(player_t *p)         { return p->powers[pw_allmap]; }
static int dsda_GetInvis(player_t *p)          { return p->powers[pw_invisibility]; }
static int dsda_GetInvul(player_t *p)          { return p->powers[pw_invulnerability]; }
static int dsda_GetInfra(player_t *p)          { return p->powers[pw_infrared]; }
static int dsda_GetSuit(player_t *p)           { return p->powers[pw_ironfeet]; }
static int dsda_GetBackpack(player_t *p)       { return p->backpack; }
static int dsda_GetArmor1(player_t *p)         { return !hexen && p->armortype > 0 && p->armortype < 2; }
static int dsda_GetArmor2(player_t *p)         { return !hexen && p->armortype >= 2; }

static int dsda_GetTome(player_t *p)           { return p->powers[pw_weaponlevel2]; }
static int dsda_GetSpeed(player_t *p)          { return p->powers[pw_speed]; }
static int dsda_GetMorph(player_t *p)          { return hexen ? p->morphTics : heretic ? p->chickenTics : 0; }
static int dsda_GetMaulator(player_t *p)       { return p->powers[pw_minotaur]; }

// this is to match animated flight icon state
static dboolean dsda_FlightLabelState(player_t *p, dboolean active)
{
    static dboolean state;
    static dboolean active_last;

    dboolean current = (p->mo->flags2 & MF2_FLY) != 0;
    int frame = (leveltime / 3) & 15;

    if (!active)
    {
        state = false;
        active_last = false;
        return false;
    }

    if (!active_last)
    {
        state = current;
        active_last = true;
        return state;
    }

    if (frame == 15 || frame == 0)
        state = current;

    return state;
}


static int dsda_GetFlight(player_t *p) {
    dboolean active;
    int tics;

    if (p->cheats & CF_CAMERA)
      return 0;

    if (raven)
    {
        tics = p->powers[pw_flight];
        active = (heretic && (tics > FLIGHTTICS)) ||
                 (hexen && (tics == FLIGHTTICS || tics == INT_MAX));

        if (active)
        {
          dsda_FlightLabelState(p, active);
          return INFINITETICS;
        }
    }
    else // Doom fly cheat
    {
      tics = (p->cheats & CF_FLY) ? INFINITETICS : 0;
    }

    return tics;
}

static const char* dsda_RavenGetFlightLabel(const char* label) {
  player_t *p = &players[displayplayer];

  if (!raven)
    return NULL;

  return dsda_FlightLabelState(p, true) ? "FLIGHT ON" : "FLIGHT OFF";
}

static powerup_timer_t powerups[] = {
  { nyan_config_ex_powerup_backpack, dsda_GetBackpack,   PERMATICS,      POWERUP_BLINK_DEFAULT,      "BACKPACK",     dsda_tc_exhud_powerup_backpack    },
  { nyan_config_ex_powerup_armor,    dsda_GetArmor1,     MULTILEVELTICS, POWERUP_BLINK_DEFAULT,      "ARMOR ONE",    dsda_tc_exhud_powerup_armor_one   },
  { nyan_config_ex_powerup_armor,    dsda_GetArmor2,     MULTILEVELTICS, POWERUP_BLINK_DEFAULT,      "ARMOR TWO",    dsda_tc_exhud_powerup_armor_two   },
  { nyan_config_ex_powerup_berserk,  dsda_GetStrength,   LEVELTICS,      POWERUP_BLINK_DEFAULT,      "BERSERK",      dsda_tc_exhud_powerup_berserk     },
  { nyan_config_ex_powerup_areamap,  dsda_GetAllMap,     LEVELTICS,      POWERUP_BLINK_DEFAULT,      "AREAMAP",      dsda_tc_exhud_powerup_allmap      },

  { nyan_config_ex_powerup_invis,    dsda_GetInvis,      INVISTICS,      POWERUP_BLINK_DEFAULT,      "INVIS",        dsda_tc_exhud_powerup_invis       },
  { nyan_config_ex_powerup_invuln,   dsda_GetInvul,      INVULNTICS,     POWERUP_BLINK_INVUL,        "INVUL",        dsda_tc_exhud_powerup_invul       },
  { nyan_config_ex_powerup_liteamp,  dsda_GetInfra,      INFRATICS,      POWERUP_BLINK_DEFAULT,      "LIGHT",        dsda_tc_exhud_powerup_light       },
  { nyan_config_ex_powerup_radsuit,  dsda_GetSuit,       IRONTICS,       POWERUP_BLINK_DEFAULT,      "SUIT",         dsda_tc_exhud_powerup_suit        },

  { nyan_config_ex_powerup_flight,   dsda_GetFlight,     FLIGHTTICS,     POWERUP_BLINK_RAVEN_ICONS,  "FLIGHT",       dsda_tc_exhud_powerup_flight      }, 

  { nyan_config_ex_powerup_tome,     dsda_GetTome,       WPNLEV2TICS,    POWERUP_BLINK_RAVEN_ICONS,  "TOME",         dsda_tc_exhud_powerup_tome        },
  { nyan_config_ex_powerup_speed,    dsda_GetSpeed,      SPEEDTICS,      POWERUP_BLINK_RAVEN_ICONS,  "SPEED",        dsda_tc_exhud_powerup_speed       },
  { nyan_config_ex_powerup_morph,    dsda_GetMorph,      MORPHTICS,      POWERUP_BLINK_RAVEN_ICONS,  "MORPH",        dsda_tc_exhud_powerup_morph       },
  { nyan_config_ex_powerup_maulator, dsda_GetMaulator,   MAULATORTICS,   POWERUP_BLINK_RAVEN_ICONS,  "MAULATOR",     dsda_tc_exhud_powerup_maulator    },
};

static int dsda_PowerupBlink(powerup_blink_t blink_type, int tics)
{
  int blink_active;

  // full screen effects (like in doom / torch) use shorter blinks
  // Raven animated icons use longer blinks
  // Sync powerups to which effect is shown
  int normal_blink        = (tics & 8);
  int raven_icon_blink    = !(tics & 16);

  switch (blink_type)
  {
    case POWERUP_BLINK_INVUL:
      blink_active = hexen ? raven_icon_blink : normal_blink;
      break;

    case POWERUP_BLINK_RAVEN_ICONS:
      blink_active = raven_icon_blink;
      break;

    case POWERUP_BLINK_DEFAULT:
    default:
      blink_active = normal_blink;
      break;
  }

  return blink_active;
}

static int dsda_PowerupTimer(char *str, size_t max_size, int tics, int max_tics, powerup_blink_t blink_type, const char *label, const char* color)
{
  extern dboolean dsda_PowerupHideTimes(void);
  const char* repl_label = NULL;

  if (tics == 0) return 0;

  {
    const dboolean specialtics = max_tics < 0;
    const dboolean always_active = (tics < 0);
    const dboolean no_blinking = !dsda_IntConfig(nyan_config_ex_powerup_blinking);
    int blink_active;
    dboolean powerup_duration;
    dboolean powerup_active;
    const char *cm;

    blink_active = dsda_PowerupBlink(blink_type, tics);
    powerup_duration = ((tics > 0) && (tics > BLINKTHRESHOLD || blink_active));
    powerup_active = no_blinking || always_active || powerup_duration;
    cm = powerup_active ? color : dsda_TextColor(dsda_tc_exhud_powerup_blink);

    // Fix Flight label
    if (raven && !strcmp(label, "FLIGHT"))
      repl_label = dsda_RavenGetFlightLabel(label);

    if (repl_label)
      label = repl_label;

    // Just label
    if (dsda_PowerupHideTimes())
        return snprintf(str, max_size, "%s%s", cm, label);

    // full level duration
    else if (specialtics)
        return snprintf(str, max_size, "%s%s", cm, label);

    // Infinite duration
    else if (tics < 0)
    {
        const char* suffix = repl_label ? "": "INF";

        return snprintf(str, max_size, "%s%s %s", cm, label, suffix);
    }

    // Normal duration
    else
    {
        int secs = 1 + (tics / TICRATE);
        int max_secs = max_tics / TICRATE;
        if (secs > max_secs) secs = max_secs;
        return snprintf(str, max_size, "%s%s %d\"", cm, label, secs);
    }
  }
}

static void dsda_SortPowerups(player_t *player, int *idx, int *n)
{
  static int previous_tics[arrlen(powerups)];
  static unsigned int activate_tics[arrlen(powerups)];

  //
  // 1. Track when each powerup becomes active
  //

  for (int i = 0; i < arrlen(powerups); i++)
  {
    int powerup_tics = powerups[i].tics(player);

    // Powerup just turned ON (was previously OFF)
    if (powerup_tics != 0 && previous_tics[i] == 0)
      activate_tics[i] = (unsigned int)gametic;

    // Powerup is OFF (clear activation time)
    if (powerup_tics == 0)
      activate_tics[i] = 0;

    // Remember current state so we can detect next ON transition
    previous_tics[i] = powerup_tics;
  }

  //
  // 2. Gather a list of currently active and enabled powerups
  //

  *n = 0;
  for (int i = 0; i < arrlen(powerups); i++)
  {
    if (!dsda_IntConfig(powerups[i].config))
      continue;

    if (powerups[i].tics(player) != 0)
      idx[(*n)++] = i;
  }

  //
  // 3. Sort the powerups based on type and when most recently activated.
  //

  for (int i = 0; i < (*n) - 1; i++)
  {
    // Start by assuming the current powerup belongs in this sorted position
    // Then compare later active powerups to see if one should move above it
    int sort_position = i;

    for (int j = i + 1; j < (*n); j++)
    {
      int cur_pwr;
      int prev_pwr;
      int cur_group;
      int prev_group;
      unsigned int cur_tic;
      unsigned int prev_tic;
      int cur_tics_left;
      int prev_tics_left;

      dboolean move_above = false;
  
      cur_pwr = idx[j];                 // powerup currntly being sorted 
      prev_pwr = idx[sort_position];    // powerup currently occupying this slot

      // Sort by powerup group first.
      cur_group   = dsda_PowerupGroup(&powerups[cur_pwr], powerups[cur_pwr].tics(player));
      prev_group  = dsda_PowerupGroup(&powerups[prev_pwr], powerups[prev_pwr].tics(player));

      // If powerup is in a different group
      if (cur_group != prev_group)
      {
        // Longer-lasting powerups (permanent, level) sort higher
        // temporary countdown powerups sort lower.
        move_above = (cur_group < prev_group);
      }

      // If powerup is in same group
      else
      {
        cur_tics_left = powerups[cur_pwr].tics(player);
        prev_tics_left = powerups[prev_pwr].tics(player);

        // Timed powerups sort by time remaining.
        if (cur_group == POWERUP_NORMAL)
        {
          if (cur_tics_left != prev_tics_left)
            move_above = (cur_tics_left > prev_tics_left);
        }
        // Non-countdown powerups sort by activation order.
        else
        {
          cur_tic  = activate_tics[cur_pwr];
          prev_tic = activate_tics[prev_pwr];

          // older powerups sort higher
          // Newer powerups are pushed lower
          if (cur_tic != 0 && prev_tic != 0 && cur_tic != prev_tic)
            move_above = (cur_tic < prev_tic);
        }
      }

      // If current powerup should sort earlier, take a higher slot
      if (move_above)
        sort_position = j;
    }

    // Move the selected powerup up into this slot
    // and push the previous one lower.
    if (sort_position != i)
    {
      int saved = idx[i];

      idx[i] = idx[sort_position];
      idx[sort_position] = saved;
    }
  }
}

// Fix "special" powerups from using a countdown when they shouldn't (cheats, flight)
static int dsda_NormalizePowerupTics(const powerup_timer_t *powerup, int tics)
{
    if (tics == 0)                            return 0;
    if (powerup->max_tics == PERMATICS)       return PERMATICS;
    if (powerup->max_tics == MULTILEVELTICS)  return MULTILEVELTICS;
    if (powerup->max_tics == LEVELTICS)       return LEVELTICS;
    return tics;
}

static void dsda_UpdateComponentText(char *str, size_t max_size)
{
  player_t *p = &players[displayplayer];

  int idx[arrlen(powerups)];
  int n = 0;

  size_t length = 0;
  dboolean powerup_active = false;
  int line_len;

  if (max_size > 0)
    str[0] = '\0';

  dsda_SortPowerups(p, idx, &n);

  for (int i = 0; i < n; i++)
  {
    const powerup_timer_t *pwr = &powerups[idx[i]];
    int tics = dsda_NormalizePowerupTics(pwr, pwr->tics(p));
    char line[64];

    if (!dsda_PowerupTimer(line, sizeof(line), tics, pwr->max_tics, pwr->blink_tics, pwr->label, dsda_TextColor(pwr->color)))
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

void dsda_InitPowerupsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->text_align_right = arg_count > 0 ? !!args[0] : false;

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdatePowerupsHC(void* data) {
    local = data;

    if(!dsda_IntConfig(nyan_config_ex_powerup_widget))
      return;

    dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
    dsda_RefreshHudText(&local->component);

    if (local->text_align_right)
      HUlib_setTextXRightAlign(&local->component.text);
}

void dsda_DrawPowerupsHC(void* data) {
    local = data;

    if(!dsda_IntConfig(nyan_config_ex_powerup_widget))
      return;

    dsda_DrawBasicText(&local->component);
}
