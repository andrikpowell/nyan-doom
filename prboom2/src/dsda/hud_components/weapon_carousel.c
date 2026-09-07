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
//	DSDA Weapon Carousel Component
//

#include "SDL.h"

#include "base.h"
#include "dsda/messenger.h"
#include "weapon_switch.h"
#include "dsda/pause.h"
#include "lprintf.h"
#include "weapon_carousel.h"

#define ICON_SPACING 64
#define VISIBLE_ICONS 2
#define ANIMATION_MS 125
#define CAROUSEL_FADE_TICS 4

static const char* const doom_names[NUMWEAPONS] = {
  [wp_fist] = "SMFIST",
  [wp_pistol] = "SMPISG",
  [wp_shotgun] = "SMSHOT",
  [wp_chaingun] = "SMMGUN",
  [wp_missile] = "SMLAUN",
  [wp_plasma] = "SMPLAS",
  [wp_bfg] = "SMBFGG",
  [wp_chainsaw] = "SMCSAW",
  [wp_supershotgun] = "SMSGN2",
};

static const char* const heretic_names[NUMWEAPONS] = {
  [wp_staff] = "SMWSTF",
  [wp_goldwand] = "SMWGLD",
  [wp_crossbow] = "SMWBOW",
  [wp_blaster] = "SMWBLS",
  [wp_skullrod] = "SMWSKL",
  [wp_phoenixrod] = "SMWPHX",
  [wp_mace] = "SMWMCE",
  [wp_gauntlets] = "SMWGNT",
};

static const char* const hexen_names[NUMCLASSES][HEXEN_NUMWEAPONS] = {
    [PCLASS_FIGHTER] = {
      [wp_first]  = "SMWFPC",
      [wp_second] = "SMWFAX",
      [wp_third]  = "SMWFHM",
      [wp_fourth] = "SMWFRS",
    },
    [PCLASS_CLERIC] = {
      [wp_first]  = "SMWCMC",
      [wp_second] = "SMWCSS",
      [wp_third]  = "SMWCFM",
      [wp_fourth] = "SMWCHS",
    },
    [PCLASS_MAGE] = {
      [wp_first]  = "SMWMWD",
      [wp_second] = "SMWMCS",
      [wp_third]  = "SMWMLG",
      [wp_fourth] = "SMWMST",
    },
};

static const weapontype_t doom_weapon_order[] = {
  wp_fist,
  wp_chainsaw,
  wp_pistol,
  wp_shotgun,
  wp_supershotgun,
  wp_chaingun,
  wp_missile,
  wp_plasma,
  wp_bfg,
};

static const weapontype_t heretic_weapon_order[] = {
  wp_staff,
  wp_gauntlets,
  wp_goldwand,
  wp_crossbow,
  wp_blaster,
  wp_skullrod,
  wp_phoenixrod,
  wp_mace,
};

static const weapontype_t hexen_weapon_order[] = {
  wp_first,
  wp_second,
  wp_third,
  wp_fourth,
};

typedef enum
{
  wpi_none = -1,
  wpi_regular,
  wpi_selected,
  wpi_disabled
} weapon_icon_state_t;

typedef struct {
  weapontype_t weapon;
  weapon_icon_state_t state;
} weapon_icon_t;

typedef struct {
  dsda_patch_component_t component;

  weapon_icon_t icons[NUMWEAPONS];
  int icon_count;
  int selected_index;

  int last_index;
  unsigned int last_time;
  int distance;

  int duration;
  int fade_alpha;
} local_component_t;

static local_component_t* local;

void dsda_InitWeaponCarouselHC(int x, int y, int vpt, int* args, int arg_count, void** data)
{
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;
  local->last_index = -1;
  local->fade_alpha = 100;
  dsda_InitPatchHC(&local->component, x, y, vpt);
}

static void ResetCarousel(local_component_t* c)
{
  c->last_index = -1;
  c->last_time = 0;
  c->distance = 0;
  c->duration = 0;
}

