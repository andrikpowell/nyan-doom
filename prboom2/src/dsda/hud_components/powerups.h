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
//	DSDA Powerups HUD Functions
//

#ifndef __DSDA_HUD_POWERUPS__
#define __DSDA_HUD_POWERUPS__

typedef int (*powerup_tics_f)(player_t *);

typedef enum {
  POWERUP_BLINK_DEFAULT,
  POWERUP_BLINK_RAVEN_ICONS,
  POWERUP_BLINK_INVUL,
  POWERUP_BLINK_LIGHT,
} powerup_blink_t;

typedef struct dsda_powerup_s {
  int config_text;
  int config_icon;
  powerup_tics_f tics;
  int max_tics;
  powerup_blink_t blink_tics;
  const char *label;
  const char *lump;
  const char *heretic_lump;
  const char *hexen_lump;
  int color;
  int raven_color;
} dsda_powerup_t;

typedef enum {
  POWERUP_STATUS_TEXT,
  POWERUP_STATUS_ICON,
} dsda_powerup_view_t;

typedef struct {
  const char *label;
  dboolean no_inf_suffix;
} dsda_powerup_label_t;

extern const dsda_powerup_t powerups[];
extern int dsda_powerup_count;

extern void dsda_SortPowerups(player_t *player, int *idx, int *n, dsda_powerup_view_t view, dboolean reverse);
extern int dsda_NormalizePowerupTics(const dsda_powerup_t *powerup, int tics);

extern dsda_powerup_label_t dsda_PowerupLabel(const dsda_powerup_t *powerup);
extern const char *dsda_PowerupIconLump(const dsda_powerup_t *powerup);
extern const char *dsda_PowerupTextColor(const dsda_powerup_t *powerup);
extern int dsda_PowerupCRColor(const dsda_powerup_t *powerup);

extern int dsda_PowerupTimer(char *str, size_t max_size, int tics, int max_tics, powerup_blink_t blink_type, dsda_powerup_label_t label, const char* color);
extern int dsda_PowerupIcon(const dsda_powerup_t *powerup, int tics);

#endif
