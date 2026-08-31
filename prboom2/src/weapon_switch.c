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
//	DSDA Weapon Switch
//

#include "doomstat.h"
#include "lprintf.h"
#include "p_pspr.h"
#include "weapon_switch.h"

#include "dsda/global.h"
#include "dsda/input.h"
#include "dsda/settings.h"

#include "dsda/hud_components/weapon_carousel.h"

// Used for prev/next weapon keys.

static const struct
{
  weapontype_t weapon;
  weapontype_t weapon_num;
} weapon_order_table[] = {
  { wp_fist,         wp_fist },
  { wp_chainsaw,     wp_fist },
  { wp_pistol,       wp_pistol },
  { wp_shotgun,      wp_shotgun },
  { wp_supershotgun, wp_shotgun },
  { wp_chaingun,     wp_chaingun },
  { wp_missile,      wp_missile },
  { wp_plasma,       wp_plasma },
  { wp_bfg,          wp_bfg }
};

// HERETIC_TODO: dynamically set these
// static const struct
// {
//     weapontype_t weapon;
//     weapontype_t weapon_num;
// } heretic_weapon_order_table[] = {
//     { wp_staff,       wp_staff },
//     { wp_gauntlets,   wp_staff },
//     { wp_goldwand,    wp_goldwand },
//     { wp_crossbow,    wp_crossbow },
//     { wp_blaster,     wp_blaster },
//     { wp_skullrod,    wp_skullrod },
//     { wp_phoenixrod,  wp_phoenixrod },
//     { wp_mace,        wp_mace },
//     { wp_beak,        wp_beak },
// };

dboolean G_WeaponSelectable(weapontype_t weapon)
{
  if (heretic)
  {
    if (gamemode == shareware)
    {
      if (weapon == wp_skullrod || weapon == wp_phoenixrod || weapon == wp_mace)
        return false;
    }

    return weapon != wp_beak && players[consoleplayer].weaponowned[weapon];
  }
  else if (hexen)
  {
    return weapon < HEXEN_NUMWEAPONS && players[consoleplayer].weaponowned[weapon];
  }

  if (gamemode == shareware)
  {
    if (weapon == wp_plasma || weapon == wp_bfg)
      return false;
  }

  // Can't select the super shotgun in Doom 1.
  if (weapon == wp_supershotgun && gamemission == doom)
  {
    return false;
  }

  // Can't select a weapon if we don't own it.
  if (!players[consoleplayer].weaponowned[weapon])
  {
    return false;
  }

  // Can't select the fist if we have the chainsaw, unless
  // we also have the berserk pack.
  if (demo_compatibility &&
      weapon == wp_fist &&
      players[consoleplayer].weaponowned[wp_chainsaw] &&
      !players[consoleplayer].powers[pw_strength])
  {
    return false;
  }

  return true;
}

static weapontype_t G_AdjustVanillaWeaponSelection(const player_t *player, weapontype_t weapon, weapontype_t current_weapon)
{
  if (hexen)
    return weapon;

  if (weapon == g_wp_fist && player->weaponowned[g_wp_chainsaw])
  {
    // Heretic - always just direct to gauntlets.
    if (heretic)
    {
      if (current_weapon != g_wp_chainsaw)
        weapon = g_wp_chainsaw;
    }
    else
    {
      // Doom - force chainsaw if not currently on chainsaw
      // + don't allow switch to fist if no berserk.
      //
      // If "prefer berserk" is active, switch to berserk first.
      dboolean berserk = player->powers[pw_strength];
      dboolean prefer_berserk = dsda_BerserkPreferred() && berserk;

      if (current_weapon != g_wp_chainsaw || !berserk)
        if (current_weapon == g_wp_fist || !prefer_berserk)
          weapon = g_wp_chainsaw;
    }
  }

  if (!heretic && weapon == wp_shotgun && gamemode == commercial &&
      player->weaponowned[wp_supershotgun] &&
      current_weapon != wp_supershotgun)
    weapon = wp_supershotgun;

  return weapon;
}

// killough 2/8/98:
// Allow user to switch to fist even if they have chainsaw.
// Switch to fist or chainsaw based on preferences.
// Switch to shotgun or SSG based on preferences.
static weapontype_t G_AdjustBoomWeaponSelection(const player_t *player, weapontype_t weapon, weapontype_t current_weapon)
{
  // Only select chainsaw from '1' if it's owned, it's not already in use,
  // and the player prefers it or the fist is already in use, or the player
  // does not have the berserker strength.
  //
  // [AR] Add berserk priority option.
  if (weapon == wp_fist && player->weaponowned[wp_chainsaw] &&
      current_weapon != wp_chainsaw)
  {
    dboolean prefer_berserk = dsda_BerserkPreferred() &&
                              player->powers[pw_strength];
    dboolean prefer_chainsaw = !prefer_berserk &&
                               P_WeaponPreferred(wp_chainsaw, wp_fist);

    if (current_weapon == wp_fist ||
        !player->powers[pw_strength] ||
        prefer_chainsaw)
      weapon = wp_chainsaw;
  }

  // Select SSG from '3' only if it's owned and the player does not have a
  // shotgun, or if the shotgun is already in use, or if the SSG is not
  // already in use and the player prefers it.
  if (weapon == wp_shotgun && gamemode == commercial &&
      player->weaponowned[wp_supershotgun] &&
      (!player->weaponowned[wp_shotgun] ||
       current_weapon == wp_shotgun ||
       (current_weapon != wp_supershotgun &&
        P_WeaponPreferred(wp_supershotgun, wp_shotgun))))
    weapon = wp_supershotgun;

  return weapon;
}