void dsda_ResetWeaponCarousel(void)
{
  if (local)
    ResetCarousel(local);
}

static int dsda_GetAlphaStep(int a)
{
  int step_size;

  if (a <= 0)   return 0;
  if (a >= 100) return 100;

  step_size = 100 / MESSAGE_FADE_STEPS;

  a = ((a + step_size / 2) / step_size) * step_size;

  return CLAMP(a, 0, 100);
}

static int CarouselFadeAlpha(int tics)
{
  const int fade_tics = CAROUSEL_FADE_TICS;
  int alpha;

  if (!dsda_UIFadeEffects())
    return 100;

  if (tics <= 0) return 0;
  if (tics > fade_tics) return 100;

  alpha = (tics * 100 + (fade_tics / 2)) / fade_tics;
  alpha = dsda_GetAlphaStep(alpha);
  return alpha;
}

// [AR] temp guard until we get internal hexen icons
static dboolean HexenCarouselIconsAvailable(void)
{
  char lump_name[9];
  int pclass;
  int weapon;
  int state;

  if (!hexen)
    return true;

  for (pclass = PCLASS_FIGHTER; pclass <= PCLASS_MAGE; pclass++)
  {
    for (weapon = 0; weapon < arrlen(hexen_weapon_order); weapon++)
    {
      for (state = 0; state < 2; ++state)
      {
        snprintf(lump_name, sizeof(lump_name), "%s%d",
                 hexen_names[pclass][hexen_weapon_order[weapon]], state);

        if (W_CheckNumForName(lump_name) == LUMP_NOT_FOUND)
          return false;
      }
    }
  }

  return true;
}

static void BuildWeaponIcons(local_component_t* c, const player_t* player)
{
  int i;

  const weapontype_t* weapon_order =  hexen   ? hexen_weapon_order :
                                      heretic ? heretic_weapon_order :
                                                doom_weapon_order;

  int weapon_count = hexen   ? arrlen(hexen_weapon_order) :
                     heretic ? arrlen(heretic_weapon_order) :
                               arrlen(doom_weapon_order);

  c->icon_count = 0;

  for (i = 0; i < weapon_count; ++i)
  {
    weapontype_t weapon = weapon_order[i];
    weapon_icon_state_t state = wpi_none;

    if (c->last_index == -1 && weapon == player->readyweapon)
    {
      c->last_index = c->icon_count;
    }

    if (G_WeaponSelectable(weapon))
    {
      if (player->nextweapon == weapon)
      {
        c->selected_index = c->icon_count;
        state = wpi_selected;
      }
      else if (G_AdjustWeaponSelection(player, weapon, player->nextweapon) != weapon)
      {
          state = wpi_disabled;
      }
      else
      {
        state = wpi_regular;
      }
    }
    else if (player->weaponowned[weapon])
    {
      state = wpi_disabled;
    }

    if (state != wpi_none)
    {
      c->icons[c->icon_count].weapon = weapon;
      c->icons[c->icon_count].state = state;
      ++c->icon_count;
    }
  }
}

void dsda_UpdateWeaponCarouselHC(void* data)
{
  player_t* player = &players[displayplayer];
  local = data;

  if (!dsda_WeaponCarousel())
    return;

  if (G_NextWeaponActivate())
  {
    local->duration = TICRATE / 2;
  }

  // [raven] Disable for chicken / morph
  if (players[displayplayer].chickenTics != 0 ||
      players[displayplayer].morphTics != 0 ||
      // [AR] temp guard until we get internal hexen icons
      !HexenCarouselIconsAvailable())
  {
    ResetCarousel(local);
    return;
  }

  if (local->duration == 0)
  {
    return;
  }

  if (automap_solid || menuactive || dsda_Paused() || player->playerstate == PST_DEAD ||
      player->health <= 0 || consoleplayer != displayplayer)
  {
    ResetCarousel(local);
    return;
  }

  if (player->switching == weapswitch_none &&
      player->pendingweapon == wp_nochange)
  {
    --local->duration;
  }

  BuildWeaponIcons(local, player);
  local->fade_alpha = CarouselFadeAlpha(local->duration);

  if (local->last_index != local->selected_index)
  {
    local->distance = local->selected_index - local->last_index;
    local->distance = ICON_SPACING * CLAMP(local->distance, -VISIBLE_ICONS, VISIBLE_ICONS);
    local->last_index = local->selected_index;
    local->last_time = SDL_GetTicks();
  }
}

static void WeaponIconName(char lump_name[9], weapon_icon_t icon)
{
  const char *name;
  dboolean custom_icon = !hexen && weaponinfo[icon.weapon].carouselicon;
  int selected = icon.state == wpi_selected;

  if (custom_icon)
    name = weaponinfo[icon.weapon].carouselicon;
  else if (hexen)
    name = hexen_names[players[displayplayer].pclass][icon.weapon];
  else if (heretic)
    name = heretic_names[icon.weapon];
  else
    name = doom_names[icon.weapon];

  snprintf(lump_name, 9, "%s%d", name, selected);

  // Fallback to generic icon if not found
  if (custom_icon && W_CheckNumForName(lump_name) == LUMP_NOT_FOUND)
  {
    // If raven pwad hasn't replaced unknown icon,
    // then use internal SMUNKN2/3.
    if (raven && !W_PWADLumpNameExists(selected ? "SMUNKN1" : "SMUNKN0"))
      snprintf(lump_name, 9, "SMUNKN%d", selected + 2);
    else
      snprintf(lump_name, 9, "SMUNKN%d", selected);
  }
}

static int WeaponIconLump(weapon_icon_t icon)
{
  char lump_name[9] = {0};

  WeaponIconName(lump_name, icon);

  return W_GetNumForName(lump_name);
}

static void DrawWeaponIcon(const local_component_t* c, int x, weapon_icon_t icon)
{
  char lump_name[9] = {0};
  int color;
  int flags;

  WeaponIconName(lump_name, icon);

  color = (icon.state == wpi_disabled) ? CR_DARKEN : CR_DEFAULT;
  flags = c->component.vpt;

  if (color != CR_DEFAULT)
    flags |= VPT_COLOR;

  V_DrawMenuFadeNamePatch(x, c->component.y, lump_name, color, c->fade_alpha, flags);
}

static int CalcOffset(local_component_t* c)
{
  if (c->distance)
  {
    const unsigned int delta = SDL_GetTicks() - c->last_time;

    if (delta < ANIMATION_MS)
    {
      const float x = 1.0f - (float)delta / ANIMATION_MS;
      return lroundf(c->distance * x * x);
    }

    c->distance = 0;
  }

  return 0;
}

void dsda_DrawWeaponCarouselHC(void* data)
{
  int offset;

  local = data;

  if (!dsda_WeaponCarousel())
    return;

  // [raven] Disable for chicken / morph
  if (players[displayplayer].chickenTics != 0 ||
      players[displayplayer].morphTics != 0 ||
      // [AR] temp guard until we get internal hexen icons
      !HexenCarouselIconsAvailable())
    return;

  if (local->duration == 0 || local->icon_count == 0)
    return;

  offset = local->component.x + CalcOffset(local);
  DrawWeaponIcon(local, offset, local->icons[local->selected_index]);

  // Right icons
  for (int i = local->selected_index + 1, k = 1; i < local->icon_count && k < 3; ++i, ++k)
  {
    DrawWeaponIcon(local, offset + k * ICON_SPACING, local->icons[i]);
  }

  // Left icons
  for (int i = local->selected_index - 1, k = 1; i >= 0 && k < 3; --i, ++k)
  {
    DrawWeaponIcon(local, offset - k * ICON_SPACING, local->icons[i]);
  }
}