// killough 3/22/98: For demo compatibility we must perform the fist
// and SSG weapons switches in P_PlayerThink(), rather than in
// G_BuildTiccmd(). For other games which rely on user preferences, we
// must use the latter. Therefore demo_compatibility determines where
// the resolved selection is applied.
weapontype_t G_AdjustWeaponSelection(const player_t *player, weapontype_t weapon, weapontype_t current_weapon)
{
  if (demo_compatibility)
    return G_AdjustVanillaWeaponSelection(player, weapon, current_weapon);

  return G_AdjustBoomWeaponSelection(player, weapon, current_weapon);
}

static weapontype_t G_NextWeapon(int direction)
{
  int i;
  int start_i;
  weapontype_t weapon = players[consoleplayer].nextweapon;

  // Find index in the table.
  for (i = 0; i < arrlen(weapon_order_table); ++i)
  {
    if (weapon_order_table[i].weapon == weapon)
      break;
  }

  if (i == arrlen(weapon_order_table))
  {
    I_Error("Invalid weapon type %d", (int)weapon);
  }

  // Switch weapon. Don't loop forever.

  start_i = i;
  do
  {
    i += direction;
    i = (i + arrlen(weapon_order_table)) % arrlen(weapon_order_table);
  }
  while (i != start_i && !G_WeaponSelectable(weapon_order_table[i].weapon));

  return G_AdjustWeaponSelection(&players[consoleplayer], weapon_order_table[i].weapon, weapon);
}

// Convert resolved weapon into legacy slot number
// AKA chainsaw becomes fist (slot 1), and SSG becomes shotgun (slot 3)
weapontype_t G_NextWeaponTranslate(weapontype_t weapon)
{
  int i;

  if (!demo_compatibility)
    return weapon;

  for (i = 0; i < arrlen(weapon_order_table); ++i)
  {
    if (weapon_order_table[i].weapon == weapon)
      return weapon_order_table[i].weapon_num;
  }

  return weapon;
}

typedef enum
{
  next_weapon_none,
  next_weapon_activate,
  next_weapon_deactivate,
  next_weapon_cmd,
} next_weapon_state_t;

static next_weapon_state_t state;
static dboolean currently_active;
static dboolean carousel_activate;

void G_NextWeaponUpdate(void)
{
  weapontype_t weapon = wp_nochange;

  if (dsda_InputActivated(dsda_input_prevweapon))
  {
    weapon = G_NextWeapon(-1);
  }
  else if (dsda_InputActivated(dsda_input_nextweapon))
  {
    weapon = G_NextWeapon(1);
  }
  // We must preview the weapon we are to change to for the carousel.
  // Before we didn't need this because we didn't need to preview.
  else if (dsda_InputDeactivated(dsda_input_prevweapon) ||
           dsda_InputDeactivated(dsda_input_nextweapon))
  {
    if (currently_active)
    {
      currently_active = false;
      state = next_weapon_deactivate;
    }
  }

  if (weapon != wp_nochange)
  {
    currently_active = true;
    carousel_activate = true;
    state = next_weapon_activate;
    players[consoleplayer].nextweapon = weapon;
  }
}

dboolean G_NextWeaponActivate(void)
{
  if (carousel_activate)
  {
    carousel_activate = false;

    if (state == next_weapon_activate)
      state = next_weapon_none;

    return true;
  }

  return false;
}

dboolean G_NextWeaponDeactivate(void)
{
  if (state == next_weapon_deactivate)
  {
    state = next_weapon_cmd;
    return true;
  }

  return false;
}

void G_NextWeaponResendCmd(void)
{
  weapontype_t weapon;

  if (players[consoleplayer].pendingweapon == wp_nochange)
  {
    weapon = players[consoleplayer].readyweapon;
  }
  else
  {
    weapon = players[consoleplayer].pendingweapon;
  }

  if (state == next_weapon_cmd &&
      players[consoleplayer].nextweapon != weapon &&
      players[consoleplayer].switching != weapswitch_none)
  {
    state = next_weapon_deactivate;
  }
}

void G_NextWeaponReset(weapontype_t weapon)
{
  state = next_weapon_none;
  currently_active = false;
  carousel_activate = false;

  players[consoleplayer].nextweapon = players[consoleplayer].readyweapon;
  players[consoleplayer].nextweapon = G_AdjustWeaponSelection(&players[consoleplayer], weapon, players[consoleplayer].nextweapon);

  dsda_ResetWeaponCarousel();
}
